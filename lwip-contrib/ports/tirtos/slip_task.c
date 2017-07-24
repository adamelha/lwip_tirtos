/*
 * poll_batmon.c
 *
 *  Created on: 9 בדצמ× 2016
 *      Author: Adam
 */


#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <driverlib/aon_batmon.h>
#include <time.h>
#include <ti/sysbios/hal/Seconds.h>

#include "Board.h"
#include "string.h"
#include <ti/drivers/spi/SPICC26XXDMA.h>
#include <ti/drivers/dma/UDMACC26XX.h>
#include <driverlib/ssi.h>

void start_slip(void);


Task_Params taskParams;
Task_Handle task0;
Error_Block eb;

Void begin(UArg arg0, UArg arg1)
{
	start_slip();

}


Void InitMyTask()
{
	/* Create 1 task with priority 15 */
	Task_Params_init(&taskParams);
	taskParams.stackSize = 560;
	taskParams.priority = 4;
	Error_init(&eb);

	task0 = Task_create((Task_FuncPtr)begin, &taskParams, &eb);

	if (task0 == NULL) {
		System_abort("Task create failed");
	}
}




