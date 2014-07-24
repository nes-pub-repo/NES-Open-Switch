/*
 *  Copyright (c) 2013, 2014
 *      NES <nes.open.switch@gmail.com>
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

#ifndef __BITMAP_C__
#	define __BITMAP_C__



#include "bitmap.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>


bool
xBitmap_vSetBits (
	bool bIsRev, xBitmask_t *poMap, uint32_t u32Count, bool bVal, uint32_t u32Pos, ...)
{
	va_list oArgs;
	
	va_start (oArgs, u32Pos);
	
	do
	{
		register uint16_t u16Idx = xBitmap_maskIndex (u32Pos);
		
		poMap[u16Idx] = bIsRev ?
			((poMap[u16Idx] & ~xBitmask_bitMaskRev (u32Pos)) | (bVal << xBitmask_bitIndexRev (u32Pos))):
			((poMap[u16Idx] & ~xBitmask_bitMask (u32Pos)) | (bVal << xBitmask_bitIndex (u32Pos)));
			
		u32Count--;
		if (u32Count > 0)
		{
			u32Pos = va_arg (oArgs, uint32_t);
		}
	}
	while (u32Count > 0);
	
	va_end (oArgs);
	return true;
}



#endif	// __BITMAP_C__
