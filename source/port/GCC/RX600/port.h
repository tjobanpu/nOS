/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PORT_H
#define PORT_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4

#define NOS_PORT_SCHED_USE_32_BITS

#if !defined(NOS_CONFIG_ISR_STACK_SIZE)
 #define NOS_CONFIG_ISR_STACK_SIZE          128
 #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (default to 128)."
#else
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #endif
#endif

#if !defined(NOS_CONFIG_MAX_UNSAFE_ISR_PRIO)
 #define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO     4
 #warning "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined (default to 4)."
#endif

#define NOS_PORT_MAX_UNSAFE_IPL             NOS_CONFIG_MAX_UNSAFE_ISR_PRIO

__attribute__( ( always_inline ) ) static inline uint32_t GetIPL(void)
{
	uint32_t ipl;
	__asm volatile(
		"MVFC	PSW,			%0		\n"
		"AND	#0x0F000000,	%0		\n"
	: "=r" (ipl) );
	return ipl;
}

__attribute__( ( always_inline ) ) static inline void SetIPL(uint32_t ipl)
{
	uint32_t psw;
	__asm volatile(
		"MVFC	PSW,			%0		\n"
		"AND	#0xF0FFFFFF,	%0		\n"
		"OR		%1,				%0		\n"
		"MVTC	%0,				PSW		\n"
	: "=r" (psw), "=r" (ipl) );
}

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint32_t _ipl = GetIPL();													\
    __asm volatile ("MVTIPL %0" :: "i" (NOS_PORT_MAX_UNSAFE_IPL) )
    

#define nOS_CriticalLeave()                                                     \
	SetIPL(_ipl);																\
}

#define nOS_ContextSwitch()                                     __asm("INT  #27")

void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);

#define NOS_ISR(vect)                                                           \
void vect(void) __attribute__ ( ( interrupt ) );                 			    \
void vect##_ISR(void);                                                          \
void vect(void)													        		\
{                                                                               \
    nOS_IsrEnter();                                                             \
    vect##_ISR();                                                               \
    __asm volatile ("MVTIPL %0" :: "i" (NOS_PORT_MAX_UNSAFE_IPL) );				\
    nOS_IsrLeave();                                                             \
}                                                                               \
__attribute__ ( ( always_inline ) ) inline void vect##_ISR(void)

#if defined(NOS_PRIVATE)
void    nOS_PortInit        (void);
#endif

void    nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* PORT_H */