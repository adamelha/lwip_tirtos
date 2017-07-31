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
 *
 */
#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <xdc/std.h>

// Not sure that the following will work. not mentioned in http://rtsc.eclipse.org/cdoc-tip/index.html#xdc/runtime/System.html
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"

// Theses should work
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

/* Define platform endianness */
#define BYTE_ORDER LITTLE_ENDIAN

#define LWIP_CHKSUM_ALGORITHM 2

/* Struct packing - GCC convention */
#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>

/*
 * If LWIP_DEBUG_WITH_FLUSH is defined to 1, every debug print will immediately be displayed in ROV.
 * This is not recommended since it slows execution significantly. Although can Help debugging a bit.
 *
 * If LWIP_DEBUG_WITH_FLUSH is 0, user must use System_flush() to flush debug buffer.
 */
#define LWIP_DEBUG_WITH_FLUSH	0

/* Print debug message */
#if LWIP_DEBUG_WITH_FLUSH
#define LWIP_PLATFORM_DIAG(x) { System_printf x; System_flush();}
#else
#define LWIP_PLATFORM_DIAG(x) { System_printf x; }
#endif

/* Print message and abandon execution.  */
#define LWIP_PLATFORM_ASSERT(x) System_printf(x); System_flush(); BIOS_exit(1)

/* SYS_LIGHTWEIGHT_PROT defaults to 0 in opt.h and sys.h provides empty default
 *  definitions for SYS_ARCH_DECL_PROTECT, SYS_ARCH_PROTECT and
 *   SYS_ARCH_UNPROTECT.  */

#define LWIP_PROVIDE_ERRNO

//#define LWIP_DEBUG
//#define LWIP_DBG_MIN_LEVEL	LWIP_DBG_LEVEL_ALL
#endif /* LWIP_ARCH_CC_H */
