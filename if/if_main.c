/*
 *  Copyright (c) 2008-2016
 *      NES Repo <nes.repo@gmail.com>
 *
 *  All rights reserved. This source file is the sole property of NES, and
 *  contain proprietary and confidential information related to NES.
 *
 *  Licensed under the NES RED License, Version 1.0 (the "License"); you may
 *  not use this file except in compliance with the License. You may obtain a
 *  copy of the License bundled along with this file. Any kind of reproduction
 *  or duplication of any part of this file which conflicts with the License
 *  without prior written consent from NES is strictly prohibited.
 *
 *  Unless required by applicable law and agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
//set ts=4 sw=4

#ifndef __IF_MAIN_C__
#	define __IF_MAIN_C__


#include "ifMIB_agent.h"
#include "neXcMIB_agent.h"

#include "if_ext.h"
#include "if_defines.h"
#include "switch_ext.h"

#include "lib/thread.h"


static xThreadInfo_t oIfThread =
{
	.u32Index = XTHREAD_ID (ModuleId_if_c, 0),
	.u8SchedPolicy = SCHED_RR,
	.u8Priority = 1,
	.poStart = &if_start,
};


void *
if_main (void *pvArgv)
{
	register void *pvRetCode = NULL;
	register uint32_t u32ModuleOp = (uintptr_t) pvArgv;
	
	switch (u32ModuleOp)
	{
	default:
		break;
		
	case ModuleOp_start_c:
		ifMIB_init ();
		neXcMIB_init ();
		
		if (xThread_create (&oIfThread) == NULL)
		{
			If_log (xLog_err_c, "xThread_create() failed\n");
			goto if_main_cleanup;
		}
		break;
	}
	
	pvRetCode = (void*) true;
	
if_main_cleanup:
	
	return pvRetCode;
}

void *
if_start (void *pvArgv)
{
	while (1)
	{
		xThread_sleep (1);
	}
	return NULL;
}


#endif	// __IF_MAIN_C__
