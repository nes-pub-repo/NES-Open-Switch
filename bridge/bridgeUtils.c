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

#ifndef __BRIDGEUTILS_C__
#	define __BRIDGEUTILS_C__



#include "bridgeUtils.h"
#include "ethernet/ieee8021BridgeMib.h"
#include "ethernet/ieee8021QBridgeMib.h"
#include "ieee8021PbMib.h"
#include "ieee8021PbbMib.h"
#include "ethernet/ethernetUtils.h"
#include "if/ifUtils.h"
#include "if/ifMIB.h"
#include "hal/halEthernet.h"

#include "lib/bitmap.h"

#include <stdbool.h>
#include <stdint.h>


static ifType_enableHandler_t ieee8021BridgePip_enableHandler;
static ifType_statusModifier_t ieee8021BridgePip_statusModify;
static ifType_stackHandler_t ieee8021BridgePip_stackHandler;


bool bridgeUtilsInit (void)
{
	register bool bRetCode = false;
	ifTypeEntry_t *poIfTypeEntry = NULL;
	
	ifTable_wrLock ();
	
	if ((poIfTypeEntry = ifTypeTable_createExt (ifType_pip_c)) == NULL)
	{
		goto bridgeUtilsInit_cleanup;
	}
	
	poIfTypeEntry->pfEnableHandler = ieee8021BridgePip_enableHandler;
	poIfTypeEntry->pfStatusModifier = ieee8021BridgePip_statusModify;
	poIfTypeEntry->pfStackHandler = ieee8021BridgePip_stackHandler;
	
	bRetCode = true;
	
bridgeUtilsInit_cleanup:
	
	ifTable_unLock ();
	return bRetCode;
}


bool
ieee8021BridgePip_enableHandler (
	ifEntry_t *poIfEntry, uint8_t u8AdminStatus)
{
	return false;
}

bool
ieee8021BridgePip_statusModify (
	ifEntry_t *poIfEntry, uint8_t u8OperStatus, bool bPropagate)
{
	return false;
}

bool
ieee8021BridgePip_stackHandler (
	ifEntry_t *poHigherIfEntry, ifEntry_t *poLowerIfEntry,
	uint8_t u8Action, bool isLocked)
{
	return true;
}


static bool
	ieee8021PbbCbpSidRowStatus_halUpdate (
		ieee8021PbbCbpServiceMappingEntry_t *poEntry, uint8_t u8RowStatus);


