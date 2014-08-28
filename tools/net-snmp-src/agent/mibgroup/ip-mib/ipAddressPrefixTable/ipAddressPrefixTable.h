/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.48 $ of : mfd-top.m2c,v $
 *
 * $Id$
 */
#ifndef IPADDRESSPREFIXTABLE_H
#define IPADDRESSPREFIXTABLE_H

#ifdef __cplusplus
extern          "C" {
#endif


/** @addtogroup misc misc: Miscellaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

    /*
     * other required module components 
     */
    /* *INDENT-OFF*  */
config_require(ip-mib/ipAddressTable/ipAddressTable)
config_require(ip-mib/ipAddressPrefixTable/ipAddressPrefixTable_interface)
config_require(ip-mib/ipAddressPrefixTable/ipAddressPrefixTable_data_access)
    /* *INDENT-ON*  */

    /*
     * OID, column number and enum definions for ipAddressPrefixTable 
     */
#include "ipAddressPrefixTable_constants.h"

    /*
     *********************************************************************
     * function declarations
     */
    void            init_ipAddressPrefixTable(void);
    void            shutdown_ipAddressPrefixTable(void);

    /*
     *********************************************************************
     * Table declarations
     */
/**********************************************************************
 **********************************************************************
 ***
 *** Table ipAddressPrefixTable
 ***
 **********************************************************************
 **********************************************************************/
    /*
     * IP-MIB::ipAddressPrefixTable is subid 32 of ip.
     * Its status is Current.
     * OID: .1.3.6.1.2.1.4.32, length: 8
     */
    /*
     *********************************************************************
     * When you register your mib, you get to provide a generic
     * pointer that will be passed back to you for most of the
     * functions calls.
     *
     * TODO:100:r: Review all context structures
     */
    /*
     * TODO:101:o: |-> Review ipAddressPrefixTable registration context.
     */
    typedef netsnmp_data_list ipAddressPrefixTable_registration;

/**********************************************************************/
    /*
     * TODO:110:r: |-> Review ipAddressPrefixTable data context structure.
     * This structure is used to represent the data for ipAddressPrefixTable.
     */
    /*
     * This structure contains storage for all the columns defined in the
     * ipAddressPrefixTable.
     */
    typedef struct ipAddressPrefixTable_data_s {

        /*
         * ipAddressPrefixOrigin(5)/IpAddressPrefixOriginTC/ASN_INTEGER/long(u_long)//l/A/w/E/r/d/h
         */
        u_long          ipAddressPrefixOrigin;

        /*
         * ipAddressPrefixOnLinkFlag(6)/TruthValue/ASN_INTEGER/long(u_long)//l/A/w/E/r/d/h
         */
        u_long          ipAddressPrefixOnLinkFlag;

        /*
         * ipAddressPrefixAutonomousFlag(7)/TruthValue/ASN_INTEGER/long(u_long)//l/A/w/E/r/d/h
         */
        u_long          ipAddressPrefixAutonomousFlag;

        /*
         * ipAddressPrefixAdvPreferredLifetime(8)/UNSIGNED32/ASN_UNSIGNED/u_long(u_long)//l/A/w/e/r/d/h
         */
        u_long          ipAddressPrefixAdvPreferredLifetime;

        /*
         * ipAddressPrefixAdvValidLifetime(9)/UNSIGNED32/ASN_UNSIGNED/u_long(u_long)//l/A/w/e/r/d/h
         */
        u_long          ipAddressPrefixAdvValidLifetime;

    } ipAddressPrefixTable_data;


    /*
     * TODO:120:r: |-> Review ipAddressPrefixTable mib index.
     * This structure is used to represent the index for ipAddressPrefixTable.
     */
    typedef struct ipAddressPrefixTable_mib_index_s {

        /*
         * ipAddressPrefixIfIndex(1)/InterfaceIndex/ASN_INTEGER/long(long)//l/a/w/e/R/d/H
         */
        long            ipAddressPrefixIfIndex;

        /*
         * ipAddressPrefixType(2)/InetAddressType/ASN_INTEGER/long(u_long)//l/a/w/E/r/d/h
         */
        u_long          ipAddressPrefixType;

        /*
         * ipAddressPrefixPrefix(3)/InetAddress/ASN_OCTET_STR/char(char)//L/a/w/e/R/d/h
         */
        /** 128 - 3(other indexes) - oid length(10) = 114 */
        char            ipAddressPrefixPrefix[114];
        size_t          ipAddressPrefixPrefix_len;

        /*
         * ipAddressPrefixLength(4)/InetAddressPrefixLength/ASN_UNSIGNED/u_long(u_long)//l/a/w/e/R/d/H
         */
        u_long          ipAddressPrefixLength;


    } ipAddressPrefixTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review ipAddressPrefixTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
     *
     * BE VERY CAREFUL TO TAKE INTO ACCOUNT THE MAXIMUM
     * POSSIBLE LENGHT FOR EVERY VARIABLE LENGTH INDEX!
     * Guessing 128 - col/entry(2)  - oid len(8)
     */
#define MAX_ipAddressPrefixTable_IDX_LEN     118


    /*
     *********************************************************************
     * TODO:130:o: |-> Review ipAddressPrefixTable Row request (rowreq) context.
     * When your functions are called, you will be passed a
     * ipAddressPrefixTable_rowreq_ctx pointer.
     */
    typedef struct ipAddressPrefixTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
        netsnmp_index   oid_idx;
        oid             oid_tmp[MAX_ipAddressPrefixTable_IDX_LEN];

        ipAddressPrefixTable_mib_index tbl_idx;

        ipAddressPrefixTable_data data;

        /*
         * flags per row. Currently, the first (lower) 8 bits are reserved
         * for the user. See mfd.h for other flags.
         */
        u_int           rowreq_flags;

        /*
         * TODO:131:o: |   |-> Add useful data to ipAddressPrefixTable rowreq context.
         */

        /*
         * storage for future expansion
         */
        netsnmp_data_list *ipAddressPrefixTable_data_list;

    } ipAddressPrefixTable_rowreq_ctx;

    typedef struct ipAddressPrefixTable_ref_rowreq_ctx_s {
        ipAddressPrefixTable_rowreq_ctx *rowreq_ctx;
    } ipAddressPrefixTable_ref_rowreq_ctx;

    /*
     *********************************************************************
     * function prototypes
     */
    int
        ipAddressPrefixTable_pre_request(ipAddressPrefixTable_registration
                                         * user_context);
    int
        ipAddressPrefixTable_post_request(ipAddressPrefixTable_registration
                                          * user_context, int rc);

    int
        ipAddressPrefixTable_rowreq_ctx_init
        (ipAddressPrefixTable_rowreq_ctx * rowreq_ctx,
         void *user_init_ctx);
    void
        ipAddressPrefixTable_rowreq_ctx_cleanup
        (ipAddressPrefixTable_rowreq_ctx * rowreq_ctx);


    ipAddressPrefixTable_rowreq_ctx
        * ipAddressPrefixTable_row_find_by_mib_index
        (ipAddressPrefixTable_mib_index * mib_idx);

    extern const oid      ipAddressPrefixTable_oid[];
    extern const int      ipAddressPrefixTable_oid_size;


