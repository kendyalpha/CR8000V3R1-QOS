/**MOD+***********************************************************************/
/* Module:    spml3fun.c                                                      */
/*                                                                           */
/* Purpose:   R8000     L3    functions                        */
/*                                                                           */
/* FiberHome        2003-2012                                                */
/*                                                                           */
/* $Revision: 1.0       $ $Date: 2012/9/2 07:09:09 $             */
/**MOD-***********************************************************************/

#define SHARED_DATA_TYPE SPM_SHARED_LOCAL
#include <nbase.h>
#include <spmincl.h>
#include <nbbi.h>




/********************
hash算法
********************/
#ifdef SHASH   

/**PROC+****************************************************************************************/
/* Name:      spm_hash_algorithm                                        */
/*                                                                              */
/* Purpose:   hash算法，计算hash值                 */
/*                                                                              */
/* Returns:                                                         */
/*                                                                              */
/*                                                                              */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_ULONG spm_hash_algorithm(NBB_CONST NBB_VOID *keyarg, NBB_SHORT keylen, NBB_ULONG initval NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG a = 0;
    NBB_ULONG len;
    NBB_BYTE *key = (NBB_BYTE *) keyarg;

    NBB_TRC_ENTRY("spm_hash_algorithm");
    
    len = (NBB_ULONG) keylen;
    a = initval + len;

    if(keyarg == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT " input  keyarg NULL"));
         goto EXIT_LABEL;
    }

    while (len >= 4)
    {
        a += key[0] + (key[1] << 8) + (key[2] << 16) + (key[3] << 24);
        a += ~(a << 15);
        a ^=  (a >> 10);
        a +=  (a << 3);
        a ^=  (a >> 6);
        a += ~(a << 11);
        a ^=  (a >> 16);
        key += 4; 
        len -= 4;
    }

    switch (len)
    {
        case 3 : a += key[2] << 16;
        case 2 : a ^= key[1] << 8;
        case 1 : a += key[0];
        default: break;
    }
    
    //NBB_TRC_DETAIL((NBB_FORMAT "  hashkey= %d", a));

    EXIT_LABEL:

    NBB_TRC_EXIT();
    return a;
}

/**PROC+****************************************************************************************/
/* Name:      spm_hashtab_create                                        */
/*                                                                              */
/* Purpose:   创建hash表                                */
/*                                                                              */
/* Returns:          chuang                                             */
/*                                                                              */
/*                                                                              */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
SHASH_TABLE *spm_hashtab_create (NBB_ULONG size, NBB_ULONG (*hash_key) (), NBB_ULONG (*hash_cmp) ()  NBB_CCXT_T NBB_CXT)
{
    SHASH_TABLE *hashtab = NULL;
    NBB_ULONG       i;

    NBB_TRC_ENTRY("spm_hashtab_create");
    
    //hashtab = ( SHASH_TABLE * ) malloc (sizeof (SHASH_TABLE));
     hashtab = (SHASH_TABLE *)NBB_MM_ALLOC(sizeof (SHASH_TABLE), NBB_NORETRY_ACT, MEM_SPM_HASH_TABLE_CB);
    if (hashtab == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "hashtab  NULL"));
         goto EXIT_LABEL;
    }
    
    //hashtab->index = (SHASH_NODE *)malloc (sizeof (SHASH_NODE ) * size);
    hashtab->index = (SHASH_NODE **)NBB_MM_ALLOC (sizeof (SHASH_NODE *) * size, NBB_NORETRY_ACT, MEM_SPM_HASH_INDEX_CB);
    if (hashtab->index == NULL) 
    {
        //free(hashtab);
        NBB_MM_FREE(hashtab, 0);
        hashtab = NULL;
        NBB_TRC_FLOW((NBB_FORMAT "hashtab_node  NULL"));
         goto EXIT_LABEL;
    }
    hashtab->size = size;
    hashtab->hash_key = hash_key;
    hashtab->hash_cmp = hash_cmp;
    hashtab->count = 0;
    
    for (i = 0; i < size; i++)   
    {
        hashtab->index[i] = NULL;
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();
    return hashtab;
}

/**PROC+****************************************************************************************/
/* Name:      spm_hashtab_search                                        */
/*                                                                              */
/* Purpose:   查找一个hash节点                  */
/*                                                                              */
/* Returns:                                                         */
/*                                                                              */
/*                                                                              */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_VOID *spm_hashtab_search (SHASH_TABLE *hashtab, NBB_VOID *keyarg NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG       key;
    NBB_ULONG       id;
    SHASH_NODE      *node = NULL;

    NBB_TRC_ENTRY("spm_hashtab_search");

    key = (*hashtab->hash_key) (keyarg NBB_CCXT);
    id = key % (hashtab->size);
    for (node = hashtab->index[id]; node != NULL; node = node->next)
    {
        if (node->key == key && (*hashtab->hash_cmp) (node->data, keyarg NBB_CCXT) == 1)
        {
                    break;
        }
    }
    NBB_TRC_EXIT();
        return node;
}

