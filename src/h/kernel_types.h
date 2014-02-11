/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Definition of Kernel Message Codes.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/17/93   |  tjt  Original implementation.
 *             |
 *-------------------------------------------------------------------------*/
#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

static char kernel_types_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Kernel Events - Distinguished by Kernel as the destination.
 *-------------------------------------------------------------------------*/
 
#define  KernelDestination          127

#define  KernelLogIn                1
#define  KernelLogOut               2
#define  KernelSelect               3
#define  KernelTaskName             4
#define  KernelSignal               5
#define  KernelAck                  6
#define  KernelStartLog             7
#define  KernelStopLog              8
#define  KernelShutdown             9
#define  KernelWho                  10

#endif

/* end of kernel_types.h */