#include "ipAddressPrefixTable_interface.h"
#include "ipAddressPrefixTable_data_access.h"
    /*
     *********************************************************************
     * GET function declarations
     */

    /*
     *********************************************************************
     * GET Table declarations
     */
/**********************************************************************
 **********************************************************************
 ***
 *** Table ipAddressPrefixTable
 ***
 **********************************************************************
 **********************************************************************/
    /*
     * IP-MIB::ipAddressPrefixTable is subid 32 of ip.
     * Its status is Current.
     * OID: .1.3.6.1.2.1.4.32, length: 8
     */
    /*
     * indexes
     */

    int
        ipAddressPrefixOrigin_get(ipAddressPrefixTable_rowreq_ctx *
                                  rowreq_ctx,
                                  u_long * ipAddressPrefixOrigin_val_ptr);
    int
        ipAddressPrefixOnLinkFlag_get(ipAddressPrefixTable_rowreq_ctx *
                                      rowreq_ctx,
                                      u_long *
                                      ipAddressPrefixOnLinkFlag_val_ptr);
    int
        ipAddressPrefixAutonomousFlag_get(ipAddressPrefixTable_rowreq_ctx *
                                          rowreq_ctx,
                                          u_long *
                                          ipAddressPrefixAutonomousFlag_val_ptr);
    int
        ipAddressPrefixAdvPreferredLifetime_get
        (ipAddressPrefixTable_rowreq_ctx * rowreq_ctx,
         u_long * ipAddressPrefixAdvPreferredLifetime_val_ptr);
    int
        ipAddressPrefixAdvValidLifetime_get(ipAddressPrefixTable_rowreq_ctx
                                            * rowreq_ctx,
                                            u_long *
                                            ipAddressPrefixAdvValidLifetime_val_ptr);


    int
        ipAddressPrefixTable_indexes_set_tbl_idx
        (ipAddressPrefixTable_mib_index * tbl_idx,
         long ipAddressPrefixIfIndex_val, u_long ipAddressPrefixType_val,
         u_char *ipAddressPrefixPrefix_val_ptr,
         size_t ipAddressPrefixPrefix_val_ptr_len,
         u_long ipAddressPrefixLength_val);
    int
        ipAddressPrefixTable_indexes_set(ipAddressPrefixTable_rowreq_ctx *
                                         rowreq_ctx,
                                         long ipAddressPrefixIfIndex_val,
                                         u_long ipAddressPrefixType_val,
                                         u_char
                                         *ipAddressPrefixPrefix_val_ptr,
                                         size_t
                                         ipAddressPrefixPrefix_val_ptr_len,
                                         u_long ipAddressPrefixLength_val);



    /*
     *********************************************************************
     * SET function declarations
     */

    /*
     *********************************************************************
     * SET Table declarations
     */


    /*
     * DUMMY markers, ignore
     *
     * TODO:099:x: *************************************************************
     * TODO:199:x: *************************************************************
     * TODO:299:x: *************************************************************
     * TODO:399:x: *************************************************************
     * TODO:499:x: *************************************************************
     */

#ifdef __cplusplus
}
#endif
#endif                          /* IPADDRESSPREFIXTABLE_H */
/**  @} */

