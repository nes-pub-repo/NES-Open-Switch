/*
 *  Copyright (c) 2013, 2014
 *      NES <nes.open.switch@gmail.com>
 *
 *  All rights reserved. This source file is the sole property of NES, and
 *  contain proprietary and confidential information related to NES.
 *
 *  Licensed under the NES PROF License, Version 1.0 (the "License"); you may
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

#define SNMP_SRC

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "systemMIB.h"

#include "lib/binaryTree.h"
#include "lib/buffer.h"
#include "lib/snmp.h"

#define ROLLBACK_BUFFER "ROLLBACK_BUFFER"



/* array length = OID_LENGTH + 1 */
static oid system_oid[] = {1,3,6,1,2,1,1,1};

static oid sysORTable_oid[] = {1,3,6,1,2,1,1,9};



/**
 *	initialize systemMIB group mapper
 */
void
systemMIB_init (void)
{
	extern oid system_oid[];
	
	DEBUGMSGTL (("systemMIB", "Initializing\n"));
	
	/* register system scalar mapper */
	netsnmp_register_scalar_group (
		netsnmp_create_handler_registration (
			"system_mapper", &system_mapper,
			system_oid, OID_LENGTH (system_oid) - 1,
			HANDLER_CAN_RWRITE
		),
		SYSDESCR,
		SYSORLASTCHANGE
	);
	
	
	/* register systemMIB group table mappers */
	sysORTable_init ();
}


/**
 *	scalar mapper(s)
 */
system_t oSystem;

