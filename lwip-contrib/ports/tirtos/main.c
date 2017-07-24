/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 * RT timer modifications by Christiaan Simons
 */

#include <string.h>

#include "lwip/init.h"

#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "netif/slipif.h"


#include "apps/udpecho_raw/udpecho_raw.h"
#include <driverlib/aon_batmon.h>
#include "lwip/opt.h"
#include "lwip/debug.h"
#include <xdc/runtime/System.h>
#include <math.h>
#include <stdio.h>

#define LWIP_PORT_INIT_SLIP_IPADDR(addr)   IP4_ADDR((addr), 192, 168,   1, 17)
#define LWIP_PORT_INIT_SLIP_GW(addr)       IP4_ADDR((addr), 192, 168,   1, 1)
#define LWIP_PORT_INIT_SLIP_NETMASK(addr)  IP4_ADDR((addr), 255, 255, 255, 0)
#define UDP_PORT	10000

#define SLIP_ADDRS &ipaddr_slip, &netmask_slip, &gw_slip,

#define MAX_COMMAND_LENGTH	5// Including null terminator
#define MAX_RETURN_LENGTH	25// Size of buffer of max number of bytes returned from device

#define RESP_PREFIX	"Device Response: "

#if LWIP_UDP

static struct udp_pcb *udpecho_raw_pcb;
char retBuf[MAX_RETURN_LENGTH];

char retError[] = "API Error!";


// Macros for convenience
#define ZERO_OUT_RET_BUFS() \
	memset(retBuf, 0, sizeof(retBuf));


#define UPDATE_PAYLOAD(pbuf, retBuf) \
		mem_free(pbuf->payload); \
		pbuf->tot_len += (strlen(retBuf) + 1 - pbuf->len); \
		pbuf->payload = mem_malloc(strlen(retBuf) + 1); \
		pbuf->len = strlen(retBuf) + 1; \
		memcpy(pbuf->payload, retBuf, strlen(retBuf) + 1)


/*
 * Commands for device.
 */
#define CMD_GET_BETTERY	"BAT"
#define CMD_GET_TEMP	"TEMP"

static int32_t get_tempr()
{
	return AONBatMonTemperatureGetDegC();
}

static float getVoltage()
{
	int i;
	float voltage;
	uint32_t voltageUint = AONBatMonBatteryVoltageGet();
	uint32_t voltageIntU = voltageUint & 0x00000700;
	voltageIntU = voltageIntU >> 8;
	int voltageInt = (int)voltageIntU;
	uint32_t voltageUintRep = voltageUint;
	float f, b;
	f=0;
	for(i = 8; i > 0; i--)
	{
		b = (float)(voltageUintRep & 0x00000001);
		f += ( b * pow(2, 0 - i));
		voltageUintRep = voltageUintRep >> 1;
	}

	voltage = f + (float)voltageInt;

	return voltage;
}

static void
udpecho_raw_recv_cb_batmon(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port)
{
  LWIP_UNUSED_ARG(arg);
  char command[10];
  int32_t tempr;
  float volt;

  ZERO_OUT_RET_BUFS();

  if (p != NULL) {
	  if(p->len >= MAX_COMMAND_LENGTH) {
	  		// Send null character back to sender
		  UPDATE_PAYLOAD(p, retError);
	  }
	  else {

	  		// Copy command from payload
	  		memcpy(command, p->payload ,p->len);

	  		// Add null terminator
	  		command[p->len] = '\0';

	  		// If Temp command
	  		if(strcmp(command, CMD_GET_TEMP) == 0) {
	  			tempr = get_tempr();
	  			System_sprintf(retBuf, RESP_PREFIX "%d", tempr);
	  			UPDATE_PAYLOAD(p, retBuf);
	  		}

	  		// If Bat command
	  		else if(strcmp(command, CMD_GET_BETTERY) == 0){
	  			volt = getVoltage();
	  			System_sprintf(retBuf, RESP_PREFIX "%f", volt);
	  			UPDATE_PAYLOAD(p, retBuf);
	  		}
	  		else {
	  			// Send null character back to sender
	  			UPDATE_PAYLOAD(p, retError);
	  		}
	  	}
    /* send received packet back to sender */


    udp_sendto(upcb, p, addr, port);

    /* free the pbuf */
    pbuf_free(p);

  }
}

void
udpecho_raw_init_batmon(void)
{
  udpecho_raw_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (udpecho_raw_pcb != NULL) {
    err_t err;

    err = udp_bind(udpecho_raw_pcb, IP_ANY_TYPE, UDP_PORT);
    if (err == ERR_OK) {
      udp_recv(udpecho_raw_pcb, udpecho_raw_recv_cb_batmon, NULL);
    } else {
      /* abort? output diagnostic? */
    }
  } else {
    /* abort? output diagnostic? */
  }
}

#endif /* LWIP_UDP */

int
start_slip()
{
	ip4_addr_t ipaddr_slip, netmask_slip, gw_slip;
	u8_t num_slip = 0;
	struct netif slipif;

	lwip_init();

	// Init udp callback
	udpecho_raw_init_batmon();

	// Init SLIP attributes
	LWIP_PORT_INIT_SLIP_IPADDR(&ipaddr_slip);
	LWIP_PORT_INIT_SLIP_GW(&gw_slip);
	LWIP_PORT_INIT_SLIP_NETMASK(&netmask_slip);

	// Add thread that polls serial line
	netif_add(&slipif, SLIP_ADDRS &num_slip, slipif_init, ip_input);
	netif_set_default(&slipif);
	netif_set_up(&slipif);
	netif_set_link_up(&slipif);

	// Enable batmon module for battery and temperature
	AONBatMonEnable();

	return 0;
}