bool
ieee8021PbVlanStaticTable_vlanHandler (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021QBridgeVlanStaticEntry_t *poEntry,
	uint8_t *pu8EnabledPorts, uint8_t *pu8DisabledPorts, uint8_t *pu8UntaggedPorts)
{
	register bool bRetCode = false;
	
	if (poComponent->i32ComponentType != ieee8021BridgeBaseComponentType_sVlanComponent_c)
	{
		goto ieee8021PbVlanStaticTable_vlanHandler_success;
	}
	
	register uint16_t u16PortIndex = 0;
	
	xBitmap_scanBitRangeRev (
		pu8DisabledPorts, 0, xBitmap_bitLength (poComponent->oNe.u16Ports_len) - 1, 1, u16PortIndex)
	{
		register ieee8021PbCepEntry_t *poIeee8021PbCepEntry = NULL;
		
		if ((poIeee8021PbCepEntry = ieee8021PbCepTable_getByIndex (poComponent->u32ComponentId, u16PortIndex + 1)) == NULL)
		{
			continue;
		}
		
		register ieee8021PbEdgePortEntry_t *poIeee8021PbEdgePortEntry = NULL;
		
		if ((poIeee8021PbEdgePortEntry = ieee8021PbEdgePortTable_getByIndex (poIeee8021PbCepEntry->u32BridgeBasePortComponentId, poIeee8021PbCepEntry->u32BridgeBasePort, poEntry->u32VlanIndex)) != NULL &&
			!ieee8021PbEdgePortRowStatus_handler (poIeee8021PbEdgePortEntry, xRowStatus_notInService_c))
		{
			goto ieee8021PbVlanStaticTable_vlanHandler_cleanup;
		}
	}
	
	xBitmap_scanBitRangeRev (
		pu8EnabledPorts, 0, xBitmap_bitLength (poComponent->oNe.u16Ports_len) - 1, 1, u16PortIndex)
	{
		register ieee8021PbCepEntry_t *poIeee8021PbCepEntry = NULL;
		
		if ((poIeee8021PbCepEntry = ieee8021PbCepTable_getByIndex (poComponent->u32ComponentId, u16PortIndex + 1)) == NULL)
		{
			continue;
		}
		
		register ieee8021PbEdgePortEntry_t *poIeee8021PbEdgePortEntry = NULL;
		
		if ((poIeee8021PbEdgePortEntry = ieee8021PbEdgePortTable_getByIndex (poIeee8021PbCepEntry->u32BridgeBasePortComponentId, poIeee8021PbCepEntry->u32BridgeBasePort, poEntry->u32VlanIndex)) == NULL &&
			(poIeee8021PbEdgePortEntry = ieee8021PbEdgePortTable_createExt (poIeee8021PbCepEntry->u32BridgeBasePortComponentId, poIeee8021PbCepEntry->u32BridgeBasePort, poEntry->u32VlanIndex)) == NULL)
		{
			goto ieee8021PbVlanStaticTable_vlanHandler_cleanup;
		}
		
		if (!ieee8021PbEdgePortRowStatus_handler (poIeee8021PbEdgePortEntry, poEntry->u8RowStatus))
		{
			goto ieee8021PbVlanStaticTable_vlanHandler_cleanup;
		}
	}
	
ieee8021PbVlanStaticTable_vlanHandler_success:
	
	bRetCode = true;
	
ieee8021PbVlanStaticTable_vlanHandler_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbVlanStaticRowStatus_handler (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021QBridgeVlanStaticEntry_t *poEntry,
	uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	
	if (poComponent->i32ComponentType != ieee8021BridgeBaseComponentType_sVlanComponent_c)
	{
		goto ieee8021PbVlanStaticRowStatus_handler_success;
	}
	
	register uint16_t u16PortIndex = 0;
	
	xBitmap_scanBitRangeRev (
		poEntry->au8EgressPorts, 0, xBitmap_bitLength (poComponent->oNe.u16Ports_len) - 1, 1, u16PortIndex)
	{
		register ieee8021PbEdgePortEntry_t *poIeee8021PbEdgePortEntry = NULL;
		
		if ((poIeee8021PbEdgePortEntry = ieee8021PbEdgePortTable_getByIndex (poComponent->u32ComponentId, u16PortIndex + 1, poEntry->u32VlanIndex)) == NULL)
		{
			continue;
		}
		
		if (!ieee8021PbEdgePortRowStatus_handler (poIeee8021PbEdgePortEntry, u8RowStatus))
		{
			goto ieee8021PbVlanStaticRowStatus_handler_cleanup;
		}
	}
	
ieee8021PbVlanStaticRowStatus_handler_success:
	
	bRetCode = true;
	
ieee8021PbVlanStaticRowStatus_handler_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbILan_createEntry (
	ieee8021BridgeBaseEntry_t *poSComponent, ieee8021BridgeBasePortEntry_t *poCnpPort,
	ieee8021BridgeBaseEntry_t *poCComponent, ieee8021BridgeBasePortEntry_t *poPepPort)
{
	register bool bRetCode = false;
	register bool bPhyLocked = false;
	ifEntry_t *poPepIfEntry = NULL;
	register ieee8021BridgePhyPortEntry_t *poPepPhy = NULL;
	register ieee8021BridgePhyPortEntry_t *poCnpPhy = NULL;
	register ieee8021BridgeILanIfEntry_t *poCnpILanEntry = NULL;
	
	if (poCnpPort == NULL || poPepPort == NULL)
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if (!ifTable_createReference (ifIndex_zero_c, ifType_bridge_c, xAdminStatus_up_c, true, false, false, &poPepIfEntry))
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if ((poCnpILanEntry = ieee8021BridgeILanIfTable_createRegister (ifIndex_zero_c)) == NULL)
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if (!ifStackTable_createRegister (poPepIfEntry->u32Index, poCnpILanEntry->u32Index))
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	ieee8021BridgePhyPortTable_wrLock ();
	bPhyLocked = true;
	
	if ((poPepPhy = ieee8021BridgePhyPortTable_createExt (poPepIfEntry->u32Index, 0)) == NULL)
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if ((poCnpPhy = ieee8021BridgePhyPortTable_createExt (poCnpILanEntry->u32Index, 0)) == NULL)
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if (!ieee8021BridgePhyPortTable_attachComponent (poCComponent, poPepPort, poPepPhy))
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	if (!ieee8021BridgePhyPortTable_attachComponent (poSComponent, poCnpPort, poCnpPhy))
	{
		goto ieee8021PbILan_createEntry_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbILan_createEntry_cleanup:
	
	poPepIfEntry != NULL ? ifEntry_unLock (poPepIfEntry): false;
	
	if (!bRetCode)
	{
		if (poPepPhy != NULL && poPepPort->u32IfIndex == poPepPhy->u32IfIndex)
		{
			ieee8021BridgePhyPortTable_detachComponent (poPepPort, poPepPhy);
			ieee8021BridgePhyPortTable_removeExt (poPepPhy);
		}
		poPepIfEntry != NULL ? ifTable_removeReference (poPepIfEntry->u32Index, true, false, true): false;
		if (poCnpPhy != NULL && poCnpPort->u32IfIndex == poCnpPhy->u32IfIndex)
		{
			ieee8021BridgePhyPortTable_detachComponent (poCnpPort, poCnpPhy);
			ieee8021BridgePhyPortTable_removeExt (poCnpPhy);
		}
		poCnpILanEntry != NULL ? ieee8021BridgeILanIfTable_removeRegister (poCnpILanEntry->u32Index): false;
	}
	
	bPhyLocked ? ieee8021BridgePhyPortTable_unLock (): false;
	
	return bRetCode;
}

bool
ieee8021PbILan_removeEntry (
	ieee8021BridgeBaseEntry_t *poSComponent, ieee8021BridgeBasePortEntry_t *poCnpPort,
	ieee8021BridgeBaseEntry_t *poCComponent, ieee8021BridgeBasePortEntry_t *poPepPort)
{
	register bool bRetCode = false;
	
	
	ieee8021BridgePhyPortTable_wrLock ();
	
	if (poPepPort->u32IfIndex != 0)
	{
		register ieee8021BridgePhyPortEntry_t *poPepPhy = NULL;
		
		if ((poPepPhy = ieee8021BridgePhyPortTable_getByIndex (poPepPort->u32IfIndex, 0)) == NULL ||
			!ieee8021BridgePhyPortTable_detachComponent (poPepPort, poPepPhy) ||
			!ieee8021BridgePhyPortTable_removeExt (poPepPhy))
		{
			goto ieee8021PbILan_removeEntry_phyCleanup;
		}
	}
	
	if (poCnpPort->u32IfIndex != 0)
	{
		register ieee8021BridgePhyPortEntry_t *poCnpPhy = NULL;
		
		if ((poCnpPhy = ieee8021BridgePhyPortTable_getByIndex (poCnpPort->u32IfIndex, 0)) == NULL ||
			!ieee8021BridgePhyPortTable_detachComponent (poCnpPort, poCnpPhy) ||
			!ieee8021BridgePhyPortTable_removeExt (poCnpPhy))
		{
			goto ieee8021PbILan_removeEntry_phyCleanup;
		}
	}
	
	bRetCode = true;
	
ieee8021PbILan_removeEntry_phyCleanup:
	
	ieee8021BridgePhyPortTable_unLock ();
	if (!bRetCode)
	{
		goto ieee8021PbILan_removeEntry_cleanup;
	}
	bRetCode = false;
	
	
	if (poPepPort->u32IfIndex == 0)
	{
		goto ieee8021PbILan_removeEntry_cnpIf;
	}
	
	if (!ifTable_removeReference (poPepPort->u32IfIndex, true, false, true))
	{
		goto ieee8021PbILan_removeEntry_cleanup;
	}
	
ieee8021PbILan_removeEntry_cnpIf:
	
	if (poCnpPort->u32IfIndex == 0)
	{
		goto ieee8021PbILan_removeEntry_success;
	}
	
	if (!ieee8021BridgeILanIfTable_removeRegister (poCnpPort->u32IfIndex))
	{
		goto ieee8021PbILan_removeEntry_success;
	}
	
ieee8021PbILan_removeEntry_success:
	
	poPepPort->u32IfIndex = 0;
	poCnpPort->u32IfIndex = 0;
	
	bRetCode = true;
	
ieee8021PbILan_removeEntry_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbCVidRegistrationRowStatus_update (
	ieee8021PbCVidRegistrationEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	
	if ((poEntry->u8RowStatus == xRowStatus_active_c && u8RowStatus == xRowStatus_active_c) ||
		(poEntry->u8RowStatus != xRowStatus_active_c && u8RowStatus != xRowStatus_active_c))
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_success;
	}
	
	register ieee8021PbCepEntry_t *poIeee8021PbCepEntry = NULL;
	
	if ((poIeee8021PbCepEntry = ieee8021PbCepTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_cleanup;
	}
	
	register ieee8021BridgeBaseEntry_t *poCComponent = NULL;
	
	if ((poCComponent = ieee8021BridgeBaseTable_getByIndex (poIeee8021PbCepEntry->u32CComponentId)) == NULL)
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_cleanup;
	}
	
	register ieee8021QBridgeVlanStaticEntry_t *poCVlanStaticEntry = NULL;
	
	if ((poCVlanStaticEntry = ieee8021QBridgeVlanStaticTable_getByIndex (poIeee8021PbCepEntry->u32CComponentId, poEntry->u32CVid)) == NULL)
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_cleanup;
	}
	
	register ieee8021PbEdgePortEntry_t *poIeee8021PbEdgePortEntry = NULL;
	
	if ((poIeee8021PbEdgePortEntry = ieee8021PbEdgePortTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort, poEntry->u32SVid)) != NULL &&
		!ieee8021QBridgeVlanStaticTable_vHandler (
			poIeee8021PbCepEntry->u32CComponentId, poEntry->u32CVid,
			u8RowStatus == xRowStatus_active_c, poEntry->u8UntaggedPep == ieee8021PbCVidRegistrationUntaggedPep_true_c, 1, poIeee8021PbEdgePortEntry->u32PepPort))
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_cleanup;
	}
	
	if (u8RowStatus == xRowStatus_active_c)
	{
		xBitmap_setBitRev (poCVlanStaticEntry->au8UntaggedPorts, poEntry->u32BridgeBasePort - 1, poEntry->u8UntaggedCep == ieee8021PbCVidRegistrationUntaggedCep_true_c);
	}
	
	if (!ieee8021QBridgeVlanStaticRowStatus_handler (poCComponent, poCVlanStaticEntry, u8RowStatus))
	{
		goto ieee8021PbCVidRegistrationRowStatus_update_cleanup;
	}
	
	if (poIeee8021PbEdgePortEntry != NULL)
	{
		u8RowStatus == xRowStatus_active_c ? poIeee8021PbEdgePortEntry->u32NumCVid++: poIeee8021PbEdgePortEntry->u32NumCVid--;
	}
	
ieee8021PbCVidRegistrationRowStatus_update_success:
	
	bRetCode = true;
	
ieee8021PbCVidRegistrationRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbEdgePortRowStatus_update (
	ieee8021PbEdgePortEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register ieee8021BridgeBaseEntry_t *poIeee8021BridgeBaseEntry = NULL;
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	register ieee8021PbCnpEntry_t *poIeee8021PbCnpEntry = NULL;
	
	if ((poIeee8021BridgeBaseEntry = ieee8021BridgeBaseTable_getByIndex (poEntry->u32CComponentId)) == NULL ||
		(poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32CComponentId, poEntry->u32PepPort)) == NULL ||
		(poIeee8021PbCnpEntry = ieee8021PbCnpTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32PepPort)) != NULL)
	{
		goto ieee8021PbEdgePortRowStatus_update_cleanup;
	}
	
	if (!ieee8021PbCnpRowStatus_handler (poIeee8021PbCnpEntry, u8RowStatus))
	{
		goto ieee8021PbEdgePortRowStatus_update_cleanup;
	}
	
	if (u8RowStatus != xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poIeee8021BridgeBaseEntry, poEntry, ieee8021BridgeBasePortType_providerEdgePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbEdgePortRowStatus_update_cleanup;
	}
	
	if (!ieee8021BridgeBasePortRowStatus_handler (poIeee8021BridgeBaseEntry, poIeee8021BridgeBasePortEntry, u8RowStatus))
	{
		goto ieee8021PbEdgePortRowStatus_update_cleanup;
	}
	
	if (u8RowStatus == xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poIeee8021BridgeBaseEntry, poEntry, ieee8021BridgeBasePortType_providerEdgePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbEdgePortRowStatus_update_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbEdgePortRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbPnpRowStatus_update (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021PbPnpEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	
	if ((poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbPnpRowStatus_update_cleanup;
	}
	
	if (!ieee8021BridgeBasePortRowStatus_handler (poComponent, poIeee8021BridgeBasePortEntry, u8RowStatus))
	{
		goto ieee8021PbPnpRowStatus_update_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbPnpRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbCepRowStatus_update (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021PbCepEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	
	if ((poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32CComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbCepRowStatus_update_cleanup;
	}
	
	if (u8RowStatus != xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_customerEdgePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbCepRowStatus_update_cleanup;
	}
	
	if (!ieee8021BridgeBasePortRowStatus_handler (poComponent, poIeee8021BridgeBasePortEntry, u8RowStatus))
	{
		goto ieee8021PbCepRowStatus_update_cleanup;
	}
	
	if (u8RowStatus == xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_customerEdgePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbCepRowStatus_update_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbCepRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbbVipRowStatus_update (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021PbbVipToPipMappingEntry_t *poVipToPipMappingEntry,
	ieee8021PbbVipEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	
	if ((poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	register uint32_t u32PipIfIndex = poVipToPipMappingEntry != NULL && poVipToPipMappingEntry->u8RowStatus == xRowStatus_active_c ? poVipToPipMappingEntry->u32PipIfIndex: 0;
	
	
	switch (u8RowStatus)
	{
	case xRowStatus_active_c:
		goto ieee8021PbbVipRowStatus_update_updateLocal;
		
	case xRowStatus_notInService_c:
	case xRowStatus_destroy_c:
		if (poEntry->pOldEntry == NULL)
		{
			if ((poEntry->pOldEntry = xBuffer_alloc (sizeof (*poEntry->pOldEntry))) == NULL)
			{
				goto ieee8021PbbVipRowStatus_update_cleanup;
			}
			memcpy (poEntry->pOldEntry, poEntry, sizeof (*poEntry->pOldEntry));
		}
		
		if (u8RowStatus == xRowStatus_destroy_c)
		{
			u32PipIfIndex = 0;
			poEntry->u32ISid = 0;
			goto ieee8021PbbVipRowStatus_update_updateLocal;
		}
		else
		{
			goto ieee8021PbbVipRowStatus_update_updateBase;
		}
		break;
	}
	
	
ieee8021PbbVipRowStatus_update_updateLocal:
	
	
	if (poEntry->pOldEntry != NULL && poEntry->pOldEntry->u32ISid == poEntry->u32ISid)
	{
		goto ieee8021PbbVipRowStatus_update_iSidDone;
	}
	
	if (poEntry->pOldEntry == NULL || poEntry->pOldEntry->u32ISid == 0)
	{
		goto ieee8021PbbVipRowStatus_update_newISid;
	}
	
	xBTree_nodeRemove (&poEntry->oISid_BTreeNode, &oIeee8021PbbVipTable_ISid_BTree);
	poEntry->pOldEntry->u32ISid = 0;
	
ieee8021PbbVipRowStatus_update_newISid:
	
	if (poEntry->u32ISid == 0 ||
		ieee8021PbbVipTable_ISid_getByIndex (poEntry->u32ChassisId, poEntry->u32ISid) != NULL)
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	xBTree_nodeAdd (&poEntry->oISid_BTreeNode, &oIeee8021PbbVipTable_ISid_BTree);
	
ieee8021PbbVipRowStatus_update_iSidDone:
	
	
	if (poEntry->pOldEntry != NULL && poEntry->pOldEntry->u32PipIfIndex == u32PipIfIndex)
	{
		goto ieee8021PbbVipRowStatus_update_pipIfIndexDone;
	}
	
	if (poEntry->pOldEntry == NULL || poEntry->pOldEntry->u32PipIfIndex == 0)
	{
		goto ieee8021PbbVipRowStatus_update_newPipIfIndex;
	}
	
	if (!ifStackTable_removeRegister (poIeee8021BridgeBasePortEntry->u32IfIndex, poEntry->pOldEntry->u32PipIfIndex))
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	{
		register ieee8021PbbPipEntry_t *poIeee8021PbbPipEntry = NULL;
		
		if ((poIeee8021PbbPipEntry = ieee8021PbbPipTable_getByIndex (poEntry->pOldEntry->u32PipIfIndex)) != NULL)
		{
			xBitmap_setBitRev (poIeee8021PbbPipEntry->au8VipMap, poEntry->u32BridgeBasePort - 1, 0);
			poIeee8021PbbPipEntry->u16NumVipPorts != 0 ? poIeee8021PbbPipEntry->u16NumVipPorts--: false;
		}
	}
	
	poEntry->pOldEntry->u32PipIfIndex = 0;
	
ieee8021PbbVipRowStatus_update_newPipIfIndex:
	
	if (u32PipIfIndex == 0)
	{
		goto ieee8021PbbVipRowStatus_update_pipIfIndexDone;
	}
	
	{
		register ieee8021PbbPipEntry_t *poIeee8021PbbPipEntry = NULL;
		
		if ((poIeee8021PbbPipEntry = ieee8021PbbPipTable_getByIndex (u32PipIfIndex)) == NULL)
		{
			goto ieee8021PbbVipRowStatus_update_cleanup;
		}
		
		if ((poIeee8021PbbPipEntry->u32IComponentId != 0 && poIeee8021PbbPipEntry->u32IComponentId != poEntry->u32BridgeBasePortComponentId) ||
			(poIeee8021PbbPipEntry->u32ChassisId != 0 && poIeee8021PbbPipEntry->u32ChassisId != poEntry->u32ChassisId))
		{
			goto ieee8021PbbVipRowStatus_update_cleanup;
		}
		
		if (poIeee8021PbbPipEntry->u32IComponentId == 0 && !ieee8021PbbPipTable_attachComponent (poComponent, poIeee8021PbbPipEntry))
		{
			goto ieee8021PbbVipRowStatus_update_cleanup;
		}
		
		if (!ifStackTable_createRegister (poIeee8021BridgeBasePortEntry->u32IfIndex, u32PipIfIndex))
		{
			goto ieee8021PbbVipRowStatus_update_cleanup;
		}
		
		xBitmap_setBitRev (poIeee8021PbbPipEntry->au8VipMap, poEntry->u32BridgeBasePort - 1, 1);
		poIeee8021PbbPipEntry->u16NumVipPorts++;
		poEntry->u32PipIfIndex = u32PipIfIndex;
	}
	
ieee8021PbbVipRowStatus_update_pipIfIndexDone:
	
	
ieee8021PbbVipRowStatus_update_updateBase:
	
	if (u8RowStatus != xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_virtualInstancePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	if (!ieee8021BridgeBasePortRowStatus_handler (poComponent, poIeee8021BridgeBasePortEntry, u8RowStatus))
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	if (u8RowStatus == xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_virtualInstancePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbbVipRowStatus_update_cleanup;
	}
	
	
	switch (u8RowStatus)
	{
	case xRowStatus_active_c:
	case xRowStatus_destroy_c:
		if (poEntry->pOldEntry != NULL)
		{
			xBuffer_free (poEntry->pOldEntry);
			poEntry->pOldEntry = NULL;
		}
		break;
	}
	
	bRetCode = true;
	
ieee8021PbbVipRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbbPipTable_attachComponent (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021PbbPipEntry_t *poEntry)
{
	register bool bRetCode = false;
	register bool bPhyLocked = false;
	register ieee8021BridgePhyPortEntry_t *poPipPhy = NULL;
	register ieee8021BridgeBasePortEntry_t *poPipPort = NULL;
	
	if (poEntry->u16NumVipPorts == 0 && poEntry->u32IComponentId != 0 &&
		poEntry->u32IComponentId != poComponent->u32ComponentId && !ieee8021PbbPipTable_detachComponent (poEntry))
	{
		goto ieee8021PbbPipTable_attachComponent_cleanup;
	}
	
	ieee8021BridgePhyPortTable_rdLock ();
	bPhyLocked = true;
	
	if ((poPipPhy = ieee8021BridgePhyPortTable_getByIndex (poEntry->u32IfIndex, 0)) == NULL)
	{
		goto ieee8021PbbPipTable_attachComponent_cleanup;
	}
	
	if ((poPipPort = ieee8021BridgeBasePortTable_createExt (poComponent, ieee8021BridgeBasePort_zero_c)) == NULL)
	{
		goto ieee8021PbbPipTable_attachComponent_cleanup;
	}
	poPipPort->i32Type = ieee8021BridgeBasePortType_providerInstancePort_c;
	
	if (!ieee8021BridgePhyPortTable_attachComponent (poComponent, poPipPort, poPipPhy))
	{
		goto ieee8021PbbPipTable_attachComponent_cleanup;
	}
	
	poEntry->u32IComponentId = poComponent->u32ComponentId;
	poEntry->u32PipPort = poPipPort->u32Port;
	
	if (!ieee8021PbbPipRowStatus_handler (poEntry, xRowStatus_active_c | xRowStatus_fromParent_c))
	{
		goto ieee8021PbbPipTable_attachComponent_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbbPipTable_attachComponent_cleanup:
	
	bPhyLocked ? ieee8021BridgePhyPortTable_unLock (): false;
	!bRetCode && poPipPort != NULL ? ieee8021BridgeBasePortTable_removeExt (poComponent, poPipPort): false;
	return bRetCode;
}

bool
ieee8021PbbPipTable_detachComponent (
	ieee8021PbbPipEntry_t *poEntry)
{
	register bool bRetCode = false;
	register ieee8021BridgePhyPortEntry_t *poPipPhy = NULL;
	
	ieee8021BridgePhyPortTable_rdLock ();
	
	if ((poPipPhy = ieee8021BridgePhyPortTable_getByIndex (poEntry->u32IfIndex, 0)) == NULL)
	{
		goto ieee8021PbbPipTable_detachComponent_cleanup;
	}
	
	register ieee8021BridgeBaseEntry_t *poComponent = NULL;
	
	if (poPipPhy->oIf.u32ComponentId == 0)
	{
		goto ieee8021PbbPipTable_detachComponent_success;
	}
	if ((poComponent = ieee8021BridgeBaseTable_getByIndex (poPipPhy->oIf.u32ComponentId)) == NULL)
	{
		goto ieee8021PbbPipTable_detachComponent_cleanup;
	}
	
	register ieee8021BridgeBasePortEntry_t *poPipPort = NULL;
	
	if (poPipPhy->oIf.u32Port == 0 || (poPipPort = ieee8021BridgeBasePortTable_getByIndex (poPipPhy->oIf.u32ComponentId, poPipPhy->oIf.u32Port)) == NULL)
	{
		goto ieee8021PbbPipTable_detachComponent_cleanup;
	}
	
	if (!ieee8021BridgePhyPortTable_detachComponent (poPipPort, poPipPhy))
	{
		goto ieee8021PbbPipTable_detachComponent_cleanup;
	}
	
	if (!ieee8021BridgeBasePortTable_removeExt (poComponent, poPipPort))
	{
		goto ieee8021PbbPipTable_detachComponent_cleanup;
	}
	
	poEntry->u32IComponentId = 0;
	poEntry->u32PipPort = 0;
	
ieee8021PbbPipTable_detachComponent_success:
	
	bRetCode = true;
	
ieee8021PbbPipTable_detachComponent_cleanup:
	
	ieee8021BridgePhyPortTable_unLock ();
	return bRetCode;
}

bool
ieee8021PbbPipRowStatus_update (
	ieee8021PbbPipEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	ifEntry_t *poPipIfEntry = NULL;
	
	if (u8RowStatus == xRowStatus_destroy_c && !ieee8021PbbPipTable_detachComponent (poEntry))
	{
		goto ieee8021PbbPipRowStatus_update_cleanup;
	}
	
	if (!ifTable_createReference (poEntry->u32IfIndex, 0, 0, false, false, false, &poPipIfEntry) ||
		!ifAdminStatus_handler (poPipIfEntry, u8RowStatus == xRowStatus_active_c ? ifAdminStatus_up_c: ifAdminStatus_down_c, false))
	{
		goto ieee8021PbbPipRowStatus_update_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbbPipRowStatus_update_cleanup:
	
	poPipIfEntry != NULL ? ifEntry_unLock (poPipIfEntry): false;
	return bRetCode;
}

bool
ieee8021PbbVipToPipMappingRowStatus_update (
	ieee8021PbbVipToPipMappingEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register uint8_t u8RealStatus = u8RowStatus == xRowStatus_destroy_c ? xRowStatus_notInService_c: u8RowStatus;
	register ieee8021PbbVipEntry_t *poIeee8021PbbVipEntry = NULL;
	
	if ((poIeee8021PbbVipEntry = ieee8021PbbVipTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) != NULL &&
		!ieee8021PbbVipRowStatus_handler (poIeee8021PbbVipEntry, u8RealStatus | xRowStatus_fromParent_c))
	{
		goto ieee8021PbbVipToPipMappingRowStatus_update_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbbVipToPipMappingRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbbCbpServiceMappingRowStatus_update (
	ieee8021PbbCbpEntry_t *poCbpPort,
	ieee8021PbbCbpServiceMappingEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	
	if ((poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
	}
	
	
	switch (u8RowStatus)
	{
	case xRowStatus_active_c:
		goto ieee8021PbbCbpServiceMappingRowStatus_update_updateLocal;
		
	case xRowStatus_notInService_c:
	case xRowStatus_destroy_c:
		if (poEntry->pOldEntry == NULL)
		{
			if ((poEntry->pOldEntry = xBuffer_alloc (sizeof (*poEntry->pOldEntry))) == NULL)
			{
				goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
			}
			memcpy (poEntry->pOldEntry, poEntry, sizeof (*poEntry->pOldEntry));
		}
		
		if (u8RowStatus == xRowStatus_destroy_c)
		{
			poEntry->u32LocalSid = 0;
			goto ieee8021PbbCbpServiceMappingRowStatus_update_updateLocal;
		}
		else
		{
			goto ieee8021PbbCbpServiceMappingRowStatus_update_success;
		}
		break;
	}
	
	
ieee8021PbbCbpServiceMappingRowStatus_update_updateLocal:
	
	if (u8RowStatus != xRowStatus_active_c && !ieee8021PbbCbpSidRowStatus_halUpdate (poEntry, u8RowStatus))
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
	}
	
	
	if (poCbpPort->bExternal ||
		(poEntry->pOldEntry != NULL && poEntry->pOldEntry->u32LocalSid == poEntry->u32LocalSid))
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_sidDone;
	}
	
	if (poEntry->pOldEntry == NULL || poEntry->pOldEntry->u32LocalSid == 0)
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_newSid;
	}
	
	{
		register ieee8021PbbVipEntry_t *poIeee8021PbbVipEntry = NULL;
		
		if ((poIeee8021PbbVipEntry = ieee8021PbbVipTable_ISid_getByIndex (poCbpPort->u32ChassisId, poEntry->pOldEntry->u32LocalSid)) != NULL)
		{
			goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
		}
		
		if (!ifStackTable_removeRegister (poIeee8021PbbVipEntry->u32PipIfIndex, poIeee8021BridgeBasePortEntry->u32IfIndex))
		{
			goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
		}
		
		poEntry->pOldEntry->u32LocalSid = 0;
	}
	
ieee8021PbbCbpServiceMappingRowStatus_update_newSid:
	
	if (poEntry->u32LocalSid == 0)
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_sidDone;
	}
	
	{
		register ieee8021PbbVipEntry_t *poIeee8021PbbVipEntry = NULL;
		
		if ((poIeee8021PbbVipEntry = ieee8021PbbVipTable_ISid_getByIndex (poCbpPort->u32ChassisId, poEntry->u32LocalSid)) != NULL)
		{
			goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
		}
		
		if (!ifStackTable_createRegister (poIeee8021PbbVipEntry->u32PipIfIndex, poIeee8021BridgeBasePortEntry->u32IfIndex))
		{
			goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
		}
	}
	
ieee8021PbbCbpServiceMappingRowStatus_update_sidDone:
	
	
	if (u8RowStatus == xRowStatus_active_c && !ieee8021PbbCbpSidRowStatus_halUpdate (poEntry, u8RowStatus))
	{
		goto ieee8021PbbCbpServiceMappingRowStatus_update_cleanup;
	}
	
	
ieee8021PbbCbpServiceMappingRowStatus_update_success:
	
	switch (u8RowStatus)
	{
	case xRowStatus_active_c:
	case xRowStatus_destroy_c:
		if (poEntry->pOldEntry != NULL)
		{
			xBuffer_free (poEntry->pOldEntry);
			poEntry->pOldEntry = NULL;
		}
		break;
	}
	
	bRetCode = true;
	
ieee8021PbbCbpServiceMappingRowStatus_update_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbbCbpSidRowStatus_halUpdate (
	ieee8021PbbCbpServiceMappingEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register uint8_t u8HalOpCode =
		u8RowStatus == xRowStatus_active_c && poEntry->u8RowStatus != xRowStatus_active_c ? halEthernet_sidEnable_c:
		u8RowStatus != xRowStatus_active_c && poEntry->u8RowStatus == xRowStatus_active_c ? halEthernet_sidDisable_c: halEthernet_sidNone_c;
		
	if (u8HalOpCode != halEthernet_sidNone_c && !halEthernet_cbpSidConfigure (poEntry, u8HalOpCode, NULL))
	{
		goto ieee8021PbbCbpSidRowStatus_halUpdate_cleanup;
	}
	
	bRetCode = true;
	
ieee8021PbbCbpSidRowStatus_halUpdate_cleanup:
	
	return bRetCode;
}

bool
ieee8021PbbCbpRowStatus_update (
	ieee8021BridgeBaseEntry_t *poComponent,
	ieee8021PbbCbpEntry_t *poEntry, uint8_t u8RowStatus)
{
	register bool bRetCode = false;
	register ieee8021BridgeBasePortEntry_t *poIeee8021BridgeBasePortEntry = NULL;
	
	if ((poIeee8021BridgeBasePortEntry = ieee8021BridgeBasePortTable_getByIndex (poEntry->u32BridgeBasePortComponentId, poEntry->u32BridgeBasePort)) == NULL)
	{
		goto ieee8021PbbCbpRowStatus_update_cleanup;
	}
	
	
	if (u8RowStatus == xRowStatus_active_c && poIeee8021BridgeBasePortEntry->u32IfIndex == 0)
	{
		register bool bPhyLocked = false;
		register ieee8021BridgeILanIfEntry_t *poCbpILanEntry = NULL;
		
		if ((poCbpILanEntry = ieee8021BridgeILanIfTable_createRegister (ifIndex_zero_c)) == NULL)
		{
			goto ieee8021PbbCbpRowStatus_update_phyUpCleanup;
		}
		
		ieee8021BridgePhyPortTable_wrLock ();
		bPhyLocked = true;
		
		register ieee8021BridgePhyPortEntry_t *poCbpPhy = NULL;
		
		if ((poCbpPhy = ieee8021BridgePhyPortTable_createExt (poCbpILanEntry->u32Index, 0)) == NULL)
		{
			goto ieee8021PbbCbpRowStatus_update_phyUpCleanup;
		}
		xBitmap_setBitRev (poCbpPhy->au8TypeCapabilities, ieee8021BridgeBasePortTypeCapabilities_customerBackbonePort_c, 1);
		
		poIeee8021BridgeBasePortEntry->u32IfIndex = poCbpILanEntry->u32Index;
		poEntry->bExternal = false;
		bRetCode = true;
		
ieee8021PbbCbpRowStatus_update_phyUpCleanup:
		
		bPhyLocked ? ieee8021BridgePhyPortTable_unLock (): false;
		if (!bRetCode)
		{
			goto ieee8021PbbCbpRowStatus_update_cleanup;
		}
	}
	
	
	if (u8RowStatus != xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_customerBackbonePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbbCbpRowStatus_update_cleanup;
	}
	
	if (!ieee8021BridgeBasePortRowStatus_handler (poComponent, poIeee8021BridgeBasePortEntry, u8RowStatus))
	{
		goto ieee8021PbbCbpRowStatus_update_cleanup;
	}
	
	if (u8RowStatus == xRowStatus_active_c &&
		!ieee8021BridgeXPortRowStatus_halUpdate (poComponent, poEntry, ieee8021BridgeBasePortType_customerBackbonePort_c, poEntry->u8RowStatus, u8RowStatus))
	{
		goto ieee8021PbbCbpRowStatus_update_cleanup;
	}
	
	
	if (u8RowStatus == xRowStatus_destroy_c && poIeee8021BridgeBasePortEntry->u32IfIndex != 0 && !poEntry->bExternal)
	{
		ieee8021BridgePhyPortTable_wrLock ();
		
		register ieee8021BridgePhyPortEntry_t *poCbpPhy = NULL;
		
		if ((poCbpPhy = ieee8021BridgePhyPortTable_getByIndex (poIeee8021BridgeBasePortEntry->u32IfIndex, 0)) == NULL ||
			!ieee8021BridgePhyPortTable_removeExt (poCbpPhy))
		{
			goto ieee8021PbbCbpRowStatus_update_phyDownCleanup;
		}
		
		bRetCode = true;
		
ieee8021PbbCbpRowStatus_update_phyDownCleanup:
		
		ieee8021BridgePhyPortTable_unLock ();
		if (!bRetCode)
		{
			goto ieee8021PbbCbpRowStatus_update_cleanup;
		}
		bRetCode = false;
		
		if (!ieee8021BridgeILanIfTable_removeRegister (poIeee8021BridgeBasePortEntry->u32IfIndex))
		{
			goto ieee8021PbbCbpRowStatus_update_cleanup;
		}
		
		poIeee8021BridgeBasePortEntry->u32IfIndex = 0;
		poEntry->bExternal = true;
	}
	
	
	/* TODO */
	
	bRetCode = true;
	
ieee8021PbbCbpRowStatus_update_cleanup:
	
	return bRetCode;
}



#endif	// __BRIDGEUTILS_C__
