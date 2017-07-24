/*
 * sys_arch.c
 *
 *  Created on: 12 במרץ 2017
 *      Author: Adam
 */
#include "lwip/sys.h"
#include "arch/sys_arch.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>
#include "lwip/mem.h"
#include <xdc/runtime/HeapStd.h>

#include <ti/sysbios/BIOS.h>
#include <string.h>
#include <stdlib.h>

/********************Static Functions********************/

static u32_t ticks_to_millisecs(u32_t ticks)
{
#ifdef DEFAULT_1MS_CLOCK_TICK
	return ticks;
#else
	Types_FreqHz freq;

	// This func is not thread safe, might want to use lock.
	BIOS_getCpuFreq(&freq);
	return (u32_t)(((double)ticks / (double)(freq.lo)) * (double)1000);
#endif
}
/*****************************************************/
/********************Lib Functions********************/
/*****************************************************/

/*------------------- init -------------------*/
void sys_init()
{
	return;
}

/*------------------- Semaphores -------------------*/
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	sem->semParams.mode = Semaphore_Mode_COUNTING;
	sem->semHandle = Semaphore_create(count, &(sem->semParams), &(sem->eb));
	return (!(sem->semHandle)) ? ERR_MEM : ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
	Semaphore_delete(&(sem->semHandle));
}

void sys_sem_signal(sys_sem_t *sem)
{
	Semaphore_post(sem->semHandle);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	u32_t ticksStart, ticksEnd, ticksDifference, millisecDifference, updatedTimeout;
	updatedTimeout = (timeout == 0) ? BIOS_WAIT_FOREVER: timeout;
	int stat;
	ticksStart = Clock_getTicks();
	stat = Semaphore_pend(sem->semHandle, updatedTimeout);
	if(!stat)
	{
		return SYS_ARCH_TIMEOUT;
	}
	ticksEnd = Clock_getTicks();

	ticksDifference = ticksEnd - ticksStart;
	millisecDifference = ticks_to_millisecs(ticksDifference);

	/* Return how long until semaphore was acquired */
	return millisecDifference;
}

int sys_sem_valid(sys_sem_t *sem)
{
	return (int) ( ((sem) != NULL) && ( sem->semHandle != NULL) );
}

// Nothing to do here. Called always after sys_sem_free() which already sets the handler to NULL.
void sys_sem_set_invalid(sys_sem_t *sem)
{
	return;
}

/*------------------- Mutexes -------------------*/
/* Just Binary Semaphores */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
	mutex->semParams.mode = Semaphore_Mode_BINARY;
	mutex->semHandle = Semaphore_create(1, &(mutex->semParams), &(mutex->eb));

	return (!(mutex->semHandle)) ? ERR_MEM : ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
	sys_sem_free(mutex);
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
	sys_arch_sem_wait(mutex, 0);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
	sys_sem_signal(mutex);
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
	return sys_sem_valid(mutex);
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
	sys_sem_set_invalid(mutex);
}

/*------------------- Mailboxes -------------------*/

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	mbox->mboxHandle = Mailbox_create(MAX_MAILBOX_MESSAGE_SIZE, size, NULL, NULL);
	return (mbox->mboxHandle) ? ERR_OK : ERR_MEM;
}


void sys_mbox_free(sys_mbox_t *mbox)
{
	Mailbox_delete(&(mbox->mboxHandle));
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	Mailbox_post(mbox->mboxHandle, msg, BIOS_WAIT_FOREVER);
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	return (Mailbox_post(mbox->mboxHandle, msg, BIOS_NO_WAIT)) ? ERR_OK : ERR_MEM;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	u32_t ticksStart, ticksEnd, ticksDifference, millisecDifference, updatedTimeout;
	updatedTimeout = (timeout == 0) ? BIOS_WAIT_FOREVER: timeout;

	ticksStart = Clock_getTicks();
	if(!Mailbox_pend(mbox->mboxHandle, mbox->msg, updatedTimeout))
	{
		return SYS_ARCH_TIMEOUT;
	}
	ticksEnd = Clock_getTicks();

	// If msg is NULL, message from mailbox should be dropped.
	if(msg)
	{
		*msg = mbox->msg;
	}

	ticksDifference = ticksEnd - ticksStart;
	millisecDifference = ticks_to_millisecs(ticksDifference);

	// Return how long until message was acquired
	return millisecDifference;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	if(!Mailbox_pend(mbox->mboxHandle, mbox->msg, BIOS_NO_WAIT))
	{
			return SYS_MBOX_EMPTY;
	}

	// If msg is NULL, message from mailbox should be dropped.
	if(msg)
	{
		*msg = mbox->msg;
	}

	return ERR_OK;
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
	return (int)(((mbox) != NULL) && (mbox->mboxHandle != NULL));
}

// Nothing to do here. Called always after sys_sem_free() which already sets the handler to NULL.
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	return;
}


/*------------------- Threads -------------------*/

typedef void Thread_FuncPtr(void*);

//threadFunc is void func pointer with void* arg
//threadArg is void*
Void taskFunc(UArg threadFunc, UArg threadArg)
{
	((Thread_FuncPtr*)(threadFunc)) ((void*)threadArg);
}


sys_thread_t sys_thread_new(const char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio)
{
	// Uses lwip mem_malloc might want to use Memory_alloc or libc malloc instead.
	// For libc malloc, in cfd changed to var Memory = xdc.useModule('xdc.runtime.HeapStd');

	sys_thread_t newThread = (sys_thread_t)mem_malloc(sizeof(struct sys_thread));

	strncpy(newThread->name, name, MAX_THREAD_NAME_SIZE - 1);

	Error_init(&(newThread->eb));

	// Init thread params
	Task_Params_init(&(newThread->taskParams));
	newThread->taskParams.stackSize = stacksize;
	newThread->taskParams.priority = prio;
	newThread->taskParams.arg0 = (UArg)thread;
	newThread->taskParams.arg1 = (UArg)arg;

	newThread->taskHandle = Task_create(taskFunc, &(newThread->taskParams), &(newThread->eb));

	return newThread;
}



u32_t sys_now()
{
	u32_t ticks = Clock_getTicks();
	return ticks_to_millisecs(ticks);
}