/** system scalar mapper **/
int
system_mapper (netsnmp_mib_handler *handler,
	netsnmp_handler_registration *reginfo,
	netsnmp_agent_request_info   *reqinfo,
	netsnmp_request_info         *requests)
{
	extern oid system_oid[];
	netsnmp_request_info *request;
	int ret;
	/* We are never called for a GETNEXT if it's registered as a
	   "group instance", as it's "magically" handled for us. */
	
	switch (reqinfo->mode)
	{
	case MODE_GET:
		for (request = requests; request != NULL; request = request->next)
		{
			switch (request->requestvb->name[OID_LENGTH (system_oid) - 1])
			{
			case SYSDESCR:
				snmp_set_var_typed_value (request->requestvb, ASN_OCTET_STR, (u_char*) oSystem.au8Descr, oSystem.u16Descr_len);
				break;
			case SYSOBJECTID:
				snmp_set_var_typed_value (request->requestvb, ASN_OBJECT_ID, (u_char*) oSystem.aoObjectID, oSystem.u16ObjectID_len);
				break;
			case SYSUPTIME:
				snmp_set_var_typed_integer (request->requestvb, ASN_TIMETICKS, oSystem.u32UpTime);
				break;
			case SYSCONTACT:
				snmp_set_var_typed_value (request->requestvb, ASN_OCTET_STR, (u_char*) oSystem.au8Contact, oSystem.u16Contact_len);
				break;
			case SYSNAME:
				snmp_set_var_typed_value (request->requestvb, ASN_OCTET_STR, (u_char*) oSystem.au8Name, oSystem.u16Name_len);
				break;
			case SYSLOCATION:
				snmp_set_var_typed_value (request->requestvb, ASN_OCTET_STR, (u_char*) oSystem.au8Location, oSystem.u16Location_len);
				break;
			case SYSSERVICES:
				snmp_set_var_typed_integer (request->requestvb, ASN_INTEGER, oSystem.i32Services);
				break;
			case SYSORLASTCHANGE:
				snmp_set_var_typed_integer (request->requestvb, ASN_TIMETICKS, oSystem.u32ORLastChange);
				break;
				
			default:
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHOBJECT);
				continue;
			}
		}
		break;
		
	/*
	 * SET REQUEST
	 *
	 * multiple states in the transaction.  See:
	 * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
	 */
	case MODE_SET_RESERVE1:
		for (request = requests; request != NULL; request = request->next)
		{
			switch (request->requestvb->name[OID_LENGTH (system_oid) - 1])
			{
			case SYSCONTACT:
				ret = netsnmp_check_vb_type (requests->requestvb, ASN_OCTET_STR);
				if (ret != SNMP_ERR_NOERROR)
				{
					netsnmp_set_request_error (reqinfo, requests, ret);
				}
				break;
			case SYSNAME:
				ret = netsnmp_check_vb_type (requests->requestvb, ASN_OCTET_STR);
				if (ret != SNMP_ERR_NOERROR)
				{
					netsnmp_set_request_error (reqinfo, requests, ret);
				}
				break;
			case SYSLOCATION:
				ret = netsnmp_check_vb_type (requests->requestvb, ASN_OCTET_STR);
				if (ret != SNMP_ERR_NOERROR)
				{
					netsnmp_set_request_error (reqinfo, requests, ret);
				}
				break;
				
			default:
				netsnmp_set_request_error (reqinfo, request, SNMP_ERR_NOTWRITABLE);
				continue;
			}
		}
		break;
		
	case MODE_SET_RESERVE2:
		break;
		
	case MODE_SET_FREE:
		break;
		
	case MODE_SET_ACTION:
		for (request = requests; request != NULL; request = request->next)
		{
			switch (request->requestvb->name[OID_LENGTH (system_oid) - 1])
			{
			case SYSCONTACT:
				/* XXX: perform the value change here */
				memset (oSystem.au8Contact, 0, sizeof (oSystem.au8Contact));
				memcpy (oSystem.au8Contact, request->requestvb->val.string, request->requestvb->val_len);
				oSystem.u16Contact_len = request->requestvb->val_len;
				if (/* TODO: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					netsnmp_set_request_error (reqinfo, requests, /* some error */ TOBE_REPLACED);
				}
				break;
			case SYSNAME:
				/* XXX: perform the value change here */
				memset (oSystem.au8Name, 0, sizeof (oSystem.au8Name));
				memcpy (oSystem.au8Name, request->requestvb->val.string, request->requestvb->val_len);
				oSystem.u16Name_len = request->requestvb->val_len;
				if (/* TODO: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					netsnmp_set_request_error (reqinfo, requests, /* some error */ TOBE_REPLACED);
				}
				break;
			case SYSLOCATION:
				/* XXX: perform the value change here */
				memset (oSystem.au8Location, 0, sizeof (oSystem.au8Location));
				memcpy (oSystem.au8Location, request->requestvb->val.string, request->requestvb->val_len);
				oSystem.u16Location_len = request->requestvb->val_len;
				if (/* TODO: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					netsnmp_set_request_error (reqinfo, requests, /* some error */ TOBE_REPLACED);
				}
				break;
			}
		}
		break;
		
	case MODE_SET_COMMIT:
		break;
		
	case MODE_SET_UNDO:
		for (request = requests; request != NULL; request = request->next)
		{
			switch (request->requestvb->name[OID_LENGTH (system_oid) - 1])
			{
			case SYSCONTACT:
				/* XXX: UNDO and return to previous value for the object */
				if (/* XXX: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					/* try _really_really_ hard to never get to this point */
					netsnmp_set_request_error (reqinfo, requests, SNMP_ERR_UNDOFAILED);
				}
				break;
			case SYSNAME:
				/* XXX: UNDO and return to previous value for the object */
				if (/* XXX: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					/* try _really_really_ hard to never get to this point */
					netsnmp_set_request_error (reqinfo, requests, SNMP_ERR_UNDOFAILED);
				}
				break;
			case SYSLOCATION:
				/* XXX: UNDO and return to previous value for the object */
				if (/* XXX: error? */ TOBE_REPLACED != TOBE_REPLACED)
				{
					/* try _really_really_ hard to never get to this point */
					netsnmp_set_request_error (reqinfo, requests, SNMP_ERR_UNDOFAILED);
				}
				break;
			}
		}
		break;
		
	default:
		/* we should never get here, so this is a really bad error */
		snmp_log (LOG_ERR, "unknown mode (%d) in handle_\n", reqinfo->mode);
		return SNMP_ERR_GENERR;
	}
	
	return SNMP_ERR_NOERROR;
}


