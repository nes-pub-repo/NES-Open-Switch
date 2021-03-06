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

#ifndef __MESSAGE_H__
#	define __MESSAGE_H__

#	ifdef __cplusplus
extern "C" {
#	endif



#include "lib/list.h"
#include "lib/binaryTree.h"
#include "lib/sync.h"
#include "lib/thread.h"

#include <stdbool.h>
#include <stdint.h>


enum
{
	xMessageQueue_flagsTx_c = 0,
	xMessageQueue_flagsRx_c = 1,
	xMessageQueue_flagsCount_c = 2,
	
	xMessage_flagsAckInline_c = 0,
	xMessage_flagsCount_c = 1,
};

typedef struct xMessageQueue_t
{
	uint32_t u32Index;
	
	uint8_t au8Flags[1];
	xSList_Head_t oTxList;
	xSList_Head_t oRxList;
	xSList_Head_t oAckList;
	xRwLock_t oLock;
	xBTree_Node_t oBTreeNode;
} xMessageQueue_t;

struct xMessageInfo_t;

typedef struct xMessage_t
{
	uint32_t u32Index;
	void *pvData;
	uint8_t au8Flags[1];
	struct xMessageInfo_t *poMsgInfo;
	xSList_Node_t oQNode;
} xMessage_t;

typedef struct xMessageInfo_t
{
	uint16_t u32Type;
	uint16_t u16RxCount;
	xSList_Head_t oDstList;
	xRwLock_t oLock;
} xMessageInfo_t;


extern xMessageQueue_t *
	xMessageQueue_create (uint32_t u32Index);
extern void
	xMessageQueue_remove (xMessageQueue_t *poEntry);


extern xMessage_t *
	xMessage_allocate (
		uint16_t u32Type, void *pvData);
extern bool
	xMessage_send (
		xMessage_t *poMessage, xMessageQueue_t *poSrcQueue);
extern xMessage_t *
	xMessageAck_getMessage (xMessageQueue_t *poSrcQueue);
extern bool
	xMessageAck_remove (
		xMessage_t *poMessage, xMessageQueue_t *poSrcQueue);
extern bool
	xMessage_cleanupThread (
		xMessageQueue_t *poSrcQueue, xThreadInfo_t *pThread);


extern xMessage_t *
	xMessageDst_create (
		uint32_t u32Index, xMessage_t *poMessage);
extern xMessage_t *
	xMessageDst_getMessage (xMessageQueue_t *poDstQueue);
extern bool
	xMessageDst_remove (
		xMessage_t *poMsg, xMessageQueue_t *poDstQueue);



#	ifdef __cplusplus
}
#	endif

#endif	// __MESSAGE_H__