/**PROC+****************************************************************************************/
/* Name:      spm_hashtab_insert                                         */
/*                                                                              */
/* Purpose:   插入一个hash节点                  */
/*                                                                              */
/* Returns:     返回0 成功，                                */
/*                  返回1已经存在                            */
/*                  返回2申请内存失败                 */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_BYTE spm_hashtab_insert (SHASH_TABLE *hashtab, NBB_VOID *keyarg, NBB_VOID *newdata NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 2;
    NBB_ULONG       key;
    NBB_ULONG       id;
    SHASH_NODE  *node;
    SHASH_NODE  *newnode;

    NBB_TRC_ENTRY("spm_hashtab_insert");
    key = (*hashtab->hash_key) (keyarg NBB_CCXT);
    id = key % (hashtab->size);

    for (node = hashtab->index[id]; node != NULL; node = node->next)
    {
        if (node->key == key && (*hashtab->hash_cmp) (node->data, keyarg NBB_CCXT) == 1)
        {
            NBB_TRC_FLOW((NBB_FORMAT " node exists "));
            rv = 1;
            goto EXIT_LABEL;    
        }
    }
    
    //newnode = malloc (sizeof ( SHASH_NODE));
    newnode = (SHASH_NODE *)NBB_MM_ALLOC (sizeof (SHASH_NODE ) , NBB_NORETRY_ACT, MEM_SPM_HASH_NODE_CB);
    if (newnode == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  newnode NULL"));
        rv = 2;
        goto EXIT_LABEL;
    }
    newnode->data = newdata;
    newnode->key = key;
    newnode->next = hashtab->index[id];
    hashtab->index[id] = newnode;
    hashtab->count++;
    rv = 0;

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/**PROC+****************************************************************************************/
/* Name:      TsHashtabDel                                          */
/*                                                                              */
/* Purpose:   删除一个hash节点                  */
/*                                                                              */
/* Returns:     数据指针,注意内存释放      */
/*                                                              */
/*                                                              */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_VOID *spm_hashtab_del (SHASH_TABLE *hashtab, NBB_VOID *keyarg NBB_CCXT_T NBB_CXT)
{  
    NBB_VOID        *rv = NULL;  
    NBB_ULONG       key;  
    NBB_ULONG       id;  
    SHASH_NODE  *node;  
    SHASH_NODE  *prev;  

    NBB_TRC_ENTRY("spm_hashtab_del");
    key = (*hashtab->hash_key) (keyarg NBB_CCXT);  
    id = key % (hashtab->size);  

    for (node = prev = hashtab->index[id]; node != NULL; node = node->next)    
    {      
        if (node->key == key && (*hashtab->hash_cmp) (node->data, keyarg NBB_CCXT) == 1)        
        {          
            if (node == prev)
            {
                hashtab->index[id] = node->next;  
            }
            else
            {
                prev->next = node->next;
            }

            rv = node->data;  
            
            //free ( node);
             NBB_MM_FREE(node, MEM_SPM_HASH_NODE_CB);
            hashtab->count--;          
            goto EXIT_LABEL;      
        }     
        prev = node;    
    } 
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;
}

#endif

/********************
公共函数
********************/
#ifdef PUBLIC

/**PROC+****************************************************************************************/
/* Name:      spm_l3_checkporttunnl                                  */
/*                                                                              */
/* Purpose:   判断端口是不是TUNNEL口         */
/*                                                                              */
/* Returns:     0是TUNNEL                           */
/*                  1不是TUNNEL                             */
/*                  2未查到端口号                       */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_VOID spm_l3_init( NBB_CXT_T NBB_CXT)
{
    NBB_USHORT      vpnid;

    for (vpnid = 0 ; vpnid < SPM_L3_VRF_MAX; vpnid++)      /* 创建所有的vrf,并且不删除 */
    {
        #ifdef USE_DEV
            ApiC3CreateL3Vpn( 0 , vpnid);       /* 注意区分芯片单盘 */
        #else
        
        #endif
    }

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_ftnprocess
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_checkporttunnl( NBB_ULONG  portindex ,LSPPROT_KEY  *tunnelkey NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 2;

    NBB_TRC_ENTRY("spm_l3_checkporttunnl");
    
    /*由L2代码提供接口查询端口属性*/
    rv = spm_get_lsp_prot_key_from_port_inedx( portindex ,tunnelkey NBB_CCXT);
    if (rv == 1)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  NOT TUNNEL  port"));
        goto EXIT_LABEL;
    }
    else if (rv == 0)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "TUNNEL  ingress =%x  egress=%x  tunnelid=%x",
                                tunnelkey->ingress ,tunnelkey->egress, tunnelkey->tunnelid));
        goto EXIT_LABEL;
    }
    else if (rv == 2)
    {
        NBB_TRC_FLOW((NBB_FORMAT "   NOT FOUND"));
        goto EXIT_LABEL;
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_getipv6index( NBB_ULONG  portindex ,NBB_ULONG  *ipv6 ,NBB_ULONG  *ipv6index NBB_CCXT_T NBB_CXT)
{
    NBB_INT                     rv = 2;
    NBB_ULONG                   index;
    ATG_DCI_IPV6_CACHE_KEY      ipv6arp;

    NBB_TRC_ENTRY("spm_l3_getipv6index");

    OS_MEMSET(&ipv6arp, 0, sizeof(ATG_DCI_IPV6_CACHE_KEY));
    ipv6arp.index = portindex;
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &(ipv6arp.ipv6[0]), ipv6, 16 );
    
    /*由L2代码提供接口查询*/
    rv = spm_get_idx_from_ncache_key(&ipv6arp, &index NBB_CCXT);
    if (rv == 1)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  NOT ipv6 arp "));
        goto EXIT_LABEL;
    }
    else if (rv == 0)
    {
        *ipv6index = index;
        NBB_TRC_DETAIL((NBB_FORMAT "getipv6index  ipv6index=%x", *ipv6index));
        goto EXIT_LABEL;
    }
    else if (rv == 2)
    {
        NBB_TRC_FLOW((NBB_FORMAT "   NOT FOUND"));
        goto EXIT_LABEL;
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/**PROC+****************************************************************************************/
/* Name:      spm_l3_prefixmatch                                   */
/*                                                                              */
/* Purpose:   判断IP是否与这个前缀匹配 */
/*                                                                              */
/* Returns:         0匹配                           */
/*                         1不匹配                          */
/*                                                              */
/* Params:                                                              */
/*                                                                              */
/* Operation:                                                   */
/*                                                                              */
/**PROC-****************************************************************************************/
NBB_CONST NBB_BYTE spmmaskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,0xf8, 0xfc, 0xfe, 0xff};


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_prefixmatch4 (NBB_ULONG prefixip ,NBB_BYTE prefixlen, NBB_ULONG ip NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_ULONG   offset;
    NBB_ULONG   shift;
    NBB_BYTE    aip[4];
    NBB_BYTE    bip[4];

    NBB_TRC_ENTRY("spm_l3_prefixmatch4");

    OS_MEMSET( &aip[0], 0, 4 );
    OS_MEMSET( &bip[0], 0, 4 );
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &aip[0], &prefixip, 4 );
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &bip[0], &ip, 4 );
    NBB_BYTE *preip = aip;
    NBB_BYTE *dip = bip;
    
    offset = prefixlen /8;
    shift =  prefixlen % 8;

    if ( shift )
    {
        if ( spmmaskbit[shift] & (preip[offset] ^ dip[offset]))
        {
            NBB_TRC_FLOW((NBB_FORMAT "  not match"));
            goto EXIT_LABEL;    
        }
        
    }
    while (offset--)
    {
        if ( preip[offset] != dip[offset] )
        {
            NBB_TRC_FLOW((NBB_FORMAT "   not match"));
            goto EXIT_LABEL;
        }
        
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

NBB_INT spm_l3_prefixmatch6 (NBB_BYTE *prefixip ,NBB_BYTE prefixlen, NBB_BYTE *ip NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_ULONG   offset;
    NBB_ULONG   shift;

    NBB_TRC_ENTRY("spm_l3_prefixmatch6");

    
    //NBB_BYTE *preip = *prefixip;
    //NBB_BYTE *dip = *ip;
    NBB_BYTE *preip = NULL;
    NBB_BYTE *dip = NULL;

    preip = prefixip;
    dip = ip;
    
    
    offset = prefixlen /8;
    shift =  prefixlen % 8;

    if ( shift )
    {
        if ( spmmaskbit[shift] & (preip[offset] ^ dip[offset]))
        {
            NBB_TRC_FLOW((NBB_FORMAT "  not match"));
            goto EXIT_LABEL;    
        }
        
    }
    while (offset--)
    {
        if ( preip[offset] != dip[offset] )
        {
            NBB_TRC_FLOW((NBB_FORMAT "   not match"));
            goto EXIT_LABEL;
        }
        
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_checkipdirectconnect( NBB_BYTE addrtype,NBB_BYTE *ip NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    NBB_BYTE    mask;
    NBB_BYTE    dip[16];
    NBB_BYTE    *mip6 = NULL;
    NBB_ULONG   ip4 = 0;
    NBB_ULONG   mip4 = 0;

    VRFUROUTE   *route = NULL;

    NBB_TRC_ENTRY("spm_l3_checkipdirectconnect");
    
    //rv = 0;
    //goto EXIT_LABEL;  

    if (ip == NULL)
    {
        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e checkip ip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if( addrtype == 0 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &mip4, ip, 4 );
        if ( mip4 == 0 )
        {
            NBB_TRC_FLOW((NBB_FORMAT " IP 0 direct connect "));
            rv = 0;
            goto EXIT_LABEL;    
        }
    }
    else 
    {
        mip6 = ip;
        #if 0
        //rv = NBB_MEMCMP( mip6, 0, 16 );
        if ( rv == 0 )
        {
            NBB_TRC_FLOW((NBB_FORMAT " IP 0 direct connect "));
            //rv = 0;
            goto EXIT_LABEL;    
        }
        #endif
    }
    route =  (VRFUROUTE *)AVLL_FIRST(SHARED.dcroute_tree);
    while (route != NULL)
    {
        if( addrtype == 0 )
        {
            //coverity[bad_sizeof]
            NBB_MEMCPY( &ip4, &(route->key.dip[3]), 4 );
            mask = route->key.mask;
            NBB_TRC_DETAIL((NBB_FORMAT " match ip4    =%x ", ip4));
            NBB_TRC_DETAIL((NBB_FORMAT " match mask =%d ", mask));
            NBB_TRC_DETAIL((NBB_FORMAT " match mip4  =%x ", mip4));
            rv = spm_l3_prefixmatch4( ip4, mask, mip4 NBB_CCXT);
            if (rv == 0)
            {
                NBB_TRC_FLOW((NBB_FORMAT " IPV4 direct connect "));
                goto EXIT_LABEL;    
            }
        }
        else 
        {
            OS_MEMSET( &dip[0], 0, 16 );
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( &dip[0], &(route->key.dip[0]), 16 );
            mask = route->key.mask;
            rv = spm_l3_prefixmatch6( &dip[0], mask, mip6 NBB_CCXT);
            if (rv == 0)
            {
                NBB_TRC_FLOW((NBB_FORMAT " IPV6 direct connect "));
                goto EXIT_LABEL;    
            }
        }
        route = (VRFUROUTE *)AVLL_NEXT( SHARED.dcroute_tree, route->route_node);
    }
    rv = 1;
    NBB_TRC_FLOW((NBB_FORMAT "  not direct connect "));

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_checkip( NBB_USHORT vrfid, NBB_BYTE addrtype,NBB_BYTE *ip NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    NBB_BYTE    mask;
    NBB_BYTE    dip[16];
    NBB_BYTE    *mip6 = NULL;
    NBB_ULONG   ip4 = 0;
    NBB_ULONG   mip4 = 0;

    VRFUROUTE   *route = NULL;

    NBB_TRC_ENTRY("spm_l3_checkip");
    
    //rv = 0;
    //goto EXIT_LABEL;  

    if (ip == NULL)
    {
        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e checkip ip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if( addrtype == 0 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &mip4, ip, 4 );
        if ( mip4 == 0 )
        {
            NBB_TRC_FLOW((NBB_FORMAT " IP 0 direct connect "));
            rv = 0;
            goto EXIT_LABEL;    
        }
    }
    else 
    {
        mip6 = ip;
        #if 0
        //rv = NBB_MEMCMP( mip6, 0, 16 );
        if ( rv == 0 )
        {
            NBB_TRC_FLOW((NBB_FORMAT " IP 0 direct connect "));
            //rv = 0;
            goto EXIT_LABEL;    
        }
        #endif
    }
    route =  (VRFUROUTE *)AVLL_FIRST(SHARED.dcroute_tree);
    while (route != NULL)
    {
        if(( vrfid == route->key.vrfid) && ( route->key.mask != 32 ))
        {
            if( addrtype == 0 )
            {
                //coverity[bad_sizeof]
                NBB_MEMCPY( &ip4, &(route->key.dip[3]), 4 );
                mask = route->key.mask;
                NBB_TRC_DETAIL((NBB_FORMAT " match ip4    =%x ", ip4));
                NBB_TRC_DETAIL((NBB_FORMAT " match mask =%d ", mask));
                NBB_TRC_DETAIL((NBB_FORMAT " match mip4  =%x ", mip4));
                rv = spm_l3_prefixmatch4( ip4, mask, mip4 NBB_CCXT);
                if (rv == 0)
                {
                    NBB_TRC_FLOW((NBB_FORMAT " IPV4 direct connect "));
                    goto EXIT_LABEL;    
                }
            }
            else 
            {
                OS_MEMSET( &dip[0], 0, 16 );
                
                //coverity[bad_sizeof]
                NBB_MEMCPY( &dip[0], &(route->key.dip[0]), 16 );
                mask = route->key.mask;
                rv = spm_l3_prefixmatch6( &dip[0], mask, mip6 NBB_CCXT);
                if (rv == 0)
                {
                    NBB_TRC_FLOW((NBB_FORMAT " IPV6 direct connect "));
                    goto EXIT_LABEL;    
                }
            }
        }
        route = (VRFUROUTE *)AVLL_NEXT( SHARED.dcroute_tree, route->route_node);
    }
    rv = 1;
    NBB_TRC_FLOW((NBB_FORMAT "  not direct connect "));

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}



#endif


#ifdef ITERATION   

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_lookuptunnel(  LSPPROT_KEY *lspprotkey,NBB_ULONG  *tunnelindex NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_lookuptunnel");

    if (lspprotkey == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuptunnel lspprotkey = NULL \n", __LINE__);
        rv = ADD_TUNNELSEL_ITER_TUNNEL_PARA_ERR;
        goto  EXIT_LABEL;
    }
    if (tunnelindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuptunnel tunnelindex = NULL \n", __LINE__);
        rv = ADD_TUNNELSEL_ITER_TUNNEL_PARA_ERR;
        goto  EXIT_LABEL;
    }

    OS_MEMSET( &tunlkey, 0, sizeof( LSPPROT_KEY ) );
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &tunlkey, lspprotkey, sizeof( LSPPROT_KEY ) );
    
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.lspprothash, &tunlkey NBB_CCXT);
    if(( node == NULL ) || ( node->data == NULL ))
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e node  = NULL \n", __LINE__);
        rv = ADD_TUNNELSEL_ITER_TUNNEL_NULL;
        goto  EXIT_LABEL;
    }
    else
    {
        lspprot = (LSPPROT *)(node->data);
        if( lspprot->index != 0 )
        {
            *tunnelindex = lspprot->index;
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e tunnel data \n", __LINE__);
            rv = ADD_TUNNELSEL_ITER_TUNNEL_DATA_ERR;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_lookupftn( NBB_ULONG peerip ,NBB_ULONG  *nextindex  NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_lookupftn");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookupftn peerip = 0 \n", __LINE__);
        rv = ADD_TUNNELSEL_ITER_FTN_PARA_ERR;
        goto  EXIT_LABEL;
    }

    if (nextindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookupftn nextindex = NULL \n", __LINE__);
        rv = ADD_TUNNELSEL_ITER_FTN_PARA_ERR;
        goto  EXIT_LABEL;
    }

    ftnkey.vrfid = 0;
    ftnkey.fec = peerip;
    ftnkey.mask = 32;

    uftn =  (FTN_TAB *)AVLL_FIND( SHARED.ftn_tree ,  &ftnkey ); 
    if( uftn == NULL )
    {
        rv = ADD_TUNNELSEL_ITER_FTN_NULL;
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d uftn = NULL \n", __LINE__);
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->tunnelid != 0 )
        {
            *nextindex = uftn->tunnelid;
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d uftn = NULL \n", __LINE__);
            rv = ADD_TUNNELSEL_ITER_FTN_DATA_ERR;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_lookup( VRFUROUTE_KEY *routekey ,NBB_ULONG nextport,
                                                NBB_ULONG  nextip,NBB_ULONG  *nexthop  NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_lookup");

    ftnkey.vrfid = routekey->vrfid;
    ftnkey.mask = routekey->mask;
    ftnkey.res = 0;
    ftnkey.fec = 0;

    if( routekey->addrtype == 0 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(ftnkey.fec), &(routekey->dip[3]), 4 );
        NBB_TRC_FLOW((NBB_FORMAT "IPV4 "));
    }
    else if( routekey->addrtype == 1 )
    {
        //暂时这样
        //NBB_MEMCPY( &(ftnkey.fec), &(routekey->dip[0]), 16 );
        NBB_TRC_FLOW((NBB_FORMAT "IPV6 "));
    }
    uftn =  (FTN_TAB *)AVLL_FIND( SHARED.ftn_tree ,  &ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ftn  NULL"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->index[0] != 0 )
        {
            //*nexthop = uftn->index;
            NBB_TRC_FLOW((NBB_FORMAT "lookup ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT "lookup error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_ftnlsp( NBB_ULONG nextip , DC_LSP *udc_lsp NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_ftnlsp");

    if (nextip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ftnlsp nextip = 0 \n", __LINE__);
        rv = ADD_FTN_ITER_LSP_PARA_ERR;
        goto  EXIT_LABEL;
    }

    if (udc_lsp == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ftnlsp udc_lsp = NULL \n", __LINE__);
        rv = ADD_FTN_ITER_LSP_PARA_ERR;
        goto  EXIT_LABEL;
    }

    ftnkey.vrfid = 0;
    ftnkey.res = 0;
    ftnkey.fec = nextip;
    ftnkey.mask = 32;

    uftn =  (FTN_TAB *)AVLL_FIND( SHARED.ftn_tree ,&ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
        rv = ADD_FTN_ITER_LSP_DATA_NULL;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->dclsp[0].label2 == 0xffffffff )/*取TUNNEL的主LSP结果*/
        {
            if(( uftn->dclsp[0].nextip != 0 )
                && ( uftn->dclsp[0].nextport != 0 )
                && ( uftn->dclsp[0].label1 != 0 ))
            {
                udc_lsp->label1 = uftn->dclsp[0].label1;
                udc_lsp->nextip = uftn->dclsp[0].nextip;
                udc_lsp->nextport = uftn->dclsp[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " FTN iteration OK"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " FTN iteration  error"));
                rv = ADD_FTN_ITER_LSP_NXHOP_DATA_ERR;
                goto  EXIT_LABEL;
            }
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " FTN  iteration  label more error"));
            rv = ADD_FTN_ITER_LSP_NXHOP_LABEL_ERR;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_vrfuroute( NBB_BYTE addrtype, NBB_BYTE  *ip ,DC_UNI *udc_uni NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    VRFUROUTE_KEY       vrfukey;

    VRFUROUTE           *uroute = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_vrfuroute");

    if (ip == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e vrfuroute ip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if (udc_uni == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e vrfuroute udc_uni = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    OS_MEMSET(&vrfukey, 0, sizeof(VRFUROUTE_KEY));
    
    vrfukey.vrfid = 0;
    vrfukey.addrtype = addrtype;

    if( addrtype == 0 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(vrfukey.dip[3]), ip, 4 );
        vrfukey.mask = 32;
    }
    else if( addrtype == 1 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(vrfukey.dip[0]), ip, 16 );
        vrfukey.mask = 128;
    }
    uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.vrfuroute_tree ,  &vrfukey ); 
    if( uroute == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " UC  route iteration error"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        //暂时这样
        if( addrtype == 0 )
        {
            if(( uroute->dchop[0].ntip[3] != 0 ) && ( uroute->dchop[0].nextport != 0 ))
            {
                //coverity[bad_sizeof]
                NBB_MEMCPY( &(udc_uni->ntip[0]), &(uroute->dchop[0].ntip[0]), 16 );
                udc_uni->nextport = uroute->dchop[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " UC  route iteration ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " UC  route iteration  dchop error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
        else  if( addrtype == 1 )
        {
            if((( uroute->dchop[0].ntip[0] != 0 ) ||
                ( uroute->dchop[0].ntip[1] != 0 ) ||
                ( uroute->dchop[0].ntip[2] != 0 ) ||
                ( uroute->dchop[0].ntip[3] != 0 )) && ( uroute->dchop[0].nextport != 0 ))
            {
                //coverity[bad_sizeof]
                NBB_MEMCPY( &(udc_uni->ntip[0]), &(uroute->dchop[0].ntip[0]), 16 );
                udc_uni->nextport = uroute->dchop[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " UC  route iteration ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " UC  route iteration  dchop error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_iteration_tunnel( NBB_ULONG  portindex, DC_LSP *udc_lsp NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    SHASH_NODE  *node = NULL;

    LSPPROT_KEY     tunnelkey;
    
    LSPPROT         *lspprot = NULL;

    NBB_TRC_ENTRY("spm_l3_iteration_tunnel");

    if (portindex == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e tunnel portindex = 0 \n", __LINE__);
        rv = ADD_FTN_ITER_TUNNEL_PARA_ERR;
        goto  EXIT_LABEL;
    }

    if (udc_lsp == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e tunnel udc_lsp = NULL \n", __LINE__);
        rv = ADD_FTN_ITER_TUNNEL_PARA_ERR;
        goto  EXIT_LABEL;
    }

    rv =    spm_l3_checkporttunnl( portindex, &tunnelkey NBB_CCXT);
    if( rv != 0 )
    {
        NBB_TRC_FLOW((NBB_FORMAT " port tunnel  data error"));
        goto  EXIT_LABEL;
    }
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.lspprothash ,&tunnelkey NBB_CCXT);
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT " TUNNEL iteration  not find"));
        rv = ADD_FTN_ITER_TUNNEL_DATA_NULL;
        goto  EXIT_LABEL;
    }
    else
    {
        lspprot = (LSPPROT *)(node->data);
        if( lspprot->dclsp[0].label2 == 0xffffffff )/*取TUNNEL的主LSP结果*/
        {
            if(( lspprot->dclsp[0].nextip != 0 )
                && ( lspprot->dclsp[0].nextport != 0 ))
            {
                udc_lsp->label1 = lspprot->dclsp[0].label1;
                udc_lsp->nextip = lspprot->dclsp[0].nextip;
                udc_lsp->nextport = lspprot->dclsp[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL  iteration ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL iteration  ok, but data error"));
                rv = ADD_FTN_ITER_TUNNEL_NXHOP_DATA_ERR;
                goto  EXIT_LABEL;
            }
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL  iteration  label more error"));
            rv = ADD_FTN_ITER_TUNNEL_NXHOP_LABEL_ERR;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

#endif

#if 1


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_getftnid( NBB_ULONG peerip ,NBB_ULONG  *nextindex  NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 100;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_getftnid");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (nextindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid nextindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    ftnkey.vrfid = 0;
    ftnkey.fec = peerip;
    ftnkey.mask = 32;

    uftn =  (FTN_TAB *)AVLL_FIND( SHARED.ftn_tree, &ftnkey ); 
    if( uftn == NULL )
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peer =%x \n", __LINE__, peerip);
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->tunnelid != 0 )
        {
            *nextindex = uftn->tunnelid;
            uftn->l3num = uftn->l3num + 1;
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o gftnid peer =%x \n", __LINE__, peerip);
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peer =%x \n", __LINE__, peerip);
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_delftnid( NBB_ULONG peerip ,NBB_ULONG  *nextindex  NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 100;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_delftnid");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (nextindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid nextindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    ftnkey.vrfid = 0;
    ftnkey.fec = peerip;
    ftnkey.mask = 32;

    uftn =  (FTN_TAB *)AVLL_FIND( SHARED.ftn_tree ,  &ftnkey ); 
    if( uftn == NULL )
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peer =%x \n", __LINE__, peerip);
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->tunnelid != 0 )
        {
            *nextindex = uftn->tunnelid;
            uftn->l3num = uftn->l3num + 1;
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o gftnid peer =%x \n", __LINE__, peerip);
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e gftnid peer =%x \n", __LINE__, peerip);
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


#endif

/********************
OAM接口
********************/
#ifdef FOR_OAM   

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_routeforoam( NBB_USHORT vrfid, NBB_BYTE addrtype, 
                                    NBB_ULONG *peerip ,NBB_ULONG  *portindex NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    NBB_BYTE    i = 0;

    VRFUROUTE_KEY       vrfukey;

    NBB_ULONG           ipv4 = 0,ipv6[4];

    VRFUROUTE           *uroute = NULL;

    NBB_TRC_ENTRY("spm_l3_routeforoam");

    if (peerip == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e routeforoam peerip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e routeforoam portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }


    OS_MEMSET(&vrfukey, 0, sizeof(VRFUROUTE_KEY));
    vrfukey.vrfid = vrfid;
    vrfukey.addrtype = addrtype;
    if( addrtype == 0 )
    {
        for ( i = 32; i > 0; i--)
        {
            //coverity[bad_sizeof]
            NBB_MEMCPY( &ipv4, peerip, 4 );
            vrfukey.mask = i;
            ipv4 = (ipv4 >>(32 - i));
            vrfukey.dip[3] = (ipv4 << (32 - i));
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
        }
    }
    else if( addrtype == 1 )
    {
        for ( i = 128; i > 0; i--)
        {
            OS_MEMSET( &ipv6[0], 0, 16 );
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( &(vrfukey.dip[0]), peerip, 16 );
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( &ipv6[0], peerip, 16 );
            vrfukey.mask = i;
            if( i > 95 )
            {
                ipv6[3] = (ipv6[3] >>(128 - i));
                vrfukey.dip[3] = (ipv6[3] << (128 - i));
            }
            else if( i > 63 )
            {
                ipv6[2] = (ipv6[2] >>(96 - i));
                vrfukey.dip[2] = (ipv6[2] << (96 - i));
                vrfukey.dip[3] = 0;
            }
            else if( i > 31 )
            {
                ipv6[1] = (ipv6[1] >>(64 - i));
                vrfukey.dip[1] = (ipv6[1] << (64 - i));
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            else if( i > 0 )
            {
                ipv6[0] = (ipv6[0] >>(32 - i));
                vrfukey.dip[0] = (ipv6[0] << (32 - i));
                vrfukey.dip[1] = 0;
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( SHARED.vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                break;
            }
        }
    }
    if( uroute == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam null"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uroute->dchop[0].nextport != 0 )
        {
            *portindex = uroute->dchop[0].nextport;
            NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " UCroute dchop foroam error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_rsvpforoam(  CRTXLSP_KEY *key, NBB_ULONG  *portindex, NBB_ULONG  *index NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    CRTXLSP_KEY     txlspkey;

    CRTXLSP         *txlsp = NULL;

    NBB_TRC_ENTRY("spm_l3_rsvpforoam");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpforoam key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpforoam portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (index == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpforoam index = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if( key->lspid == 0 )
    {
        OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &tunlkey, key, sizeof( LSPPROT_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, &tunlkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            lspprot = (LSPPROT *)(node->data);
            if( lspprot->index != 0 )
            {
                *index = lspprot->index;
                *portindex = lspprot->dclsp[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    else
    {
        OS_MEMSET(&txlspkey, 0, sizeof(CRTXLSP_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &txlspkey, key, sizeof( CRTXLSP_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->crlsptxhash, &txlspkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            txlsp = (CRTXLSP *)(node->data);
            if( txlsp->dclsp.index != 0 )
            {
                *index = txlsp->dclsp.ftnidx;
                *portindex = txlsp->dclsp.nextport;
                NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_rxlabelforoam(  CRRXLSP_KEY *key, NBB_ULONG  *label NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    CRRXLSP_KEY     rxlspkey;

    CRRXLSP         *rxlsp = NULL;

    NBB_TRC_ENTRY("spm_l3_rxlabelforoam");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rxlabelforoam key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (label == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rxlabelforoam label = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&rxlspkey, 0, sizeof(CRRXLSP_KEY));
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &rxlspkey, key, sizeof( CRRXLSP_KEY ) );
    
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.crlsprxhash, &rxlspkey NBB_CCXT);
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT " rxlsp lookup  not find"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        rxlsp = (CRRXLSP *)(node->data);
        if( rxlsp->posid != 0 )
        {
            *label = rxlsp->base.inlabel;
            NBB_TRC_DETAIL((NBB_FORMAT " rxlsp label=%d ", *label));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " rxlsp lookup  label error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_txlabelforoam(  CRTXLSP_KEY *key, NBB_ULONG  *label NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    CRTXLSP_KEY     txlspkey;

    CRTXLSP         *txlsp = NULL;

    NBB_TRC_ENTRY("spm_l3_txlabelforoam");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e txlabelforoam key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (label == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e txlabelforoam label = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&txlspkey, 0, sizeof(CRTXLSP_KEY));
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &txlspkey, key, sizeof( CRTXLSP_KEY ) );
    
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.crlsptxhash, &txlspkey NBB_CCXT);
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  not find"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        txlsp = (CRTXLSP *)(node->data);
        if( txlsp->dclsp.index != 0 )
        {
            *label = txlsp->dclsp.label1;
            NBB_TRC_DETAIL((NBB_FORMAT " txlsp label=%d ", *label));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  label error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_ldpforoam(  FTN_KEY *key, NBB_ULONG  *portindex, NBB_ULONG  *index NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_ldpforoam");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpforoam key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpforoam portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if (index == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpforoam index = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    OS_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &ftnkey, key, sizeof( FTN_KEY ) );
    uftn =  (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,  &ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->tunnelid != 0 )
        {
            *index = uftn->tunnelid;
            *portindex = uftn->dclsp[0].nextport;
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_oamfordcip( NBB_ULONG portindex ,NBB_ULONG  *nextip NBB_CCXT_T NBB_CXT)
{
    NBB_INT             rv = 1;

    VRFUROUTE       *uroute = NULL;

    NBB_TRC_ENTRY("spm_l3_oamfordcip");

    if (portindex == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e oamfordcip portindex = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (nextip == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e oamfordcip nextip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->dcroute_tree);
    while (uroute != NULL)
    {
        if( uroute->nexthop[0].nextport == portindex )
        {
            NBB_TRC_FLOW((NBB_FORMAT " UCroute ok"));
            break;
        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->dcroute_tree, uroute->route_node);
    }
    if( uroute == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " UCroute fordcip null"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uroute->dchop[0].nextport != 0 )
        {
            *nextip = uroute->key.dip[3];
            NBB_TRC_FLOW((NBB_FORMAT " UCroute fordcip ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " UCroute dchop fordcip error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_peerforport(  NBB_USHORT vrfid, NBB_ULONG  peerip, NBB_ULONG  *index NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    NBB_ULONG   portindex = 0;
    NBB_ULONG   lspindex = 0;

    FTN_KEY         ftnkey;
    
    CRTXLSP_KEY     txlspkey;

    TUNNELCH_KEY    tunskey;

    SHASH_NODE      *node = NULL;

    TUNNELCH        *tunnels = NULL;

    NBB_TRC_ENTRY("spm_l3_peerforport");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e peerforport peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (index == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e peerforport index = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }


    OS_MEMSET(&tunskey, 0, sizeof(TUNNELCH_KEY));

    tunskey.res = 0;
    tunskey.vrfid = vrfid;
    tunskey.peerip = peerip;
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.tunnelchhash, &tunskey NBB_CCXT); 
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT "tunnels Error: node null"));
        rv = 2;
        goto  EXIT_LABEL;
    }
    else
    {
        tunnels = (TUNNELCH *)(node->data);
    }
    if( tunnels->base.ingress == 0)
    {
        OS_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
        ftnkey.vrfid = 0;
        ftnkey.res = 0;
        ftnkey.mask = 32;
        ftnkey.fec = peerip;
        rv = spm_l3_ldpforoam( &ftnkey,&portindex, &lspindex NBB_CCXT);
        if (rv == 0)
        {
            *index = portindex;
            rv = 0;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT "NOT ldp  outport"));
            rv = 3;
            goto EXIT_LABEL;
        }
    }
    else
    {
        OS_MEMSET(&txlspkey, 0, sizeof(CRTXLSP_KEY));
        txlspkey.ingress = tunnels->base.ingress;
        txlspkey.egress = tunnels->base.egress;
        txlspkey.tunnelid = tunnels->base.tunnelid;
        txlspkey.tunnelid = 0;
        rv = spm_l3_rsvpforoam( &txlspkey, &portindex, &lspindex NBB_CCXT);
        if (rv == 0)
        {
            *index = portindex;
            rv = 0;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT "NOT tunnel outport"));
            rv = 4;
            goto EXIT_LABEL;
        }
    }
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_peerforhqos(  NBB_USHORT vrfid, NBB_ULONG  peerip ,
                                                        CRTXLSP_KEY *key,SPM_PORT_INFO_CB  *portfo NBB_CCXT_T NBB_CXT)
{
    NBB_INT             rv = 1;

    FTN_KEY             ftnkey;

    TUNNELCH_KEY    tunskey;

    LSPPROT_KEY      lsppkey;

    CRTXLSP_KEY      txlspkey;

    CRTXLSP            *txlsp = NULL;

    LSPPROT            *lspprot = NULL;

    SHASH_NODE      *node = NULL;

    TUNNELCH          *tunnels = NULL;


    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_peerforhqos peerip = 0 \n", __LINE__);
        rv = ADD_UCROUTE_PEER_FOR_QOS_PARA_ERR;
        goto  EXIT_LABEL;
    }
    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_peerforhqos key = 0 \n", __LINE__);
        rv = ADD_UCROUTE_PEER_FOR_QOS_PARA_ERR;
        goto  EXIT_LABEL;
    }
    if (portfo == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_peerforhqos portfo = 0 \n", __LINE__);
        rv = ADD_UCROUTE_PEER_FOR_QOS_PARA_ERR;
        goto  EXIT_LABEL;
    }
    
    OS_MEMSET(&tunskey, 0, sizeof(TUNNELCH_KEY));

    tunskey.res = 0;
    tunskey.vrfid = vrfid;
    tunskey.peerip = peerip;
    node =  (SHASH_NODE *)spm_hashtab_search( SHARED.tunnelchhash, &tunskey NBB_CCXT); 
    if(( node == NULL ) || (node->data == NULL))
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuptunnel data= NULL \n", __LINE__);
        rv = ADD_UCROUTE_PEER_FOR_QOS_TUNNEL_NULL;
        goto  EXIT_LABEL;
    }
    else
    {
        tunnels = (TUNNELCH *)(node->data);
    }
    if( tunnels->base.ingress == 0)
    {
        OS_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
        ftnkey.vrfid = 0;
        ftnkey.res = 0;
        ftnkey.mask = 32;
        ftnkey.fec = peerip;
    }
    else
    {
        OS_MEMSET(&lsppkey, 0, sizeof(LSPPROT_KEY));
        lsppkey.ingress = tunnels->base.ingress;
        lsppkey.egress = tunnels->base.egress;
        lsppkey.tunnelid = tunnels->base.tunnelid;

        node =  (SHASH_NODE *)spm_hashtab_search( SHARED.lspprothash, &lsppkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuplspprt data= NULL \n", __LINE__);
            rv = ADD_UCROUTE_PEER_FOR_QOS_LSPP_NULL;
            goto  EXIT_LABEL;
        }
        else
        {
          lspprot = (LSPPROT *)(node->data);
          if(lspprot->index != 0)
          {
            txlspkey.ingress = lsppkey.ingress;
            txlspkey.egress = lsppkey.egress;
            txlspkey.tunnelid = lsppkey.tunnelid;
            txlspkey.lspid = lspprot->lspb.lsp[0].lspid;
            OS_MEMSET(key, 0, sizeof(CRTXLSP_KEY));
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( key, &txlspkey, sizeof( CRTXLSP_KEY ) );
            node =  (SHASH_NODE *)spm_hashtab_search( SHARED.crlsptxhash, &txlspkey NBB_CCXT);
            if(( node == NULL ) || (node->data == NULL))
            {
                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuptxlsp data= NULL \n", __LINE__);
                rv = ADD_UCROUTE_PEER_FOR_QOS_TXLSP_NULL;
                goto  EXIT_LABEL;
            }
            else
            {
                txlsp = (CRTXLSP *)(node->data);
                if( txlsp->dclsp.index != 0 )
                {
                    OS_MEMSET(portfo, 0, sizeof(SPM_PORT_INFO_CB));
                    rv = spm_get_portid_from_logical_port_index(txlsp->dclsp.nextport,portfo NBB_CCXT);
                    if(rv != 0)
                    {
                        rv = ADD_UCROUTE_PEER_FOR_QOS_L2PORT_DATA_ERR;
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, 
                          "spm_get_portid_from_logical_port_index err,Line=%d,rv =%d\n",__LINE__,rv);
                    }
                    rv = 0;
                    goto  EXIT_LABEL;
                }
                else
                {
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookuptxlsp data= NULL \n", __LINE__);
                    rv = ADD_UCROUTE_PEER_FOR_QOS_TXLSP_DATA_ERR;
                    goto  EXIT_LABEL;
                }
           }
          }
          else
          {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lsppr not exit or not lsp1:1 type= NULL \n", __LINE__);
            rv = ADD_UCROUTE_PEER_FOR_QOS_LSPP_DATA_ERR;
            goto  EXIT_LABEL;
          }
        }
    }
    EXIT_LABEL:
    
    
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_lookupuni( NBB_BYTE flag,NBB_ULONG *ip, NBB_ULONG port,NBB_ULONG *nhi NBB_CCXT_T NBB_CXT)
{
    NBB_INT             rv = 100;
    
    NBB_INT             rv2 = 100;
    
    L3_UNI_T            outuni;

    UNI_KEY             unikey;

    UNIPORT             *uniports = NULL;

    NBB_TRC_ENTRY("spm_l3_lookupuni");

    if (port == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookupuni port = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (ip == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookupuni ip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if (nhi == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lookupuni nhi = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&unikey , 0, sizeof(UNI_KEY));
    if( flag == 0 )
    {
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(unikey.nextip[3]), ip, 4 );
        NBB_TRC_DETAIL((NBB_FORMAT "uni port = %d ", port));
        NBB_TRC_DETAIL((NBB_FORMAT "uni nextip = %x ", unikey.nextip[3]));
    }
    else if( flag == 1 )
    {
        NBB_MEMCPY( &(unikey.nextip[0]), ip, 16 );
    }
    unikey.nextport = port; 
    uniports =  (UNIPORT *)AVLL_FIND( v_spm_shared->outuni ,&unikey );
    if ( uniports == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT "port null"));
        rv = 101;
        goto  EXIT_LABEL;
    }
    else
    {
        *nhi = uniports->nhid;
        rv = 0;
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;
    
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_rsvpallport(CRTXLSP_KEY *key, NBB_ULONG *portindex, NBB_BYTE *portnum NBB_CCXT_T NBB_CXT) 
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    CRTXLSP_KEY     txlspkey;

    CRTXLSP         *txlsp = NULL;

    NBB_TRC_ENTRY("spm_l3_rsvp_allports");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpallport key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpallport portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portnum == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e rsvpallport portnum = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if( key->lspid == 0 )
    {
        OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &tunlkey, key, sizeof( LSPPROT_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, &tunlkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            lspprot = (LSPPROT *)(node->data);
            if( lspprot->index != 0 )
            {
                portindex[0] = lspprot->dclsp[0].nextport;
                portindex[1] = lspprot->dclsp[1].nextport;
                *portnum = lspprot->lspnum;
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    else
    {
        OS_MEMSET(&txlspkey, 0, sizeof(CRTXLSP_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &txlspkey, key, sizeof( CRTXLSP_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->crlsptxhash, &txlspkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            txlsp = (CRTXLSP *)(node->data);
            if( txlsp->dclsp.index != 0 )
            {
                *portnum = 1;
                *portindex = txlsp->dclsp.nextport;
                NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_ldpallport(  FTN_KEY *key, NBB_ULONG *portindex, NBB_BYTE *portnum NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    FTN_KEY     ftnkey;

    FTN_TAB     *uftn = NULL;

    NBB_TRC_ENTRY("spm_l3_ldpallport");

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpallport key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpallport portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if (portnum == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ldpallport portnum = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    OS_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
    
    //coverity[bad_sizeof]
    NBB_MEMCPY( &ftnkey, key, sizeof( FTN_KEY ) );
    uftn =  (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,  &ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uftn->tunnelid != 0 )
        {
            portindex[0] = uftn->dclsp[0].nextport;
            portindex[1] = uftn->dclsp[1].nextport;
            portindex[2] = uftn->dclsp[2].nextport;
            portindex[3] = uftn->dclsp[3].nextport;
            *portnum = uftn->hopnum;
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
            rv = 0;
            goto  EXIT_LABEL;
        }
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_gettxlspid(  CRTXLSP_KEY *key, NBB_ULONG  *lspid NBB_CCXT_T NBB_CXT)

{
    NBB_INT         rv = 1;

	NBB_ULONG 		i  = 0;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_gettxlspid key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (lspid == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_gettxlspid lspid = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if( key->lspid == 0 )
    {
        OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
        NBB_MEMCPY( &tunlkey, key, sizeof( LSPPROT_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, &tunlkey NBB_CCXT);
        if(( node == NULL ) || ( node->data == NULL ))
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            lspprot = (LSPPROT *)(node->data);
            if( lspprot->index != 0 )
            {  
			   for(i = 0; i<lspprot->lspnum;i++)
	           {
			       if(lspprot->lspb.lsp[i].state == 0)
			       {
                       *lspid = lspprot->lspb.lsp[i].lspid;
				   }
					
			   }
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }

    EXIT_LABEL:
    
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_get_tunl_info
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-22
 * Description		:   get tunlid in driver
 * InputParam	:	    ftn key, rsvp key lsptype 
 * OutputParam	:	    tunlid in driver and logicindex
 * ReturnValue	:	    rv
 * Relation		:	    called by L2   
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_get_tunl_info(FTN_KEY *ftnkey, CRTXLSP_KEY *rsvpkey, NBB_BYTE lsptype,NBB_ULONG  *portindex, NBB_ULONG  *index NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;
    FTN_TAB         *uftn = NULL;
    SHASH_NODE      *node = NULL;
    LSPPROT_KEY     tunlkey;    
    LSPPROT         *lspprot = NULL;
    CRTXLSP         *txlsp = NULL;
    NBB_BOOL         avll_ret_code;
     
    NBB_TRC_ENTRY("spm_l3_get_tunl_info");
    if (ftnkey == NULL)
    {   
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e get_tunl_info ftnkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (rsvpkey == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e get_tunl_info rsvpkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (portindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e get_tunl_info portindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (index == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e get_tunl_info index = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    /*ldp lsp*/
    if(ATG_DCI_FTN == lsptype)
    { 
        uftn =  (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,ftnkey ); 
        if( uftn == NULL )
        {
            NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
            rv = 1;
            goto EXIT_LABEL;
        }
        else
        {
            if( uftn->tunnelid != 0 )
            {
                *index = uftn->tunnelid;
                *portindex = uftn->dclsp[0].nextport;
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    /*rsvp lsp*/
    else if(ATG_DCI_CR_LSP == lsptype)
    {  
        if( rsvpkey->lspid == 0 )
        {
            OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
            tunlkey.ingress = rsvpkey->ingress;
            tunlkey.egress = rsvpkey->egress;
            tunlkey.tunnelid = rsvpkey->tunnelid;  
            node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, &tunlkey NBB_CCXT);
            if(( node == NULL ) || ( node->data == NULL ))
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));
                rv = 1;
                goto EXIT_LABEL;
                
            }
            else
            {
                lspprot = (LSPPROT *)(node->data);
                if( lspprot->index != 0 )
                {
                    *index = lspprot->index;
                    *portindex = lspprot->dclsp[0].nextport;
                    NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
                    rv = 0;
                    goto  EXIT_LABEL;
                }
                else
                {
                    NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                    rv = 2;
                    goto  EXIT_LABEL;
                }
            }
        }
        else
        {
            node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->crlsptxhash, rsvpkey NBB_CCXT);
            if(( node == NULL ) || ( node->data == NULL ))
            {
                NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  not find"));
                rv = 1;
                goto  EXIT_LABEL;
            }
            else
            {
                txlsp = (CRTXLSP *)(node->data);
                if( txlsp->dclsp.index != 0 )
                {
                    *index = txlsp->dclsp.ftnidx;
                    *portindex = txlsp->dclsp.nextport;
                    NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup ok"));
                    rv = 0;
                    goto  EXIT_LABEL;
                }
                else
                {
                    NBB_TRC_FLOW((NBB_FORMAT " txlsp lookup  data error"));
                    rv = 2;
                    goto  EXIT_LABEL;
                }
            }
        }
    }
    else
    {
        rv = 5;
    }
    
EXIT_LABEL:
        
    NBB_TRC_EXIT();
        
    return rv;

}
#endif

#if 1

typedef struct out_flow
{
    NBB_ULONG   nextip[4];
    NBB_USHORT  nextport;
    NBB_USHORT  vlan;
    NBB_BYTE    mask;
    NBB_BYTE    type;
    NBB_USHORT  vrfid;

}OUT_FLOW;

#if 0
NBB_INT spm_l3_routeforflow(NBB_BYTE addrtype,NBB_ULONG *peerip ,OUT_FLOW  *flow )
{
    NBB_INT     rv = 1;

    NBB_BYTE    i = 0;

    VRFUROUTE_KEY       vrfukey;

    NBB_ULONG           ipv4 = 0,ipv6[4];

    VRFUROUTE           *uroute = NULL;

    SPM_PORT_INFO_CB    portfo;

    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spm_l3_routeforoam");

    if( peerip == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT "flow error"));
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if( flow == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT "flow error"));
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&vrfukey, 0, sizeof(VRFUROUTE_KEY));
    vrfukey.vrfid = 0;
    vrfukey.addrtype= addrtype;
    if( addrtype == 0 )
    {
        for ( i = 32; i > 0; i--)
        {
            OS_MEMCPY( &ipv4, peerip, 4 );
            vrfukey.mask= i;
            ipv4 = (ipv4 >>(32-i));
            vrfukey.dip[3] = (ipv4 <<(32-i));
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
        }
    }
    else if( addrtype == 1 )
    {
        for ( i = 128; i > 0; i--)
        {
            OS_MEMCPY( &(vrfukey.dip[0]), peerip, 16 );
            OS_MEMCPY( &ipv6[0], peerip, 16 );
            vrfukey.mask= i;
            if( i > 95 )
            {
                ipv6[3] = (ipv6[3] >>(128-i));
                vrfukey.dip[3] = (ipv6[3] <<(128-i));
            }
            else if( i > 63 )
            {
                ipv6[2] = (ipv6[2] >>(96-i));
                vrfukey.dip[2] = (ipv6[2] <<(96-i));
                vrfukey.dip[3] = 0;
            }
            else if( i > 31 )
            {
                ipv6[1] = (ipv6[1] >>(64-i));
                vrfukey.dip[1] = (ipv6[1] <<(64-i));
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            else if( i > 0 )
            {
                ipv6[0] = (ipv6[0] >>(32-i));
                vrfukey.dip[0] = (ipv6[0] <<(32-i));
                vrfukey.dip[1] = 0;
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->dchop[0].ntip[0];
                flow->nextip[1] = uroute->dchop[0].ntip[1];
                flow->nextip[2] = uroute->dchop[0].ntip[2];
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->dchop[0].ntip[0];
                flow->nextip[1] = uroute->dchop[0].ntip[1];
                flow->nextip[2] = uroute->dchop[0].ntip[2];
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->dchop[0].ntip[0];
                flow->nextip[1] = uroute->dchop[0].ntip[1];
                flow->nextip[2] = uroute->dchop[0].ntip[2];
                flow->nextip[3] = uroute->dchop[0].ntip[3];
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
        }
    }
    if( uroute == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam null"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uroute->dchop[0].nextport != 0 )
        {
            //*portindex = uroute->dchop[0].nextport;
            //NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam ok"));
            OS_MEMSET(&portfo, 0, sizeof(SPM_PORT_INFO_CB));
            rv = spm_get_portid_from_logical_port_index(uroute->dchop[0].nextport, &portfo NBB_CCXT);
            if( rv == 0)     
            {
                rv = 0;
                flow->nextport = portfo.port_id;
                flow->vlan = portfo.svlan ;
            }
            else
            {
                rv = 3;
                goto  EXIT_LABEL;
            }

        }
        else
        {
            //NBB_TRC_FLOW((NBB_FORMAT " UCroute dchop foroam error"));
            rv = 2;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    //NBB_TRC_EXIT();
    
    return rv;

}

#endif

#if 1
NBB_INT spm_l3_routeforflow(NBB_USHORT vrfid,NBB_BYTE addrtype,NBB_ULONG *peerip ,OUT_FLOW  *flow )
{
    NBB_INT     rv = 1;

    NBB_BYTE    i = 0;

    VRFUROUTE_KEY       vrfukey;

    NBB_ULONG           ipv4 = 0,ipv6[4];

    VRFUROUTE           *uroute = NULL;

    SPM_PORT_INFO_CB    portfo;

    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spm_l3_routeforoam");

    if( peerip == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT "flow error"));
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if( flow == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT "flow error"));
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&vrfukey, 0, sizeof(VRFUROUTE_KEY));
    vrfukey.vrfid = vrfid;
    vrfukey.addrtype = addrtype;
    if( addrtype == 0 )
    {
        for ( i = 32; i > 0; i--)
        {
            OS_MEMCPY( &ipv4, peerip, 4 );
            vrfukey.mask = i;
            ipv4 = (ipv4 >>(32 - i));
            vrfukey.dip[3] = (ipv4 << (32 - i));
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
        }
    }
    else if( addrtype == 1 )
    {
        for ( i = 128; i > 0; i--)
        {
            OS_MEMCPY( &(vrfukey.dip[0]), peerip, 16 );
            OS_MEMCPY( &ipv6[0], peerip, 16 );
            vrfukey.mask = i;
            if( i > 95 )
            {
                ipv6[3] = (ipv6[3] >>(128 - i));
                vrfukey.dip[3] = (ipv6[3] << (128 - i));
            }
            else if( i > 63 )
            {
                ipv6[2] = (ipv6[2] >>(96 - i));
                vrfukey.dip[2] = (ipv6[2] << (96 - i));
                vrfukey.dip[3] = 0;
            }
            else if( i > 31 )
            {
                ipv6[1] = (ipv6[1] >>(64 - i));
                vrfukey.dip[1] = (ipv6[1] << (64 - i));
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            else if( i > 0 )
            {
                ipv6[0] = (ipv6[0] >>(32 - i));
                vrfukey.dip[0] = (ipv6[0] << (32 - i));
                vrfukey.dip[1] = 0;
                vrfukey.dip[2] = 0;
                vrfukey.dip[3] = 0;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->dcroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->nexthop[0].nextip[0];
                flow->nextip[1] = uroute->nexthop[0].nextip[1];
                flow->nextip[2] = uroute->nexthop[0].nextip[2];
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->lbroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->nexthop[0].nextip[0];
                flow->nextip[1] = uroute->nexthop[0].nextip[1];
                flow->nextip[2] = uroute->nexthop[0].nextip[2];
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
            uroute =  (VRFUROUTE *)AVLL_FIND( v_spm_shared->vrfuroute_tree ,  &vrfukey );
            if( uroute != NULL )
            {
                flow->nextip[0] = uroute->nexthop[0].nextip[0];
                flow->nextip[1] = uroute->nexthop[0].nextip[1];
                flow->nextip[2] = uroute->nexthop[0].nextip[2];
                flow->nextip[3] = uroute->nexthop[0].nextip[3];
                
                //flow->nextport= uroute->dchop[0].nextport;
                flow->mask = uroute->key.mask;
                flow->type = uroute->key.addrtype;
                flow->vrfid = uroute->key.vrfid;
                break;
            }
        }
    }
    if( uroute == NULL )
    {
        //NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam null"));
        rv = 1;
        goto  EXIT_LABEL;
    }
    else
    {
        if( uroute->nexthop[0].nextport != 0 )
        {
            //*portindex = uroute->dchop[0].nextport;
            //NBB_TRC_FLOW((NBB_FORMAT " UCroute foroam ok"));
            OS_MEMSET(&portfo, 0, sizeof(SPM_PORT_INFO_CB));
            rv = spm_get_portid_from_logical_port_index(uroute->nexthop[0].nextport, &portfo NBB_CCXT);
            if( rv == 0)     
            {
                rv = 0;
                flow->nextport = portfo.port_id;
                flow->vlan = portfo.svlan ;
            }
            else
            {
                rv = 0;
                goto  EXIT_LABEL;
            }

        }
        else
        {
            //NBB_TRC_FLOW((NBB_FORMAT " UCroute dchop foroam error"));
            rv = 0;
            goto  EXIT_LABEL;
        }
    }

    EXIT_LABEL:
    
    //NBB_TRC_EXIT();
    
    return rv;

}

#endif


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_getteffrid(  CRRXLSP_KEY *key, NBB_ULONG  *frrid NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    CRRXLSP_KEY     rxlspkey;

    CRRXLSP         *rxlsp = NULL;

    NBB_TRC_ENTRY("spm_l3_getteffrid");
    
    if (key == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e getteffrid key = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (frrid == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e getteffrid frrid = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if( key->lspid == 0 )
    {
        OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &tunlkey, key, sizeof( LSPPROT_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( SHARED.lspprothash, &tunlkey NBB_CCXT);
        if(( node == NULL ) || (node->data == NULL))
        {
            NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            lspprot = (LSPPROT *)(node->data);
            if( lspprot->index != 0 )
            {
                *frrid = lspprot->index;
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup ok"));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  data error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    else
    {
        OS_MEMSET(&rxlspkey, 0, sizeof(CRRXLSP_KEY));
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &rxlspkey, key, sizeof( CRRXLSP_KEY ) );
        
        node =  (SHASH_NODE *)spm_hashtab_search( SHARED.crlsprxhash ,&rxlspkey NBB_CCXT);
        if(( node == NULL ) || (node->data == NULL))
        {
            NBB_TRC_FLOW((NBB_FORMAT " rxlsp lookup  not find"));
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            rxlsp = (CRRXLSP *)(node->data);
            if( rxlsp->posid != 0 )
            {
                *frrid = rxlsp->frrid;
                NBB_TRC_DETAIL((NBB_FORMAT " rxlsp frrid=%d ", *frrid));
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " rxlsp lookup  frrid error"));
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_getlspindex( NBB_ULONG ingress, NBB_ULONG egress, NBB_ULONG tunnelid, 
                                NBB_ULONG lspid, NBB_ULONG  *lspindex )
{
    NBB_INT         rv = 1;

    SHASH_NODE      *node = NULL;

    LSPPROT_KEY     tunlkey;
        
    LSPPROT         *lspprot = NULL;

    CRTXLSP_KEY     txlspkey;

    CRTXLSP         *txlsp = NULL;

    NBB_NULL_THREAD_CONTEXT
        
        NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spm_l3_getlspindex");

    if (lspindex == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e getlspindex lspindex = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if( lspid == 0 )
    {
        tunlkey.ingress = ingress;
        tunlkey.egress = egress;
        tunlkey.tunnelid = tunnelid;
        node =  (SHASH_NODE *)spm_hashtab_search(v_spm_shared->lspprothash , &tunlkey NBB_CCXT);
        if(( node == NULL ) || (node->data == NULL))
        {
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            lspprot = (LSPPROT *)(node->data);
            if( lspprot->index != 0 )
            {
                *lspindex = lspprot->index;
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    else
    {
        txlspkey.ingress = ingress;
        txlspkey.egress = egress;
        txlspkey.tunnelid = tunnelid;
        txlspkey.lspid = lspid;

        node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->crlsptxhash , &txlspkey NBB_CCXT);
        if(( node == NULL ) || (node->data == NULL))
        {
            rv = 1;
            goto  EXIT_LABEL;
        }
        else
        {
            txlsp = (CRTXLSP *)(node->data);
            if( txlsp->dclsp.ftnidx != 0 )
            {
                *lspindex = txlsp->dclsp.ftnidx;
                rv = 0;
                goto  EXIT_LABEL;
            }
            else
            {
                rv = 2;
                goto  EXIT_LABEL;
            }
        }
    }
    EXIT_LABEL:
    
    //NBB_TRC_EXIT();
    return rv;

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addmcip( NBB_USHORT portid,NBB_USHORT vrfid )
{
    NBB_INT             rv = 100;
    NBB_INT             rv2 = 100;
    NBB_USHORT          i = 0;
    NBB_ULONG           dip[4];
    L3_ROUTE_T          l3route;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spm_l3_addmcip");

#ifdef SPU
    for ( i = 1; i < 19; i++)
    {
        ApiC3InitL3RouteStruct( &l3route );

        l3route.mLogicPort = portid;
        l3route.vrfId = vrfid;
        l3route.eRouteType = ROUTE_TYPE_LOCAL;
        l3route.ipmcIdx = 0;

        if (( i == 1 ) || ( i == 2 )
            || ( i == 5 ) || ( i == 6 )
            || ( i == 13 ) || ( i == 14 )
            || ( i == 18 ))
        {
            dip[3] = 0xe0000000;
            dip[3] = dip[3] + i;
            l3route.mIpPktType = IPV4_PKT_MULTICAST;
            l3route.mDstIpv4 = dip[3];
            l3route.mSrcIpv4 = 0;
        }
        else if ( i == 3 )
        {
            dip[0] = 0xFF010000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 1;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 4 )
        {
            dip[0] = 0xFF010000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 2;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 7 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 1;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 8 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 2;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 9 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 5;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 10 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 6;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 11 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 0xd;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 12 )
        {
            dip[0] = 0xFF020000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 0x12;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else if ( i == 15 )
        {
            dip[0] = 0xFF050000;
            dip[1] = 0;
            dip[2] = 0;
            dip[3] = 2;
            NBB_MEMCPY( &(l3route.mDstIpv6), &(dip[0]), 16 );
            l3route.mIpPktType = IPV6_PKT_MULTICAST;
            
        }
        else
        {
            continue;
        }
        
        rv = ApiC3SetL3MCRoute(BCMC31, &l3route);
        if ( SHARED.c3_num == 2 )
        {
            //l3route.tL3Ecmp.ecmpIdx = 0;
            rv2 = ApiC3SetL3MCRoute(BCMC32, &l3route);
            rv = (rv|rv2);
        }
        if( rv != 0)   
        {
            OS_PRINTF("Line=%d  spm_l3_addmcip error  i=%d rv=%d\n ",__LINE__, i,rv);
            rv = 103;
            goto  EXIT_LABEL;
        }
        else
        {
            //OS_PRINTF("Line=%d  spm_l3_addmcip ok  i=%d \n ",__LINE__, i);
        }
    }
#endif

#ifdef SRC
    rv = 0;
#endif          
#ifdef SNU
    rv = 0;
#endif      

    EXIT_LABEL:

    //NBB_TRC_EXIT();
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addonemcip( NBB_USHORT portid,NBB_USHORT vrfid ,NBB_BYTE addrtype,NBB_ULONG *dip )
{
    NBB_INT             rv = 100;
    NBB_INT             rv2 = 100;
    L3_ROUTE_T          l3route;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spm_l3_addmcip");

    if ( dip == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addonemcip dip = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

#ifdef SPU
    ApiC3InitL3RouteStruct( &l3route );

    l3route.mLogicPort = portid;
    l3route.vrfId = vrfid;
    l3route.eRouteType = ROUTE_TYPE_LOCAL;
    l3route.ipmcIdx = 0;

    if ( addrtype == 0 )
    {
        l3route.mIpPktType = IPV4_PKT_MULTICAST;
        NBB_MEMCPY( &(l3route.mDstIpv4), dip, 4 );
        l3route.mSrcIpv4 = 0;
    }
    else if ( addrtype == 1 )
    {
        l3route.mIpPktType = IPV6_PKT_MULTICAST;
        NBB_MEMCPY( &(l3route.mDstIpv6), dip, 16 );
    }
    rv = ApiC3SetL3MCRoute(BCMC31, &l3route);
    if ( SHARED.c3_num == 2 )
    {
        rv2 = ApiC3SetL3MCRoute(BCMC32, &l3route);
        rv = (rv|rv2);
    }
    if( rv != 0)   
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addonemcip rv=%d \n", __LINE__,rv);
        rv = 103;
        goto  EXIT_LABEL;
    }

#endif

#ifdef SRC
    rv = 0;
#endif          
#ifdef SNU
    rv = 0;
#endif      

    EXIT_LABEL:

    //NBB_TRC_EXIT();
    return rv;
}


#endif

#if 1


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_mpeerfrr( NBB_BYTE flag,NBB_ULONG mpeer NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    
    NBB_INT     rv2 = 1;

    PEER_KEY    mpeerkey;

    MPEERFRR    *mpeerfrr = NULL;

    BPEERFRR    *bpeerfrr = NULL;

    NBB_TRC_ENTRY("spm_l3_mpeerfrr");

    OS_MEMSET(&mpeerkey, 0, sizeof(PEER_KEY));
    mpeerkey.peer = mpeer;
    mpeerfrr =  (MPEERFRR *)AVLL_FIND( SHARED.peer_tree, &mpeerkey );
    if( mpeerfrr == NULL )
    {
        rv = 0;
        goto  EXIT_LABEL;
    }
    else
    {
        bpeerfrr =  (BPEERFRR *)AVLL_FIRST( mpeerfrr->bpeer_tree );
        while (bpeerfrr != NULL)
        {
            #ifdef SPU
            if(( bpeerfrr->key.peer != 0 ) && ( bpeerfrr->frrid != 0 ))
            {
                OS_PRINTF("del ftn setfrr frrid =%d \n", bpeerfrr->frrid);
                rv = ApiC3SetFrrGroupStatus( BCMC31, bpeerfrr->frrid, flag);
                if ( SHARED.c3_num == 2 )
                {
                    rv2 = ApiC3SetFrrGroupStatus( BCMC32, bpeerfrr->frrid, flag);
                    rv = (rv|rv2);
                }
                if( rv != 0)   
                {
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e mpeerfrr mp.bp.frr.flag =%x.%x.%d.%d \n", 
                                      __LINE__, mpeer, bpeerfrr->key.peer,bpeerfrr->frrid, flag);
                                      
                    //OS_PRINTF("Line=%d error setfrr frrid=%d rv=%d\n",__LINE__, bpeerfrr->frrid,rv);
                    //rv = 102;
                    //goto  EXIT_LABEL;
                }
                else
                {
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o mpeerfrr mp.bp.frr.flag =%x.%x.%d.%d \n",
                                      __LINE__, mpeer, bpeerfrr->key.peer,bpeerfrr->frrid, flag);
                                      
                    //OS_PRINTF("Line=%d ok setfrr frrid=%d\n",__LINE__, bpeerfrr->frrid);
                }
            }
            #endif
            #ifdef SRC
                rv = 0;
            #endif  
            #ifdef SNU
                rv = 0;
            #endif  
            bpeerfrr = (BPEERFRR *)AVLL_NEXT( mpeerfrr->bpeer_tree, bpeerfrr->bp_node);
        }
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_pwvccomp(NBB_VOID *keyarg1, NBB_VOID *keyarg2 NBB_CCXT_T NBB_CXT)
{

    PWVC_KEY *key1 = (PWVC_KEY *)keyarg1;
    PWVC_KEY *key2 = (PWVC_KEY *)keyarg2;
    NBB_INT rv = 0;

    //NBB_TRC_ENTRY("spm_l3_uniportcomp");

    rv = compare_ulong(&key1->vpid, &key2->vpid NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }

    rv = compare_ulong(&key1->vcid, &key2->vcid NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }
    
    EXIT_LABEL:
    
    //NBB_TRC_EXIT();

    return(rv);

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_pwvrfcomp(NBB_VOID *keyarg1, NBB_VOID *keyarg2 NBB_CCXT_T NBB_CXT)
{

    PWVRF_KEY *key1 = (PWVRF_KEY *)keyarg1;
    PWVRF_KEY *key2 = (PWVRF_KEY *)keyarg2;
    NBB_INT rv = 0;

    //NBB_TRC_ENTRY("spm_l3_uniportcomp");

    rv = compare_ushort(&key1->vrfid, &key2->vrfid NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }

    EXIT_LABEL:
    
    //NBB_TRC_EXIT();

    return(rv);

}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_pwmcomp(NBB_VOID *keyarg1, NBB_VOID *keyarg2 NBB_CCXT_T NBB_CXT)
{

    PWMC_KEY *key1 = (PWMC_KEY *)keyarg1;
    PWMC_KEY *key2 = (PWMC_KEY *)keyarg2;
    NBB_INT rv = 0;

    //NBB_TRC_ENTRY("spm_l3_uniportcomp");

    rv = compare_ushort(&key1->mcid, &key2->mcid NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }

    EXIT_LABEL:
    
    //NBB_TRC_EXIT();

    return(rv);

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_tunlselkeycomp(NBB_VOID *keyarg1, NBB_VOID *keyarg2 NBB_CCXT_T NBB_CXT)
{

    TUNNELCH_KEY *key1 = (TUNNELCH_KEY *)keyarg1;
    TUNNELCH_KEY *key2 = (TUNNELCH_KEY *)keyarg2;
    NBB_INT rv = 0;

    //NBB_TRC_ENTRY("spm_l3_uniportcomp");

    rv = compare_ushort(&key1->vrfid, &key2->vrfid NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }
	rv = compare_ulong(&key1->peerip, &key2->peerip NBB_CCXT);
    if (rv != 0)
    {
        goto EXIT_LABEL;
    }

    EXIT_LABEL:
    
    //NBB_TRC_EXIT();

    return(rv);

}

/******************************************************************************
 * FunctionName 	: 	spm_l3_ftn_alloc_cb
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-25
 * Description		:   alloc ftn cb memory
 * InputParam	:	    peerip ,vpnum,mcnum,vrfnum
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_ftn_alloc_cb( NBB_ULONG peerip,NBB_ULONG vpnum,NBB_ULONG mcnum,NBB_ULONG vrfnum NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;
    FTN_TAB         *uftn = NULL;
    FTN_KEY         ftnkey;
    NBB_BOOL        avll_ret_code;

    NBB_TRC_ENTRY("spm_l3_ftn_alloc_cb");

    if (0 == peerip)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ftn_alloc_cb peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    NBB_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
    ftnkey.fec = peerip;
    ftnkey.mask = 32;
    ftnkey.vrfid = 0;
    ftnkey.res = 0;
    uftn =  (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,&ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
        
        /*ldp tunnel 表项为空时，申请内存空间*/
        uftn = (FTN_TAB *)NBB_MM_ALLOC(sizeof(FTN_TAB), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_NODE_CB);
        if(NULL == uftn)
        {
            NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
            rv = 888;
            goto EXIT_LABEL;
        }
        NBB_MEMSET(uftn, 0, sizeof(FTN_TAB));
        AVLL_INIT_NODE(uftn->ftn_node);

        /*vc配置在lsp前下发，申请的虚拟内存标志*/
        uftn->memflag = 1;
        uftn->l2num = (vpnum + mcnum);
        uftn->l3num = vrfnum ;
        
        /*ftn表项的待删除标志初始值*/
        uftn->delflag = 0;
        NBB_MEMCPY(&(uftn->key),&ftnkey,sizeof(FTN_KEY));
        avll_ret_code = AVLL_INSERT(v_spm_shared->ftn_tree, uftn->ftn_node);
        NBB_ASSERT(avll_ret_code);
        rv = 0;
    }
    else
    {
        uftn->l2num = vpnum + mcnum;
        uftn->l3num = vrfnum;
        rv = 0;
    }

    EXIT_LABEL:
    
    NBB_TRC_EXIT();

    return(rv);
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_ftn_free_cb
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-25
 * Description		:   free ftn cb memory 
 * InputParam	:	    peerip,vpnum,mcnum,vrfnum
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L2    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_ftn_free_cb( NBB_ULONG peerip,NBB_ULONG vpnum,NBB_ULONG mcnum,NBB_ULONG vrfnum NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;
    NBB_INT         rv2 = 1;
    FTN_TAB         *uftn = NULL;
    FTN_KEY         ftnkey;
    ATG_DCI_BFD_LDP_LSP_DATA        bfdldp;
    NBB_BOOL        avll_ret_code;

    NBB_TRC_ENTRY("spm_l3_ftn_free_cb");

    if (0 == peerip)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ftn_free_cb peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    NBB_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
    ftnkey.fec = peerip;
    ftnkey.mask = 32;
    ftnkey.vrfid = 0;
    ftnkey.res = 0;
    uftn =  (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,&ftnkey ); 
    if( uftn == NULL )
    {
        NBB_TRC_FLOW((NBB_FORMAT " FTN iteration error"));
        rv = 1;
        goto EXIT_LABEL;
    }
    else
    {
        uftn->l2num = vpnum + mcnum;
        uftn->l3num = vrfnum;
        rv = 0;
    }

    if((uftn->l2num == 0) && (uftn->l3num == 0))
    {
        /*待删除标志,删lsp时，如果lsp被依赖就置位*/
        if(uftn->delflag == 2)
        {
            if ( ftnkey.vrfid == 0 )
            {
                if( uftn->pwposid != 0)
                {
                    #ifdef SPU              
                    rv = spm_l3_ftnupdatefib( 2, 0, uftn->key.mask, uftn->key.fec NBB_CCXT);
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: Route DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ftnupdatefib,rv=%d.peer=%x\n", __LINE__,
                                                                                             rv,uftn->key.fec);
                        goto  EXIT_LABEL;
                    }
                    rv = ApiC3DelL3TxPw(BCMC31, uftn->pwposid);
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DelL3TxPw(BCMC32, uftn->pwposid);
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: L3TxPw DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DelL3TxPw,rv=%d.pwposid=%d\n", __LINE__,
                                                                                                 rv,uftn->pwposid);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->pwposid = 0;
                    }
                    #endif
                    
                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                if( uftn->base.nexttype == 1)
                {
                    spm_lps_del_entity( TUNNEL_TYPE, uftn->tunnelid);
                }
                if( uftn->base.nexttype == 2)
                {
                    aps_del_tunnel_ecmp(uftn->ecmpid);
                }
                if( uftn->tunnelid != 0)
                {
                    #ifdef SPU
                    rv = ApiC3DeleteTunnel( BCMC31, uftn->tunnelid );
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DeleteTunnel( BCMC32, uftn->tunnelid );
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: Tunnel DEV rv=%d ", rv)); 
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DeleteTunnel,rv=%d,tunlid=%d\n", __LINE__,
                                                                                                  rv,uftn->tunnelid);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->tunnelid = 0;
                    }
                    #endif
                    
                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                if( uftn->posid[0] != 0)
                {
                    #ifdef SPU
                    rv = ApiC3DelTxLsp( BCMC31, uftn->posid[0] );
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DelTxLsp( BCMC32, uftn->posid[0] );
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: Lsp1 DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DelTxLsp,rv=%d,posid=%d\n", __LINE__,
                                                                                             rv,uftn->posid[0]);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->posid[0] = 0;
                    }
                    #endif
                    
                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                if( uftn->posid[1] != 0)
                {
                    #ifdef SPU
                    rv = ApiC3DelTxLsp( BCMC31, uftn->posid[1] );
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DelTxLsp( BCMC32, uftn->posid[1] );
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: lsp2 DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DelTxLsp,rv=%d,posid=%d\n", __LINE__,
                                                                                             rv,uftn->posid[1]);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->posid[1] = 0;
                    }
                    #endif

                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                if( uftn->posid[2] != 0)
                {
                    #ifdef SPU
                    rv = ApiC3DelTxLsp( BCMC31, uftn->posid[2] );
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DelTxLsp( BCMC32, uftn->posid[2] );
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: lsp2 DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DelTxLsp,rv=%d,posid=%d\n", __LINE__,
                                                                                             rv,uftn->posid[2]);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->posid[2] = 0;
                    }
                    #endif

                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                if( uftn->posid[3] != 0)
                {
                    #ifdef SPU
                    rv = ApiC3DelTxLsp( BCMC31, uftn->posid[3] );
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3DelTxLsp( BCMC32, uftn->posid[3] );
                        rv = (rv|rv2);
                    }
                    if( rv != 0)
                    {
                        NBB_TRC_DETAIL((NBB_FORMAT "Del FTN Error: lsp2 DEV rv=%d ", rv));
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DelTxLsp,rv=%d,posid=%d\n", __LINE__,
                                                                                             rv,uftn->posid[3]);
                        goto  EXIT_LABEL;
                    }
                    else
                    {
                        uftn->posid[3] = 0;
                    }
                    #endif

                    #ifdef SRC
                    rv = 0;
                    #endif
                    #ifdef SNU
                    rv = 0;
                    #endif
                }
                rv = 0;
            }
            if( rv == 0)      /* 删除成功，删除对应的配置*/
            {
                OS_MEMSET( &bfdldp, 0, sizeof( ATG_DCI_BFD_LDP_LSP_DATA ) );
                bfdldp.vrf_id = ftnkey.vrfid ;
                bfdldp.fec_prefix_len = ftnkey.mask;
                bfdldp.fec_prefix = ftnkey.fec;
                spm_set_ldp_lsp_state( &bfdldp, 1);
            
                //spm_l3_updatepeerecmp( uftn->key.fec, 0 NBB_CCXT);
                //spm_l3_mpeerfrr( 1, uftn->key.fec NBB_CCXT);
                AVLL_DELETE( SHARED.ftn_tree , uftn->ftn_node);
                NBB_MM_FREE(uftn, MEM_SPM_FTN_TAB_NODE_CB);
            }

        }
    }
    

    EXIT_LABEL:
    
    NBB_TRC_EXIT();

    return(rv);
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_lspprot_alloc_cb
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-25
 * Description		:   alloc lspprot cb memory
 * InputParam	:	    tunlkey ,vpnum,mcnum,vrfnum
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_lspprot_alloc_cb( LSPPROT_KEY *tunlkey,NBB_ULONG vpnum,NBB_ULONG mcnum,
                                                           NBB_ULONG vrfpeernum NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;
    SHASH_NODE      *node = NULL;
    LSPPROT         *lspprot = NULL;

    NBB_TRC_ENTRY("spm_l3_lspprot_alloc_cb");
    
    if (NULL == tunlkey)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lspprot_alloc_cb tunlkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, tunlkey NBB_CCXT);
    if(( node == NULL ) || ( node->data == NULL ))
    {
        NBB_TRC_FLOW((NBB_FORMAT " TUNNEL lookup  not find"));

        /*rsvp tunnel 为空时，申请内存空间*/
        lspprot = (LSPPROT *)NBB_MM_ALLOC(sizeof(LSPPROT), NBB_NORETRY_ACT, MEM_SPM_LSPPROT_CB);
        if(NULL == lspprot)
        {
            NBB_TRC_FLOW((NBB_FORMAT " lspp alloc error"));
            rv = 888;
            goto EXIT_LABEL;
        }
        NBB_MEMSET(lspprot, 0, sizeof(LSPPROT));
        NBB_MEMCPY(&(lspprot->key),tunlkey,sizeof(LSPPROT_KEY));

        /*vc在lsp之前下发，申请内存空间标示 */
        lspprot->memflag = 1;
        lspprot->l2num = (vpnum + mcnum);
        lspprot->l3num = vrfpeernum;
        spm_hashtab_insert ( v_spm_shared->lspprothash , &(lspprot->key) , lspprot NBB_CCXT);
        rv = 0;
    }
    else
    {
        lspprot = (LSPPROT *)(node->data);
        lspprot->l2num = (vpnum + mcnum);
        lspprot->l3num = vrfpeernum;
        rv = 0;
    }

    EXIT_LABEL:
        
    NBB_TRC_EXIT();
        
    return rv;
    
    
 
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_lspprot_free_cb
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-25
 * Description		:   free lspprot cb memory
 * InputParam	:	    tunlkey ,vpnum,mcnum,vrfpeernum
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_lspprot_free_cb( LSPPROT_KEY *tunlkey,NBB_ULONG vpnum,NBB_ULONG mcnum,
                                                            NBB_ULONG vrfpeernum NBB_CCXT_T NBB_CXT)
{
    NBB_INT         rv = 1;
    NBB_INT         rv2 = 1;
    SHASH_NODE      *node = NULL;
    LSPPROT         *lspprot = NULL;

    NBB_TRC_ENTRY("spm_l3_lspprot_free_cb");

    if (NULL == tunlkey)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lspprot_free_cb tunlkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    node =  (SHASH_NODE *)spm_hashtab_search( v_spm_shared->lspprothash, tunlkey NBB_CCXT);
    if((node == NULL) || (node->data == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT " lspprot data error"));
        rv = 1;
        goto EXIT_LABEL; 
    }
    else
    {
        lspprot = (LSPPROT *)(node->data);
        lspprot->l2num = vpnum + mcnum;
        lspprot->l3num = vrfpeernum;
        rv = 0;
    }

    /*当rsvp tunnel被L2,L3引用计数为0，并被置为待删除标志时，删除tunnel相关资源*/
    if((lspprot->l2num == 0) && (lspprot->l3num == 0))
    {
        if(lspprot->delflag == 2)
        {
        #ifdef SPU
            if ( lspprot->index != 0)
            {
                spm_lps_del_entity( TUNNEL_TYPE, lspprot->index);
                NBB_TRC_DETAIL((NBB_FORMAT "Del LSPPROT index=%x ", lspprot->index));
                rv = ApiC3DeleteTunnel( BCMC31, lspprot->index);
                if ( SHARED.c3_num == 2 )
                {
                    rv2 = ApiC3DeleteTunnel( BCMC32, lspprot->index);
                    rv = (rv|rv2);
                }
                if( rv != 0)
                {
                    NBB_TRC_DETAIL((NBB_FORMAT "Del lspp Error: tunl drv rv=%d ", rv));
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e ApiC3DeleteTunnel,rv=%d.tunlindx=%d\n", __LINE__,
                                                                                                 rv,lspprot->index);
                    goto  EXIT_LABEL;
                }
                else
                {
                    lspprot->index = 0;
                }
            }
            
        #endif
            rv = 0;
            /* 删除成功，删除hash表对应的配置*/
            if( rv == 0)     
            {
                spm_hashtab_del( SHARED.lspprothash , tunlkey NBB_CCXT);
                NBB_MM_FREE(lspprot, MEM_SPM_LSPPROT_CB);
            }
        }
    }
    

    EXIT_LABEL:
        
    NBB_TRC_EXIT();
        
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addvpldp( SPM_VP_INFO_REFRESH_CB *vpfo, NBB_ULONG vpid,NBB_ULONG vcid,NBB_ULONG tunid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    
    NBB_INT     rv2 = 1;

    NBB_ULONG   peerip = 0;
    
    PEIP_KEY        peerkey;

    PWVC_KEY    pwvckey;

    PEERFTN     *peerftn = NULL;

    PWVC        *pwvc = NULL;

    NBB_TRC_ENTRY("spm_l3_addvpldp");


    if (vpfo == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp vpfo = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    if (vpfo->vc_key.peer_ip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vpid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp vpid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vcid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp vcid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    peerip = vpfo->vc_key.peer_ip;
    
    //OS_MEMSET(&peerkey, 0, sizeof(PEIP_KEY));
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        peerftn = (PEERFTN *)NBB_MM_ALLOC(sizeof(PEERFTN), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_PEER_CB);
        if(NULL == peerftn)
        {
            OS_PRINTF("Line=%d peerftn malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(peerftn, 0, sizeof(PEERFTN));
        AVLL_INIT_NODE(peerftn->pe_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(peerftn->key), &peerkey, sizeof(PEIP_KEY));
        
        AVLL_INIT_TREE(peerftn->vp_tree,
                            spm_l3_pwvccomp,
                            NBB_OFFSETOF(PWVC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->vrf_tree,
                            spm_l3_pwvrfcomp,
                            NBB_OFFSETOF(PWVRF, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVRF, vrf_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->mcid_tree,
                            spm_l3_pwmcomp,
                            NBB_OFFSETOF(PWMC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
        
        peerftn->vpnum = 1;
        peerftn->tunnel = tunid;
        
        //coverity[no_effect_test]
        AVLL_INSERT( SHARED.peftn_tree, peerftn->pe_node);

        OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
        pwvckey.vpid = vpid;
        pwvckey.vcid = vcid;
        pwvc = (PWVC *)NBB_MM_ALLOC(sizeof(PWVC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VC_CB);
        if(NULL == pwvc)
        {
            OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(pwvc, 0, sizeof(PWVC));
        AVLL_INIT_NODE(pwvc->vc_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(pwvc->key), &pwvckey, sizeof(PWVC_KEY));

        AVLL_INSERT( peerftn->vp_tree, pwvc->vc_node);
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(pwvc->l2vp), vpfo, sizeof(SPM_VP_INFO_REFRESH_CB));
        
        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvpldp peer.vp.vc =%x.%x.%x \n", 
        //              __LINE__, peerip,vpid,vcid);
        
    }
    else
    {
        OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
        pwvckey.vpid = vpid;
        pwvckey.vcid = vcid;
        pwvc =  (PWVC *)AVLL_FIND( peerftn->vp_tree,  &pwvckey );
        if( pwvc == NULL )
        {
            pwvc = (PWVC *)NBB_MM_ALLOC(sizeof(PWVC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VC_CB);
            if(NULL == pwvc)
            {
                OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
                rv = 888;
                goto  EXIT_LABEL;
            }
            NBB_MEMSET(pwvc, 0, sizeof(PWVC));
            AVLL_INIT_NODE(pwvc->vc_node);  
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( &(pwvc->key), &pwvckey, sizeof(PWVC_KEY));
            AVLL_INSERT( peerftn->vp_tree, pwvc->vc_node);
            peerftn->vpnum++;
            
            //coverity[bad_sizeof]
            NBB_MEMCPY( &(pwvc->l2vp), vpfo, sizeof(SPM_VP_INFO_REFRESH_CB));
            
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvpldp peer.vp.vc =%x.%x.%x \n", 
           // __LINE__, peerip,vpid,vcid);
            
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp peer.vp.vc =%x.%x.%x \n", 
            //__LINE__, peerip,vpid,vcid);
            
        }
    }

    /*查看tunnel是否存在，不存在，申请ftn内存资源*/
    rv = spm_l3_ftn_alloc_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_alloc_cb,rv=%d\n", __LINE__,rv);  
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_delvpldp( NBB_ULONG peerip, NBB_ULONG vpid,NBB_ULONG vcid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;

    PEIP_KEY        peerkey;

    PWVC_KEY    pwvckey;

    PEERFTN     *peerftn = NULL;

    PWVC        *pwvc = NULL;
    NBB_ULONG   ldpcitenum = 0;

    NBB_TRC_ENTRY("spm_l3_delvpldp");
    
    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvpldp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vpid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvpldp vpid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vcid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvpldp vcid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
        
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 101;
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvpldp peer.vp.vc =%x.%x.%x \n", __LINE__, peerip,vpid,vcid);
        goto  EXIT_LABEL;
    }
    else
    {
        OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
        pwvckey.vpid = vpid;
        pwvckey.vcid = vcid;
        pwvc =  (PWVC *)AVLL_FIND( peerftn->vp_tree,  &pwvckey );
        if( pwvc == NULL )
        {
            rv = 102;
            
            //OS_PRINTF("Line=%d error freefrrid mpeer=%x bpeer=%x rv=%d\n",__LINE__, mpeer ,bpeer ,rv);
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvpldp peer.vp.vc =%x.%x.%x \n", 
                                        __LINE__, peerip,vpid,vcid);
            goto  EXIT_LABEL;
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvpldp peer.vp.vc =%x.%x.%x \n", 
            //                          __LINE__, peerip,vpid,vcid);
            AVLL_DELETE( peerftn->vp_tree , pwvc->vc_node);
            NBB_MM_FREE(pwvc, MEM_SPM_FTN_TAB_VC_CB);
            peerftn->vpnum--;
        }
    }

    /*释放ftn cb资源接口*/
    rv = spm_l3_ftn_free_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_free_cb,rv=%d\n", __LINE__,rv);
    }
    
    if(( peerftn->vpnum == 0) && ( peerftn->vrfnum == 0) && ( peerftn->mcnum == 0))
    {
        AVLL_DELETE( SHARED.peftn_tree, peerftn->pe_node);
        NBB_MM_FREE(peerftn, MEM_SPM_FTN_TAB_PEER_CB);
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatevp( NBB_ULONG peerip,NBB_ULONG tunid ,
                                                                            NBB_ULONG portindex NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    
    NBB_INT     rv2 = 1;

    NBB_ULONG   vpid;

    NBB_ULONG   vcid;

    PEIP_KEY        peerkey;

    PEERFTN     *peerftn = NULL;

    PWVC        *pwvc = NULL;

    SPM_VP_INFO_REFRESH_CB  vpinfo;

    NBB_TRC_ENTRY("spm_l3_updatevp");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    OS_MEMSET(&vpinfo, 0, sizeof(SPM_VP_INFO_REFRESH_CB));
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 0;
        goto  EXIT_LABEL;
    }
    else
    {
        pwvc =  (PWVC *)AVLL_FIRST( peerftn->vp_tree );
        
        //OS_PRINTF("update L2 vp peer.tun =%x.%x \n", __LINE__, peerip,tunid);
        peerftn->tunnel = tunid;
        while (pwvc != NULL)
        {

            vpid = pwvc->key.vpid;
            vcid = pwvc->key.vcid;
            if(( vpid != 0) && ( vcid != 0))
            {
            #ifdef SPU
                rv = ApiC3SetVcTunnel( BCMC31, vpid, vcid, tunid);
                if ( SHARED.c3_num == 2 )
                {
                    rv2 = ApiC3SetVcTunnel( BCMC32, vpid, vcid, tunid);
                    rv = (rv|rv2);
                }
            #else
                rv = 0;
            #endif
            
                if( rv != 0)   
                {
                    //OS_PRINTF("Line=%d e updatevp peer.tun =%x.%x \n", __LINE__, peerip,tunid);
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevp peer.vp.vc =%x.%x.%x \n", 
                                                __LINE__, peerip,vpid,vcid);
                }
                else
                {
                    //OS_PRINTF("Line=%d o updatevp peer.tun =%x.%x \n", __LINE__, peerip,tunid);
                    //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatevp peer.vp.vc =%x.%x.%x \n", 
                    //                          __LINE__, peerip,vpid,vcid);

                    OS_MEMCPY(&vpinfo, &(pwvc->l2vp), sizeof(SPM_VP_INFO_REFRESH_CB));
                    spm_vpn_refresh_vc_nni_cfg( &vpinfo, portindex NBB_CCXT);
                }
            }
            else
            {
                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevp peer.vp.vc =%x.%x.%x \n", 
                                            __LINE__, peerip,vpid,vcid);
            }

            pwvc = (PWVC *)AVLL_NEXT( peerftn->vp_tree, pwvc->vc_node);
        }
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addvrfldp( NBB_ULONG peerip, NBB_USHORT vrfid,NBB_ULONG tunid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    
    NBB_INT     rv2 = 1;

    PEIP_KEY    peerkey;

    PWVRF_KEY   pwvrfkey;

    PEERFTN     *peerftn = NULL;

    PWVRF       *pwvrf = NULL;

    NBB_TRC_ENTRY("spm_l3_addvrfldp");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvrfldp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vrfid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvrfldp vrf = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    //OS_MEMSET(&peerkey, 0, sizeof(PEIP_KEY));
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        peerftn = (PEERFTN *)NBB_MM_ALLOC(sizeof(PEERFTN), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_PEER_CB);
        if(NULL == peerftn)
        {
            OS_PRINTF("Line=%d peerftn malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(peerftn, 0, sizeof(PEERFTN));
        AVLL_INIT_NODE(peerftn->pe_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(peerftn->key), &peerkey, sizeof(PEIP_KEY));
        
        AVLL_INIT_TREE(peerftn->vp_tree,
                            spm_l3_pwvccomp,
                            NBB_OFFSETOF(PWVC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->vrf_tree,
                            spm_l3_pwvrfcomp,
                            NBB_OFFSETOF(PWVRF, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVRF, vrf_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->mcid_tree,
                            spm_l3_pwmcomp,
                            NBB_OFFSETOF(PWMC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
        peerftn->vrfnum = 1;
        peerftn->tunnel = tunid;
        
        //coverity[no_effect_test]
        AVLL_INSERT( SHARED.peftn_tree, peerftn->pe_node);

        OS_MEMSET(&pwvrfkey, 0, sizeof(PWVRF_KEY));
        pwvrfkey.vrfid = vrfid;
        pwvrf = (PWVRF *)NBB_MM_ALLOC(sizeof(PWVRF), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VRF_CB);
        if(NULL == pwvrf)
        {
            OS_PRINTF("Line=%d pwvrf malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(pwvrf, 0, sizeof(PWVRF));
        AVLL_INIT_NODE(pwvrf->vrf_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(pwvrf->key), &pwvrfkey, sizeof(PWVRF_KEY));

        AVLL_INSERT( peerftn->vrf_tree, pwvrf->vrf_node);
        
        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
    }
    else
    {
        OS_MEMSET(&pwvrfkey, 0, sizeof(PWVRF_KEY));
        pwvrfkey.vrfid = vrfid;
        pwvrf =  (PWVRF *)AVLL_FIND( peerftn->vrf_tree,  &pwvrfkey );
        if( pwvrf == NULL )
        {
            pwvrf = (PWVRF *)NBB_MM_ALLOC(sizeof(PWVRF), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VRF_CB);
            if(NULL == pwvrf)
            {
                OS_PRINTF("Line=%d pwvrf malloc mem NULL \n",__LINE__);
                rv = 888;
                goto  EXIT_LABEL;
            }
            NBB_MEMSET(pwvrf, 0, sizeof(PWVRF));
            AVLL_INIT_NODE(pwvrf->vrf_node);  
            NBB_MEMCPY( &(pwvrf->key), &pwvrfkey, sizeof(PWVRF_KEY));

            AVLL_INSERT( peerftn->vrf_tree, pwvrf->vrf_node);

            peerftn->vrfnum++;
            
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
        }
    }

    /*如果路由引用的tunnel不存在，就申请资源*/
    rv = spm_l3_ftn_alloc_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_alloc_cb,rv=%d\n", __LINE__,rv); 
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_delvrfldp( NBB_ULONG peerip, NBB_USHORT vrfid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;

    PEIP_KEY        peerkey;

    PWVRF_KEY   pwvrfkey;

    PEERFTN     *peerftn = NULL;

    PWVRF       *pwvrf = NULL;

    NBB_TRC_ENTRY("spm_l3_delvrfldp");
    
    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfldp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vrfid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfldp vrf = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

        
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 101;
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
        goto  EXIT_LABEL;
    }
    else
    {
        OS_MEMSET(&pwvrfkey, 0, sizeof(PWVRF_KEY));
        pwvrfkey.vrfid = vrfid;
        pwvrf =  (PWVRF *)AVLL_FIND( peerftn->vrf_tree,  &pwvrfkey );
        if( pwvrf == NULL )
        {
            rv = 102;
            
            //OS_PRINTF("Line=%d error freefrrid mpeer=%x bpeer=%x rv=%d\n",__LINE__, mpeer ,bpeer ,rv);
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            goto  EXIT_LABEL;
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            AVLL_DELETE( peerftn->vrf_tree , pwvrf->vrf_node);
            NBB_MM_FREE(pwvrf, MEM_SPM_FTN_TAB_VRF_CB);
            peerftn->vrfnum--;
        }
    }

    rv = spm_l3_ftn_free_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_free_cb,rv=%d\n", __LINE__,rv);
    }
    if(( peerftn->vpnum == 0) && ( peerftn->vrfnum == 0) && ( peerftn->mcnum == 0))
    {
        AVLL_DELETE( SHARED.peftn_tree, peerftn->pe_node);
        NBB_MM_FREE(peerftn, MEM_SPM_FTN_TAB_PEER_CB);
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatevrf( NBB_ULONG peerip,NBB_ULONG tunid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;

    NBB_INT     rv2 = 1;

    NBB_USHORT  vrfid;
    
    PEIP_KEY        peerkey;

    PEERFTN     *peerftn = NULL;

    PWVRF       *pwvrf = NULL;

    TUNNELCH_KEY        tunnelskey;

    TUNNELCH            *tunnels = NULL;

    PWTXNODE            *pwnodes = NULL;

    SHASH_NODE          *node = NULL;

    L3_TX_PW_T          txpw;
    L3_TX_PW_T          temp_txpw;

    NBB_TRC_ENTRY("spm_l3_updatevrf");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 0;
        goto  EXIT_LABEL;
    }
    else
    {
        pwvrf =  (PWVRF *)AVLL_FIRST( peerftn->vrf_tree );
        
        //OS_PRINTF("update L2 vp peer.tun =%x.%x \n", __LINE__, peerip,tunid);
        peerftn->tunnel = tunid;
        while (pwvrf != NULL)
        {
        #ifdef SPU

            tunnelskey.res = 0;
            tunnelskey.vrfid = pwvrf->key.vrfid;
            tunnelskey.peerip = peerip;
            node =  (SHASH_NODE *)spm_hashtab_search( SHARED.tunnelchhash, &tunnelskey NBB_CCXT); 
            if ((node == NULL) || (node->data == NULL))
            {
                pwvrf = (PWVRF *)AVLL_NEXT( peerftn->vrf_tree, pwvrf->vrf_node);
                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf null vrf.pe =%x.%x \n", __LINE__, 
                                                                                    tunnelskey.vrfid,peerip);
                continue;
            }
            else
            {
                tunnels = (TUNNELCH *)(node->data);
                tunnels->tunindex = tunid;
            }
            pwnodes = (PWTXNODE *)AVLL_FIRST(tunnels->pwtree);
            while (pwnodes != NULL)
            {
                if( pwnodes->posid != 0 ) 
                {
                    OS_MEMSET(&txpw , 0, sizeof(L3_TX_PW_T));
                    OS_MEMSET(&temp_txpw , 0, sizeof(L3_TX_PW_T));
                    txpw.mVrf = tunnelskey.vrfid;
                    txpw.mPeerIp = tunnelskey.peerip;
                    txpw.mLabel = pwnodes->key.label;
                    
                    txpw.posId = pwnodes->posid;
                    txpw.tunnelIdx = tunid; 
                    if( tunnelskey.vrfid != 0)
                    {
                        txpw.pushPwNum = 1;
                    }

                    rv = fhdrv_psn_l3_get_txpw(BCMC31, &txpw, &temp_txpw);
                    if (rv == 0)
                    {
                        txpw.eNniMode = temp_txpw.eNniMode;
                    }
                    else
                    {
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, 
                                "Line=%d e updatevrf fhdrv_psn_l3_get_txpw posid.eNniMode =%d.%d\n", 
                                        __LINE__, txpw.posId, txpw.eNniMode);
                    }
                    
                    //txpw.flags |= L3_TX_PW_COUNTER_ON;
                    rv = ApiC3ModL3TxPw( BCMC31, &txpw);
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3ModL3TxPw( BCMC32, &txpw);
                        rv = (rv|rv2);
                    }
                    if( rv != 0) 
                    {
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf vrf.pe.lab =%x.%x.%x \n", 
                                                    __LINE__, tunnelskey.vrfid,peerip,pwnodes->key.label);
                    }
                    else
                    {
                        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatevrf vrf.pe.lab =%x.%x.%x \n", 
                        //                          __LINE__, tunnelskey.vrfid,peerip,pwnodes->key.label);
                    }
                }
                pwnodes = (PWTXNODE *)AVLL_NEXT( tunnels->pwtree, pwnodes->pw_node);
            }
        #endif
            pwvrf = (PWVRF *)AVLL_NEXT( peerftn->vrf_tree, pwvrf->vrf_node);
        }
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_add_mcid_ldp(NBB_USHORT usVplsId,NBB_USHORT usMcId, NBB_BYTE ucLspType, 
                        FTN_KEY stFtnKey, CRTXLSP_KEY stCrLspKey,NBB_ULONG portindex NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;

    PEIP_KEY        peerkey;

    PEERFTN     *peerftn = NULL;

    PWMC_KEY    pwmckey;
    
    PWMC        *pwmc = NULL;
    
    NBB_TRC_ENTRY("spm_l3_add_mcid_ldp");

    if (usVplsId == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid vplsid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (usMcId == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid mcid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (portindex == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid portindex = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (ucLspType == 0)
    {
        if (stFtnKey.fec == 0)
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid ftnkey = 0 \n", __LINE__);
            rv = 104;
            goto  EXIT_LABEL;
        }
    }
       else if (ucLspType == 1)
    {
        if (stCrLspKey.egress == 0)
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid crlspkey = 0 \n", __LINE__);
            rv = 104;
            goto  EXIT_LABEL;
        }
    }
    else
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addmcid indata error \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    peerkey.peer = stFtnKey.fec;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        peerftn = (PEERFTN *)NBB_MM_ALLOC(sizeof(PEERFTN), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_PEER_CB);
        if(NULL == peerftn)
        {
            OS_PRINTF("Line=%d peerftn malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(peerftn, 0, sizeof(PEERFTN));
        AVLL_INIT_NODE(peerftn->pe_node); 
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(peerftn->key), &peerkey, sizeof(PEIP_KEY));
        
        AVLL_INIT_TREE(peerftn->vp_tree,
                            spm_l3_pwvccomp,
                            NBB_OFFSETOF(PWVC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->vrf_tree,
                            spm_l3_pwvrfcomp,
                            NBB_OFFSETOF(PWVRF, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVRF, vrf_node));/*lint !e413 */

        AVLL_INIT_TREE(peerftn->mcid_tree,
                            spm_l3_pwmcomp,
                            NBB_OFFSETOF(PWMC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
        
        peerftn->mcnum = 1;
        
        //coverity[no_effect_test]
        AVLL_INSERT( SHARED.peftn_tree, peerftn->pe_node);

        OS_MEMSET(&pwmckey, 0, sizeof(pwmckey));
        pwmckey.vplsid = usVplsId;
        pwmckey.mcid = usMcId;
        pwmc = (PWMC *)NBB_MM_ALLOC(sizeof(PWMC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_MC_CB);
        if(NULL == pwmc)
        {
            OS_PRINTF("Line=%d pwmc malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(pwmc, 0, sizeof(PWMC));
        AVLL_INIT_NODE(pwmc->mc_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(pwmc->key), &pwmckey, sizeof(PWMC_KEY));
        pwmc->port = portindex;
        AVLL_INSERT( peerftn->mcid_tree, pwmc->mc_node);
        
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addmcid peer.mc =%x.%d \n", 
                                    __LINE__, stFtnKey.fec,usMcId);
                                    
    }
    else
    {
        OS_MEMSET(&pwmckey, 0, sizeof(PWMC_KEY));
        pwmckey.vplsid = usVplsId;
        pwmckey.mcid = usMcId;
        pwmc =  (PWMC *)AVLL_FIND( peerftn->mcid_tree,  &pwmckey );
        if( pwmc == NULL )
        {
            pwmc = (PWMC *)NBB_MM_ALLOC(sizeof(PWMC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_MC_CB);
            if(NULL == pwmc)
            {
                OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
                rv = 888;
                goto  EXIT_LABEL;
            }
            NBB_MEMSET(pwmc, 0, sizeof(PWMC));
            AVLL_INIT_NODE(pwmc->mc_node);  
            NBB_MEMCPY( &(pwmc->key), &pwmckey, sizeof(PWMC_KEY));
            pwmc->port = portindex;
            AVLL_INSERT( peerftn->mcid_tree, pwmc->mc_node);

            peerftn->mcnum++;
            
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addmcid peer.mc =%x.%d \n", 
            //                          __LINE__, stFtnKey.fec,usMcId);
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp peer.vp.vc =%x.%x.%x \n", 
            //                          __LINE__, peerip,vpid,vcid);
        }
    }
    rv = 0;
    
    rv = spm_l3_ftn_alloc_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_alloc_cb,rv=%d\n", __LINE__,rv);
    }
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_del_mcid_ldp(NBB_USHORT usVplsId,NBB_USHORT usMcId, NBB_BYTE ucLspType, 
                                                            FTN_KEY stFtnKey ,CRTXLSP_KEY stCrLspKey NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;

    PEIP_KEY        peerkey;

    PEERFTN     *peerftn = NULL;

    PWMC_KEY    pwmckey;
    
    PWMC        *pwmc = NULL;

    NBB_TRC_ENTRY("spm_l3_del_mcid_ldp");
    
    if (usVplsId == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid vplsid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (usMcId == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid mcid = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (ucLspType == 0)
    {
        if (stFtnKey.fec == 0)
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid ftnkey = 0 \n", __LINE__);
            rv = 104;
            goto  EXIT_LABEL;
        }
    }
       else if (ucLspType == 1)
    {
        if (stCrLspKey.egress == 0)
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid crlspkey = 0 \n", __LINE__);
            rv = 104;
            goto  EXIT_LABEL;
        }
    }
    else
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid indata error \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

        
    peerkey.peer = stFtnKey.fec;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 101;
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid peer.mc =%x.%d \n", 
                                    __LINE__, stFtnKey.fec,usMcId);
        goto  EXIT_LABEL;
    }
    else
    {
        OS_MEMSET(&pwmckey, 0, sizeof(PWMC_KEY));
        pwmckey.vplsid = usVplsId;
        pwmckey.mcid = usMcId;
        pwmc =  (PWMC *)AVLL_FIND( peerftn->mcid_tree,  &pwmckey );
        if( pwmc == NULL )
        {
            rv = 102;
            
            //OS_PRINTF("Line=%d error freefrrid mpeer=%x bpeer=%x rv=%d\n",__LINE__, mpeer ,bpeer ,rv);
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid peer.mc =%x.%d \n", 
                                        __LINE__, stFtnKey.fec,usMcId);
            goto  EXIT_LABEL;
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delmcid peer.mc =%x.%d \n", 
            //                          __LINE__, stFtnKey.fec,usMcId);
            AVLL_DELETE( peerftn->mcid_tree , pwmc->mc_node);
            NBB_MM_FREE(pwmc, MEM_SPM_FTN_TAB_MC_CB);
            peerftn->mcnum--;
        }
    }
    
    rv = spm_l3_ftn_free_cb(peerkey.peer,peerftn->vpnum,peerftn->mcnum,peerftn->vrfnum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_ftn_free_cb,rv=%d\n", __LINE__,rv);  
    }
    if(( peerftn->vpnum == 0) && ( peerftn->vrfnum == 0) && ( peerftn->mcnum == 0))
    {
        AVLL_DELETE( SHARED.peftn_tree, peerftn->pe_node);
        NBB_MM_FREE(peerftn, MEM_SPM_FTN_TAB_PEER_CB);
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatemcid( NBB_ULONG peerip,NBB_ULONG newport NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;

    NBB_USHORT  mcid = 0;

    NBB_USHORT  vplsid = 0;

    NBB_ULONG   oldport = 0;
    
    PEIP_KEY        peerkey;

    PEERFTN     *peerftn = NULL;

    PWMC_KEY    pwmckey;
    
    PWMC        *pwmc = NULL;

    NBB_TRC_ENTRY("spm_l3_updatemcid");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    if (newport == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc newport = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    
    peerkey.peer = peerip;
    peerftn =  (PEERFTN *)AVLL_FIND( SHARED.peftn_tree, &peerkey);
    if( peerftn == NULL )
    {
        rv = 0;
        goto  EXIT_LABEL;
    }
    else
    {
        pwmc =  (PWMC *)AVLL_FIRST( peerftn->mcid_tree );
        while (pwmc != NULL)
        {
            oldport = pwmc->port;
            mcid = pwmc->key.mcid;
            vplsid = pwmc->key.vplsid;
            if(( mcid != 0) && ( oldport != 0))
            {
                //rv = spm_vpls_refresh_arad_vp(vplsid,mcid,oldport,newport NBB_CCXT);
                if( rv != 0)   
                {
                    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc peer.mc.old.new =%x.%d.%d.%d \n", 
                                                __LINE__, peerip,mcid,oldport,newport);
                }
                else
                {
                    pwmc->port = newport;
                    
                    //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatemc peer.mc.old.new =%x.%d.%d.%d \n",
                    //                          __LINE__, peerip,mcid,oldport,newport);
                }
            }
            else
            {
                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc peer.mc.old.new =%x.%d.%d.%d \n", 
                                            __LINE__, peerip,mcid,oldport,newport);
            }
            pwmc = (PWMC *)AVLL_NEXT( peerftn->mcid_tree, pwmc->mc_node);
        }
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_addvrfrsvp
 * Author		: 	    wjhe
 * CreateDate		:	2015-12-30
 * Description		:   build relation between rsvp tunnel and vrfid ,peerip 
 * InputParam	:	    peerip,vrfid,tunlkey,tunlid(in driver)
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3  tunnsel module  
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addvrfrsvp( NBB_ULONG peerip, NBB_USHORT vrfid,LSPPROT_KEY *tunlkey,NBB_ULONG tunid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;
	LSPPROT_KEY tunnl_key;
	RSVPCRLSP        *rsvplsp = NULL;
	PWVPN       *pwvpn = NULL;
	TUNNELCH_KEY   tunnselkey;
	NBB_BOOL         avll_ret_code;

    NBB_TRC_ENTRY("spm_l3_addvrfrsvp");

    if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_addvrfrsvp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
    if (vrfid == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_addvrfrsvp vrf = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
	if (tunlkey == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_addvrfrsvp tunnlkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, tunlkey);
    if( rsvplsp == NULL )
    {
        rsvplsp = (RSVPCRLSP *)NBB_MM_ALLOC(sizeof(RSVPCRLSP), NBB_NORETRY_ACT, MEM_SPM_CR_LSP_TAB_RSVP_CB);
        if(NULL == rsvplsp)
        {
            OS_PRINTF("Line=%d rsvplsp malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(rsvplsp, 0, sizeof(RSVPCRLSP));
        AVLL_INIT_NODE(rsvplsp->rsvp_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(rsvplsp->key), tunlkey, sizeof(LSPPROT_KEY));
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d %x.%x.%x \n", __LINE__,rsvplsp->key.ingress,
			                                                  rsvplsp->key.egress,rsvplsp->key.tunnelid);
        AVLL_INIT_TREE(rsvplsp->vp_tree,
                            spm_l3_pwvccomp,
                            NBB_OFFSETOF(PWVC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */

        AVLL_INIT_TREE(rsvplsp->vrfpeer_tree,
                            spm_l3_tunlselkeycomp,
                            NBB_OFFSETOF(PWVPN, key),/*lint !e413 */
                            NBB_OFFSETOF(PWVPN, vrf_node));/*lint !e413 */

        AVLL_INIT_TREE(rsvplsp->mcid_tree,
                            spm_l3_pwmcomp,
                            NBB_OFFSETOF(PWMC, key),/*lint !e413 */
                            NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
        rsvplsp->vrfpeernum = 1;
        rsvplsp->tunlid = tunid;
        
        //coverity[no_effect_test]
        avll_ret_code = AVLL_INSERT( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
        NBB_ASSERT(avll_ret_code);
		OS_MEMSET(&tunnselkey, 0, sizeof(TUNNELCH_KEY));
        tunnselkey.vrfid = vrfid;
		tunnselkey.peerip = peerip;
        pwvpn = (PWVPN *)NBB_MM_ALLOC(sizeof(PWVPN), NBB_NORETRY_ACT, MEM_SPM_CR_LSP_TAB_VPN_CB);
        if(NULL == pwvpn)
        {
            OS_PRINTF("Line=%d pwvpn malloc mem NULL \n",__LINE__);
            rv = 888;
            goto  EXIT_LABEL;
        }
        NBB_MEMSET(pwvpn, 0, sizeof(PWVPN));
        AVLL_INIT_NODE(pwvpn->vrf_node);  
        
        //coverity[bad_sizeof]
        NBB_MEMCPY( &(pwvpn->key), &tunnselkey, sizeof(TUNNELCH_KEY));

        avll_ret_code = AVLL_INSERT( rsvplsp->vrfpeer_tree, pwvpn->vrf_node);
        NBB_ASSERT(avll_ret_code);
    }
    else
    {
        OS_MEMSET(&tunnselkey, 0, sizeof(TUNNELCH_KEY));
        tunnselkey.vrfid = vrfid;
		tunnselkey.peerip = peerip;
        pwvpn =  (PWVPN *)AVLL_FIND( rsvplsp->vrfpeer_tree,  &tunnselkey );
        if( pwvpn == NULL )
        {
            pwvpn = (PWVPN *)NBB_MM_ALLOC(sizeof(PWVPN), NBB_NORETRY_ACT, MEM_SPM_CR_LSP_TAB_VPN_CB);
            if(NULL == pwvpn)
            {
                OS_PRINTF("Line=%d pwvpn malloc mem NULL \n",__LINE__);
                rv = 888;
                goto  EXIT_LABEL;
            }
            NBB_MEMSET(pwvpn, 0, sizeof(PWVPN));
            AVLL_INIT_NODE(pwvpn->vrf_node);  
            NBB_MEMCPY( &(pwvpn->key), &tunnselkey, sizeof(TUNNELCH_KEY));

            avll_ret_code = AVLL_INSERT( rsvplsp->vrfpeer_tree, pwvpn->vrf_node);
            NBB_ASSERT(avll_ret_code);
            rsvplsp->vrfpeernum++;
            
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            
        }
        else
        {
            //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvrfldp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
        }
    }

    rv = spm_l3_lspprot_alloc_cb(tunlkey,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_alloc_cb,rv=%d\n", __LINE__,rv);
    }
    
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_delvrfrsvp
 * Author		: 	    wjhe
 * CreateDate		:	2015-12-31
 * Description		:   del relation :vrfid peeip and tunnel 
 * InputParam	:	    peerip,vrfid,tunlkey
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3 tunsel module   
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_delvrfrsvp( NBB_ULONG peerip, NBB_USHORT vrfid,LSPPROT_KEY *tunlkey NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;
	PWVPN       *pwvpn = NULL;
	RSVPCRLSP   *rsvplsp = NULL;
	TUNNELCH_KEY   tunnselkey;

    NBB_TRC_ENTRY("spm_l3_delvrfldp");
    
    if (tunlkey == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfrsvp tunlkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

	if (peerip == 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfrsvp peerip = 0 \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }
      
    rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, tunlkey);
    if( rsvplsp == NULL )
    {
        rv = 101;
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfrsvp ing.egr.tunl =%x.%x.%x \n", __LINE__, tunlkey->ingress,
			tunlkey->egress,tunlkey->tunnelid);
        goto  EXIT_LABEL;
    }
    else
    {
        OS_MEMSET(&tunnselkey, 0, sizeof(TUNNELCH_KEY));
        tunnselkey.vrfid = vrfid;
		tunnselkey.peerip = peerip;
        pwvpn =  (PWVPN *)AVLL_FIND( rsvplsp->vrfpeer_tree,  &tunnselkey );
        if( pwvpn == NULL )
        {
            rv = 102;
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvrfrsvp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            goto  EXIT_LABEL;
        }
        else
        {
            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvrfrsvp vrf.pe =%x.%x \n", __LINE__, vrfid,peerip);
            AVLL_DELETE( rsvplsp->vrfpeer_tree , pwvpn->vrf_node);
            NBB_MM_FREE(pwvpn, MEM_SPM_CR_LSP_TAB_VPN_CB);
            rsvplsp->vrfpeernum--;
        }
    }

    rv = spm_l3_lspprot_free_cb(tunlkey,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_free_cb,rv=%d\n", __LINE__,rv);
    }
    
    if(( rsvplsp->vpnum == 0) && ( rsvplsp->vrfpeernum == 0) && ( rsvplsp->mcnum == 0))
    {
        AVLL_DELETE( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
        NBB_MM_FREE(rsvplsp, MEM_SPM_CR_LSP_TAB_RSVP_CB);
    }
    
    EXIT_LABEL:

    NBB_TRC_EXIT();

    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_updatevrf_rsvp
 * Author		: 	    wjhe	
 * CreateDate		:	2015-12
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatevrf_rsvp( LSPPROT_KEY *tunlkey,NBB_ULONG tunid NBB_CCXT_T NBB_CXT)
{
    NBB_INT     rv = 1;
    NBB_INT     rv2 = 1;
    TUNNELCH_KEY        tunnelskey;
    TUNNELCH            *tunnels = NULL;
    PWTXNODE            *pwnodes = NULL;
    SHASH_NODE          *node = NULL;
    L3_TX_PW_T          txpw;
    L3_TX_PW_T          temp_txpw;
	RSVPCRLSP           *rsvplsp = NULL;
	PWVPN               *pwvpn = NULL;

    NBB_TRC_ENTRY("updatevrf_rsvp");

    if (tunlkey == NULL)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf_rsvp tunlkey = NULL \n", __LINE__);
        rv = 104;
        goto  EXIT_LABEL;
    }

    rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, tunlkey);
    if( rsvplsp == NULL )
    {
        rv = 0;
        goto  EXIT_LABEL;
    }
    else
    {
        pwvpn =  (PWVPN *)AVLL_FIRST( rsvplsp->vrfpeer_tree );
        
        //OS_PRINTF("update L2 vp peer.tun =%x.%x \n", __LINE__, peerip,tunid);
        rsvplsp->tunlid = tunid;
        while (pwvpn != NULL)
        {
        #ifdef SPU

            tunnelskey.res = 0;
            tunnelskey.vrfid = pwvpn->key.vrfid;
            tunnelskey.peerip = pwvpn->key.peerip;
            node =  (SHASH_NODE *)spm_hashtab_search( SHARED.tunnelchhash, &tunnelskey NBB_CCXT); 
            if ((node == NULL) || (node->data == NULL))
            {
                pwvpn = (PWVPN *)AVLL_NEXT( rsvplsp->vrfpeer_tree , pwvpn->vrf_node);
                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf_rsvp null vrf.pe =%x.%x \n", __LINE__, 
                                                   tunnelskey.vrfid,tunnelskey.peerip);
                continue;
            }
            else
            {
                tunnels = (TUNNELCH *)(node->data);
                tunnels->tunindex = tunid;
            }
            pwnodes = (PWTXNODE *)AVLL_FIRST(tunnels->pwtree);
            while (pwnodes != NULL)
            {
                if( pwnodes->posid != 0 ) 
                {
                    OS_MEMSET(&txpw , 0, sizeof(L3_TX_PW_T));
                    OS_MEMSET(&temp_txpw , 0, sizeof(L3_TX_PW_T));
                    txpw.mVrf = tunnelskey.vrfid;
                    txpw.mPeerIp = tunnelskey.peerip;
                    txpw.mLabel = pwnodes->key.label;
                    
                    txpw.posId = pwnodes->posid;
                    txpw.tunnelIdx = tunid; 
                    if( tunnelskey.vrfid != 0)
                    {
                        txpw.pushPwNum = 1;
                    }

                    rv = fhdrv_psn_l3_get_txpw(BCMC31, &txpw, &temp_txpw);
                    if (rv == 0)
                    {
                        txpw.eNniMode = temp_txpw.eNniMode;
                    }
                    else
                    {
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, 
                                "Line=%d e updatevrf fhdrv_psn_l3_get_txpw posid.eNniMode =%d.%d\n", 
                                        __LINE__, txpw.posId, txpw.eNniMode);
                    }
                    
                    //txpw.flags |= L3_TX_PW_COUNTER_ON;
                    rv = ApiC3ModL3TxPw( BCMC31, &txpw);
                    if ( SHARED.c3_num == 2 )
                    {
                        rv2 = ApiC3ModL3TxPw( BCMC32, &txpw);
                        rv = (rv|rv2);
                    }
                    if( rv != 0) 
                    {
                        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatevrf vrf.pe.lab =%x.%x.%x \n", 
                                                    __LINE__, tunnelskey.vrfid,tunnelskey.peerip,pwnodes->key.label);
                    }
                    else
                    {
                        //BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatevrf vrf.pe.lab =%x.%x.%x \n", 
                        //                          __LINE__, tunnelskey.vrfid,peerip,pwnodes->key.label);
                    }
                }
                pwnodes = (PWTXNODE *)AVLL_NEXT( tunnels->pwtree, pwnodes->pw_node);
            }
        #endif
            pwvpn = (PWVPN *)AVLL_NEXT(rsvplsp->vrfpeer_tree, pwvpn->vrf_node);
        }
    }
    rv = 0;
    
    EXIT_LABEL:
    
    NBB_TRC_EXIT();
    
    return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_addvprsvp
 * Author		: 	    wjhe
 * CreateDate		:	2015-12-31
 * Description		:   build relation  lsp ,l2vp
 * InputParam	:	    vpinfo,vpid,vcid,tunnel key, tunlid (in driver)
 * OutputParam	:	    no
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_addvprsvp( SPM_VP_INFO_REFRESH_CB *vpfo, NBB_ULONG vpid,NBB_ULONG vcid,
                                                               LSPPROT_KEY *key, NBB_ULONG tunlid NBB_CCXT_T NBB_CXT)
{
    NBB_INT		     rv = 1;
	PWVC_KEY	     pwvckey;
	PWVC		    *pwvc = NULL;
	RSVPCRLSP        *rsvplsp = NULL;
	NBB_BOOL         avll_ret_code;

	NBB_TRC_ENTRY("spm_l3_addvprsvp");


	if (vpfo == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvprsvp vpfo = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (key == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvprsvp key = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (vpid == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvprsvp vpid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (vcid == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvprsvp vcid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, key);
	if( rsvplsp == NULL )
	{
		rsvplsp = (RSVPCRLSP *)NBB_MM_ALLOC(sizeof(RSVPCRLSP), NBB_NORETRY_ACT, MEM_SPM_CR_LSP_TAB_RSVP_CB);
		if(NULL == rsvplsp)
		{
			OS_PRINTF("Line=%d rsvplsp malloc mem NULL \n",__LINE__);
			rv = 888;
			goto  EXIT_LABEL;
		}
		NBB_MEMSET(rsvplsp, 0, sizeof(RSVPCRLSP));
		AVLL_INIT_NODE(rsvplsp->rsvp_node);  
		NBB_MEMCPY( &(rsvplsp->key), key, sizeof(LSPPROT_KEY));
		
		AVLL_INIT_TREE(rsvplsp->vp_tree,
		             		spm_l3_pwvccomp,
		             		NBB_OFFSETOF(PWVC, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */

		AVLL_INIT_TREE(rsvplsp->vrfpeer_tree,
		             		spm_l3_tunlselkeycomp,
		             		NBB_OFFSETOF(PWVPN, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWVPN, vrf_node));/*lint !e413 */

		AVLL_INIT_TREE(rsvplsp->mcid_tree,
		             		spm_l3_pwmcomp,
		             		NBB_OFFSETOF(PWMC, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
		
		rsvplsp->vpnum = 1;
		rsvplsp->tunlid = tunlid;
		avll_ret_code = AVLL_INSERT( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
		NBB_ASSERT(avll_ret_code);

		OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
		pwvckey.vpid = vpid;
		pwvckey.vcid = vcid;
		pwvc = (PWVC *)NBB_MM_ALLOC(sizeof(PWVC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VC_CB);
		if(NULL == pwvc)
		{
			OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
			rv = 888;
			goto  EXIT_LABEL;
		}
		NBB_MEMSET(pwvc, 0, sizeof(PWVC));
		AVLL_INIT_NODE(pwvc->vc_node);  
		NBB_MEMCPY( &(pwvc->key), &pwvckey, sizeof(PWVC_KEY));
		avll_ret_code = AVLL_INSERT( rsvplsp->vp_tree, pwvc->vc_node);
		NBB_ASSERT(avll_ret_code);

		/*保存L2 VP 信息*/
		NBB_MEMCPY( &(pwvc->l2vp), vpfo, sizeof(SPM_VP_INFO_REFRESH_CB));	
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvprsvp lspkey.vp.vc =%lx.%lx.%lx.%lx.%lx \n", __LINE__, 
				                                        key->ingress,key->egress,key->tunnelid,vpid,vcid);
	}
	else
	{
		OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
		pwvckey.vpid = vpid;
		pwvckey.vcid = vcid;
		pwvc =  (PWVC *)AVLL_FIND( rsvplsp->vp_tree,  &pwvckey );
		if( pwvc == NULL )
		{
			pwvc = (PWVC *)NBB_MM_ALLOC(sizeof(PWVC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_VC_CB);
			if(NULL == pwvc)
			{
				OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
				rv = 888;
				goto  EXIT_LABEL;
			}
			NBB_MEMSET(pwvc, 0, sizeof(PWVC));
			AVLL_INIT_NODE(pwvc->vc_node);  
			NBB_MEMCPY( &(pwvc->key), &pwvckey, sizeof(PWVC_KEY));
			avll_ret_code = AVLL_INSERT( rsvplsp->vp_tree, pwvc->vc_node);
            NBB_ASSERT(avll_ret_code);
			rsvplsp->vpnum++;

			NBB_MEMCPY( &(pwvc->l2vp), vpfo, sizeof(SPM_VP_INFO_REFRESH_CB));
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addvprsvp lspkey.vp.vc =%lx.%lx.%lx.%lx.%lx \n", __LINE__, 
				                                        key->ingress,key->egress,key->tunnelid,vpid,vcid);
			
		}
		else
		{
			//BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e addvpldp peer.vp.vc =%x.%x.%x \n", __LINE__, peerip,vpid,vcid);
		}
	}

    rv = spm_l3_lspprot_alloc_cb(key,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_alloc_cb,rv=%d\n", __LINE__,rv);  
    }
	rv = 0;

    
	EXIT_LABEL:
	
	NBB_TRC_EXIT();
	
	return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_delvprsvp
 * Author		: 	    wjhe
 * CreateDate		:	2015-12-31
 * Description		:   remove relation  lsp ,l2vp
 * InputParam	:	    
 * OutputParam	:	    L3 info
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_delvprsvp( LSPPROT_KEY *key , NBB_ULONG vpid,NBB_ULONG vcid NBB_CCXT_T NBB_CXT)
{
    NBB_INT		    rv = 1;
	PWVC_KEY	    pwvckey;
	RSVPCRLSP       *rsvplsp = NULL;
	PWVC		    *pwvc = NULL;

	NBB_TRC_ENTRY("spm_l3_delvprsvp");
	
	if (key == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvprsvp key = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (vpid == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvprsvp vpid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (vcid == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delvprsvp vcid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
		
	rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, key);
	if( rsvplsp == NULL )
	{
		rv = 101;
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvprsvp lspkey.vp.vc =%x.%x.%x.%x.%x \n", __LINE__, 
				                                  key->ingress,key->egress,key->tunnelid,vpid,vcid);
		goto  EXIT_LABEL;
	}
	else
	{
		OS_MEMSET(&pwvckey, 0, sizeof(PWVC_KEY));
		pwvckey.vpid = vpid;
		pwvckey.vcid = vcid;
		pwvc =  (PWVC *)AVLL_FIND( rsvplsp->vp_tree,  &pwvckey );
		if( pwvc == NULL )
		{
			rv = 102;
			
			//OS_PRINTF("Line=%d error freefrrid mpeer=%x bpeer=%x rv=%d\n",__LINE__, mpeer ,bpeer ,rv);
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvprsvp lspkey.vp.vc =%x.%x.%x.%x.%x \n", __LINE__, 
				                                  key->ingress,key->egress,key->tunnelid,vpid,vcid);
			goto  EXIT_LABEL;
		}
		else
		{
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delvprsvp lspkey.vp.vc =%x.%x.%x.%x.%x \n", __LINE__, 
				                                  key->ingress,key->egress,key->tunnelid,vpid,vcid);
			AVLL_DELETE( rsvplsp->vp_tree , pwvc->vc_node);
			NBB_MM_FREE(pwvc, MEM_SPM_FTN_TAB_VC_CB);
			rsvplsp->vpnum--;
		}
	}

    rv = spm_l3_lspprot_free_cb(key,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_free_cb,rv=%d\n", __LINE__,rv);
    }
	if(( rsvplsp->vpnum == 0) && ( rsvplsp->vrfpeernum == 0) && ( rsvplsp->mcnum == 0))
	{
		AVLL_DELETE( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
		NBB_MM_FREE(rsvplsp, MEM_SPM_CR_LSP_TAB_RSVP_CB);
	} 
	EXIT_LABEL:

	NBB_TRC_EXIT();

	return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_updatevp_rsvp
 * Author		: 	    wjhe
 * CreateDate		:	2015-12-31
 * Description		:   update lsp ,l2vp
 * InputParam	:	    
 * OutputParam	:	    L3 info
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatevp_rsvp( LSPPROT_KEY *key,NBB_ULONG portindex,NBB_ULONG tunlid NBB_CCXT_T NBB_CXT)
{
    NBB_INT		    rv = 1;
	NBB_INT         rv2 = 1;
	NBB_ULONG 	    vpid;
	NBB_ULONG	    vcid;
	LSPPROT_KEY		tunlkey;
	RSVPCRLSP       *rsvplsp = NULL;
	PWVC		    *pwvc = NULL;
	SPM_VP_INFO_REFRESH_CB	vpinfo;

	NBB_TRC_ENTRY("spm_l3_updatevp_rsvp");

	if (key == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_updatevp_rsvp  key = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}

	OS_MEMSET(&vpinfo, 0, sizeof(SPM_VP_INFO_REFRESH_CB));
	OS_MEMSET(&tunlkey, 0, sizeof(LSPPROT_KEY));
	tunlkey.ingress = key->ingress;
	tunlkey.egress = key->egress;
	tunlkey.tunnelid = key->tunnelid;
	rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, &tunlkey);
	if( rsvplsp == NULL )
	{
		rv = 0;
		goto  EXIT_LABEL;
	}
	else
	{
		pwvc =  (PWVC *)AVLL_FIRST( rsvplsp->vp_tree );
		rsvplsp->tunlid = tunlid;
		while (pwvc != NULL)
		{
			vpid = pwvc->key.vpid;
			vcid = pwvc->key.vcid;
			if(( vpid != 0) && ( vcid != 0))
			{
                #ifdef SPU
                rv = ApiC3SetVcTunnel( BCMC31, vpid, vcid, tunlid);
                if ( SHARED.c3_num == 2 )
                {
                    rv2 = ApiC3SetVcTunnel( BCMC32, vpid, vcid, tunlid);
                    rv = (rv|rv2);
                }
            #else
                rv = 0;
            #endif
				OS_MEMCPY(&vpinfo, &(pwvc->l2vp), sizeof(SPM_VP_INFO_REFRESH_CB));
				BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o refresh lspkey.vp.vc =%x.%x.%x.%d.%x.%x \n", __LINE__, 
				                                     key->ingress,key->egress,key->tunnelid,tunlid,vpid,vcid);
				spm_vpn_refresh_vc_nni_cfg( &vpinfo, portindex NBB_CCXT);
			}
			else
			{
				BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatevprsvp lspkey.vp.vc =%x.%x.%x.%x.%x \n", __LINE__, 
				                                     key->ingress,key->egress,key->tunnelid,vpid,vcid);
			}

			pwvc = (PWVC *)AVLL_NEXT( rsvplsp->vp_tree, pwvc->vc_node);
		}
	}
	rv = 0;
	
	EXIT_LABEL:
	
	NBB_TRC_EXIT();
	
	return rv;    
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_add_mcid_rsvp
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-24
 * Description		:   build relation  lsp ,l2mc
 * InputParam	:	    
 * OutputParam	:	    L3 info
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_add_mcid_rsvp(NBB_USHORT usVplsId,NBB_USHORT usMcId,LSPPROT_KEY *tunnelkey,
                                                          NBB_ULONG portindex NBB_CCXT_T NBB_CXT)
{
    NBB_INT		 rv = 1;
	RSVPCRLSP   *rsvplsp = NULL;
	PWMC_KEY	pwmckey;
	PWMC		*pwmc = NULL;
	NBB_BOOL    avll_ret_code;

	NBB_TRC_ENTRY("spm_l3_add_mcid_rsvp");
    if (usVplsId == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e add_mcid_rsvp usVplsId = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (usMcId == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e add_mcid_rsvp mcid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (tunnelkey == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e add_mcid_rsvp tunnelkey = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	if (portindex == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e add_mcid_rsvp portindex = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	
	rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, tunnelkey);
	if( rsvplsp == NULL )
	{
		rsvplsp = (RSVPCRLSP *)NBB_MM_ALLOC(sizeof(RSVPCRLSP), NBB_NORETRY_ACT, MEM_SPM_CR_LSP_TAB_RSVP_CB);
		if(NULL == rsvplsp)
		{
			OS_PRINTF("Line=%d rsvplsp malloc mem NULL \n",__LINE__);
			rv = 888;
			goto  EXIT_LABEL;
		}
		NBB_MEMSET(rsvplsp, 0, sizeof(RSVPCRLSP));
		AVLL_INIT_NODE(rsvplsp->rsvp_node);  
		NBB_MEMCPY( &(rsvplsp->key), tunnelkey, sizeof(LSPPROT_KEY));
		
		AVLL_INIT_TREE(rsvplsp->vp_tree,
		             		spm_l3_pwvccomp,
		             		NBB_OFFSETOF(PWVC, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWVC, vc_node));/*lint !e413 */
        
        AVLL_INIT_TREE(rsvplsp->vrfpeer_tree,
		             		spm_l3_tunlselkeycomp,
		             		NBB_OFFSETOF(PWVPN, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWVPN, vrf_node));/*lint !e413 */
		

	    AVLL_INIT_TREE(rsvplsp->mcid_tree,
		             		spm_l3_pwmcomp,
		             		NBB_OFFSETOF(PWMC, key),/*lint !e413 */
		             		NBB_OFFSETOF(PWMC, mc_node));/*lint !e413 */
		
		rsvplsp->mcnum = 1;
		avll_ret_code = AVLL_INSERT( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
        NBB_ASSERT(avll_ret_code);
		OS_MEMSET(&pwmckey, 0, sizeof(PWMC_KEY));
		pwmckey.mcid = usMcId;
		pwmc = (PWMC *)NBB_MM_ALLOC(sizeof(PWMC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_MC_CB);
		if(NULL == pwmc)
		{
			OS_PRINTF("Line=%d pwmc malloc mem NULL \n",__LINE__);
			rv = 888;
			goto  EXIT_LABEL;
		}
		NBB_MEMSET(pwmc, 0, sizeof(PWMC));
		AVLL_INIT_NODE(pwmc->mc_node);  
		NBB_MEMCPY( &(pwmc->key), &pwmckey, sizeof(PWMC_KEY));
		pwmc->port = portindex;
		avll_ret_code = AVLL_INSERT( rsvplsp->mcid_tree, pwmc->mc_node);
		NBB_ASSERT(avll_ret_code);
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
	}
	else
	{
		OS_MEMSET(&pwmckey, 0, sizeof(PWMC_KEY));
		pwmckey.mcid = usMcId;
		pwmc =  (PWMC *)AVLL_FIND( rsvplsp->mcid_tree,  &pwmckey );
		if( pwmc == NULL )
		{
			pwmc = (PWMC *)NBB_MM_ALLOC(sizeof(PWMC), NBB_NORETRY_ACT, MEM_SPM_FTN_TAB_MC_CB);
			if(NULL == pwmc)
			{
				OS_PRINTF("Line=%d pwvc malloc mem NULL \n",__LINE__);
				rv = 888;
				goto  EXIT_LABEL;
			}
			NBB_MEMSET(pwmc, 0, sizeof(PWMC));
			AVLL_INIT_NODE(pwmc->mc_node);  
			NBB_MEMCPY( &(pwmc->key), &pwmckey, sizeof(PWMC_KEY));
			pwmc->port = portindex;
			avll_ret_code = AVLL_INSERT( rsvplsp->mcid_tree, pwmc->mc_node);
            NBB_ASSERT(avll_ret_code);
			rsvplsp->mcnum++;
			
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
		}
		else
		{
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o addmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
		}
	}

    rv = spm_l3_lspprot_alloc_cb(tunnelkey,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_alloc_cb,rv=%d\n", __LINE__,rv);
    }
	rv = 0;
	
	EXIT_LABEL:
	
	NBB_TRC_EXIT();
	
	return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_del_mcid_rsvp
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-24
 * Description		:   remove relation  lsp ,l2mc
 * InputParam	:	    mcid,tunnel key
 * OutputParam	:	    no
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_del_mcid_rsvp(NBB_USHORT usMcId, LSPPROT_KEY *tunnelkey NBB_CCXT_T NBB_CXT)
{
    NBB_INT		rv = 1;
    RSVPCRLSP   *rsvplsp = NULL;
	PWMC_KEY	pwmckey;	
	PWMC		*pwmc = NULL;

	NBB_TRC_ENTRY("spm_l3_del_mcid_ldp");
	
	if (usMcId == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid mcid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}	
	if (tunnelkey == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid mcid = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}	
	rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, tunnelkey);
	if( rsvplsp == NULL )
	{
		rv = 101;
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
		goto  EXIT_LABEL;
	}
	else
	{
		OS_MEMSET(&pwmckey, 0, sizeof(PWMC_KEY));
		pwmckey.mcid = usMcId;
		pwmc =  (PWMC *)AVLL_FIND( rsvplsp->mcid_tree,  &pwmckey );
		if( pwmc == NULL )
		{
			rv = 102;
			
			//OS_PRINTF("Line=%d error freefrrid mpeer=%x bpeer=%x rv=%d\n",__LINE__, mpeer ,bpeer ,rv);
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e delmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
			goto  EXIT_LABEL;
		}
		else
		{
			BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o delmcid lspkey.mc =%lx.%lx.%lx.%d \n", __LINE__,
			                                tunnelkey->ingress,tunnelkey->egress,tunnelkey->tunnelid,usMcId);
			AVLL_DELETE( rsvplsp->mcid_tree , pwmc->mc_node);
			NBB_MM_FREE(pwmc, MEM_SPM_FTN_TAB_MC_CB);
			rsvplsp->mcnum--;
		}
	}

    rv = spm_l3_lspprot_free_cb(tunnelkey,rsvplsp->vpnum,rsvplsp->mcnum,rsvplsp->vrfpeernum);
    if(rv != 0)
    {
        BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e spm_l3_lspprot_free_cb,rv=%d\n", __LINE__,rv);
    }
	if(( rsvplsp->vpnum == 0) && ( rsvplsp->vrfpeernum == 0) && ( rsvplsp->mcnum == 0))
	{
		AVLL_DELETE( SHARED.rsvplsp_tree, rsvplsp->rsvp_node);
		NBB_MM_FREE(rsvplsp, MEM_SPM_CR_LSP_TAB_RSVP_CB );
	}
	
	EXIT_LABEL:

	NBB_TRC_EXIT();

	return rv;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_updatemcid_rsvp
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-24
 * Description		:   update  l2 mc rsvp (port)
 * InputParam	:	    tunnel key ,portindex
 * OutputParam	:	    L3 info
 * ReturnValue	:	    0 sucess ,1 failed
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_updatemcid_rsvp(LSPPROT_KEY *key,NBB_ULONG newport NBB_CCXT_T NBB_CXT)
{
    NBB_INT		   rv = 1;
	NBB_USHORT	   mcid = 0;
	NBB_ULONG	   oldport = 0;
    NBB_USHORT     vplsid = 0;
	PEIP_KEY	   peerkey;
	RSVPCRLSP      *rsvplsp = NULL;
	PWMC		   *pwmc = NULL;

	NBB_TRC_ENTRY("updatemcid_rsvp");

	if (key == NULL)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc key = NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	
	if (newport == 0)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc newport = 0 \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
	
    rsvplsp =  (RSVPCRLSP *)AVLL_FIND( SHARED.rsvplsp_tree, key);
	if( rsvplsp == NULL )
	{
		rv = 0;
		goto  EXIT_LABEL;
	}
	else
	{
		pwmc =  (PWMC *)AVLL_FIRST( rsvplsp->mcid_tree );
		while (pwmc != NULL)
		{
			oldport = pwmc->port;
			mcid = pwmc->key.mcid;
            vplsid = pwmc->key.vplsid;
			if(( mcid != 0) && ( oldport != 0))
			{
				//rv = spm_vpls_refresh_arad_vp(vplsid, mcid,oldport,newport NBB_CCXT);
				if( rv != 0)   
				{
					BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc peer.mc.old.new =.%d.%d.%d \n", __LINE__,mcid,oldport,newport);
				}
				else
				{
					pwmc->port = newport;
				 	BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d o updatemc peer.mc.old.new =.%d.%d.%d \n", __LINE__,mcid,oldport,newport);
				}
			}
			else
			{
				BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e updatemc peer.mc.old.new =.%d.%d.%d \n", __LINE__,mcid,oldport,newport);
			}
			pwmc = (PWMC *)AVLL_NEXT( rsvplsp->mcid_tree, pwmc->mc_node);
		}
	}
	rv = 0;
	
	EXIT_LABEL:
	
	NBB_TRC_EXIT();
	
	return rv;
	
}

#endif



#if 1

NBB_INT spm_l3_getid_txpw(NBB_USHORT vrfid, NBB_ULONG peerip, NBB_ULONG label, 
                                            NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
 
    return rv;
}

NBB_INT spm_l3_getid_rxpw(NBB_ULONG label,NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
    
    return rv;
}

NBB_INT spm_l3_getid_rsvptxlsp( NBB_ULONG ingress, NBB_ULONG egress, NBB_ULONG tunnelid, NBB_ULONG lspid,
                                NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
    
    return rv;
}

NBB_INT spm_l3_getid_rsvprxlsp( NBB_ULONG ingress, NBB_ULONG egress, NBB_ULONG tunnelid, NBB_ULONG lspid,
                                NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
    
    return rv;
}

NBB_INT spm_l3_getid_ldptxlsp( NBB_USHORT vrfid, NBB_USHORT mask, NBB_ULONG fec, 
                                                NBB_ULONG nextip,NBB_ULONG nextport,
                                                    NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
    
    return rv;
}

NBB_INT spm_l3_getid_ldprxlsp(NBB_ULONG label,NBB_UINT *posid, NBB_UINT *countid)
{
    NBB_INT     rv = 1;
    NBB_NULL_THREAD_CONTEXT
        
    NBB_GET_THREAD_CONTEXT();
    rv = 0;


    
    EXIT_LABEL:
    
    return rv;
}



#endif



/********************
清除内存
********************/
#if 1


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_ftn(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    FTN_TAB     *uftn = NULL;

    PEERFTN     *peerftn = NULL;

    PWVC        *pwvc = NULL;

    PWVRF       *pwvrf = NULL;

    PWMC        *pwmc = NULL;

    PEEREPTAB   *peerp = NULL;

    ECMPNODE    *ecmpnode = NULL;

    NBB_TRC_ENTRY("spm_l3_free_ftn");

    uftn = (FTN_TAB *)AVLL_FIRST(SHARED.ftn_tree);
    while (uftn != NULL)
    {
            AVLL_DELETE( SHARED.ftn_tree , uftn->ftn_node);
            NBB_MM_FREE(uftn, MEM_SPM_FTN_TAB_NODE_CB);
            uftn = (FTN_TAB *)AVLL_FIRST(SHARED.ftn_tree);
    }

    peerftn =  (PEERFTN *)AVLL_FIRST( SHARED.peftn_tree );
    while (peerftn != NULL)
    {       
        pwvc =  (PWVC *)AVLL_FIRST( peerftn->vp_tree );
        while (pwvc != NULL)
        {
            AVLL_DELETE( peerftn->vp_tree, pwvc->vc_node);
                   NBB_MM_FREE(pwvc, MEM_SPM_FTN_TAB_VC_CB);
                 pwvc =  (PWVC *)AVLL_FIRST( peerftn->vp_tree );
                 
            //pwvc = (PWVC *)AVLL_NEXT( peerftn->vp_tree, pwvc->vc_node);
        }
        
        pwvrf =  (PWVRF *)AVLL_FIRST( peerftn->vrf_tree );
        while (pwvrf != NULL)
        {
            AVLL_DELETE( peerftn->vrf_tree, pwvrf->vrf_node);
                   NBB_MM_FREE(pwvrf, MEM_SPM_FTN_TAB_VRF_CB);
                 pwvrf =  (PWVRF *)AVLL_FIRST( peerftn->vrf_tree );
                 
            //pwvrf = (PWVRF *)AVLL_NEXT( peerftn->vrf_tree, pwvrf->vrf_node);
        }
        
        pwmc =  (PWMC *)AVLL_FIRST( peerftn->mcid_tree );
        while (pwmc != NULL)
        {
            AVLL_DELETE( peerftn->mcid_tree, pwmc->mc_node);
                   NBB_MM_FREE(pwmc, MEM_SPM_FTN_TAB_MC_CB);
                 pwmc =  (PWMC *)AVLL_FIRST( peerftn->mcid_tree );
                 
            //pwmc = (PWMC *)AVLL_NEXT( peerftn->mcid_tree, pwmc->mc_node);
        }
        
            AVLL_DELETE( SHARED.peftn_tree , peerftn->pe_node);
            NBB_MM_FREE(peerftn, MEM_SPM_FTN_TAB_PEER_CB);
          peerftn =  (PEERFTN *)AVLL_FIRST( SHARED.peftn_tree );
          
        //peerftn = (PEERFTN *)AVLL_NEXT( SHARED.peftn_tree, peerftn->pe_node);
    }

    peerp =  (PEEREPTAB *)AVLL_FIRST( SHARED.peecmp_tree );
    while (peerp != NULL)
    {
        ecmpnode =  (ECMPNODE *)AVLL_FIRST( peerp->ecmpnode_tree);
        while (ecmpnode != NULL)
        {
                    AVLL_DELETE( peerp->ecmpnode_tree, ecmpnode->en_node);
                    NBB_MM_FREE(ecmpnode, MEM_SPM_FTN_TAB_ECMPNODE_CB);
                    ecmpnode =  (ECMPNODE *)AVLL_FIRST( peerp->ecmpnode_tree);
                    
            //ecmpnode = (ECMPNODE *)AVLL_NEXT( peerp->ecmpnode_tree, ecmpnode->en_node);
        }
            AVLL_DELETE( SHARED.peecmp_tree, peerp->pp_node);
            NBB_MM_FREE(peerp, MEM_SPM_FTN_TAB_PEECMP_CB);
            peerp =  (PEEREPTAB *)AVLL_FIRST( SHARED.peecmp_tree );
            
            //peerp = (PEEREPTAB *)AVLL_NEXT( SHARED.peecmp_tree, peerp->pp_node);
    }



    NBB_TRC_EXIT();

    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_ilm(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    ILM_TAB     *ilms = NULL;
    SHASH_NODE  *node = NULL;
    NBB_VOID       *data = NULL;
    NBB_ULONG   i = 0;

    NBB_TRC_ENTRY("spm_l3_free_ilm");

    if( SHARED.ilmhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    
    //NBB_VERIFY_MEMORY(SHARED.ilmhash,sizeof(SHASH_TABLE),MEM_SPM_HASH_TABLE_CB);
    //NBB_VERIFY_MEMORY(SHARED.ilmhash->index,sizeof(SHASH_TABLE *)*16000,MEM_SPM_HASH_INDEX_CB);
    for (i = 0; i < 16000; i++)
    {
        node = SHARED.ilmhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                   ilms = (ILM_TAB *)(node->data);
                          data = spm_hashtab_del( SHARED.ilmhash, &(ilms->key) NBB_CCXT);
                          if(data == NULL)
                           {
                                BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e freeilm lab=%x \n", 
                                                            __LINE__, ilms->key.inlabel);
                           }
                           else
                           {                               
                                NBB_MM_FREE( ilms, MEM_SPM_ILM_TAB_CB);
                           }
            }
            node = SHARED.ilmhash->index[i];
            
            //node = node->next;
        }   
    }
    
    EXIT_LABEL:
    NBB_TRC_EXIT();

    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_crlsprx(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  
    CRRXLSP     *rxlsp = NULL;
    SHASH_NODE  *node = NULL;
    NBB_VOID       *data = NULL;
    NBB_ULONG   i = 0;

    NBB_TRC_ENTRY("spm_l3_free_crlsprx");

    if( SHARED.crlsprxhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    
    //NBB_VERIFY_MEMORY(SHARED.crlsprxhash,sizeof(SHASH_TABLE),MEM_SPM_HASH_TABLE_CB);
    //NBB_VERIFY_MEMORY(SHARED.crlsprxhash->index,sizeof(SHASH_TABLE *)*16000,MEM_SPM_HASH_INDEX_CB);
    for (i = 0; i < 16000; i++)
    {
        node = SHARED.crlsprxhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                        rxlsp = (CRRXLSP *)(node->data);
                        data = spm_hashtab_del( SHARED.crlsprxhash, &(rxlsp->key) NBB_CCXT);
                        if(data == NULL)
                        {
                            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e freecrlsprx in.gr.tun.lsp=%x.%x.%x.%x \n",
                            __LINE__, rxlsp->key.ingress,rxlsp->key.egress,rxlsp->key.tunnelid,rxlsp->key.lspid);
                        }
                        else
                        {                               
                            NBB_MM_FREE( rxlsp, MEM_SPM_CR_LSP_RX_CB);
                        }
            }
            node = SHARED.crlsprxhash->index[i];
            
            //node = node->next;
        }   
    }
    
    EXIT_LABEL:
    NBB_TRC_EXIT();

    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_crlsptx(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    CRTXLSP     *txlsp = NULL;
    SHASH_NODE  *node = NULL;
    NBB_VOID       *data = NULL;
    NBB_ULONG   i = 0;

    NBB_TRC_ENTRY("spm_l3_free_crlsptx");

    if( SHARED.crlsptxhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    
    //NBB_VERIFY_MEMORY(SHARED.crlsptxhash,sizeof(SHASH_TABLE),MEM_SPM_HASH_TABLE_CB);
    //NBB_VERIFY_MEMORY(SHARED.crlsptxhash->index,sizeof(SHASH_TABLE *)*16000,MEM_SPM_HASH_INDEX_CB);
    for (i = 0; i < 16000; i++)
    {
        node = SHARED.crlsptxhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                        txlsp = (CRTXLSP *)(node->data);
                        data = spm_hashtab_del( SHARED.crlsptxhash ,&(txlsp->key) NBB_CCXT);
                        if(data == NULL)
                        {
                            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e freecrlsptx in.gr.tun.lsp=%x.%x.%x.%x \n", 
                            __LINE__, txlsp->key.ingress,txlsp->key.egress,txlsp->key.tunnelid,txlsp->key.lspid);
                        }
                        else
                        {                               
                            NBB_MM_FREE( txlsp, MEM_SPM_CR_LSP_TX_CB);
                        }
            }
               node = SHARED.crlsptxhash->index[i];
               
            //node = node->next;
        }   
    }
    
    EXIT_LABEL:
    NBB_TRC_EXIT();

    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_ucroute(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  
    UNIPORT     *uniports = NULL;
    MPEERFRR    *mpeerfrr = NULL;
    BPEERFRR    *bpeerfrr = NULL;
    ECMPTAB     *ecmp = NULL;
    BFDFRR      *bfdfrrp = NULL;
    IPFRR       *ipfrrp = NULL;
      VRFUROUTE    *uroute = NULL;


    NBB_TRC_ENTRY("spm_l3_free_ucroute");

    uniports =  (UNIPORT *)AVLL_FIRST( SHARED.outuni );
    while (uniports != NULL)
    {
            AVLL_DELETE( SHARED.outuni, uniports->uni_node);
            NBB_MM_FREE(uniports, MEM_SPM_VRFUROUTE_UNI_CB);
            uniports =  (UNIPORT *)AVLL_FIRST( SHARED.outuni );
            
        //uniports = (UNIPORT *)AVLL_NEXT( SHARED.outuni, uniports->uni_node);
    }

    mpeerfrr =  (MPEERFRR *)AVLL_FIRST( SHARED.peer_tree );
    while (mpeerfrr != NULL)
    {       
        bpeerfrr =  (BPEERFRR *)AVLL_FIRST( mpeerfrr->bpeer_tree );
        while (bpeerfrr != NULL)
        {
                   AVLL_DELETE( mpeerfrr->bpeer_tree, bpeerfrr->bp_node);
            NBB_MM_FREE(bpeerfrr, MEM_SPM_VRFUROUTE_BPEER_CB);
            bpeerfrr =  (BPEERFRR *)AVLL_FIRST( mpeerfrr->bpeer_tree );
            
            //bpeerfrr = (BPEERFRR *)AVLL_NEXT( mpeerfrr->bpeer_tree, bpeerfrr->bp_node);
        }
            AVLL_DELETE( SHARED.peer_tree, mpeerfrr->mp_node);
            NBB_MM_FREE(mpeerfrr, MEM_SPM_VRFUROUTE_MPEER_CB);
            mpeerfrr =  (MPEERFRR *)AVLL_FIRST( SHARED.peer_tree );
            
        //mpeerfrr = (MPEERFRR *)AVLL_NEXT( SHARED.peer_tree, mpeerfrr->mp_node);
    }

    ecmp =  (ECMPTAB *)AVLL_FIRST( SHARED.ecmp_tree );
    while (ecmp != NULL)
    {
             AVLL_DELETE( SHARED.ecmp_tree, ecmp->ep_node);
             NBB_MM_FREE(ecmp, MEM_SPM_VRFUROUTE_ECMP_CB);
        ecmp =  (ECMPTAB *)AVLL_FIRST( SHARED.ecmp_tree );
        
        //ecmp = (ECMPTAB *)AVLL_NEXT( SHARED.ecmp_tree, ecmp->ep_node);
    }

    bfdfrrp =  (BFDFRR *)AVLL_FIRST( SHARED.bfdfrr_tree );
    while (bfdfrrp != NULL)
    {       
            AVLL_DELETE( SHARED.bfdfrr_tree, bfdfrrp->bf_node);
            NBB_MM_FREE(bfdfrrp, MEM_SPM_VRFUROUTE_BFDFRR_CB);
        bfdfrrp =  (BFDFRR *)AVLL_FIRST( SHARED.bfdfrr_tree );
        
        //bfdfrrp = (BFDFRR *)AVLL_NEXT( SHARED.bfdfrr_tree, bfdfrrp->bf_node);
    }

    ipfrrp =  (IPFRR *)AVLL_FIRST( SHARED.ipfrr_tree );
    while (ipfrrp != NULL)
    {
            AVLL_DELETE( SHARED.ipfrr_tree, ipfrrp->ipf_node);
            NBB_MM_FREE(ipfrrp, MEM_SPM_VRFUROUTE_IPFRR_CB);
        ipfrrp =  (IPFRR *)AVLL_FIRST( SHARED.ipfrr_tree );
        
        //ipfrrp = (IPFRR *)AVLL_NEXT( SHARED.ipfrr_tree, ipfrrp->ipf_node);
    }

       uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.dcroute_tree);
       while (uroute != NULL)
       {
            AVLL_DELETE( SHARED.dcroute_tree, uroute->route_node);
            spm_l3_memfree(uroute);
            uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.dcroute_tree);
        }

        uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.vrfuroute_tree);
        while (uroute != NULL)
        {
             AVLL_DELETE( SHARED.vrfuroute_tree, uroute->route_node);
             spm_l3_memfree(uroute);
             uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.vrfuroute_tree);
         }

        uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.lbroute_tree);
        while (uroute != NULL)
        {
             AVLL_DELETE( SHARED.lbroute_tree, uroute->route_node);
             spm_l3_memfree(uroute);
             uroute =  (VRFUROUTE *)AVLL_FIRST(SHARED.lbroute_tree);
         }
         
        SHARED.ipsroute.delipsok = 0;
        SHARED.ipsroute.addipsok = 0;
        SHARED.ipsroute.delipsnum = 0;
        SHARED.ipsroute.addipsnum = 0;

    NBB_TRC_EXIT();

    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_mcroute(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
  
    MCROUTE *mroute = NULL;

    NBB_TRC_ENTRY("spm_l3_free_mcroute");

    mroute = (MCROUTE *)AVLL_FIRST(SHARED.mcroute_tree);
    while (mroute != NULL)
    {
        AVLL_DELETE( SHARED.mcroute_tree, mroute->route_node);
             NBB_MM_FREE(mroute, MEM_SPM_VRFMROUTE_NODE_CB);
             mroute = (MCROUTE *)AVLL_FIRST(SHARED.mcroute_tree);
             
        //mroute = (MCROUTE *)AVLL_NEXT( SHARED.mcroute_tree, mroute->route_node);
    }
    
    NBB_TRC_EXIT();

    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_tunnelch(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    TUNNELCH    *tunnels = NULL;
    PWTXNODE    *pwnode = NULL;
    SHASH_NODE  *node = NULL;
    NBB_VOID       *data = NULL;
    NBB_ULONG   i = 0;

    NBB_TRC_ENTRY("spm_l3_free_tunnelch");

    if( SHARED.tunnelchhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    
    //NBB_VERIFY_MEMORY(SHARED.tunnelchhash,sizeof(SHASH_TABLE),MEM_SPM_HASH_TABLE_CB);
    //NBB_VERIFY_MEMORY(SHARED.tunnelchhash->index,sizeof(SHASH_TABLE *)*8000,MEM_SPM_HASH_INDEX_CB);
    for (i = 0; i < 8000; i++)
    {
        node = SHARED.tunnelchhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                tunnels = (TUNNELCH *)(node->data);
                pwnode =  (PWTXNODE *)AVLL_FIRST(tunnels->pwtree);
                while (pwnode != NULL)
                {
                    AVLL_DELETE( tunnels->pwtree, pwnode->pw_node);
                                NBB_MM_FREE(pwnode, MEM_SPM_VRFUROUTE_PWNODE_CB);
                                pwnode =  (PWTXNODE *)AVLL_FIRST(tunnels->pwtree);
                                
                    //pwnode = (PWTXNODE *)AVLL_NEXT( tunnels->pwtree, pwnode->pw_node);
                }
                 data = spm_hashtab_del( SHARED.tunnelchhash ,&(tunnels->key) NBB_CCXT);
                        if(data == NULL)
                        {
                            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e freetun-ch vrf.pe=%x.%x \n", 
                            __LINE__, tunnels->key.vrfid,tunnels->key.peerip);
                        }
                        else
                        {                               
                            NBB_MM_FREE(tunnels, MEM_SPM_TUNNELCH_CB);
                        }
            }
            node = SHARED.tunnelchhash->index[i];
            
            //node = node->next;
        }   
    }
    
    EXIT_LABEL:
    NBB_TRC_EXIT();

    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_lspprot(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    LSPPROT     *lspprot = NULL;
    SHASH_NODE  *node = NULL;
    NBB_VOID       *data = NULL;
    NBB_ULONG   i = 0;
	PWVC		*pwvc = NULL;
	RSVPCRLSP   *rsvplsp = NULL;
	PWVPN       *pwvpn = NULL;
	PWMC        *pwmc = NULL;

    NBB_TRC_ENTRY("spm_l3_free_lspprot");

    if( SHARED.lspprothash == NULL )
    {
        goto  EXIT_LABEL;
    }
    
    //NBB_VERIFY_MEMORY(SHARED.lspprothash,sizeof(SHASH_TABLE),MEM_SPM_HASH_TABLE_CB);
    //NBB_VERIFY_MEMORY(SHARED.lspprothash->index,sizeof(SHASH_TABLE *)*8000,MEM_SPM_HASH_INDEX_CB);
    for (i = 0; i < 8000; i++)
    {
        node = SHARED.lspprothash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                        lspprot = (LSPPROT *)(node->data);
                        data = spm_hashtab_del( SHARED.lspprothash , &(lspprot->key) NBB_CCXT);             
                        if(data == NULL)
                        {
                            BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e freelsp-pro in.eg.tun=%x.%x.%x \n", 
                            __LINE__, lspprot->key.ingress,lspprot->key.egress,lspprot->key.tunnelid);
                        }
                        else
                        {                               
                            NBB_MM_FREE(lspprot, MEM_SPM_LSPPROT_CB);
                        }
            }
            node = SHARED.lspprothash->index[i];
            
            //node = node->next;
        }   
    }

	/*删除 rsvp 与 vp，vrf，关系树*/
	rsvplsp =  (RSVPCRLSP *)AVLL_FIRST( SHARED.rsvplsp_tree );
	while(rsvplsp != NULL)
	{
        pwvpn = (PWVPN *)AVLL_FIRST(rsvplsp->vrfpeer_tree);
		while(pwvpn != NULL)
		{
             AVLL_DELETE( rsvplsp->vrfpeer_tree, pwvpn->vrf_node);
             NBB_MM_FREE(pwvpn, MEM_SPM_CR_LSP_TAB_VPN_CB);
             pwvpn =  (PWVPN *)AVLL_FIRST( rsvplsp->vrfpeer_tree);
		}

		pwvc = (PWVC*)AVLL_FIRST(rsvplsp->vp_tree);
		while(pwvc != NULL)
		{
            AVLL_DELETE(rsvplsp->vp_tree,pwvc->vc_node);
			NBB_MM_FREE(pwvc,MEM_SPM_FTN_TAB_VC_CB);
			pwvc = (PWVC*)AVLL_FIRST(rsvplsp->vp_tree);
		}

		pwmc = (PWMC*)AVLL_FIRST(rsvplsp->mcid_tree);
		while(pwmc != NULL)
		{
            AVLL_DELETE(rsvplsp->mcid_tree,pwmc->mc_node);
			NBB_MM_FREE(pwmc,MEM_SPM_FTN_TAB_MC_CB);
			pwmc = (PWMC*)AVLL_FIRST(rsvplsp->mcid_tree);
		}

		AVLL_DELETE(SHARED.rsvplsp_tree ,rsvplsp->rsvp_node);
		NBB_MM_FREE(rsvplsp,MEM_SPM_CR_LSP_TAB_RSVP_CB);
		rsvplsp = (RSVPCRLSP*)AVLL_FIRST(SHARED.rsvplsp_tree);
		
	}
	

    EXIT_LABEL:
    NBB_TRC_EXIT();

    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_vrfins(NBB_CXT_T NBB_CXT)
{
  /***************************************************************************/
  /* Local Variables                                                         */
  /***************************************************************************/
    VRFINS  *vrfins = NULL;

    NBB_TRC_ENTRY("spm_l3_free_vrfins");

    vrfins = (VRFINS *)AVLL_FIRST(SHARED.vrfins_tree);
    while (vrfins != NULL)
    {   
             AVLL_DELETE( SHARED.vrfins_tree, vrfins->ins_node);
             NBB_MM_FREE(vrfins, MEM_SPM_VRFINS_CB);
          vrfins = (VRFINS *)AVLL_FIRST(SHARED.vrfins_tree);
          
        //vrfins = (VRFINS *)AVLL_NEXT( SHARED.vrfins_tree, vrfins->ins_node);
    }
    
    NBB_TRC_EXIT();

    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_l3_free_mem( NBB_CXT_T NBB_CXT )
{

    spm_l3_free_ftn(NBB_CXT);

    spm_l3_free_ilm(NBB_CXT);
    spm_l3_free_crlsprx(NBB_CXT);
    spm_l3_free_crlsptx(NBB_CXT);

    spm_l3_free_ucroute(NBB_CXT);
    spm_l3_free_mcroute(NBB_CXT);

    spm_l3_free_tunnelch(NBB_CXT);
    spm_l3_free_lspprot(NBB_CXT);

    spm_l3_free_vrfins(NBB_CXT);

    BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "clear L3cfg end \n");

    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spm_free_cfgmem( )
{
    spm_bfd_clean_all_cfg();
    spm_clean_all_l2_cfg();
    spm_l3_free_mem();
    spm_qos_clear_all_cfg();
    spm_aps_conf_clear();
    #ifdef SRC
    
    spm_ptp_clear_config();
        
    #endif    
}

/******************************************************************************
 * FunctionName 	: 	spm_init_drv_vpn
 * Author		: 	    dengkai
 * CreateDate		:	20165-3-23
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	      
 * OtherInfo		:	spm_rcv_dci_cmd_common_ctrl
******************************************************************************/
NBB_VOID spm_init_drv_vpn( )
{

     NBB_INT sCounter = 0;

#ifdef SPU     
    /* add by dengkai */
    for (sCounter = 0; sCounter < 4000; sCounter++)
    {
        ApiC3CreateL3Vpn(0, sCounter);
        if (SHARED.c3_num == 2)
        {
            ApiC3CreateL3Vpn(1, sCounter);
        }
    }

	/*add by zhangzhm*/
	//8000条VPWS
	for (sCounter = 1; sCounter <= 8000; sCounter++)
	{
		ApiC3CreateMplsVpn(0, 1,sCounter);
		if (SHARED.c3_num == 2)
        {
            ApiC3CreateMplsVpn(1, 1,sCounter);
        }
	}

	//8000条MSPW
	for (sCounter = 1; sCounter <= 8000; sCounter++)
	{
		
		ApiC3CreateMplsVpn(0, 4,sCounter);
		if (SHARED.c3_num == 2)
        {
            ApiC3CreateMplsVpn(1, 4,sCounter);
        }
	}

	//4000条VPLS
	for (sCounter = 1; sCounter <= 4000; sCounter++)
	{
		
		ApiC3CreateMplsVpn(0, 2,sCounter);
		if (SHARED.c3_num == 2)
        {
            ApiC3CreateMplsVpn(1, 2,sCounter);
        }
	}
 #endif
}
#endif


#if 1

/******************************************************************************
 * FunctionName 	: 	spm_l3_mod_lspp_para
 * Author		: 	    wjhe
 * CreateDate		:	2015-8-31
 * Description		:   refresh lsp return time ,delay time ,return type	
 * InputParam	:	    
 * OutputParam	:	    L3 info
 * ReturnValue	:	    no
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_INT spm_l3_mod_lspp_para(LSPPROT  *lspprot NBB_CCXT_T NBB_CXT)
{
    NBB_INT rv = 1;
	NBB_INT i = 0;
	SPM_LINEAR_PROTECT_SWITCH 		stLps;

	NBB_TRC_ENTRY("spm_l3_refresh_lspReturntime");

	if (NULL == lspprot)
	{
		BMU_SLOG(BMU_INFO, SPM_L3_LOG_DIR, "Line=%d e lspprot NULL \n", __LINE__);
		rv = 104;
		goto  EXIT_LABEL;
	}
                
	OS_MEMSET(&stLps, 0, sizeof(SPM_LINEAR_PROTECT_SWITCH));
	stLps.revertiveType = lspprot->base.returntype;
	stLps.holdOffTime = lspprot->base.delaytime;
	stLps.wtrTime = lspprot->base.waittime;

	stLps.workEntity.port = 0;
	stLps.workEntity.slot = 0;
	stLps.workEntity.tpId = 0;
	stLps.workEntity.type = 3;
	
	stLps.workEntity.rsvpKey.ingress = lspprot->key.ingress;
	stLps.workEntity.rsvpKey.egress = lspprot->key.egress;
	stLps.workEntity.rsvpKey.tunnel_id = lspprot->key.tunnelid;

	stLps.protEntity.port = 0;
	stLps.protEntity.slot = 0;
	stLps.protEntity.tpId = 0;
	stLps.protEntity.type = 3;
	stLps.protEntity.rsvpKey.ingress = lspprot->key.ingress;
	stLps.protEntity.rsvpKey.egress = lspprot->key.egress;
	stLps.protEntity.rsvpKey.tunnel_id = lspprot->key.tunnelid;

	for(i = 0; i<lspprot->lspnum;i++)
	{
		if((lspprot->lspb.lsp[i].state) == 0)
		{
		  stLps.workEntity.rsvpKey.lsp_id = lspprot->lspb.lsp[i].lspid;
		  stLps.workEntity.portIndex = lspprot->dclsp[i].nextport;
		}
		else if((lspprot->lspb.lsp[i].state) == 1)
		{
		  stLps.protEntity.rsvpKey.lsp_id = lspprot->lspb.lsp[i].lspid;
		  stLps.protEntity.portIndex = lspprot->dclsp[i].nextport;
		}
	}

	stLps.protType = TUNNEL_TYPE;
	stLps.protGroupId = lspprot->index;
	stLps.lpsType = FRR_1B1;
	
    spm_lps_add_entity(&stLps);
	rv = 0;

	EXIT_LABEL:

	return rv;
	
	NBB_TRC_EXIT();
}


#endif



/********************
调试接口
********************/
#define  TEST

#ifdef TEST  
#define TRACE_OUT_LEN    1001
NBB_BOOL spm_mem_info(NBB_PROC_TYPE proc_type,
                          NBB_PROC_ID proc_id,
                          NBB_CONST NBB_CHAR *string
                          NBB_CCXT_T NBB_CXT)
{
  MM_USAGE_CB *mm_usage_cb;
  MM_USAGE_KEY key;
  NBB_PROC_TYPE current_proc_type;
  NBB_PROC_ID current_proc_id;
  NBB_PROC_TYPE proc_type_from_proc_id;
  NBB_PROC_TYPE save_proc_type = 0;
  NBB_BUF_SIZE buf_memory_alloc;
  NBB_BUF_SIZE buf_memory_count;
  NBB_BUF_SIZE buf_memory_alloc_hwm;
  NBB_BUF_SIZE buf_memory_count_hwm;
  NBB_BYTE buf_memory_wrapped = FALSE;
  NBB_BYTE cb_memory_wrapped = FALSE;
  NBB_PROC_ID buf_memory_wrapped_proc_id = 0;
  NBB_PROC_ID cb_memory_wrapped_proc_id = 0;
  NBB_BUF_SIZE proc_id_cb_alloc_total = 0;
  NBB_BUF_SIZE proc_type_buf_alloc_total = 0;
  NBB_BUF_SIZE proc_type_cb_alloc_total = 0;
  NBB_BUF_SIZE overall_buf_alloc_total = 0;
  NBB_BUF_SIZE overall_cb_alloc_total = 0;
  NBB_INT proc_id_cb_count_total = 0;
  NBB_INT proc_type_buf_count_total = 0;
  NBB_INT proc_type_cb_count_total = 0;
  NBB_INT overall_buf_count_total = 0;
  NBB_INT overall_cb_count_total = 0;
  
  //NBB_CHAR trace_output[TRACE_OUT_LEN *5];
  NBB_CHAR *trace_output = NULL;
  NBB_INT loop = 0;
  NBB_CHAR trace_temp[TRACE_OUT_LEN];
  NBB_ULONG str_len = 0;
  NBB_BOOL ret_code = FALSE;
  PROCESS_ENTRY *proc_entry;
  NBB_CHAR date_time_string[NBB_DATE_TIME_STRING_SIZE];

  NBB_TRC_ENTRY("spm_mem_info");

  trace_output = NBB_MM_ALLOC(TRACE_OUT_LEN * 50, NBB_NORETRY_ACT, 0);
  
  if(NULL == trace_output)
  {
    goto EXIT_LABEL;
  }
  
  NBB_GET_INT_LOCK(NBB_INT_LOCK_PROCESS_DATA);
  NBB_GET_INT_LOCK(NBB_INT_LOCK_MM);

  //if (nbb_mm_trace_usage)
  {
    NBB_TRC_FLOW((NBB_FORMAT "memory usage tracing enabled"));

    NBB_SPRINTF(trace_output, "=======================================");
    str_len =  OS_STRLEN(trace_output);
                                                              
    NBB_SPRINTF(trace_output + str_len, "\n\nN-BASE Memory Usage Statistics - %s\n\n", string);
    str_len =  OS_STRLEN(trace_output);

    NBB_GET_DATE_TIME_STRING(date_time_string);
    NBB_SPRINTF(trace_output + str_len, "Created: %s\n\n\n", date_time_string);
    str_len = OS_STRLEN(trace_output);

    if ((proc_type == NBB_MM_LOG_ALL) && (proc_id == NBB_MM_LOG_ALL))
    {

      NBB_TRC_FLOW((NBB_FORMAT "We have neither proc ID or proc type"));
      mm_usage_cb = (MM_USAGE_CB *)AVLL_FIRST(nbb_mm_usage_stats);

      if (mm_usage_cb == NULL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "There is no mem usage data available"));
        NBB_SPRINTF(trace_output + str_len, "No memory usage data available\n\n");
        str_len = OS_STRLEN(trace_output);
        goto EXIT_LABEL;
      }
    }
    else
    {

      NBB_TRC_FLOW((NBB_FORMAT "we have either proc type or proc id"));
      if (proc_id != NBB_MM_LOG_ALL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "get proc type from proc id"));
        proc_entry = (NBB_VALIDATE_PROC_ID(proc_id));

        if (proc_entry == NULL)
        {
          NBB_TRC_FLOW((NBB_FORMAT "Not a valid process id"));
          NBB_SPRINTF(trace_output + str_len, "Process ID %H is not valid\n\n", proc_id);
          str_len =  OS_STRLEN(trace_output);
          goto EXIT_LABEL;
        }
        proc_type_from_proc_id = proc_entry->pcc_type;

        if (proc_type == NBB_MM_LOG_ALL)
        {
        
          NBB_TRC_FLOW((NBB_FORMAT "no proc type so use one obtained"));
          proc_type = proc_type_from_proc_id;
        }
        else
        {
          NBB_TRC_FLOW((NBB_FORMAT "proc id and proc type"));
          if (proc_type != proc_type_from_proc_id)
          {
            NBB_TRC_FLOW((NBB_FORMAT "proc id does not match proc type"));
            NBB_SPRINTF(trace_output + str_len, "The process id %H " , proc_id);
            str_len = OS_STRLEN(trace_output);
            NBB_SPRINTF(trace_output + str_len, "is not an instance of ");
            str_len =  OS_STRLEN(trace_output);
            NBB_SPRINTF(trace_output +  str_len, "the process type %H.\n\n", proc_type);
            str_len =  OS_STRLEN(trace_output);
            goto EXIT_LABEL;
          }
        }
      }

      key.proc_type = proc_type;
      key.proc_id = proc_id;
      key.mem_type = 0;
      mm_usage_cb = (MM_USAGE_CB *)AVLL_FIND_OR_FIND_NEXT(nbb_mm_usage_stats,
                                                                         &key);
      
      if (mm_usage_cb == NULL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "failed to find usage cb"));
        NBB_SPRINTF(trace_output + str_len, "No mem usage stats available\n\n");
        str_len =  OS_STRLEN(trace_output);                                                            
        goto EXIT_LABEL;
      }

      if ((mm_usage_cb->key.proc_type != proc_type))
      {
        NBB_TRC_FLOW((NBB_FORMAT "There were no stats for process type %lx",
                                                                   proc_type));
        NBB_SPRINTF(trace_output + str_len, "No mem usage stats for proc type: %lx\n\n",
                                                                    proc_type);
        str_len =  OS_STRLEN(trace_output); 
        goto EXIT_LABEL;
      }

      if (proc_id != NBB_MM_LOG_ALL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "we specified process id %H", proc_id));
        if (mm_usage_cb->key.proc_id != proc_id)
        {
          NBB_TRC_FLOW((NBB_FORMAT "There were no stats for process id %H",
                                                                   proc_id));
          NBB_SPRINTF(trace_output + str_len, "No mem usage stats for proc id: %H\n\n",
                                                                      proc_id);
          str_len =  OS_STRLEN(trace_output);
          
          goto EXIT_LABEL;
        }
      }
    }

    current_proc_type = mm_usage_cb->key.proc_type;
    current_proc_id = mm_usage_cb->key.proc_id;

    while (mm_usage_cb != NULL)
    {
      NBB_TRC_FLOW((NBB_FORMAT "Process type: %lx",
                                                  mm_usage_cb->key.proc_type));
      NBB_SPRINTF(trace_output + str_len, "Process type: %lx \n|\n",
                                                   mm_usage_cb->key.proc_type);
      
      str_len =  OS_STRLEN(trace_output);

      while ((mm_usage_cb != NULL) &&
             (mm_usage_cb->key.proc_type == current_proc_type))
      {
        NBB_TRC_FLOW((NBB_FORMAT "Process ID: %H", mm_usage_cb->key.proc_id));
        NBB_SPRINTF(trace_output + str_len, "+--Process ID: %H \n|\n",
                                                     mm_usage_cb->key.proc_id);
        str_len =  OS_STRLEN(trace_output);
        
        NBB_SPRINTF(trace_output + str_len, "|%-14s%-12s%-12s%-12s%-12s\n",
                      "CB Type", "Mem Alloc", "H.W.M.", "Num Alloc", "H.W.M.");
       
        str_len =  OS_STRLEN(trace_output);

        buf_memory_alloc = 0;
        buf_memory_count = 0;
        buf_memory_alloc_hwm = 0;
        buf_memory_count_hwm = 0;

        while ((mm_usage_cb != NULL) &&
               (mm_usage_cb->key.proc_id == current_proc_id))
        {
          NBB_TRC_FLOW((NBB_FORMAT "Print memory details"));
          if (mm_usage_cb->key.mem_type == MEM_BUFFER_STATS)
          {

            NBB_TRC_FLOW((NBB_FORMAT "Buffer Memory"));
            buf_memory_alloc = mm_usage_cb->mem_alloc;
            buf_memory_count = mm_usage_cb->mem_count;
            buf_memory_alloc_hwm = mm_usage_cb->hwm_alloc;
            buf_memory_count_hwm = mm_usage_cb->hwm_count;

            if (mm_usage_cb->wrapped != FALSE)
            {
              NBB_TRC_FLOW((NBB_FORMAT "memory has been wrapped"));
              buf_memory_wrapped = TRUE;
              buf_memory_wrapped_proc_id = mm_usage_cb->key.proc_id;
            }

            proc_type_buf_alloc_total += mm_usage_cb->mem_alloc;
            proc_type_buf_count_total += mm_usage_cb->mem_count;
            overall_buf_alloc_total += mm_usage_cb->mem_alloc;
            overall_buf_count_total += mm_usage_cb->mem_count;
          }
          else
          {
            NBB_TRC_FLOW((NBB_FORMAT "CB Memory"));
            NBB_SPRINTF(trace_output + str_len,
                                 "|%-14lx%-12lu%-12lu%-12d%-12d",
                                 mm_usage_cb->key.mem_type,
                                 mm_usage_cb->mem_alloc,
                                 mm_usage_cb->hwm_alloc,
                                 mm_usage_cb->mem_count,
                                 mm_usage_cb->hwm_count);
            str_len =  OS_STRLEN(trace_output);
         
            if (mm_usage_cb->wrapped != FALSE)
            {
              NBB_TRC_FLOW((NBB_FORMAT "memory has been wrapped"));
              cb_memory_wrapped = TRUE;
              cb_memory_wrapped_proc_id = mm_usage_cb->key.proc_id;
              NBB_SPRINTF(trace_output + str_len, " Warning: wrapped PID %lx",
                                                     mm_usage_cb->key.proc_id);

              str_len =  OS_STRLEN(trace_output);
            }
            NBB_SPRINTF(trace_output + str_len, "\n");
            str_len =  OS_STRLEN(trace_output);
            
            proc_id_cb_alloc_total += mm_usage_cb->mem_alloc;
            proc_id_cb_count_total += mm_usage_cb->mem_count;
            proc_type_cb_alloc_total += mm_usage_cb->mem_alloc;
            proc_type_cb_count_total += mm_usage_cb->mem_count;
            overall_cb_alloc_total += mm_usage_cb->mem_alloc;
            overall_cb_count_total += mm_usage_cb->mem_count;
          }

          save_proc_type = mm_usage_cb->key.proc_type;
          mm_usage_cb = (MM_USAGE_CB *)AVLL_FIND_NEXT(nbb_mm_usage_stats,
                                                          &(mm_usage_cb->key));
        }

        NBB_SPRINTF(trace_output + str_len, "|   -------------------------------------");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len, "-----------------------");

        str_len =  OS_STRLEN(trace_output);

        NBB_SPRINTF(trace_output + str_len,
                            "\n|     CB total: %-11lu%-12s%-12d%-12s",
                            proc_id_cb_alloc_total, "N/A",
                            proc_id_cb_count_total, "N/A");

        str_len =  OS_STRLEN(trace_output);

        if (cb_memory_wrapped != FALSE)
        {
          NBB_TRC_FLOW((NBB_FORMAT "display cb memory warning"));
          NBB_SPRINTF(trace_output + str_len, " Warning: wrapped PID %lx",
                                                    cb_memory_wrapped_proc_id);

          str_len =  OS_STRLEN(trace_output);
          cb_memory_wrapped = FALSE;
        }
        NBB_SPRINTF(trace_output + str_len, "\n");

        str_len =  OS_STRLEN(trace_output);

        NBB_SPRINTF(trace_output + str_len, "|   -------------------------------------");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len, "-----------------------");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len,
                          "\n|     Buf total:%-11lu%-12lu%-12d%-12d",
                          buf_memory_alloc,
                          buf_memory_alloc_hwm,
                          buf_memory_count,
                          buf_memory_count_hwm);

        str_len =  OS_STRLEN(trace_output);

        if (buf_memory_wrapped != FALSE)
        {
          NBB_TRC_FLOW((NBB_FORMAT "display buffer memory warning"));
          NBB_SPRINTF(trace_output + str_len, " Warning: wrapped PID %lx",
                                                   buf_memory_wrapped_proc_id);

          str_len =  OS_STRLEN(trace_output);
          buf_memory_wrapped = FALSE;
        }
        NBB_SPRINTF(trace_output + str_len, "\n");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len, "|   -------------------------------------");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len, "-----------------------");

        str_len =  OS_STRLEN(trace_output);
        NBB_SPRINTF(trace_output + str_len, "\n|\n");

        str_len =  OS_STRLEN(trace_output);

        proc_id_cb_alloc_total = 0;
        proc_id_cb_count_total = 0;

        if (proc_id != NBB_MM_LOG_ALL)
        {
          NBB_TRC_FLOW((NBB_FORMAT "Only displaying one proc id"));
          mm_usage_cb = NULL;
        }

        if (mm_usage_cb != NULL)
        {
          NBB_TRC_FLOW((NBB_FORMAT "Get new proc ID"));
          current_proc_id = mm_usage_cb->key.proc_id;
        }
      }

      NBB_SPRINTF(trace_output + str_len, "+-Process type totals: \n|\n");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "|   =====================================");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "===========\n");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "|%-38s%12s\n",
                                "Mem Alloc","Num Alloc");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len,
                             "|    CB memory total     : %-12lu%-12d\n",
                             proc_type_cb_alloc_total,
                             proc_type_cb_count_total);

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len,
                             "|    Buffer memory total : %-12lu%-12d\n",
                             proc_type_buf_alloc_total,
                             proc_type_buf_count_total);

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "|   =====================================");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "===========\n");

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "|\n|\n+-End of stats for process type %lx\n",
                                                               save_proc_type);

      str_len =  OS_STRLEN(trace_output);
      NBB_SPRINTF(trace_output + str_len, "\n\n");

      str_len =  OS_STRLEN(trace_output);


      proc_type_cb_alloc_total = 0;
      proc_type_cb_count_total = 0;
      proc_type_buf_alloc_total = 0;
      proc_type_buf_count_total = 0;

      if (proc_type != NBB_MM_LOG_ALL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "Only displaying one proc type"));
        mm_usage_cb = NULL;
      }

      if (mm_usage_cb != NULL)
      {
        NBB_TRC_FLOW((NBB_FORMAT "Get new proc type"));
        current_proc_type = mm_usage_cb->key.proc_type;
      }
    }


    NBB_SPRINTF(trace_output + str_len, "\n\nOverall system totals\n\n", save_proc_type);

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, "=====================================");

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, "============\n");

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, "%-36s%12s\n", "Mem Alloc","Num Alloc");

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, " OVERALL CB MEMORY     :%-12lu%-12d\n",
                               overall_cb_alloc_total, overall_cb_count_total);

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, " OVERALL BUFFER MEMORY :%-12lu%-12d\n",
                             overall_buf_alloc_total, overall_buf_count_total);

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, "=====================================");

    str_len =  OS_STRLEN(trace_output);
    NBB_SPRINTF(trace_output + str_len, "============\n\n\n");
    str_len =  OS_STRLEN(trace_output);                                                              
    


    ret_code = TRUE;
  }

EXIT_LABEL:

  NBB_RELEASE_INT_LOCK(NBB_INT_LOCK_MM);

  NBB_RELEASE_INT_LOCK(NBB_INT_LOCK_PROCESS_DATA);

  NBB_TRC_EXIT();

  if(NULL != trace_output)
  {
      while(str_len >= TRACE_OUT_LEN)
      {
         NBB_OS_STRNCPY(trace_temp,trace_output + (loop*(TRACE_OUT_LEN - 1)),TRACE_OUT_LEN - 1);
         trace_temp[1000] = '\0';
         NBB_PRINTF(trace_temp);
         OS_MEMSET(trace_temp,0,TRACE_OUT_LEN);
         str_len = str_len - TRACE_OUT_LEN;
         loop++;
         
      }
      
      NBB_OS_STRNCPY(trace_temp,trace_output + (loop*(TRACE_OUT_LEN - 1)),str_len);
      trace_temp[str_len] = '\0';
      NBB_PRINTF(trace_temp);
      NBB_MM_FREE(trace_output,0);
  }
  return(ret_code);

}



/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID spmshowmem( NBB_BYTE type )
{

    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spmshowmem");
    if( type == 0 )
    {
        spm_mem_info( 0, 0, "System memory information" NBB_CCXT);
    }
    else if( type == 1 )
    {
        spm_mem_info( 0x02010000, 0x01105000, "BMU memory information" NBB_CCXT);
    }
    else if( type == 2 )
    {
        spm_mem_info( 0x02020000, 0x01106000, "SPM memory information" NBB_CCXT);
    }
        
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_ULONG showhashkey(NBB_CONST NBB_VOID *keyarg, NBB_SHORT keylen, NBB_ULONG initval )
{
    NBB_ULONG a = 0;
    NBB_ULONG len;
    NBB_BYTE *key = (NBB_BYTE *) keyarg;
    
    len = (NBB_ULONG) keylen;
    a = initval + len;
    if(keyarg == NULL)
    {
         goto EXIT_LABEL;
    }
    while (len >= 4)
    {
        a += key[0] + (key[1] << 8) + (key[2] << 16) + (key[3] << 24);
        a += ~(a << 15);
        a ^=  (a >> 10);
        a +=  (a << 3);
        a ^=  (a >> 6);
        a += ~(a << 11);
        a ^=  (a >> 16);
        key += 4;
        len -= 4;
    }
    switch (len)
    {
        case 3 : a += key[2] << 16;
        case 2 : a ^= key[1] << 8;
        case 1 : a += key[0];
        default: break;
    }

    EXIT_LABEL:

    return a;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID *showhashtabsearch (SHASH_TABLE *hashtab, NBB_VOID *keyarg ,NBB_SHORT keylen )
{
    NBB_ULONG       key;
    NBB_ULONG       id;
    SHASH_NODE      *node = NULL;

    //NBB_TRC_ENTRY("spm_hashtab_search");

    key = showhashkey( keyarg , keylen , 0);
    id = key % (hashtab->size);

    for (node = hashtab->index[id]; node != NULL; node = node->next)
    {
        if (node->key == key && (*hashtab->hash_cmp)(node->data, keyarg) == 1)
        {
                    break;
        }
    }
    
    //NBB_TRC_EXIT();
    return node;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID lspshowftn(NBB_USHORT vrfid, NBB_BYTE mask, NBB_ULONG fec)
{
    NBB_BYTE            i;
    FTN_KEY             ftnkey;
    FTN_TAB             *uftn = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spmshowftn");

    if (( vrfid == 0) && ( mask == 0) && ( fec == 0))
    {
        NBB_PRINTF(" vrfid\n");
        NBB_PRINTF(" mask\n");
        NBB_PRINTF(" fec\n");
        goto  EXIT_LABEL;
    }
    OS_MEMSET(&ftnkey, 0, sizeof(FTN_KEY));
    ftnkey.vrfid = vrfid;
    ftnkey.mask = mask;
    ftnkey.fec = fec;

    uftn = (FTN_TAB *)AVLL_FIND( v_spm_shared->ftn_tree ,  &ftnkey ); 
    if (uftn == NULL)
    {
        NBB_PRINTF(" ftn 不存在!\n");
        goto  EXIT_LABEL;
    }
    
    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" almtype       = %d\n", uftn->base.almtype);
    NBB_PRINTF(" nexttype      = %d\n", uftn->base.nexttype);
    
    NBB_PRINTF(" 下一跳\n");
    NBB_PRINTF(" nhop num     = %d\n", uftn->hopnum);
    NBB_PRINTF(" ecmpid       = %d\n", uftn->ecmpid);
    
    for ( i = 0; i < uftn->hopnum; i++)
    {
        NBB_PRINTF("\n********************************\n");
        NBB_PRINTF(" nextip         = %x\n", uftn->nexthop[i].nextip);
        NBB_PRINTF(" nextport       = %d\n", uftn->nexthop[i].nextport);
        NBB_PRINTF(" outlabel       = %d\n", uftn->nexthop[i].outlabel);
        NBB_PRINTF(" nexttypeb      = %d\n", uftn->nexthop[i].nexttypeb);

        NBB_PRINTF(" dc label3      = %d\n", uftn->dclsp[i].label3);
        NBB_PRINTF(" dc label2      = %d\n", uftn->dclsp[i].label2);
        NBB_PRINTF(" dc label1      = %d\n", uftn->dclsp[i].label1);
        NBB_PRINTF(" dc nextip      = %x\n", uftn->dclsp[i].nextip);
        NBB_PRINTF(" dc nextport    = %x\n", uftn->dclsp[i].nextport);
        NBB_PRINTF(" dc index       = %x\n", uftn->dclsp[i].index);
        NBB_PRINTF(" ftnidx         = %x\n", uftn->index[i]);
        NBB_PRINTF(" posid          = %x\n", uftn->posid[i]);
        NBB_PRINTF("\n******************************************\n");
    }
    NBB_PRINTF(" tunnelid    = %x\n", uftn->tunnelid);
    NBB_PRINTF(" pwnhi       = %x\n", uftn->pwnhi);
    NBB_PRINTF(" l2num       = %d\n", uftn->l2num);
    NBB_PRINTF(" l3num       = %d\n", uftn->l3num);
    NBB_PRINTF(" memflag     = %d\n", uftn->memflag);
    NBB_PRINTF(" delflag     = %d\n", uftn->delflag);
    NBB_PRINTF("\n******************************************\n");
    NBB_PRINTF(" error code  = %d\n", uftn->return_code);
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showftnallkey( )
{
    NBB_ULONG           sum = 0;
    FTN_TAB             *uftn = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    
    //NBB_TRC_ENTRY("spmshowuroute");

    NBB_PRINTF("|%-6s%-12s%-20s%-20s%-20s%-20s\n","VRF", "FEC/MASK", 
                                                    "NEXTHOP", 
                                                    "NEXTHOP", 
                                                    "NEXTHOP", 
                                                    "NEXTHOP");
    uftn =  (FTN_TAB *)AVLL_FIRST(v_spm_shared->ftn_tree);
    while (uftn != NULL)
    {
        //NBB_PRINTF("vrfid=%d mask=%d fec=%x \n",
        NBB_PRINTF("|%-6d%-11x/%d%-12x/%x/%d%-12x/%x/%d%-12x/%x/%d%-12x/%x/%d\n",
                                uftn->key.vrfid,
                                uftn->key.fec,
                                uftn->key.mask,
                                uftn->nexthop[0].outlabel,
                                uftn->nexthop[0].nextip,
                                uftn->nexthop[0].nextport,
                                uftn->nexthop[1].outlabel,
                                uftn->nexthop[1].nextip,
                                uftn->nexthop[1].nextport,
                                uftn->nexthop[2].outlabel,
                                uftn->nexthop[2].nextip,
                                uftn->nexthop[2].nextport,
                                uftn->nexthop[3].outlabel,
                                uftn->nexthop[3].nextip,
                                uftn->nexthop[3].nextport);

        sum++;
        uftn = (FTN_TAB *)AVLL_NEXT( v_spm_shared->ftn_tree, uftn->ftn_node);
    }
    NBB_PRINTF("\n  FTN   sum = %d \n", sum);
    

    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID lspshowilm( NBB_ULONG label)
{
    NBB_BYTE        i;
    ILM_KEY         ilmkey;
    ILM_TAB         *ilms = NULL;
    SHASH_NODE      *node = NULL;
    
    //NBB_TRC_ENTRY("spmshowilm");
    
    OS_MEMSET(&ilmkey, 0, sizeof(ILM_KEY));
    ilmkey.inlabel = label;
    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->ilmhash , &ilmkey, 4);

    if (( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" ilm 不存在!\n");
        goto  EXIT_LABEL;
    }
    
    //NBB_PRINTF(" node     = %x\n", node);
    NBB_PRINTF(" key            = %x\n", node->key);
    
    //NBB_PRINTF(" next             = %x\n", node->next);
        ilms = (ILM_TAB *)(node->data);

    NBB_PRINTF(" inlabel        = %d\n", ilms->key.inlabel);

    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" vrfid          = %d\n", ilms->base.vrfid);
    NBB_PRINTF(" feclgth        = %d\n", ilms->base.feclgth);
    NBB_PRINTF(" fec            = %x\n", ilms->base.fec);
    NBB_PRINTF(" almtype        = %d\n", ilms->base.almtype);
    NBB_PRINTF(" nextype        = %d\n", ilms->base.nextype);
    NBB_PRINTF(" labelaction    = %d\n", ilms->base.labelaction);
    NBB_PRINTF(" ntype          = %d\n", ilms->ntype);

    NBB_PRINTF("下一跳\n");
    NBB_PRINTF("nhop num= %d\n", ilms->nhopnum);
    for ( i = 0; i < ilms->nhopnum; i++)
    {
        NBB_PRINTF("\n********************************\n");
        NBB_PRINTF(" nextip         = %x\n", ilms->nexthop[i].nextip);
        NBB_PRINTF(" nextport       = %d\n", ilms->nexthop[i].nextport);
        NBB_PRINTF(" outlabel       = %d\n", ilms->nexthop[i].outlabel);
        NBB_PRINTF(" nexttypeb      = %d\n", ilms->nexthop[i].nextypeb);
        NBB_PRINTF(" action         = %d\n", ilms->nexthop[i].action);

        NBB_PRINTF("\n******************************************\n");
    }

    NBB_PRINTF(" posid      = %x\n", ilms->posid);
    NBB_PRINTF(" frrid      = %x\n", ilms->frrid);
    NBB_PRINTF(" ecmpid     = %x\n", ilms->ecmpid);
    NBB_PRINTF("\n******************************************\n");
    NBB_PRINTF(" error code = %d\n", ilms->return_code);
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showilmallkey( )
{
    NBB_ULONG       sum = 0;
    ILM_TAB         *ilms = NULL;
    SHASH_NODE      *node = NULL;
    NBB_ULONG       i = 0;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    
    NBB_PRINTF("|%-6s%-30s%-30s%-30s%-30s\n","LABEL", 
                                                "NEXTHOP", 
                                                "NEXTHOP", 
                                                "NEXTHOP", 
                                                "NEXTHOP");
    if( v_spm_shared->ilmhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    for (i = 0; i < 16000; i++)
    {
        node = v_spm_shared->ilmhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                ilms = (ILM_TAB *)(node->data);
                
                //NBB_PRINTF("inlabel = %x \n", ilms->key.inlabel);
                NBB_PRINTF("|%-6x%-12x/%x/%d%-12x/%x/%d%-12x/%x/%d%-12x/%x/%d\n",
                                ilms->key.inlabel,
                                ilms->nexthop[0].outlabel,
                                ilms->nexthop[0].nextip,
                                ilms->nexthop[0].nextport,
                                ilms->nexthop[1].outlabel,
                                ilms->nexthop[1].nextip,
                                ilms->nexthop[1].nextport,
                                ilms->nexthop[2].outlabel,
                                ilms->nexthop[2].nextip,
                                ilms->nexthop[2].nextport,
                                ilms->nexthop[3].outlabel,
                                ilms->nexthop[3].nextip,
                                ilms->nexthop[3].nextport);
                sum++;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " ilm hash ERROR : index = %d", i));
            }
            node = node->next;
        }   
    }
    NBB_PRINTF("\n  ILM   sum = %d \n", sum);
    

    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID lspshowlsprx( NBB_ULONG ingress, NBB_ULONG     egress, NBB_ULONG tunnelid, NBB_ULONG lspid)
{

    CRRXLSP_KEY     rxlspkey;
    CRRXLSP         *rxlsp = NULL;
    SHASH_NODE      *node = NULL;
    
    //NBB_TRC_ENTRY("spmshowlsprx");
    
    OS_MEMSET(&rxlspkey, 0, sizeof(CRRXLSP_KEY));
    rxlspkey.ingress = ingress;
    rxlspkey.egress = egress;
    rxlspkey.tunnelid = tunnelid;
    rxlspkey.lspid = lspid;
    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->crlsprxhash, &rxlspkey, 16);

    if (( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" lsprx 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" node           = %x\n", node);
    NBB_PRINTF(" key            = %x\n", node->key);
    NBB_PRINTF(" next           = %x\n", node->next);
        rxlsp = (CRRXLSP *)(node->data);

    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" vrfid          = %d\n", rxlsp->base.vrfid);
    NBB_PRINTF(" inportindex    = %x\n", rxlsp->base.inportindex);
    NBB_PRINTF(" inlabel        = %d\n", rxlsp->base.inlabel);
    NBB_PRINTF(" outlabel       = %d\n", rxlsp->base.outlabel);
    NBB_PRINTF(" nextip         = %x\n", rxlsp->base.nextip);
    NBB_PRINTF(" nextport       = %x\n", rxlsp->base.nextport);
    NBB_PRINTF(" action         = %d\n", rxlsp->base.action);
    NBB_PRINTF(" almtype        = %d\n", rxlsp->base.almtype);  
    NBB_PRINTF(" tefrrtype      = %d\n", rxlsp->base.tefrrtype);
    NBB_PRINTF(" tefrrnode      = %d\n", rxlsp->base.tefrrnode);    



    NBB_PRINTF("  TE FRR\n");
    NBB_PRINTF(" inportindex    = %x\n", rxlsp->tefrr.inportindex);
    NBB_PRINTF(" inlabel        = %d\n", rxlsp->tefrr.inlabel);
    NBB_PRINTF(" nextip         = %x\n", rxlsp->tefrr.nextip);
    NBB_PRINTF(" nextport       = %x\n", rxlsp->tefrr.nextport);
    NBB_PRINTF(" outlabel       = %d\n", rxlsp->tefrr.outlabel);
    NBB_PRINTF(" action         = %d\n", rxlsp->tefrr.action);  
    
    NBB_PRINTF(" posid          = %x\n", rxlsp->posid);
    NBB_PRINTF("\n************************************\n");
    NBB_PRINTF(" error code     = %d\n", rxlsp->return_code);
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showlsprxallkey( )
{
    NBB_ULONG       sum = 0;
    CRRXLSP         *rxlsp = NULL;
    SHASH_NODE      *node = NULL;
    NBB_ULONG       i = 0;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    if( v_spm_shared->crlsprxhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    for (i = 0; i < 16000; i++)
    {
        node = v_spm_shared->crlsprxhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                rxlsp = (CRRXLSP *)(node->data);
                NBB_PRINTF("ingress=%x egress=%x tunnelid=%x lspid=%x \n",
                                rxlsp->key.ingress,
                                rxlsp->key.egress,
                                rxlsp->key.tunnelid,
                                rxlsp->key.lspid);
                sum++;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " rxlsp hash ERROR : index = %d", i));
            }
            node = node->next;
        }   
    }
    NBB_PRINTF("\n  LSP RX   sum = %d \n", sum);
    

    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID lspshowlsptx( NBB_ULONG ingress, NBB_ULONG     egress, NBB_ULONG tunnelid, NBB_ULONG lspid)
{
    CRTXLSP_KEY     txlspkey;
    CRTXLSP         *txlsp = NULL;
    SHASH_NODE      *node = NULL;
    
    //NBB_TRC_ENTRY("spmshowlsprx");
    
    OS_MEMSET(&txlspkey, 0, sizeof(CRTXLSP_KEY));
    txlspkey.ingress = ingress;
    txlspkey.egress = egress;
    txlspkey.tunnelid = tunnelid;
    txlspkey.lspid = lspid;
    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->crlsptxhash, &txlspkey, 16);

    if (( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" lsptx 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" node           = %x\n", node);
    NBB_PRINTF(" key            = %x\n", node->key);
    NBB_PRINTF(" next           = %x\n", node->next);
        txlsp = (CRTXLSP *)(node->data);

    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" vrfid          = %d\n", txlsp->base.vrfid);
    NBB_PRINTF(" outlabel       = %d\n", txlsp->base.outlabel);
    NBB_PRINTF(" nextip         = %x\n", txlsp->base.nextip);
    NBB_PRINTF(" nextport       = %x\n", txlsp->base.nextport); 
    NBB_PRINTF(" tefrrtype      = %d\n", txlsp->base.tefrrtype);
    NBB_PRINTF(" tefrrnode      = %d\n", txlsp->base.tefrrnode);
    NBB_PRINTF(" almtype        = %d\n", txlsp->base.almtype);  

    NBB_PRINTF("  下一跳\n");
    NBB_PRINTF(" nextip         = %x\n", txlsp->tefrr.nextip);
    NBB_PRINTF(" nextport       = %x\n", txlsp->tefrr.nextport);
    NBB_PRINTF(" outlabel       = %d\n", txlsp->tefrr.outlabel);
    NBB_PRINTF(" action         = %d\n", txlsp->tefrr.action);  
    
    NBB_PRINTF("\n********************************\n");
    NBB_PRINTF(" dc label3      = %d\n", txlsp->dclsp.label3);
    NBB_PRINTF(" dc label2      = %d\n", txlsp->dclsp.label2);
    NBB_PRINTF(" dc label1      = %d\n", txlsp->dclsp.label1);
    NBB_PRINTF(" dc nextip      = %x\n", txlsp->dclsp.nextip);
    NBB_PRINTF(" dc nextport    = %x\n", txlsp->dclsp.nextport);
    NBB_PRINTF(" dc index       = %x\n", txlsp->dclsp.index);
    NBB_PRINTF(" dc ftnidx      = %x\n", txlsp->dclsp.ftnidx);
    NBB_PRINTF("\n************************************\n");
    NBB_PRINTF(" error code     = %d\n", txlsp->return_code);
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showlsptxallkey( )
{
    NBB_ULONG       sum = 0;
    CRTXLSP         *txlsp = NULL;
    SHASH_NODE      *node = NULL;
    NBB_ULONG       i = 0;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    if( v_spm_shared->crlsptxhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    for (i = 0; i < 16000; i++)
    {
        node = v_spm_shared->crlsptxhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                txlsp = (CRTXLSP *)(node->data);
                NBB_PRINTF("ingress=%x egress=%x tunnelid=%x lspid=%x \n",
                                txlsp->key.ingress,
                                txlsp->key.egress,
                                txlsp->key.tunnelid,
                                txlsp->key.lspid);
                sum++;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " txlsp hash ERROR : index = %d", i));
            }
            node = node->next;
        }   
    }
    NBB_PRINTF("\n  LSP TX   sum = %d \n", sum);
    

    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID lspshowlsppr( NBB_ULONG ingress, NBB_ULONG egress, NBB_ULONG tunnelid)
{
    NBB_BYTE        i;
    LSPPROT_KEY     lspprotkey;
    LSPPROT         *lspprot = NULL;
    SHASH_NODE      *node = NULL;
    
    //NBB_TRC_ENTRY("spmshowlsppr");
    
    OS_MEMSET(&lspprotkey, 0, sizeof(LSPPROT_KEY));
    lspprotkey.ingress = ingress;
    lspprotkey.egress = egress;
    lspprotkey.tunnelid = tunnelid;
    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->lspprothash, &lspprotkey, 12);

    if (( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" lsppr 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" node       = %x\n", node);
    NBB_PRINTF(" key            = %x\n", node->key);
    NBB_PRINTF(" next           = %x\n", node->next);
        lspprot = (LSPPROT *)(node->data);

    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" prtype         = %d\n", lspprot->base.prtype);
    NBB_PRINTF(" returntype = %d\n", lspprot->base.returntype);
    NBB_PRINTF(" delaytime      = %d\n", lspprot->base.delaytime);
    NBB_PRINTF(" waittime       = %d\n", lspprot->base.waittime);
    
    NBB_PRINTF("  lspnum        = %d\n", lspprot->lspnum);

    NBB_PRINTF("  LSP 1:1\n");
    for ( i = 0; i < lspprot->lspnum; i++)
    {
        NBB_PRINTF("\n********************************\n");
        NBB_PRINTF(" lspid          = %x\n", lspprot->lspb.lsp[i].lspid);
        NBB_PRINTF(" state          = %d\n", lspprot->lspb.lsp[i].state);

        NBB_PRINTF(" label3         = %d\n", lspprot->dclsp[i].label3);
        NBB_PRINTF(" label2         = %d\n", lspprot->dclsp[i].label2);
        NBB_PRINTF(" label1         = %d\n", lspprot->dclsp[i].label1);
        NBB_PRINTF(" nextip         = %x\n", lspprot->dclsp[i].nextip);
        NBB_PRINTF(" nextport       = %x\n", lspprot->dclsp[i].nextport);
        NBB_PRINTF(" index          = %x\n", lspprot->dclsp[i].index);
        NBB_PRINTF(" ftnidx         = %x\n", lspprot->dclsp[i].ftnidx);
        NBB_PRINTF("\n***********************************\n");
    }
    NBB_PRINTF(" index      = %x\n", lspprot->index);
    NBB_PRINTF(" l2num      = %d\n", lspprot->l2num);
    NBB_PRINTF(" l3num      = %d\n", lspprot->l3num);
    NBB_PRINTF(" memflag    = %d\n", lspprot->memflag);
    NBB_PRINTF(" delflag    = %d\n", lspprot->delflag);
    NBB_PRINTF(" frrid      = %d\n", lspprot->frrid);
    NBB_PRINTF("\n***********************************\n");
    NBB_PRINTF(" error code = %d\n", lspprot->return_code);
    
    NBB_PRINTF("\n RSVP forever 1:1 CFG\n");
    NBB_PRINTF(" worklsp = |%-10x |%-10x |%-10x |%-5x \n", lspprot->worklspkey.ingress,
                lspprot->worklspkey.egress, lspprot->worklspkey.tunnelid, lspprot->worklspkey.lspid);
    NBB_PRINTF(" oldhwlsp[0] = |%-10x |%-10x |%-10x |%-5x \n", lspprot->oldhwlspkey[0].ingress,
                lspprot->oldhwlspkey[0].egress, lspprot->oldhwlspkey[0].tunnelid, lspprot->oldhwlspkey[0].lspid);
    NBB_PRINTF(" oldhwlsp[1] = |%-10x |%-10x |%-10x |%-5x \n", lspprot->oldhwlspkey[1].ingress,
                lspprot->oldhwlspkey[1].egress, lspprot->oldhwlspkey[1].tunnelid, lspprot->oldhwlspkey[1].lspid);
    NBB_PRINTF(" hwlsp[0] = |%-10x |%-10x |%-10x |%-5x \n", lspprot->hwlspkey[0].ingress,
                lspprot->hwlspkey[0].egress, lspprot->hwlspkey[0].tunnelid, lspprot->hwlspkey[0].lspid);
    NBB_PRINTF(" hwlsp[1] = |%x |%x |%x |%x \n", lspprot->hwlspkey[1].ingress,
                lspprot->hwlspkey[1].egress, lspprot->hwlspkey[1].tunnelid, lspprot->hwlspkey[1].lspid);
    NBB_PRINTF(" reverseflag       =%d\n", lspprot->reverseflag);
    NBB_PRINTF("  workstate        =%d\n", lspprot->hwstate);
    NBB_PRINTF("  posid[0-1]       = |%10x |%10x\n", lspprot->hwposid[0], lspprot->hwposid[1]);
    
    
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showlspprallkey( )
{
    NBB_ULONG       sum = 0;
    LSPPROT         *lspprot = NULL;
    SHASH_NODE      *node = NULL;
    NBB_ULONG       i = 0;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    
    if( v_spm_shared->lspprothash == NULL )
    {
        goto  EXIT_LABEL;
    }
    for (i = 0; i < 8000; i++)
    {
        node = v_spm_shared->lspprothash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                lspprot = (LSPPROT *)(node->data);
                NBB_PRINTF("ingress=%x egress=%x tunnelid=%x \n",
                                lspprot->key.ingress,
                                lspprot->key.egress,
                                lspprot->key.tunnelid);
                sum++;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " lspprot hash ERROR : index = %d", i));
            }
            node = node->next;
        }   
    }
    NBB_PRINTF("\n LSPPROT  sum = %d \n", sum);
    

    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showuroute(NBB_USHORT vrfid, NBB_BYTE addrtype, NBB_BYTE mask, 
                            NBB_ULONG dip,NBB_ULONG dip1,NBB_ULONG dip2,NBB_ULONG dip3)
{
    NBB_ULONG           i;
    VRFUROUTE_KEY       vrfukey;
    VRFUROUTE           *uroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spmshowuroute");

    OS_MEMSET(&vrfukey, 0, sizeof(VRFUROUTE_KEY));
    vrfukey.vrfid = vrfid;
    vrfukey.addrtype = addrtype;
    vrfukey.mask = mask;
    if( addrtype == 0 )
    {
        vrfukey.dip[3] = dip;
    }
    else if( addrtype == 1 )//暂时
    {
        vrfukey.dip[0] = dip;
        vrfukey.dip[1] = dip1;
        vrfukey.dip[2] = dip2;
        vrfukey.dip[3] = dip3;
    }
    NBB_PRINTF("\n vrfid = %d, addrtype = %d, mask = %d\n", vrfukey.vrfid, 
                                        vrfukey.addrtype, vrfukey.mask);
    uroute = (VRFUROUTE *)AVLL_FIND( v_spm_shared->vrfuroute_tree ,  &vrfukey ); 
    if( uroute != NULL )
    {
        NBB_PRINTF("  基本配置NDC\n");
        NBB_PRINTF(" routetype      = %d\n", uroute->base.routetype);
        NBB_PRINTF(" prmode         = %d\n", uroute->base.prmode);
        NBB_PRINTF(" ecmpmode       = %d\n", uroute->base.ecmpmode);
        NBB_PRINTF(" ecmpnum        = %d\n", uroute->base.ecmpnum);
        
        NBB_PRINTF(" 下一跳\n");
        NBB_PRINTF(" ipv4 nhop num  = %d\n", uroute->ipv4num);
        for ( i = 0; i < 4; i++) 
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nextip      = %x\n", uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport    = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label       = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb   = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid       = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip       = %x\n", uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port     = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex   = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n*************************************\n");
        }
        NBB_PRINTF(" ipv6 nhop num   = %d\n", uroute->ipv6num);
        for ( i = 0; i < 4; i++)
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nexttype    = %d\n", uroute->nexthop[i].nexttype);
            NBB_PRINTF(" nextip      = %x.%x.%x.%x\n", uroute->nexthop[i].nextip[0],
                                                    uroute->nexthop[i].nextip[1],
                                                    uroute->nexthop[i].nextip[2],
                                                    uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport    = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label       = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb   = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid       = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip       = %x.%x.%x.%x\n", uroute->dchop[i].ntip[0],
                                                    uroute->dchop[i].ntip[1],
                                                    uroute->dchop[i].ntip[2],
                                                    uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port     = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex   = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n************************************\n");
        }
        NBB_PRINTF(" flag            = %d\n", uroute->flag);
        NBB_PRINTF(" frrtype         = %d\n", uroute->frrtype);
        NBB_PRINTF(" ecmpflag        = %d\n", uroute->ecmpflag);
        NBB_PRINTF(" frrecmpid       = %d\n", uroute->frrecmpid);
        NBB_PRINTF("  内存块\n");
        NBB_PRINTF(" memindex        = %d\n", uroute->memindex);
        NBB_PRINTF(" memflag         = %d\n", uroute->memflag);
        
        //NBB_PRINTF(" prev             = %x\n", uroute->prev);
        NBB_PRINTF(" next            = %x\n", uroute->next);
        NBB_PRINTF("error code       = %d\n", uroute->return_code);
    
    }

    uroute = (VRFUROUTE *)AVLL_FIND( v_spm_shared->dcroute_tree ,  &vrfukey ); 
    if( uroute != NULL )
    {
        NBB_PRINTF("  基本配置DC\n");
        NBB_PRINTF(" routetype      = %d\n", uroute->base.routetype);
        NBB_PRINTF(" prmode         = %d\n", uroute->base.prmode);
        NBB_PRINTF(" ecmpmode       = %d\n", uroute->base.ecmpmode);
        NBB_PRINTF(" ecmpnum        = %d\n", uroute->base.ecmpnum);
        
        NBB_PRINTF(" 下一跳\n");
        NBB_PRINTF(" ipv4 nhop num  = %d\n", uroute->ipv4num);
        for ( i = 0; i < uroute->ipv4num; i++)
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nextip      = %x\n", uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport    = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label       = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb   = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid       = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip       = %x\n", uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port     = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex   = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n*************************************\n");
        }
        NBB_PRINTF(" ipv6 nhop num   = %d\n", uroute->ipv6num);
        for ( i = 0; i < uroute->ipv6num; i++)
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nexttype    = %d\n", uroute->nexthop[i].nexttype);
            NBB_PRINTF(" nextip      = %x.%x.%x.%x\n", uroute->nexthop[i].nextip[0],
                                                    uroute->nexthop[i].nextip[1],
                                                    uroute->nexthop[i].nextip[2],
                                                    uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport    = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label       = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb   = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid       = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip       = %x.%x.%x.%x\n", uroute->dchop[i].ntip[0],
                                                    uroute->dchop[i].ntip[1],
                                                    uroute->dchop[i].ntip[2],
                                                    uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port     = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex   = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n************************************\n");
        }
        NBB_PRINTF(" flag            = %d\n", uroute->flag);
        NBB_PRINTF(" frrtype         = %d\n", uroute->frrtype);
        NBB_PRINTF(" ecmpflag        = %d\n", uroute->ecmpflag);
        NBB_PRINTF(" frrecmpid       = %d\n", uroute->frrecmpid);
        NBB_PRINTF("  内存块\n");
        NBB_PRINTF(" memindex        = %d\n", uroute->memindex);
        NBB_PRINTF(" memflag         = %d\n", uroute->memflag);
        
        //NBB_PRINTF(" prev             = %x\n", uroute->prev);
        NBB_PRINTF(" next            = %x\n", uroute->next);
        NBB_PRINTF("error code       = %d\n", uroute->return_code);
    
    }
    uroute = (VRFUROUTE *)AVLL_FIND( v_spm_shared->lbroute_tree ,  &vrfukey ); 
    if( uroute != NULL )
    {
        NBB_PRINTF("  基本配置LB\n");
        NBB_PRINTF(" routetype      = %d\n", uroute->base.routetype);
        NBB_PRINTF(" prmode         = %d\n", uroute->base.prmode);
        NBB_PRINTF(" ecmpmode       = %d\n", uroute->base.ecmpmode);
        NBB_PRINTF(" ecmpnum        = %d\n", uroute->base.ecmpnum);
        
        NBB_PRINTF(" 下一跳\n");
        NBB_PRINTF(" ipv4 nhop num  = %d\n", uroute->ipv4num);
        for ( i = 0; i < uroute->ipv4num; i++)
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nextip     = %x\n", uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport   = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label      = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb  = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid      = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip      = %x\n", uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port    = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex  = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n*************************************\n");
        }
        NBB_PRINTF(" ipv6 nhop num  = %d\n", uroute->ipv6num);
        for ( i = 0; i < uroute->ipv6num; i++)
        {
            NBB_PRINTF("\n********************************\n");
            NBB_PRINTF(" nexttype   = %d\n", uroute->nexthop[i].nexttype);
            NBB_PRINTF(" nextip     = %x.%x.%x.%x\n", uroute->nexthop[i].nextip[0],
                                                    uroute->nexthop[i].nextip[1],
                                                    uroute->nexthop[i].nextip[2],
                                                    uroute->nexthop[i].nextip[3]);
            NBB_PRINTF(" nextport   = %d\n", uroute->nexthop[i].nextport);
            NBB_PRINTF(" label      = %d\n", uroute->nexthop[i].label);
            NBB_PRINTF(" nexttypeb  = %d\n", uroute->nexthop[i].nexttypeb);
            NBB_PRINTF(" bfdid      = %d\n", uroute->nexthop[i].bfdid);

            NBB_PRINTF(" dc ip      = %x.%x.%x.%x\n", uroute->dchop[i].ntip[0],
                                                    uroute->dchop[i].ntip[1],
                                                    uroute->dchop[i].ntip[2],
                                                    uroute->dchop[i].ntip[3]);
            NBB_PRINTF(" dc port    = %d\n", uroute->dchop[i].nextport);
            NBB_PRINTF(" nextindex  = %x\n", uroute->nextindex[i]);
            NBB_PRINTF("\n************************************\n");
        }
        NBB_PRINTF(" flag           = %d\n", uroute->flag);
        NBB_PRINTF(" frrtype        = %d\n", uroute->frrtype);
        NBB_PRINTF(" ecmpflag       = %d\n", uroute->ecmpflag);
        NBB_PRINTF(" frrecmpid      = %d\n", uroute->frrecmpid);
        NBB_PRINTF("  内存块\n");
        NBB_PRINTF(" memindex       = %d\n", uroute->memindex);
        NBB_PRINTF(" memflag        = %d\n", uroute->memflag);
        
        //NBB_PRINTF(" prev             = %x\n", uroute->prev);
        NBB_PRINTF(" next           = %x\n", uroute->next);
        NBB_PRINTF("error code      = %d\n", uroute->return_code);
    
    }
    if (uroute == NULL)
    {
        //NBB_PRINTF(" route 不存在!\n");
        goto  EXIT_LABEL;
    }

    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showsumucroute( NBB_BYTE type )
{
    NBB_ULONG           sum = 0;
    VRFUROUTE           *uroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spmshowuroute");

    if(( type == 0 ) || ( type == 1 ))
    {
        sum = 0;
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->dcroute_tree);
        while (uroute != NULL)
        {
            sum++;
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->dcroute_tree, uroute->route_node);
        }
        NBB_PRINTF("\n DC Route  sum = %d \n", sum);
    }
    if(( type == 0 ) || ( type == 2 ))
    {
        sum = 0;
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->vrfuroute_tree);
        while (uroute != NULL)
        {
            sum++;
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->vrfuroute_tree, uroute->route_node);
        }
        NBB_PRINTF("\n UDC Route  sum = %d \n", sum);
    }
    if(( type == 0 ) || ( type == 3 ))
    {
        sum = 0;
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->lbroute_tree);
        while (uroute != NULL)
        {
            sum++;
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->lbroute_tree, uroute->route_node);
        }
        NBB_PRINTF("\n Host Route  sum = %d \n", sum);
    }
    NBB_PRINTF("\n add route ipsnum = %d \n",  v_spm_shared->ipsroute.addipsnum);
    NBB_PRINTF("\n add route ok num = %d \n",  v_spm_shared->ipsroute.addipsok);
    NBB_PRINTF("\n del route  ipsnum = %d \n",  v_spm_shared->ipsroute.delipsnum);
    NBB_PRINTF("\n del route  ok num = %d \n",  v_spm_shared->ipsroute.delipsok);
    
    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showrouteallkey( NBB_BYTE type )
{
    VRFUROUTE           *uroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spmshowuroute");


       NBB_PRINTF("|%-6s%-6s%-6s%-24s\n","VRF", "V4/6", "MASK", "DIP");
    if(( type == 0 ) || ( type == 1 ))
    {
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->dcroute_tree);
        while (uroute != NULL)
        {
            //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
        
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->dcroute_tree, uroute->route_node);
        }
    }
    if(( type == 0 ) || ( type == 2 ))
    {
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->vrfuroute_tree);
        while (uroute != NULL)
        {
            //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->vrfuroute_tree, uroute->route_node);
        }
    }
    if(( type == 0 ) || ( type == 3 ))
    {
        uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->lbroute_tree);
        while (uroute != NULL)
        {
            //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
            uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->lbroute_tree, uroute->route_node);
        }
        
    }

    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showmroute( NBB_USHORT vrfid,NBB_BYTE addrtype,
        NBB_ULONG sip0, NBB_ULONG sip1, NBB_ULONG sip2, NBB_ULONG sip3, 
        NBB_ULONG gip0, NBB_ULONG gip1, NBB_ULONG gip2, NBB_ULONG gip3)
{
    NBB_BYTE            i;
    MCROUTE_KEY     mroutekey;
    MCROUTE         *mroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    OS_MEMSET(&mroutekey, 0, sizeof(MCROUTE_KEY));
    mroutekey.vrfid = vrfid;
    mroutekey.addrtype = addrtype;
    mroutekey.ips[0] = sip0;
    mroutekey.ips[1] = sip1;
    mroutekey.ips[2] = sip2;
    mroutekey.ips[3] = sip3;
    
    mroutekey.ipg[0] = gip0;
    mroutekey.ipg[1] = gip1;
    mroutekey.ipg[2] = gip2;
    mroutekey.ipg[3] = gip3;

    mroute = (MCROUTE *)AVLL_FIND( v_spm_shared->mcroute_tree ,  &mroutekey ); 
    if (mroute == NULL)
    {
        NBB_PRINTF(" mc route 不存在!\n");
        goto  EXIT_LABEL;
    }
    
    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" mcid       = %d\n", mroute->base.mcid);
    NBB_PRINTF(" inport      = %d\n", mroute->base.inport);
    NBB_PRINTF(" outport      = %d\n", mroute->base.outport);
    NBB_PRINTF(" portnumm      = %d\n", mroute->base.portnumm);
    for ( i = 0; i < mroute->base.portnumm ; i++)
    {
        NBB_PRINTF("\n********************************\n");
        NBB_PRINTF(" mc port %d\n", i + 1);
        NBB_PRINTF(" outport      = %d\n", mroute->base.outport[i]);
        NBB_PRINTF(" outcud      = %x\n", mroute->outcud[i]);
        NBB_PRINTF(" posid      = %x\n", mroute->posid[i]);
        NBB_PRINTF(" outslot      = %d\n", mroute->outslot[i]);
    }
    

    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showmrouteallkey( )
{
    NBB_ULONG           sum = 0;
    MCROUTE         *mroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    //NBB_TRC_ENTRY("spmshowuroute");
    NBB_PRINTF("|%-6s%-6s%-24s%-36s\n","VRF", "V4/6", "SIP", "GIP");
    mroute =  (MCROUTE *)AVLL_FIRST(v_spm_shared->mcroute_tree);
    while (mroute != NULL)
    {
        NBB_PRINTF("|%-6d%-6d%-12x.%x.%x.%x%-12x.%x.%x.%x\n",
                                   mroute->key.vrfid,
                                   mroute->key.addrtype,
                                   mroute->key.ips[0],mroute->key.ips[1],mroute->key.ips[2],mroute->key.ips[3],
                            mroute->key.ipg[0],mroute->key.ipg[1],mroute->key.ipg[2],mroute->key.ipg[3]);

        sum++;
        mroute = (MCROUTE *)AVLL_NEXT( v_spm_shared->mcroute_tree, mroute->route_node);
    }
    NBB_PRINTF("\n  MCROUTE   sum = %d \n", sum);
    

    EXIT_LABEL:
        
    //NBB_TRC_EXIT();
    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showtunnelsel( NBB_USHORT vrfid, NBB_ULONG peerip)
{
    TUNNELCH_KEY    tunnelskey;
    TUNNELCH        *tunnels = NULL;
    SHASH_NODE      *node = NULL;
    
    
    OS_MEMSET(&tunnelskey, 0, sizeof(TUNNELCH_KEY));
    tunnelskey.vrfid = vrfid;
    tunnelskey.peerip = peerip;

    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->tunnelchhash, &tunnelskey, 8);
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" tunnelsel 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" node       = %x\n", node);
    NBB_PRINTF(" key            = %x\n", node->key);
    NBB_PRINTF(" next           = %x\n", node->next);
        tunnels = (TUNNELCH *)(node->data);

    NBB_PRINTF("  基本配置\n");
    NBB_PRINTF(" ingress        = %x\n", tunnels->base.ingress);
    NBB_PRINTF(" egress         = %x\n", tunnels->base.egress);
    NBB_PRINTF(" tunnelid       = %x\n", tunnels->base.tunnelid);
    NBB_PRINTF("\n********************************\n");
    NBB_PRINTF(" delflag        = %x\n", tunnels->delflag );
    NBB_PRINTF(" pwnum          = %x\n", tunnels->pwnum);
    NBB_PRINTF(" tunnelindex    = %x\n", tunnels->tunindex);
    NBB_PRINTF("\n************************************\n");
    NBB_PRINTF(" error code     = %d\n", tunnels->return_code);
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showtxpw( NBB_USHORT vrfid, NBB_ULONG peerip, NBB_ULONG label)
{
    TUNNELCH_KEY    tunnelskey;
    TUNNELCH        *tunnels = NULL;
    SHASH_NODE      *node = NULL;
    PWTXNODE        *pwnode = NULL;

    PW_KEY          pwkey;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    OS_MEMSET(&tunnelskey, 0, sizeof(TUNNELCH_KEY));
    tunnelskey.vrfid = vrfid;
    tunnelskey.peerip = peerip;

    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->tunnelchhash, &tunnelskey, 8);
    if(( node == NULL ) || (node->data == NULL))
    {
        NBB_PRINTF(" tunnelsel 不存在!\n");
        goto  EXIT_LABEL;
    }
        tunnels = (TUNNELCH *)(node->data);

    pwkey.label = label;
    pwnode =  (PWTXNODE *)AVLL_FIND( tunnels->pwtree ,  &pwkey );
    if (pwnode == NULL)
    {
        NBB_PRINTF(" txpw 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" nhi        = %x\n", pwnode->nhi );
    NBB_PRINTF(" posid      = %x\n", pwnode->posid );
    NBB_PRINTF(" routenum   = %x\n", pwnode->routenum);
    EXIT_LABEL:
        
    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showtxpwallkeybak( NBB_USHORT vrfid, NBB_ULONG peerip)
{
    TUNNELCH_KEY    tunnelskey;
    TUNNELCH        *tunnels = NULL;
    SHASH_NODE      *node = NULL;
    PWTXNODE        *pwnode = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    OS_MEMSET(&tunnelskey, 0, sizeof(TUNNELCH_KEY));
    tunnelskey.vrfid = vrfid;
    tunnelskey.peerip = peerip;

    node = (SHASH_NODE *)showhashtabsearch( v_spm_shared->tunnelchhash, &tunnelskey, 8);
    if ((node == NULL) || (node->data == NULL))
    {
        NBB_PRINTF(" tunnelsel 不存在!\n");
        goto  EXIT_LABEL;
    }
        tunnels = (TUNNELCH *)(node->data);
        
    NBB_PRINTF(" pwnum :%x\n", tunnels->pwnum);
    pwnode =  (PWTXNODE *)AVLL_FIRST( tunnels->pwtree );
    NBB_PRINTF("|%-12s%-12s%-12s%-12s\n","LABEL", "NHI", "POSID", "ROUTENUM");
    while (pwnode != NULL)
    {
        NBB_PRINTF("|%-12x%-12x%-12x%-12d\n",
                                   pwnode->key.label,
                                   pwnode->nhi,
                                   pwnode->posid,
                            pwnode->routenum);
        pwnode = (PWTXNODE *)AVLL_NEXT( tunnels->pwtree, pwnode->pw_node);
    }
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showtxpwallkey()
{
    NBB_ULONG       i = 0;
    TUNNELCH        *tunnels = NULL;
    SHASH_NODE      *node = NULL;
    PWTXNODE        *pwnode = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    if( v_spm_shared->tunnelchhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    NBB_PRINTF("|%-12s%-12s%-12s%-12s%-12s\n","VRFPEER","LABEL", "NHI", "POSID", "ROUTENUM");
    for (i = 0; i < 8000; i++)
    {
        node = v_spm_shared->tunnelchhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                tunnels = (TUNNELCH *)(node->data);
                pwnode =  (PWTXNODE *)AVLL_FIRST(tunnels->pwtree);
                while (pwnode != NULL)
                {
                    NBB_PRINTF("|%-1d/%x%-12d%-12x%-12d%-12d\n",
                                        tunnels->key.vrfid,
                                        tunnels->key.peerip,
                                                pwnode->key.label,
                                                pwnode->nhi,
                                                pwnode->posid,
                                            pwnode->routenum);
                    pwnode = (PWTXNODE *)AVLL_NEXT( tunnels->pwtree, pwnode->pw_node);
                }
            }
            node = node->next;
        }   
    }
    
    EXIT_LABEL:
        
    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showl3uni( NBB_ULONG index, NBB_ULONG dip0,NBB_ULONG dip1,NBB_ULONG dip2,NBB_ULONG dip3)
{
    UNI_KEY         unikey;
    UNIPORT         *uniports = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    OS_MEMSET(&unikey , 0, sizeof(UNI_KEY));

    unikey.nextip[0] = dip0;    
    unikey.nextip[1] = dip1;    
    unikey.nextip[2] = dip2;    
    unikey.nextip[3] = dip3;    
    unikey.nextport = index;    
    uniports =  (UNIPORT *)AVLL_FIND( v_spm_shared->outuni ,&unikey);
    if ( uniports == NULL )
    {
        NBB_PRINTF(" l3uni 不存在!\n");
        goto  EXIT_LABEL;
    }
    NBB_PRINTF(" slot           = %d\n", uniports->port.slot);
    NBB_PRINTF(" port           = %d\n", uniports->port.port);
    NBB_PRINTF(" vlan           = %d\n", uniports->port.vlan);

    NBB_PRINTF(" nhi        = %x\n", uniports->nhid);
    NBB_PRINTF(" posid      = %x\n", uniports->posid );
    NBB_PRINTF(" routenum   = %d\n", uniports->routenum);
    EXIT_LABEL:
        
    return;
}



/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showl3uniallkey()
{
    UNIPORT         *uniports = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    uniports =  (UNIPORT *)AVLL_FIRST( v_spm_shared->outuni);
     NBB_PRINTF("|%-6s%-24s%-24s%-24s\n","PINDEX", 
                                                "IP", 
                                                "OUTPORT", 
                                                "ROUTENUM");
    while (uniports != NULL)
    {
        NBB_PRINTF("|%-6d%-24x.%x.%x.%x%-24d/%d/%d/%d%-12d\n",
                                uniports->key.nextport,
                                uniports->key.nextip[0],
                                uniports->key.nextip[1],
                                uniports->key.nextip[2],
                                uniports->key.nextip[3],
                                uniports->port.slot,
                                uniports->port.port,
                                uniports->port.vlan,
                                uniports->posid,
                         uniports->routenum);

        uniports = (UNIPORT*)AVLL_NEXT( v_spm_shared->outuni, uniports->uni_node);
    }
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showfrrallkey()
{
    MPEERFRR    *mpeerfrr = NULL;
    BPEERFRR    *bpeerfrr = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s%-6s%-12s\n","Mpeer", "bpnum", "Bpeer", "Frrid", "Frrnum");
    mpeerfrr =  (MPEERFRR *)AVLL_FIRST( v_spm_shared->peer_tree );
    while (mpeerfrr != NULL)
    {   
        bpeerfrr =  (BPEERFRR *)AVLL_FIRST( mpeerfrr->bpeer_tree );
        while (bpeerfrr != NULL)
        {
            NBB_PRINTF("|%-12x%-12d%-12x%-6d%-12d\n",
                                                    mpeerfrr->key.peer,
                                                    mpeerfrr->bpeernum,
                                                    bpeerfrr->key.peer,
                                                    bpeerfrr->frrid,
                                                    bpeerfrr->frrnum);
            bpeerfrr = (BPEERFRR *)AVLL_NEXT( mpeerfrr->bpeer_tree, bpeerfrr->bp_node);
        }
        mpeerfrr = (MPEERFRR *)AVLL_NEXT( v_spm_shared->peer_tree, mpeerfrr->mp_node);
    }
    
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showecmpallkey()
{
    ECMPTAB     *ecmp = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-24s%-12s%-12s%-12s%-12s\n","NHI", "Ecmpid", "PEcmpnum", "PEER","num");
    ecmp =  (ECMPTAB *)AVLL_FIRST( v_spm_shared->ecmp_tree );
    while (ecmp != NULL)
    {
        NBB_PRINTF("|%-12x.%x.%x.%x%-12d%-6d/%d%-12x.%x.%x.%x%-6d\n",
                                ecmp->key.nhi1,
                                ecmp->key.nhi2,
                                ecmp->key.nhi3,
                                ecmp->key.nhi4,
                                ecmp->ecmpid,
                                ecmp->pflag,
                                ecmp->ecmpnum,
                                ecmp->peerip[0],
                                ecmp->peerip[1],
                                ecmp->peerip[2],
                                ecmp->peerip[3],
                                ecmp->num);

        ecmp = (ECMPTAB *)AVLL_NEXT( v_spm_shared->ecmp_tree, ecmp->ep_node);
    }
    
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showftntol2()
{
    PEERFTN     *peerftn = NULL;
    PWVC        *pwvc = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s%-12s%-12s\n","Peer", "Tunnel", "Vpnum", "Vpid", "Vcid");
    peerftn =  (PEERFTN *)AVLL_FIRST( v_spm_shared->peftn_tree );
    while (peerftn != NULL)
    {   
        pwvc =  (PWVC *)AVLL_FIRST( peerftn->vp_tree );
        while (pwvc != NULL)
        {
            NBB_PRINTF("|%-12x%-12d%-12d%-12x%-12x\n",
                                                    peerftn->key.peer,
                                                    peerftn->tunnel,
                                                        peerftn->vpnum,
                                                    pwvc->key.vpid,
                                                    pwvc->key.vcid);
            pwvc = (PWVC *)AVLL_NEXT( peerftn->vp_tree, pwvc->vc_node);
        }
        peerftn = (PEERFTN *)AVLL_NEXT( v_spm_shared->peftn_tree, peerftn->pe_node);
    }
    
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showftntol3()
{
    PEERFTN     *peerftn = NULL;
    PWVRF       *pwvrf = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s%-12s\n","Peer", "Tunnel", "Vrfum", "Vrfid");
    peerftn =  (PEERFTN *)AVLL_FIRST( v_spm_shared->peftn_tree );
    while (peerftn != NULL)
    {   
        pwvrf =  (PWVRF *)AVLL_FIRST( peerftn->vrf_tree );
        while (pwvrf != NULL)
        {
            NBB_PRINTF("|%-12x%-12d%-12d%-12d \n",
                                                    peerftn->key.peer,
                                                    peerftn->tunnel,
                                                        peerftn->vrfnum,
                                                    pwvrf->key.vrfid);
            pwvrf = (PWVRF *)AVLL_NEXT( peerftn->vrf_tree, pwvrf->vrf_node);
        }
        peerftn = (PEERFTN *)AVLL_NEXT( v_spm_shared->peftn_tree, peerftn->pe_node);
    }
    
    EXIT_LABEL:
        
    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showftntol2mc()
{
    PEERFTN     *peerftn = NULL;
    PWMC        *pwmc = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s%-12s\n","Peer", "Tunnel", "Mcnum", "Mcid");
    peerftn =  (PEERFTN *)AVLL_FIRST( v_spm_shared->peftn_tree );
    while (peerftn != NULL)
    {   
        pwmc =  (PWMC *)AVLL_FIRST( peerftn->mcid_tree );
        while (pwmc != NULL)
        {
            NBB_PRINTF("|%-12x%-12d%-12d%-12d \n",
                                                    peerftn->key.peer,
                                                    peerftn->tunnel,
                                                        peerftn->mcnum,
                                                    pwmc->key.mcid);
            pwmc = (PWMC *)AVLL_NEXT( peerftn->mcid_tree, pwmc->mc_node);
        }
        peerftn = (PEERFTN *)AVLL_NEXT( v_spm_shared->peftn_tree, peerftn->pe_node);
    }
    
    EXIT_LABEL:
        
    return;
}

/******************************************************************************
 * FunctionName 	: 	l3showrsvptol2vp
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-04
 * Description		:   show rsvp l2vp	
 * InputParam	:	    no
 * OutputParam	:	    no
 * ReturnValue	:	    no
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_VOID    l3showrsvptol2vp()
{
	PWVC		    *pwvc = NULL;
	RSVPCRLSP       *rsvplsp = NULL; 
	NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

	NBB_PRINTF("|%-12s%-12s%-12s%-12s%-12s%-12s%-12s\n","Ingress","Engress", "Tunnel", "Vpnum", "tunnlid","Vpid", "Vcid");
	rsvplsp =  (RSVPCRLSP *)AVLL_FIRST( v_spm_shared->rsvplsp_tree);
	while (rsvplsp != NULL)
	{	
		pwvc =  (PWVC *)AVLL_FIRST( rsvplsp->vp_tree );
		while (pwvc != NULL)
		{
			NBB_PRINTF("|%-12x%-12x%-12d%-12d%-12d%-12x%-12x\n",
		                        					rsvplsp->key.ingress,
		                        					rsvplsp->key.egress,
		                        					rsvplsp->key.tunnelid,
		                        			        rsvplsp->vpnum,
		                        			        rsvplsp->tunlid,
		                        					pwvc->key.vpid,
		                        					pwvc->key.vcid);
			pwvc = (PWVC *)AVLL_NEXT( rsvplsp->vp_tree, pwvc->vc_node);
		}
		rsvplsp = (RSVPCRLSP *)AVLL_NEXT( v_spm_shared->rsvplsp_tree, rsvplsp->rsvp_node);
	}
	
	return;
	
}

/******************************************************************************
 * FunctionName 	: 	l3showrsvptovrf
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-04
 * Description		:   show rsvp l3 vrf	
 * InputParam	:	    no
 * OutputParam	:	    no
 * ReturnValue	:	    no
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_VOID    l3showrsvptovrf()
{
	PWVPN       *pwvpn = NULL;
	RSVPCRLSP       *rsvplsp = NULL; 
	NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

	NBB_PRINTF("|%-12s%-12s%-12s%-12s%-12s%-12s%-12s\n","Ingress","Engress", "Tunnel", 
		                                              "Vrfpeerum", "tunlid", "vrfid","peeip");
	rsvplsp =  (RSVPCRLSP *)AVLL_FIRST( v_spm_shared->rsvplsp_tree);
	while (rsvplsp != NULL)
	{	
		pwvpn =  ( PWVPN*)AVLL_FIRST( rsvplsp->vrfpeer_tree);
		while (pwvpn != NULL)
		{
			NBB_PRINTF("|%-12x%-12x%-12d%-12d%-12x%-12x%-12x\n",
		                        					rsvplsp->key.ingress,
		                        					rsvplsp->key.egress,
		                        					rsvplsp->key.tunnelid,
		                        			        rsvplsp->vrfpeernum,
		                        			        rsvplsp->tunlid,
		                        					pwvpn->key.vrfid,
		                        					pwvpn->key.peerip);
			pwvpn = (PWVPN *)AVLL_NEXT( rsvplsp->vrfpeer_tree, pwvpn->vrf_node);
		}
		rsvplsp = (RSVPCRLSP *)AVLL_NEXT( v_spm_shared->rsvplsp_tree, rsvplsp->rsvp_node);
	}
	
	return;
	
}

/******************************************************************************
 * FunctionName 	: 	l3showrsvptol2mc
 * Author		: 	    wjhe
 * CreateDate		:	2016-1-26
 * Description		:   show rsvp l2 mc	
 * InputParam	:	    no
 * OutputParam	:	    no
 * ReturnValue	:	    no
 * Relation		:	    no 
 * OtherInfo		:	
******************************************************************************/
NBB_VOID    l3showrsvptol2mc()
{
    PWMC		    *pwmc = NULL;
	RSVPCRLSP       *rsvplsp = NULL;
	NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

	NBB_PRINTF("|%-12s%-12s%-12s%-12s%-12s%-12s\n","Ingress","Egress" ,"Tunnel", "Mcnum", "Mcid","port");
	rsvplsp =  (RSVPCRLSP *)AVLL_FIRST( v_spm_shared->rsvplsp_tree);
	while (rsvplsp != NULL)
	{	
		pwmc =  (PWMC *)AVLL_FIRST( rsvplsp->mcid_tree );
		while (pwmc != NULL)
		{
			NBB_PRINTF("|%-12x%-12x%-12d%-12d%-12d%-12d \n",
		                        					rsvplsp->key.ingress,
		                        					rsvplsp->key.egress,
		                        					rsvplsp->key.tunnelid,
		                        			        rsvplsp->mcnum,
		                        					pwmc->key.mcid,
		                        					pwmc->port);
			pwmc = (PWMC *)AVLL_NEXT( rsvplsp->mcid_tree, pwmc->mc_node);
		}
		rsvplsp = (RSVPCRLSP *)AVLL_NEXT( v_spm_shared->rsvplsp_tree, rsvplsp->rsvp_node);
	}
	
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showftntoecmp()
{
    PEEREPTAB       *peecmp = NULL;
    ECMPNODE        *ecmpnd = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s%-12s\n","Peer", "Ecmpnum", "Ecmpid", "Citenum");
    peecmp =  (PEEREPTAB *)AVLL_FIRST( v_spm_shared->peecmp_tree );
    while (peecmp != NULL)
    {   
        ecmpnd =  (ECMPNODE *)AVLL_FIRST( peecmp->ecmpnode_tree );
        while (ecmpnd != NULL)
        {
            NBB_PRINTF("|%-12x%-12d%-12d%-12d \n",
                                                    peecmp->key.peer,
                                                        peecmp->num,
                                                        ecmpnd->key.ecmpid,
                                                    ecmpnd->citenum);
            ecmpnd = (ECMPNODE *)AVLL_NEXT( peecmp->ecmpnode_tree, ecmpnd->en_node);
        }
        peecmp = (PEEREPTAB *)AVLL_NEXT( v_spm_shared->peecmp_tree, peecmp->pp_node);
    }
    
    EXIT_LABEL:
        
    return;
}

NBB_VOID l3showpeerecmp( NBB_ULONG peer,NBB_USHORT ecmpid)
{

    PEEREP_KEY      peerkey;
    ECMPEN_KEY      ecmpenkey;
    PEEREPTAB       *peecmp = NULL;
    ECMPNODE        *ecmpnd = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-6s%-12s%-12s\n","Peer", "Ecmpid", "meb", "mebnum");



    peerkey.peer = peer;
    peecmp =  (PEEREPTAB *)AVLL_FIND( v_spm_shared->peecmp_tree, &peerkey ); 
    if (peecmp != NULL)
    {   
        ecmpenkey.ecmpid = ecmpid;
        ecmpnd =  (ECMPNODE *)AVLL_FIND( peecmp->ecmpnode_tree, &ecmpenkey );
        if (ecmpnd != NULL)
        {
            NBB_PRINTF("|%-12x%-6d%-12x.%x.%x.%x%-12d \n",
                                                    peecmp->key.peer,
                                                        ecmpnd->key.ecmpid,
                                                    ecmpnd->nhi[0],
                                                    ecmpnd->nhi[1],
                                                    ecmpnd->nhi[2],
                                                    ecmpnd->nhi[3],              
                                                    ecmpnd->citenum);
            
        }
    }
    
    EXIT_LABEL:
        
    return;
}

NBB_VOID l3showbfdfrr()
{
    BFDFRR          *bfdfrrp = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    NBB_PRINTF("|%-12s%-12s%-12s\n","Bfdid", "Frrid", "Citenum");
    bfdfrrp =  (BFDFRR *)AVLL_FIRST( v_spm_shared->bfdfrr_tree );
    while (bfdfrrp != NULL)
    {   
        NBB_PRINTF("|%-12x%-12d%-12d \n",bfdfrrp->key.bfdid,
                                                        bfdfrrp->frrid,
                                                    bfdfrrp->citenum);
    
        bfdfrrp = (BFDFRR *)AVLL_NEXT( v_spm_shared->bfdfrr_tree, bfdfrrp->bf_node);
    }
    
    EXIT_LABEL:
        
    return;
}

NBB_VOID l3showtunnelselallkey( )
{
    NBB_ULONG       sum = 0;
    TUNNELCH        *tunnels = NULL;
    SHASH_NODE      *node = NULL;
    NBB_ULONG       i = 0;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();

    if( v_spm_shared->tunnelchhash == NULL )
    {
        goto  EXIT_LABEL;
    }
    for (i = 0; i < 8000; i++)
    {
        node = v_spm_shared->tunnelchhash->index[i];
        while(node != NULL)
        {
            if(node->data != NULL)
            {
                tunnels = (TUNNELCH *)(node->data);
                NBB_PRINTF("vrfid=%d  peerip=%x \n",
                                tunnels->key.vrfid,
                                tunnels->key.peerip);
                sum++;
            }
            else
            {
                NBB_TRC_FLOW((NBB_FORMAT " tunnels hash ERROR : index = %d", i));
            }
            node = node->next;
        }   
    }
    NBB_PRINTF("\n TUNNELCH  sum = %d \n", sum);
    

    EXIT_LABEL:
        
    return;
}

/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showroutebyvrf( NBB_USHORT vrfid )
{
    VRFUROUTE           *uroute = NULL;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();


       NBB_PRINTF("|%-6s%-6s%-6s%-24s\n","VRF", "V4/6", "MASK", "DIP");
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->dcroute_tree);
    while (uroute != NULL)
    {
        //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
        if( vrfid == uroute->key.vrfid )
        {
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->dcroute_tree, uroute->route_node);
    }
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->vrfuroute_tree);
    while (uroute != NULL)
    {
        //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
        if( vrfid == uroute->key.vrfid )
        {
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->vrfuroute_tree, uroute->route_node);
    }
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->lbroute_tree);
    while (uroute != NULL)
    {
        //NBB_PRINTF("vrfid=%d type=%d mask=%d dip= %x.%x.%x.%x \n",
        if( vrfid == uroute->key.vrfid )
        {
            NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                    uroute->key.vrfid,
                                    uroute->key.addrtype,
                                    uroute->key.mask,
                                    uroute->key.dip[0],
                                    uroute->key.dip[1],
                                    uroute->key.dip[2],
                                    uroute->key.dip[3]);
        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->lbroute_tree, uroute->route_node);
    }
        
    EXIT_LABEL:
        
    return;
}


/******************************************************************************
 * FunctionName 	: 	spm_l3_fun
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    rv
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3showroutebyip( NBB_BYTE addrtype,NBB_ULONG dip0, NBB_ULONG dip1, NBB_ULONG dip2, NBB_ULONG dip3)
{
    VRFUROUTE           *uroute = NULL;
    NBB_ULONG           dip = 0;
    NBB_BYTE            mask;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();



       NBB_PRINTF("|%-6s%-6s%-6s%-24s\n","VRF", "V4/6", "MASK", "DIP");
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->dcroute_tree);
    while (uroute != NULL)
    {
        if( addrtype == uroute->key.addrtype )
        {   
            if( addrtype == 0)
            {
                dip = dip3;
                mask = uroute->key.mask;
                dip = (dip >>(32 - mask));
                dip = (dip << (32 - mask));

                if( uroute->key.dip[3] == dip)
                {
                    NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                            uroute->key.vrfid,
                                            uroute->key.addrtype,
                                            uroute->key.mask,
                                            uroute->key.dip[0],
                                            uroute->key.dip[1],
                                            uroute->key.dip[2],
                                            uroute->key.dip[3]);
                }
            }
            else if( addrtype == 1)
            {
                mask = uroute->key.mask;
                if(( mask > 95 ) && ( uroute->key.dip[2] == dip2)
                            && ( uroute->key.dip[1] == dip1)
                            && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip3;
                    dip = (dip >>(128 - mask));
                    dip = (dip << (128 - mask));
                    if( uroute->key.dip[3] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 63 ) && ( uroute->key.dip[1] == dip1)
                                && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip2;
                    dip = (dip >>(96 - mask));
                    dip = (dip << (96 - mask));
                    if( uroute->key.dip[2] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 31 ) && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip1;
                    dip = (dip >>(64 - mask));
                    dip = (dip << (64 - mask));
                    if( uroute->key.dip[1] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if( mask > 0 )
                {
                    dip = dip0;
                    dip = (dip >>(32 - mask));
                    dip = (dip << (32 - mask));
                    if( uroute->key.dip[0] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }


            }

        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->dcroute_tree, uroute->route_node);
    }
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->vrfuroute_tree);
    while (uroute != NULL)
    {
        if( addrtype == uroute->key.addrtype )
        {   
            if( addrtype == 0)
            {
                dip = dip3;
                mask = uroute->key.mask;
                dip = (dip >>(32 - mask));
                dip = (dip << (32 - mask));
                
                //uroute->key.dip[3] == dip;
                if( uroute->key.dip[3] == dip)
                {
                    NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                            uroute->key.vrfid,
                                            uroute->key.addrtype,
                                            uroute->key.mask,
                                            uroute->key.dip[0],
                                            uroute->key.dip[1],
                                            uroute->key.dip[2],
                                            uroute->key.dip[3]);
                }
            }
            else if( addrtype == 1)
            {
                mask = uroute->key.mask;
                if(( mask > 95 ) && ( uroute->key.dip[2] == dip2)
                            && ( uroute->key.dip[1] == dip1)
                            && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip3;
                    dip = (dip >>(128 - mask));
                    dip = (dip << (128 - mask));
                    if( uroute->key.dip[3] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 63 ) && ( uroute->key.dip[1] == dip1)
                                && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip2;
                    dip = (dip >>(96 - mask));
                    dip = (dip << (96 - mask));
                    if( uroute->key.dip[2] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 31 ) && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip1;
                    dip = (dip >>(64 - mask));
                    dip = (dip << (64 - mask));
                    if( uroute->key.dip[1] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if( mask > 0 )
                {
                    dip = dip0;
                    dip = (dip >>(32 - mask));
                    dip = (dip << (32 - mask));
                    if( uroute->key.dip[0] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }


            }

        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->vrfuroute_tree, uroute->route_node);
    }
    
    uroute =  (VRFUROUTE *)AVLL_FIRST(v_spm_shared->lbroute_tree);
    while (uroute != NULL)
    {

        if( addrtype == uroute->key.addrtype )
        {   
            if( addrtype == 0)
            {
                dip = dip3;
                mask = uroute->key.mask;
                dip = (dip >>(32 - mask));
                dip = (dip << (32 - mask));
                
                //uroute->key.dip[3] == dip;
                if( uroute->key.dip[3] == dip)
                {
                    NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                            uroute->key.vrfid,
                                            uroute->key.addrtype,
                                            uroute->key.mask,
                                            uroute->key.dip[0],
                                            uroute->key.dip[1],
                                            uroute->key.dip[2],
                                            uroute->key.dip[3]);
                }
            }
            else if( addrtype == 1)
            {
                mask = uroute->key.mask;
                if(( mask > 95 ) && ( uroute->key.dip[2] == dip2)
                            && ( uroute->key.dip[1] == dip1)
                            && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip3;
                    dip = (dip >>(128 - mask));
                    dip = (dip << (128 - mask));
                    if( uroute->key.dip[3] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 63 ) && ( uroute->key.dip[1] == dip1)
                                && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip2;
                    dip = (dip >>(96 - mask));
                    dip = (dip << (96 - mask));
                    if( uroute->key.dip[2] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if(( mask > 31 ) && ( uroute->key.dip[0] == dip0))
                {
                    dip = dip1;
                    dip = (dip >>(64 - mask));
                    dip = (dip << (64 - mask));
                    if( uroute->key.dip[1] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }
                else if( mask > 0 )
                {
                    dip = dip0;
                    dip = (dip >>(32 - mask));
                    dip = (dip << (32 - mask));
                    if( uroute->key.dip[0] == dip)
                    {
                        NBB_PRINTF("|%-6d%-6d%-6d%-12x.%x.%x.%x\n",
                                                uroute->key.vrfid,
                                                uroute->key.addrtype,
                                                uroute->key.mask,
                                                uroute->key.dip[0],
                                                uroute->key.dip[1],
                                                uroute->key.dip[2],
                                                uroute->key.dip[3]);
                    }
                }


            }

        }
        uroute = (VRFUROUTE *)AVLL_NEXT( v_spm_shared->lbroute_tree, uroute->route_node);
    }
        
    EXIT_LABEL:
        
    return;
}


NBB_VOID l3log()
{
    BMU_SLOG(BMU_CRIT, SPM_L3_LOG_DIR, "");
}

/******************************************************************************
 * FunctionName 	: 	l3help
 * Author		: 	    dengkai
 * CreateDate		:	2015-3-11
 * Description		:   KEY 
 * InputParam	:	    KEY
 * OutputParam	:	    no
 * ReturnValue	:	    no
 * Relation		:	    called by L3    
 * OtherInfo		:	
******************************************************************************/
NBB_VOID l3help()
{
    char **psg;
    
    static char *helpsg[] =
    {
        "l3showftntol2",                "NULL",
        "l3showftntol3",                "NULL",
        "l3showftntol2mc",              "NULL",
        "l3showrsvptol2vp",             "NULL",
        "l3showrsvptovrf",              "NULL",
        "l3showrsvptol2mc",             "NULL",
        "l3showftntoecmp",          "NULL",
        "l3showpeerecmp",               "peerip ecmpid",
        "l3showbfdfrr",             "NULL",
        "l3showfrrallkey",              "NULL",
        "l3showecmpallkey",         "NULL",
        "l3showtxpw",                   "vrfid peerip label",
        "l3showtxpwallkey",         "vrfid peerip",
        "spmshowmem",               "type",
        "l3showl3uni",                  "port ip0,1,2,3",
        "l3showl3uniallkey",                "NULL",
        "l3showmroute",             "vrfid addrtype sip0,1,2,3 gip0,1,2,3",
        "l3showmrouteallkey",           "NULL",
        "l3showtunnelsel",              "vrfid peerip",
        "l3showtunnelselallkey",            "NULL",
        "l3showuroute",             "vrfid addrtype mask dip",
        "l3showsumucroute",         "type",
        "l3showrouteallkey",            "NULL",
        "l3showroutebyvrf",         "vrfid",
        "l3showroutebyip",              "ipv4",
        NULL
    };

    NBB_PRINTF ("\n");
    for (psg = helpsg; *psg != NULL; psg += 2)
    {
        NBB_PRINTF(" %-20s  %-30s\n", *psg, *(psg + 1));
    }
    NBB_PRINTF ("\n");
}

NBB_VOID lsphelp()
{
    char **psg;

    static char *helpsg[] =
    {                       
        "lspshowftn",                   "vrfid  mask fec",
        "l3showftnallkey",              "NULL",
        "lspshowilm",                   "inlabel",
        "l3showilmallkey",              "NULL",
        "lspshowlsprx",             "ingress egress tunnelid lspid",
        "l3showlsprxallkey",                "NULL",
        "lspshowlsptx",             "ingress egress tunnelid lspid",
        "l3showlsptxallkey",            "NULL",
        "lspshowlsppr",             "ingress egress tunnelid",
        "l3showlspprallkey",            "NULL",
        NULL
    };

    NBB_PRINTF ("\n");
    for (psg = helpsg; *psg != NULL; psg += 2)
    {
        NBB_PRINTF (" %-20s %-30s\n", *psg, *(psg + 1));
    }
    NBB_PRINTF ("\n");
}

NBB_VOID apphelp()
{
    char **psg;

    static char *helpsg[] =
    {           
        "l2help",                   "phyport logport vc vpws vpls arp",
        "l3help",                   "uroute mroute pwnhi",
        "lsphelp",                  "ftn ilm rxlsp txlsp tunnel",
        "hwhelp",                   "hardware",
        "bfdhelp",                  "bfd",
        "tpoamhelp",                "tpoam",
        "qoshelp",              "qos",
        "apshelp",              "aps",
        "ptphelp",              "1588",
              "spm_set_print(PrintLevel)",  "PrintLevel : 1/3/5 = Info/Err/Off" ,
        NULL
    };

    NBB_PRINTF ("\n");
    for (psg = helpsg; *psg != NULL; psg += 2)
    {
        printf(" %-20s  %-30s\n", *psg, *(psg + 1));
    }
    NBB_PRINTF ("\n");
}



#endif