/**
 *	table mapper(s) & helper(s)
 */
/** initialize sysORTable table mapper **/
void
sysORTable_init (void)
{
	extern oid sysORTable_oid[];
	netsnmp_handler_registration *reg;
	netsnmp_iterator_info *iinfo;
	netsnmp_table_registration_info *table_info;
	
	reg = netsnmp_create_handler_registration (
		"sysORTable", &sysORTable_mapper,
		sysORTable_oid, OID_LENGTH (sysORTable_oid),
		HANDLER_CAN_RONLY
		);
		
	table_info = xBuffer_cAlloc (sizeof (netsnmp_table_registration_info));
	netsnmp_table_helper_add_indexes (table_info,
		ASN_INTEGER /* index: sysORIndex */,
		0);
	table_info->min_column = SYSORID;
	table_info->max_column = SYSORUPTIME;
	
	iinfo = xBuffer_cAlloc (sizeof (netsnmp_iterator_info));
	iinfo->get_first_data_point = &sysORTable_getFirst;
	iinfo->get_next_data_point = &sysORTable_getNext;
	iinfo->get_data_point = &sysORTable_get;
	iinfo->table_reginfo = table_info;
	iinfo->flags |= NETSNMP_ITERATOR_FLAG_SORTED;
	
	netsnmp_register_table_iterator (reg, iinfo);
	
	/* Initialise the contents of the table here */
}

static int8_t
sysORTable_BTreeNodeCmp (
	xBTree_Node_t *pNode1, xBTree_Node_t *pNode2, xBTree_t *pBTree)
{
	register sysOREntry_t *pEntry1 = xBTree_entry (pNode1, sysOREntry_t, oBTreeNode);
	register sysOREntry_t *pEntry2 = xBTree_entry (pNode2, sysOREntry_t, oBTreeNode);
	
	return
		(pEntry1->i32Index < pEntry2->i32Index) ? -1:
		(pEntry1->i32Index == pEntry2->i32Index) ? 0: 1;
}

xBTree_t oSysORTable_BTree = xBTree_initInline (&sysORTable_BTreeNodeCmp);

/* create a new row in the (unsorted) table */
sysOREntry_t *
sysORTable_createEntry (
	int32_t i32Index)
{
	sysOREntry_t *poEntry = NULL;
	
	if ((poEntry = xBuffer_cAlloc (sizeof (sysOREntry_t))) == NULL)
	{
		return NULL;
	}
	
	poEntry->i32Index = i32Index;
	if (xBTree_nodeFind (&poEntry->oBTreeNode, &oSysORTable_BTree) != NULL)
	{
		xBuffer_free (poEntry);
		return NULL;
	}
	
	xBTree_nodeAdd (&poEntry->oBTreeNode, &oSysORTable_BTree);
	return poEntry;
}

sysOREntry_t *
sysORTable_getByIndex (
	int32_t i32Index)
{
	register sysOREntry_t *poTmpEntry = NULL;
	register xBTree_Node_t *poNode = NULL;
	
	if ((poTmpEntry = xBuffer_cAlloc (sizeof (sysOREntry_t))) == NULL)
	{
		return NULL;
	}
	
	poTmpEntry->i32Index = i32Index;
	if ((poNode = xBTree_nodeFind (&poTmpEntry->oBTreeNode, &oSysORTable_BTree)) == NULL)
	{
		xBuffer_free (poTmpEntry);
		return NULL;
	}
	
	xBuffer_free (poTmpEntry);
	return xBTree_entry (poNode, sysOREntry_t, oBTreeNode);
}

sysOREntry_t *
sysORTable_getNextIndex (
	int32_t i32Index)
{
	register sysOREntry_t *poTmpEntry = NULL;
	register xBTree_Node_t *poNode = NULL;
	
	if ((poTmpEntry = xBuffer_cAlloc (sizeof (sysOREntry_t))) == NULL)
	{
		return NULL;
	}
	
	poTmpEntry->i32Index = i32Index;
	if ((poNode = xBTree_nodeFindNext (&poTmpEntry->oBTreeNode, &oSysORTable_BTree)) == NULL)
	{
		xBuffer_free (poTmpEntry);
		return NULL;
	}
	
	xBuffer_free (poTmpEntry);
	return xBTree_entry (poNode, sysOREntry_t, oBTreeNode);
}

