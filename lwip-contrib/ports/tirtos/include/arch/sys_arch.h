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
#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>

/* Maximum message buffer for mailboxes */
#define MAX_MAILBOX_MESSAGE_SIZE	1024

/* Maximum size of a name for a thread */
#define MAX_THREAD_NAME_SIZE		100

/* Defining this will treat every clock tick as a millisecond.
 * This is instead of calling bios API BIOS_getCpuFreq which is not thread safe and will require additional locking */
#define DEFAULT_1MS_CLOCK_TICK

#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL  NULL

typedef u32_t sys_prot_t;

void sys_init(void);


/* Semaphores */
struct sys_sem {
  Semaphore_Handle semHandle;
  Semaphore_Params semParams;
  Error_Block eb;
};

typedef struct sys_sem 	sys_sem_t;

int sys_sem_valid(sys_sem_t *sem);
void sys_sem_set_invalid(sys_sem_t *sem);

/* Mutexes */
typedef sys_sem_t			sys_mutex_t;

int sys_mutex_valid(sys_mutex_t *mutex);
void sys_mutex_set_invalid(sys_mutex_t *mutex);

/* Mailboxes */
struct sys_mbox {
  void *msg[MAX_MAILBOX_MESSAGE_SIZE];
  Mailbox_Handle mboxHandle;
};
typedef struct sys_mbox		sys_mbox_t;

int sys_mbox_valid(sys_mbox_t *mbox);
void sys_mbox_set_invalid(sys_mbox_t *mbox);

/* Threads */
struct sys_thread {
  char name[MAX_THREAD_NAME_SIZE];
  Task_Handle taskHandle;
  Task_Params taskParams;
  Error_Block eb;
};
typedef struct sys_thread *	sys_thread_t;

// Already declared in sys.h
//sys_thread_t sys_thread_new(const char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio);

//u32_t sys_now();

#endif /* LWIP_ARCH_SYS_ARCH_H */

