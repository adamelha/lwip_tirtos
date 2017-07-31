/*
 * sio.c
 *
 *  Created on: 16 במרץ 2017
 *      Author: Adam
 */

#include "lwip/sio.h"
#include <ti/drivers/UART.h>
#include <Board.h>

#include "arch/sys_arch.h"
#include "lwip/sys.h"

#define UART_BAUD_RATE	115200

static UART_Handle hUart = NULL;
static UART_Params uartParams;

sys_sem_t semRead;

size_t gBytesRead;

void uartReadCb(UART_Handle handle, void *buf, size_t count)
{
	gBytesRead += count;
	sys_sem_signal(&semRead);
}


/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
	UART_Params_init(&uartParams);
	uartParams.baudRate = UART_BAUD_RATE;
	uartParams.writeDataMode = UART_DATA_BINARY;
	uartParams.readReturnMode = UART_RETURN_FULL;

	uartParams.writeMode = UART_MODE_BLOCKING;

	//readMode uses callback instead of blocking because we need to send it an interrupt if we want to cancel the polling.
	uartParams.readDataMode = UART_DATA_BINARY;
	uartParams.readMode = UART_MODE_CALLBACK;
	uartParams.readEcho = UART_ECHO_ON;
	uartParams.readCallback = uartReadCb;
	uartParams.readTimeout = 2;
	hUart = UART_open(Board_UART, &uartParams);

	sys_sem_new(&semRead, 1);
	//Take lock. Unlock only by uartReadCb() when uart work done.
	sys_arch_sem_wait(&semRead, 0);
	return hUart;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
	int written = UART_write((UART_Handle)fd, &c, 1);
}

/**
 * Receives a single character from the serial device.
 *
 * @param fd serial device handle
 *
 * @note This function will block until a character is received.
 */
u8_t sio_recv(sio_fd_t fd)
{
	u8_t c;

	UART_read((UART_Handle)fd, &c, 1);
	sys_arch_sem_wait(&semRead, 0);

	return c;
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
	u32_t numberOfBytesRead;

	UART_read((UART_Handle)fd, data, len);
	sys_arch_sem_wait(&semRead, 0);

	// Assert correct number of bytes read
	while(gBytesRead < len)
	{

	}

	numberOfBytesRead = gBytesRead;
	gBytesRead = 0;

	return numberOfBytesRead;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{

	u32_t numberOfBytesRead;

	UART_read(fd, data, len);

	sys_arch_sem_wait(&semRead, 1);

	numberOfBytesRead = gBytesRead;

	gBytesRead = 0;

	return numberOfBytesRead;
}

/**
 * Writes to the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data to send
 * @param len length (in bytes) of data to send
 * @return number of bytes actually sent
 *
 * @note This function will block until all data can be sent.
 */
u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len)
{
	u32_t written = (u32_t)UART_write(fd, data, len);

	return written;
}

/**
 * Aborts a blocking sio_read() call.
 *
 * @param fd serial device handle
 */
void sio_read_abort(sio_fd_t fd)
{
	UART_readCancel(fd);

	sys_sem_signal(&semRead);
}