/* remove a row from the table */
void
sysORTable_removeEntry (sysOREntry_t *poEntry)
{
	if (poEntry == NULL ||
		xBTree_nodeFind (&poEntry->oBTreeNode, &oSysORTable_BTree) == NULL)
	{
		return;    /* Nothing to remove */
	}
	
	xBTree_nodeRemove (&poEntry->oBTreeNode, &oSysORTable_BTree);
	xBuffer_free (poEntry);   /* XXX - release any other internal resources */
	return;
}

/* example iterator hook routines - using 'getNext' to do most of the work */
netsnmp_variable_list *
sysORTable_getFirst (
	void **my_loop_context, void **my_data_context,
	netsnmp_variable_list *put_index_data, netsnmp_iterator_info *mydata)
{
	*my_loop_context = xBTree_nodeGetFirst (&oSysORTable_BTree);
	return sysORTable_getNext (my_loop_context, my_data_context, put_index_data, mydata);
}

netsnmp_variable_list *
sysORTable_getNext (
	void **my_loop_context, void **my_data_context,
	netsnmp_variable_list *put_index_data, netsnmp_iterator_info *mydata)
{
	sysOREntry_t *poEntry = NULL;
	netsnmp_variable_list *idx = put_index_data;
	
	if (*my_loop_context == NULL)
	{
		return NULL;
	}
	poEntry = xBTree_entry (*my_loop_context, sysOREntry_t, oBTreeNode);
	
	snmp_set_var_typed_integer (idx, ASN_INTEGER, poEntry->i32Index);
	*my_data_context = (void*) poEntry;
	*my_loop_context = (void*) xBTree_nodeGetNext (&poEntry->oBTreeNode, &oSysORTable_BTree);
	return put_index_data;
}

bool
sysORTable_get (
	void **my_data_context,
	netsnmp_variable_list *put_index_data, netsnmp_iterator_info *mydata)
{
	sysOREntry_t *poEntry = NULL;
	register netsnmp_variable_list *idx1 = put_index_data;
	
	poEntry = sysORTable_getByIndex (
		*idx1->val.integer);
	if (poEntry == NULL)
	{
		return false;
	}
	
	*my_data_context = (void*) poEntry;
	return true;
}

/* sysORTable table mapper */
int
sysORTable_mapper (
	netsnmp_mib_handler *handler,
	netsnmp_handler_registration *reginfo,
	netsnmp_agent_request_info *reqinfo,
	netsnmp_request_info *requests)
{
	netsnmp_request_info *request;
	netsnmp_table_request_info *table_info;
	sysOREntry_t *table_entry;
	
	switch (reqinfo->mode)
	{
	/*
	 * Read-support (also covers GetNext requests)
	 */
	case MODE_GET:
		for (request = requests; request != NULL; request = request->next)
		{
			table_entry = (sysOREntry_t*) netsnmp_extract_iterator_context (request);
			table_info = netsnmp_extract_table_info (request);
			if (table_entry == NULL)
			{
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHINSTANCE);
				continue;
			}
			
			switch (table_info->colnum)
			{
			case SYSORID:
				snmp_set_var_typed_value (request->requestvb, ASN_OBJECT_ID, (u_char*) table_entry->aoID, table_entry->u16ID_len);
				break;
			case SYSORDESCR:
				snmp_set_var_typed_value (request->requestvb, ASN_OCTET_STR, (u_char*) table_entry->au8Descr, table_entry->u16Descr_len);
				break;
			case SYSORUPTIME:
				snmp_set_var_typed_integer (request->requestvb, ASN_TIMETICKS, table_entry->u32UpTime);
				break;
				
			default:
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHOBJECT);
				break;
			}
		}
		break;
		
	}
	
	return SNMP_ERR_NOERROR;
}
