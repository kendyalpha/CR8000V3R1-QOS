/******************************************************************************

                  版权所有 (C), 1999-2013, 烽火通信科技股份有限公司

******************************************************************************
   文 件 名   : spm_qos_proc.c
   版 本 号   : 初稿
   作    者   : zenglu
   生成日期   : 2012年9月18日
   最近修改   :
   功能描述   : QOS处理
   函数列表   :
   修改历史   :
   1.日    期   : 2012年9月18日
    作    者   : zenglu
    修改内容   : 创建文件

******************************************************************************/
#define SHARED_DATA_TYPE SPM_SHARED_LOCAL

#include <nbase.h>
#include <spmincl.h>
#include <TmApi.h>

/* 全局打印变量qos_vc_cfg_print */
extern NBB_BYTE qos_vc_cfg_print;

/* 全局打印变量qos_vrf_cfg_print */
extern NBB_BYTE qos_vrf_cfg_print;

/* 全局打印变量qos_txlsp_cfg_print */
extern NBB_BYTE qos_txlsp_cfg_print;

/* 全局打印变量qos_log_cfg_print */
extern NBB_BYTE qos_log_cfg_print;





/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 创建出口盘的VC
   输入参数  : usr group的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_create_voq(NBB_USHORT slot, NBB_LONG port,
    NBB_ULONG node_index, NBB_ULONG pwid NBB_CCXT_T NBB_CXT)
{
    NBB_BYTE fapid = 0; /*本槽位*/
    NBB_LONG fapid_index = 0; /*从0开始*/
    NBB_LONG baseQueue = 0;
    NBB_LONG baseVCId = 0;
    int rv = ATG_DCI_RC_OK;
    NBB_LONG slot_index = 0; /*从0开始*/
#if defined (SPU) || defined (SRC)
    NBB_LONG unit = 0;
#endif

    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*该操作只在出口盘执行*/
    if(slot != SHARED.local_slot_id)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
    
    /*检查本盘内的端口是否有效:有效-1,无效-0*/
#if defined (SPU) || defined (SRC)
    ApiAradCheckPortValid(unit, port + PTN_690_PORT_OFFSET, &rv);
    if (0 == rv) /*端口不存在跳过*/
    {
        printf("**QOS ERROR** %s %d ApiAradCheckPortValid port =%ld is unvalid\n",
                    __FUNCTION__,__LINE__,port);
        rv = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
#endif

    /* 判断slot是否开启hqos的开关 */
    rv = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if(ATG_DCI_RC_OK != rv)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 判断slot是否开启hqos的开关 */
    rv = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if(ATG_DCI_RC_OK != rv)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    for(fapid_index = 0; fapid_index < MAX_HQOS_SLOT_NUM; fapid_index++)
    {
        if(0 == SHARED.hqos_port_index[fapid_index])
        {
            continue; /*非HQOS槽位*/
        }
        
        /*端口8优先级初始化的时候已经建立过了*/
#if 0
#if defined (SPU) || defined (SRC)
        rv = ApiSetupBaseSeEtm(0, port);
#endif

        if (rv != 0)         /*执行结果判断*/
        {
            NBB_TRC_FLOW(("Error! ApiSetupBaseSeEtm err: %d(port=%ld)\n", rv, port));
            goto EXIT_LABEL;
        }
#endif
        baseQueue = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;
        baseVCId = offset * node_index + fapid_index * NUM_COS + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

        /*在出口端创建VC并和开启了hqos开关的单盘入口端VOQ建立关联*/
        rv = ApiAradHqosTrafficSet(unit, baseQueue, NUM_COS, SHARED.hqos_port_index[fapid_index], 
                    port + PTN_690_PORT_OFFSET, baseVCId, pwid);
#endif

        if (rv != 0)    /*执行结果判断*/
        {
            spm_hqos_arad_traffic_set_error_log(baseQueue,
                SHARED.hqos_port_index[fapid_index], port + PTN_690_PORT_OFFSET,
                baseVCId, pwid,__FUNCTION__,__LINE__,rv NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return rv;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 删除出口盘的VC
   输入参数  : usr group的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_voq(NBB_USHORT slot, NBB_USHORT port,
    NBB_ULONG node_index, NBB_LONG pw_id NBB_CCXT_T NBB_CXT)
{
    NBB_BYTE fapid = 0; /*本槽位*/
    NBB_LONG fapid_index = 0; /*从0开始*/
    NBB_ULONG baseQueue = 0;
    NBB_ULONG baseVCId = 0;
    int rv = 0;
    NBB_LONG slot_index = 0; /*从0开始*/
#if defined (SPU) || defined (SRC)
    NBB_LONG unit = 0;
#endif

    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(slot != SHARED.local_slot_id)
    {
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
#if defined (SPU) || defined (SRC)
    ApiAradCheckPortValid(unit, port + PTN_690_PORT_OFFSET, &rv);
    if (0 == rv) /*端口不存在跳过*/
    {
        rv = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
#endif

    /* 判断slot是否开启hqos的开关 */
    rv = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != rv)
    {
        rv = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    
    /* 判断slot是否开启hqos的开关 */
    rv = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != rv)
    {
        rv = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    for(fapid_index = 0; fapid_index < MAX_HQOS_SLOT_NUM; fapid_index++)
    {
        if(0 == SHARED.hqos_port_index[fapid_index])
        {
            continue;/*非HQOS槽位*/
        }
        
        /*端口8优先级初始化的时候已经建立过了*/
#if 0
#if defined (SPU) || defined (SRC)
        rv = ApiSetupBaseSeEtm(0, port);
#endif

        if (rv != 0)         /*执行结果判断*/
        {
            NBB_TRC_FLOW(("Error! ApiSetupBaseSeEtm err: %d(port=%ld)\n", rv, port));
            goto EXIT_LABEL;
        }
#endif
        baseQueue = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;
        baseVCId = offset * node_index + fapid_index * NUM_COS + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

        /*在出口端删除创建的VC并删除和入口端VOQ建立的关联*/
        rv = ApiAradHqosTrafficDelete(unit, baseQueue, NUM_COS, SHARED.hqos_port_index[fapid_index], 
                    port + PTN_690_PORT_OFFSET, baseVCId, pw_id);
#endif
        if (rv != 0)    /*执行结果判断*/
        {
           spm_hqos_arad_traffic_del_error_log(baseQueue,
            SHARED.hqos_port_index[fapid_index],
            port + PTN_690_PORT_OFFSET, baseVCId, pw_id,
            __FUNCTION__,__LINE__,rv NBB_CCXT);

           /*异常跳出*/
           goto EXIT_LABEL;
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return rv;
}





/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 删除出口盘的VC
   输入参数  : usr group的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_USER_GROUP_CB *spm_find_group_cb(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    SPM_QOS_USER_GROUP_CB *p_group = NULL;

    NBB_TRC_ENTRY("spm_find_group_cb");

    p_group = AVLL_FIND(SHARED.qos_user_group_tree, &index);
    if (NULL == p_group)
    {
        printf("**QOS ERROR** %s %d can't find USR GROUP CFG =%ld\n",
                    __FUNCTION__,__LINE__,index);

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return p_group;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 删除出口盘的VC
   输入参数  : usr group的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_USER_GROUP_CB * spm_check_group_down_cb(NBB_ULONG index,
    NBB_USHORT slot, NBB_USHORT port, NBB_ULONG *policy_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_USER_GROUP_CB *p_group = NULL;

    NBB_TRC_ENTRY("spm_find_group_cb");

    if (0 == index) /*非法*/
    {
        printf("**QOS ERROR** %s %d group index = 0\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    p_group = AVLL_FIND(SHARED.qos_user_group_tree, &index);
    if (NULL == p_group)
    {
        printf("**QOS ERROR** %s %d can't find USR GROUP CFG =%ld\n",
                    __FUNCTION__,__LINE__,index);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (NULL == p_group->down_cfg_cb)
    {
        printf("**QOS ERROR** %s %d can't find USR GROUP DOWN CFG =%ld\n",
                    __FUNCTION__,__LINE__,index);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if ((0 != p_group->port) && (port != p_group->port))
    {
        printf("**QOS ERROR** %s %d group port is not the same\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if ((0 != p_group->slot) && (slot != p_group->slot))
    {
        printf("**QOS ERROR** %s %d group slot is not the same\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    *policy_index = p_group->down_cfg_cb->qos_policy_index;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return p_group;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 申请内存并申请lsp id 初始化HQOS树
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_HQOS_LSP_TX_CB *spm_alloc_hqos_lsp_tx_cb(SPM_QOS_TUNNEL_KEY *pkey,
    NBB_LONG defaulte NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LSP_TX_CB *pstTbl = NULL;
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_ULONG id = 0;
    NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pkey)
    {
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK == defaulte)
    {
        /*申请空闲的lsp_id*/
        ret = spm_hqos_apply_lsp_id(&id NBB_CCXT);

        /*申请lspid失败*/
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d spm_hqos_apply_lsp_id failed\n",
                        __FUNCTION__,__LINE__);

            /*异常跳出*/
            goto EXIT_LABEL;
        }   
    }

    /* 分配一个新的lsp_tx配置块qos表配置条目。*/
    pstTbl = (SPM_HQOS_LSP_TX_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_LSP_TX_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_LSP_TX_CB);

    /*分配内存失败*/
    if (pstTbl == NULL)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 初始lsp_tx配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_LSP_TX_CB));
    OS_MEMCPY(&pstTbl->lsp_tx_key, pkey, sizeof(SPM_QOS_TUNNEL_KEY));
    pstTbl->lsp_id = id;

    /*QOS vc 树初始化*/
    avll_key_offset = NBB_OFFSETOF(SPM_HQOS_VC_CB, vc_key);
    AVLL_INIT_TREE(pstTbl->vc_tree, spm_vc_key_compare,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_HQOS_VC_CB, spm_hqos_vc_node));

    /*QOS vrf 树初始化*/
    avll_key_offset = NBB_OFFSETOF(SPM_HQOS_VRF_CB, vrf_key);
    AVLL_INIT_TREE(pstTbl->vrf_tree, spm_qos_vrf_comp,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_HQOS_VRF_CB, spm_hqos_vrf_node));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_lsp_tx_node);

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 分配内存的同时分配PW ID
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_HQOS_VC_CB *spm_alloc_hqos_vc_cb(ATG_DCI_VC_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_VC_CB *pstTbl = NULL;
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_ULONG id = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pkey)
    {
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }
    ret = spm_hqos_apply_pw_id(&id NBB_CCXT);

    /*申请pwid失败*/
    if (ATG_DCI_RC_OK != ret)
    {
        printf("**QOS ERROR** %s %d spm_hqos_apply_pw_id failed\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 分配一个新的逻辑配置块qos表配置条目。*/
    pstTbl = (SPM_HQOS_VC_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_VC_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_VC_CB);

    /*分配内存失败*/
    if (pstTbl == NULL)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 初始逻辑配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_VC_CB));
    OS_MEMCPY(&(pstTbl->vc_key), pkey, sizeof(ATG_DCI_VC_KEY));
    pstTbl->pw_id = id;

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_vc_node);

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 申请内存并申请lsp id 初始化HQOS树
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_HQOS_VRF_CB *spm_alloc_hqos_vrf_cb(SPM_QOS_VRF_INSTANSE_KEY *pkey,
    NBB_LONG defaulte NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_ULONG id = 0;
    SPM_HQOS_VRF_CB *pstTbl = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pkey)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK == defaulte)
    {
        ret = spm_hqos_apply_pw_id(&id NBB_CCXT);

        /*申请pwid失败*/
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d spm_hqos_apply_pw_id failed\n",
                        __FUNCTION__,__LINE__);

            /*异常跳出*/
            goto EXIT_LABEL;
        }   
    }
    

    /* 分配hqos vrf 的A节点*/
    pstTbl = (SPM_HQOS_VRF_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_VRF_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_VRF_CB);

    /*分配内存失败*/
    if (pstTbl == NULL)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 初始hqos vrf vpn配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_VRF_CB));
    OS_MEMCPY(&(pstTbl->vrf_key), pkey, sizeof(SPM_QOS_VRF_INSTANSE_KEY));
    pstTbl->pw_id = id;

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_vrf_node);

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}








/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_hqos_lsp_tx_cb(SPM_HQOS_LSP_TX_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*释放内存失败*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free lsp",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LSP_TX_CB), MEM_SPM_HQOS_LSP_TX_CB);
    ret = spm_hqos_release_lsp_id(pstTbl->lsp_id NBB_CCXT);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LSP_TX_CB);
    pstTbl = NULL;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_hqos_vc_cb(SPM_HQOS_VC_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*释放内存失败*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free vc",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_VC_CB), MEM_SPM_HQOS_VC_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_VC_CB);
    pstTbl = NULL;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_hqos_vrf_cb(SPM_HQOS_VRF_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*释放内存失败*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free vrf",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_VRF_CB), MEM_SPM_HQOS_VRF_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);


    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_VRF_CB);
    pstTbl = NULL;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_hqos_log_group_cb(SPM_HQOS_LOG_GROUP_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*释放内存失败*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free group",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LOG_GROUP_CB), MEM_SPM_HQOS_LOG_GROUP_CB);
    spm_hqos_release_lsp_id(pstTbl->lsp_id NBB_CCXT);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LOG_GROUP_CB);
    pstTbl = NULL;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_hqos_log_usr_cb(SPM_HQOS_LOG_USR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*释放内存失败*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free usr",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LOG_USR_CB), MEM_SPM_HQOS_LOG_USR_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LOG_USR_CB);
    pstTbl = NULL;

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_connect_hqos_wred_cnt(ATG_DCI_QOS_BEHAVIOR_QUE_CONGST *cfg NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY("spm_connect_hqos_wred_cnt");

    if (NULL == cfg)
    {
        printf("**QOS ERROR** %s %d CFG== NULL\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        ret = spm_connect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        ret = spm_connect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_connect_wred_cnt(cfg->yellow_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_connect_wred_cnt(cfg->red_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : defaulte = ATG_DCI_RC_OK:申请LSP ID
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_disconnect_hqos_wred_cnt(ATG_DCI_QOS_BEHAVIOR_QUE_CONGST *cfg
    NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY("spm_disconnect_hqos_wred_cnt");

    if (NULL == cfg)
    {
        printf("**QOS ERROR** %s %d CFG== NULL\n",
                    __FUNCTION__,__LINE__);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        ret = spm_disconnect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {
        
            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        ret = spm_disconnect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {
        
            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_disconnect_wred_cnt(cfg->yellow_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_disconnect_wred_cnt(cfg->red_wred_id NBB_CCXT);

        /*设置不同的返回值*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : slot -- 出口槽位
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_hqos_wred(NBB_USHORT slot,
    ATG_DCI_QOS_BEHAVIOR_QUE_CONGST *cfg,
    NBB_ULONG flow_id,
    NBB_ULONG node_index NBB_CCXT_T NBB_CXT)
{
    NBB_INT ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (SRC)
    NBB_LONG unit = 0;
#endif
    NBB_ULONG voq = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG slot_index = 0;
    SPM_QOS_WRED_CB *green_cb = NULL;
    SPM_QOS_WRED_CB *yellow_cb = NULL;
    SPM_QOS_WRED_CB *red_cb = NULL;

    NBB_TRC_ENTRY("spm_add_hqos_wred");

    if (NULL == cfg)
    {
        printf("**QOS ERROR** %s %d cfg == NULL\n",
                    __FUNCTION__,__LINE__);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    voq = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;

    /*队尾丢弃*/
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
#if defined (SPU) || defined (SRC)
        ApiAradSetQueueTailDrop(UNIT_0, voq, cfg->queue_id, -1, 10 * 1024 * 1024);
        ret = ApiSetQueueWred(UNIT_0,voq,cfg->queue_id,0,-1,100,100,0);
#endif
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*色盲模式*/
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        green_cb = spm_qos_find_wred_cb(cfg->green_wred_id NBB_CCXT);
        if (NULL == green_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (NULL == green_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find green wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)
        ret = ApiAradSetQueueWred(unit, voq, cfg->queue_id, 1, -1, green_cb->basic_cfg_cb->end_threshold,
            green_cb->basic_cfg_cb->start_threshold, green_cb->basic_cfg_cb->discard_percent);
#endif
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d ret =%d ApiAradSetQueueWred\n",
                    __FUNCTION__,__LINE__,ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS que wred",ret,
						"voq","queue_id","WRED_MODE_COL_BIND","color",
						voq,cfg->queue_id,cfg->mode,-1));

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /*区分颜色wred*/
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        green_cb = spm_qos_find_wred_cb(cfg->green_wred_id NBB_CCXT);
        if (NULL == green_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (NULL == green_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find green wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)
        ret = ApiAradSetQueueWred(unit, voq, cfg->queue_id, 1, QOS_COLOR_GREEN, green_cb->basic_cfg_cb->end_threshold,
            green_cb->basic_cfg_cb->start_threshold, green_cb->basic_cfg_cb->discard_percent);
#endif
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d ret =%d ApiAradSetQueueWred\n",
                    __FUNCTION__,__LINE__,ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS que wred",ret,
						"voq","queue_id","WRED_MODE_COLOR","color",
						voq,cfg->queue_id,cfg->mode,0));

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        yellow_cb = spm_qos_find_wred_cb(cfg->yellow_wred_id NBB_CCXT);
        if (NULL == yellow_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (NULL == yellow_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find yellow wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)
        ret = ApiAradSetQueueWred(unit, voq, cfg->queue_id, 1, QOS_COLOR_YELLOW, yellow_cb->basic_cfg_cb->end_threshold,
            yellow_cb->basic_cfg_cb->start_threshold, yellow_cb->basic_cfg_cb->discard_percent);
#endif
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d ret =%d ApiAradSetQueueWred\n",
                    __FUNCTION__,__LINE__,ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS que wred",ret,
						"voq","queue_id","WRED_MODE_COLOR","color",
						voq,cfg->queue_id,cfg->mode,1));

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        red_cb = spm_qos_find_wred_cb(cfg->red_wred_id NBB_CCXT);
        if (NULL == red_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (NULL == red_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find red wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)
        ret = ApiAradSetQueueWred(unit, voq, cfg->queue_id, 1, QOS_COLOR_RED, red_cb->basic_cfg_cb->end_threshold,
            red_cb->basic_cfg_cb->start_threshold, red_cb->basic_cfg_cb->discard_percent);
#endif
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d ret =%d ApiAradSetQueueWred\n",
                    __FUNCTION__,__LINE__,ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS que wred",ret,
						"voq","queue_id","WRED_MODE_COLOR","color",
						voq,cfg->queue_id,cfg->mode,2));

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }


    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : slot -- 出口槽位
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_vrftree_counter(SPM_HQOS_LSP_TX_CB *pB,
    SPM_QOS_VRF_INSTANSE_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_VRF_CB *pcb = NULL;
    NBB_ULONG Counter = 0;

    NBB_TRC_ENTRY(__FUNCTION__);
    
    if((NULL != pB) && (NULL != pkey))
    {
        for(pcb = (SPM_HQOS_VRF_CB *)AVLL_FIRST(pB->vrf_tree); pcb != NULL;
            pcb = (SPM_HQOS_VRF_CB *)AVLL_NEXT(pB->vrf_tree,pcb->spm_hqos_vrf_node))
        {
            if((pcb->vrf_key.peer_ip == pkey->peer_ip) && (pcb->vrf_key.vrf_id == pkey->vrf_id))
            {
                Counter++;
            }   
        }
    }

    /*异常跳出*/
    NBB_TRC_EXIT();
    return Counter;
}

/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : slot -- 出口槽位
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_find_vrf_pwid(SPM_HQOS_LSP_TX_CB *pB,
    SPM_HQOS_VRF_CB *pvrf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_VRF_CB *pcb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);
    
    if((NULL != pB) && (NULL != pvrf))
    {
        for(pcb = (SPM_HQOS_VRF_CB *)AVLL_FIRST(pB->vrf_tree); pcb != NULL;
            pcb = (SPM_HQOS_VRF_CB *)AVLL_NEXT(pB->vrf_tree,pcb->spm_hqos_vrf_node))
        {
            if((pcb->vrf_key.peer_ip == pvrf->vrf_key.peer_ip) && (pvrf->vrf_key.vrf_id == pcb->vrf_key.vrf_id))
            {
                if(0 == pcb->pw_id)
                {
                    OS_SPRINTF(ucMessage,"%s %s,line=%d :ERROR vrf pw id ==0\n\n",
                       HQOS_VRF_CFG,__FUNCTION__,__LINE__);
                    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

                    spm_qos_vrf_key_pd(&(pvrf->vrf_key),HQOS_VRF_ERROR,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
                    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            			HQOS_VRF_CFG,ATG_DCI_RC_UNSUCCESSFUL,
            			ucMessage,__FUNCTION__,"LINE","",
            			0,0,__LINE__,0)); 
                    ret = ATG_DCI_RC_UNSUCCESSFUL;

                    /*异常跳出*/
                    goto EXIT_LABEL;
                }
                else if(0 == pvrf->pw_id)
                {
                    pvrf->pw_id = pcb->pw_id;
                    break;
                }
            }   
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : slot -- 出口槽位
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_lsp(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_LSP_TX_CB *pcb,
    NBB_ULONG upflag,
    NBB_ULONG node_index,
    NBB_ULONG index,
    NBB_ULONG nhi NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b = NULL;
    SPM_QOS_ACTION_CB *pA = NULL;
    NBB_BYTE fapid = 0;
    NBB_BYTE flag = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(index != 0);

    /*指针为空*/
    if (0 == index)
    {
        printf("**QOS ERROR**%s,%d policy index ==0\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS policy index ==0",ATG_DCI_RC_UNSUCCESSFUL,
    					"","","","",
    					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (NULL == pcb)
    {
        printf("**QOS ERROR**%s,%d pcb == NULL\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS pcb == NULL",ATG_DCI_RC_UNSUCCESSFUL,
    					"","","","",
    					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pB = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(index NBB_CCXT);

    /*policy配置不存在*/
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
    {
        printf("**QOS ERROR**%s,%d can't find policy or basic_cfg\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS can't find policy or basic_cfg",ATG_DCI_RC_UNSUCCESSFUL,
    					"policy index","","","",
    					index,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (0 != pB->basic_cfg_cb->mode) /*非调度节点*/
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    c2b = AVLL_FIRST(pB->c2b_tree);
    if (NULL == c2b)
    {
        printf("**QOS ERROR**%s,%d policy c2b==NULL\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS policy c2b==NULL",ATG_DCI_RC_UNSUCCESSFUL,
    					"","","","",
    					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pA = spm_qos_find_action_cb(c2b->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION不存在*/
    if (NULL == pA)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*如果是本槽位则进行建树*/
    {
        if (ATG_DCI_RC_OK != upflag) /*首次创建*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspCreate(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != ret) /*失败*/
        {
            spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,ret NBB_CCXT);
            spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET, pcb->lsp_id,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (pA->shaping_cfg_cb) /*流量整形*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspBandwidthSet(0, pcb->lsp_id, pA->shaping_cfg_cb->cir,
                pA->shaping_cfg_cb->pir, pA->shaping_cfg_cb->cbs, pA->shaping_cfg_cb->pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_lsp_bandwidth_error_log(pcb->lsp_id,
                    pA->shaping_cfg_cb->cir,pA->shaping_cfg_cb->pir,
                    pA->shaping_cfg_cb->cbs, 
                    pA->shaping_cfg_cb->pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        if ((pA->schedule_cfg_cb) && (0 != pA->schedule_cfg_cb->mode)) /*调度策略*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspWfqWeightSet(0, pcb->lsp_id, pA->schedule_cfg_cb->weight);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_lsp_weight_error_log(pcb->lsp_id, 
                    pA->schedule_cfg_cb->weight,__FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*有一个子tlv配置错误则删除LSP*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspDelete(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d ret = %ld ApiAradHqosLspDelete\n",
                    __FUNCTION__,__LINE__,ret); 
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiAradHqosLspDelete",ret,
						"slot","port","lsp id","upflag",
						slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
                spm_hqos_arad_del_lsp_error_log( port + PTN_690_PORT_OFFSET, pcb->lsp_id,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
        }

        /*只对出口盘做引用计数，如果配置失败，其他单盘无法下发配置*/
        (pB->cnt)++;
        (pA->cnt)++;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_vc(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_VC_CB *pcb,
    SPM_HQOS_LSP_TX_CB *plsp,
    ATG_DCI_VC_UP_VPN_QOS_POLICY *data,
    NBB_ULONG upflag NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *c2b_1 = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b_2 = NULL;
    
    //SPM_QOS_VRF_VPN_CB vrf_cb = {0};
    SPM_QOS_POLICY_CB *pB1 = NULL;
    SPM_QOS_POLICY_CB *pB2 = NULL;
    SPM_QOS_ACTION_CB *pA1 = NULL;
    SPM_QOS_ACTION_CB *pA2 = NULL;
    NBB_ULONG que4_1 = 0;
    NBB_ULONG que8_5 = 0;
    NBB_BYTE qflag = ATG_DCI_RC_UNSUCCESSFUL;
    NBB_BYTE fapid = 0;
    NBB_BYTE flag = ATG_DCI_RC_OK;
    NBB_BYTE i = 0;
    NBB_BYTE offset = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(plsp != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(data != NULL);

    /*指针为空*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->flow_queue_qos_policy_index NBB_CCXT);

    /*policy配置不存在*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*非调度节点*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*表明policy里面有action*/
    c2b_1 = AVLL_FIRST(pB1->c2b_tree);
    c2b_2 = AVLL_FIRST(pB2->c2b_tree);
    if ((NULL == c2b_1) || (NULL == c2b_2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pA1 = spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /* ACTION不存在 */
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*入口盘出口盘都需配置*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /* 如果是本槽位则进行建树 */
    {
        if (ATG_DCI_RC_OK != upflag) /* 不是更新 */
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwCreate(UNIT_0, port + PTN_690_PORT_OFFSET, plsp->lsp_id, pcb->pw_id,0);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_create_pw_error_log(port + PTN_690_PORT_OFFSET,plsp->lsp_id,
                    pcb->pw_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
                spm_free_hqos_vc_cb(pcb NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            
            /* 建立连接关系:建立开启了hqos开关的槽位与业务出槽位出端口之间的voq连接关系 */
            ret = spm_hqos_create_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)
                ApiAradHqosPwDelete(UNIT_0,pcb->pw_id,plsp->lsp_id);
#endif
                spm_free_hqos_vc_cb(pcb NBB_CCXT);

                /* 信任驱动删除正确 */ 
                goto EXIT_LABEL;
            }
        }
        if (pA1->shaping_cfg_cb)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwBandwidthSet(UNIT_0, pcb->pw_id, pA1->shaping_cfg_cb->cir,
                  pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_pw_bandwidth_error_log(pcb->pw_id, pA1->shaping_cfg_cb->cir,
                    pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
#endif
        }
        if ((pA1->schedule_cfg_cb) && (0 != pA1->schedule_cfg_cb->mode)) /* 调度策略 */
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwWfqWeightSet(0, pcb->pw_id, pA1->schedule_cfg_cb->weight);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_pw_weight_error_log(pcb->pw_id, pA1->schedule_cfg_cb->weight,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
#endif
        }
        for (i = 0; i < pA2->que_shape_num; i++)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosTrafficBandwidthSet(0, pcb->pw_id, pA2->que_shaping_cfg_cb[i].queue_id,
                  pA2->que_shaping_cfg_cb[i].cir,
                  pA2->que_shaping_cfg_cb[i].pir, pA2->que_shaping_cfg_cb[i].cbs,
                  pA2->que_shaping_cfg_cb[i].pbs);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_traffic_bandwidth_error_log(pcb->pw_id, 
                    pA2->que_shaping_cfg_cb[i].queue_id,
                    pA2->que_shaping_cfg_cb[i].cir,
                    pA2->que_shaping_cfg_cb[i].pir,
                    pA2->que_shaping_cfg_cb[i].cbs,
                    pA2->que_shaping_cfg_cb[i].pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
#endif
        }
        for (i = 0; i < pA2->que_schedul_num; i++)
        {
            offset = pA2->que_schedul_cfg_cb[i].queue_id % 4;
            if (pA2->que_schedul_cfg_cb[i].queue_id < 4)
            {

                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    que4_1 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            else
            {
                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    que8_5 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            qflag = ATG_DCI_RC_OK;
        }
        if (ATG_DCI_RC_OK == qflag)
        {
#if defined (SPU) || defined (SRC)

            /* 将pw的队列调度模式写入驱动中 */
            ret = ApiAradHqosTrafficScheModeSet(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, que8_5);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_trafficsche_error_log(port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, 
                    que8_5,__FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
#endif
        }

        //A_NODE_LABEL:
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*有一个子tlv配置错误则删除LSP*/
        {
        
            /* 删除已经创建的voq全连接 */
            spm_hqos_del_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
#if defined (SPU) || defined (SRC)
            ApiAradHqosPwDelete(UNIT_0, pcb->pw_id, plsp->lsp_id);
#endif
            spm_free_hqos_vc_cb(pcb NBB_CCXT);
            ret = ATG_DCI_RC_ADD_FAILED;
            goto EXIT_LABEL;
        }

        /*只对出口盘做引用计数，如果配置失败，其他单盘无法下发配置*/
        (pB1->cnt)++;
        (pB2->cnt)++;
        (pA1->cnt)++;
        (pA2->cnt)++;
        (plsp->pw_cnt)++;
        
        /*for (i = 0; i < pA2->que_congest_num; i++)
        {
            ret = spm_connect_hqos_wred_cnt(&(pA2->que_congst_cfg_cb[i]) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
        }*/
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_vrf(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_VRF_CB *pcb,
    SPM_HQOS_LSP_TX_CB *plsp,
    ATG_DCI_VRF_INSTANCE_UP_VPN_QOS *data,
    NBB_ULONG upflag NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *c2b_1 = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b_2 = NULL;
    SPM_QOS_POLICY_CB *pB1 = NULL;
    SPM_QOS_POLICY_CB *pB2 = NULL;
    SPM_QOS_ACTION_CB *pA1 = NULL;
    SPM_QOS_ACTION_CB *pA2 = NULL;
    NBB_ULONG que4_1 = 0;
    NBB_ULONG que8_5 = 0;
    NBB_BYTE qflag = ATG_DCI_RC_UNSUCCESSFUL;
    NBB_BYTE fapid = 0;
    NBB_BYTE flag = ATG_DCI_RC_OK;
    NBB_BYTE i = 0;
    NBB_BYTE offset = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);
    
    /* 输入参数指针必须有效 */
    NBB_ASSERT(plsp != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(data != NULL);

    /*指针为空*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->flow_queue_qos_policy_index NBB_CCXT);

    /*policy配置不存在*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*非调度节点,policy的basic_cfg_cb字段需配置为调度节点模式而非多c-b对模式*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*c-b对树非空*/
    c2b_1 = AVLL_FIRST(pB1->c2b_tree);
    c2b_2 = AVLL_FIRST(pB2->c2b_tree);
    if ((NULL == c2b_1) || (NULL == c2b_2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*调度节点c-b对中action非空*/
    pA1 = spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION不存在*/
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the Action can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*入口盘出口盘都需配置*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }

    }

    /*获取本槽位的槽位号*/
    spm_hardw_getslot(&fapid);

    /*如果是本槽位是目的槽位则进行建树,即只在出口盘下发hqos的配置*/
    if (fapid == slot) 
    {
        if (ATG_DCI_RC_OK != upflag) /* 新建,不是更新 */
        {
#if defined (SPU) || defined (SRC)

            /*创建PW,绑定到指定LSP,该LSP必须先创建(只在出口盘创建)*/
            ret = ApiAradHqosPwCreate(UNIT_0, port + PTN_690_PORT_OFFSET, plsp->lsp_id, pcb->pw_id,0);
#endif
            if (ATG_DCI_RC_OK != ret)
            { 
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR ApiAradHqosPwCreate failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VC_ERROR,ret NBB_CCXT);
                spm_hqos_arad_create_pw_error_log(port + PTN_690_PORT_OFFSET,plsp->lsp_id,
                                         pcb->pw_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
                spm_free_hqos_vrf_cb(pcb NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            
            /*建立出口盘与所有其他开启了hqos开关的单盘之间的连接关系,包括建立出口盘自己与自己之间的连接关系*/
            ret = spm_hqos_create_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_hqos_create_voq failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)
                ApiAradHqosPwDelete(UNIT_0,pcb->pw_id,plsp->lsp_id);
#endif
                spm_free_hqos_vrf_cb(pcb NBB_CCXT);

                /*信任驱动删除正确*/
                goto EXIT_LABEL;
            }
        }

        /*PW限速,即vrf限速*/
        if (pA1->shaping_cfg_cb)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwBandwidthSet(0, pcb->pw_id, pA1->shaping_cfg_cb->cir,
                pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_pw_bandwidth_error_log(pcb->pw_id, pA1->shaping_cfg_cb->cir,
                    pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }

        /* vrf的调度策略,即多个vrf处于WFQ模式时各PW的权重 */
        if ((pA1->schedule_cfg_cb) && (0 != pA1->schedule_cfg_cb->mode))
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwWfqWeightSet(0, pcb->pw_id, pA1->schedule_cfg_cb->weight);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_pw_weight_error_log(pcb->pw_id, pA1->schedule_cfg_cb->weight,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);

                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }

        /*vrf中各队列整形的限速*/
        for (i = 0; i < pA2->que_shape_num; i++)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosTrafficBandwidthSet(0, pcb->pw_id,pA2->que_shaping_cfg_cb[i].queue_id,
                pA2->que_shaping_cfg_cb[i].cir,
                pA2->que_shaping_cfg_cb[i].pir, pA2->que_shaping_cfg_cb[i].cbs,
                pA2->que_shaping_cfg_cb[i].pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_traffic_bandwidth_error_log(pcb->pw_id, 
                    pA2->que_shaping_cfg_cb[i].queue_id,
                    pA2->que_shaping_cfg_cb[i].cir,
                    pA2->que_shaping_cfg_cb[i].pir,
                    pA2->que_shaping_cfg_cb[i].cbs,
                    pA2->que_shaping_cfg_cb[i].pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        for (i = 0; i < pA2->que_schedul_num; i++)
        {
            offset = pA2->que_schedul_cfg_cb[i].queue_id % 4;
            if (pA2->que_schedul_cfg_cb[i].queue_id < 4)
            {
                /* mode : 0/1 = SP/WFQ */
                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    /* que4_1---32bit   weight---8bit */
                    que4_1 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            else
            {
                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    que8_5 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            qflag = ATG_DCI_RC_OK;
        }
        if (ATG_DCI_RC_OK == qflag)
        {
#if defined (SPU) || defined (SRC)

            /*PW中八个队列的调度策略*/
            ret = ApiAradHqosTrafficScheModeSet(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, que8_5);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vrf_key_pd(&(pcb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_set_trafficsche_error_log(port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, 
                    que8_5,__FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }

        //A_NODE_LABEL:
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*有一个子tlv配置错误则删除LSP*/
        {
            /* 在出口端删除创建的VC并删除和入口端VOQ建立的关联 */
            spm_hqos_del_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwDelete(0, pcb->pw_id, plsp->lsp_id);
#endif
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /*只对出口盘做引用计数，如果配置失败，其他单盘无法下发配置*/
        (pB1->cnt)++;
        (pB2->cnt)++;
        (pA1->cnt)++;
        (pA2->cnt)++;
        (plsp->pw_cnt)++;
        
        /*for (i = 0; i < pA2->que_congest_num; i++)
        {
            ret = spm_connect_hqos_wred_cnt(&(pA2->que_congst_cfg_cb[i]) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
        }*/
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_log_group(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_LOG_GROUP_CB *pcb,
    NBB_ULONG upflag,
    NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b = NULL;
    SPM_QOS_ACTION_CB *pA = NULL;
    NBB_BYTE fapid = 0;
    NBB_BYTE flag = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(index != 0);

    /*指针为空*/
    if (0 == index)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR policy inedx is 0.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (NULL == pcb)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR SPM_HQOS_LOG_GROUP_CB pcb is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pB = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(index NBB_CCXT);

    /* policy配置不存在 */
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if (0 != pB->basic_cfg_cb->mode) /* 非调度节点 */
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    c2b = AVLL_FIRST(pB->c2b_tree);
    if (NULL == c2b)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pA = spm_qos_find_action_cb(c2b->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION不存在*/
    if (NULL == pA)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*如果是本槽位则进行建树*/
    {
        if (ATG_DCI_RC_OK != upflag) /*首次创建*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspCreate(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != ret) /*失败*/
        {
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1, "s d s s s s d d d d", 
						   "HQOS crete lsp",ret,"slot","port","lsp id","upflag",
						   slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
            spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET, 
                        pcb->lsp_id,__FUNCTION__,__LINE__,ret NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (pA->shaping_cfg_cb) /*流量整形*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspBandwidthSet(0, pcb->lsp_id,
                      pA->shaping_cfg_cb->cir,
                      pA->shaping_cfg_cb->pir,
                      pA->shaping_cfg_cb->cbs,
                      pA->shaping_cfg_cb->pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1, "s d s s s s d d d d", 
						       "HQOS ApiAradHqosLspBandwidthSet",ret,
						       "slot","port","lsp id","upflag",
						       slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
                spm_hqos_arad_set_lsp_bandwidth_error_log(pcb->lsp_id,
                        pA->shaping_cfg_cb->cir,
                        pA->shaping_cfg_cb->pir,
                        pA->shaping_cfg_cb->cbs, 
                        pA->shaping_cfg_cb->pbs,
                        __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        if ((pA->schedule_cfg_cb) && (0 != pA->schedule_cfg_cb->mode)) /*调度策略*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspWfqWeightSet(0, pcb->lsp_id, pA->schedule_cfg_cb->weight);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						       "HQOS ApiAradHqosLspWfqWeightSet",ret,
						       "slot","port","lsp id","upflag",
						       slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
                spm_hqos_arad_set_lsp_weight_error_log(pcb->lsp_id, 
                    pA->schedule_cfg_cb->weight,__FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*有一个子tlv配置错误则删除LSP*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspDelete(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1, "s d s s s s d d d d", 
						       "HQOS ApiAradHqosLspDelete",ret,
						       "slot","port","lsp id","upflag",
						       slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
                spm_hqos_arad_del_lsp_error_log(port + PTN_690_PORT_OFFSET, 
                         pcb->lsp_id,__FUNCTION__,__LINE__,ret NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
        }

        /* 只对出口盘做引用计数,如果配置失败,其他单盘无法下发配置 */
        (pB->cnt)++;
        (pA->cnt)++;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_log_usr(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_LOG_USR_CB *pcb,
    SPM_HQOS_LOG_GROUP_CB *plsp,
    ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *data,
    NBB_ULONG upflag NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *c2b_1 = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b_2 = NULL;
    SPM_QOS_POLICY_CB *pB1 = NULL;
    SPM_QOS_POLICY_CB *pB2 = NULL;
    SPM_QOS_ACTION_CB *pA1 = NULL;
    SPM_QOS_ACTION_CB *pA2 = NULL;
    NBB_ULONG que4_1 = 0;
    NBB_ULONG que8_5 = 0;
    NBB_BYTE qflag = ATG_DCI_RC_UNSUCCESSFUL;
    NBB_BYTE fapid = 0;
    NBB_BYTE flag = ATG_DCI_RC_OK;
    NBB_BYTE i = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    NBB_BYTE offset = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(plsp != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(data != NULL);

    /*指针为空*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->user_qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->prio_queue_qos_policy_index NBB_CCXT);

    /*policy配置不存在*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*非调度节点*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    c2b_1 = AVLL_FIRST(pB1->c2b_tree);
    c2b_2 = AVLL_FIRST(pB2->c2b_tree);
    if ((NULL == c2b_1) || (NULL == c2b_2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pA1 = (SPM_QOS_ACTION_CB *)spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = (SPM_QOS_ACTION_CB *)spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /* ACTION不存在 */
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*入口盘出口盘都需配置*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }

    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*如果是本槽位则进行建树*/
    {
#if 1
        if (ATG_DCI_RC_OK == plsp->port) 
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspCreate(0, port + PTN_690_PORT_OFFSET,plsp->lsp_id);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET,
                    plsp->lsp_id,__FUNCTION__,
                    __LINE__,ret NBB_CCXT);
                spm_free_hqos_log_usr_cb(pcb NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*异常跳出*/
                goto EXIT_LABEL;
            }
        }
#endif
        if (ATG_DCI_RC_OK != upflag) /*不是更新*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwCreate(UNIT_0, port + PTN_690_PORT_OFFSET, plsp->lsp_id, pcb->pw_id,0);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						                          "HQOS ApiAradHqosPwCreate",ret,
						                          "slot","port","lsp id","pw_id",
						                          slot,port + PTN_690_PORT_OFFSET,plsp->lsp_id,pcb->pw_id));
                                spm_hqos_arad_create_pw_error_log(port + PTN_690_PORT_OFFSET,plsp->lsp_id,
                                pcb->pw_id,__FUNCTION__,__LINE__,ret NBB_CCXT);

                if (ATG_DCI_RC_OK == plsp->port)
                {
#if defined (SPU) || defined (SRC)
                    ApiHqosLspDelete(0, port + PTN_690_PORT_OFFSET, plsp->lsp_id);
#endif
                }
                spm_free_hqos_log_usr_cb(pcb NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }

            /*建立连接关系*/
            ret = spm_hqos_create_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
#if defined (SPU) || defined (SRC)
                ApiAradHqosPwDelete(UNIT_0,pcb->pw_id,plsp->lsp_id);
#endif
                if (ATG_DCI_RC_OK == plsp->port)
                {
#if defined (SPU) || defined (SRC)
                    ApiAradHqosLspDelete(0, port, plsp->lsp_id);
#endif
                }
                spm_free_hqos_log_usr_cb(pcb NBB_CCXT);

                /*信任驱动删除正确*/
                goto EXIT_LABEL;
            }
        }
        if (pA1->shaping_cfg_cb)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwBandwidthSet(0, pcb->pw_id, pA1->shaping_cfg_cb->cir,
                pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_hqos_arad_set_pw_bandwidth_error_log(pcb->pw_id, pA1->shaping_cfg_cb->cir,
                    pA1->shaping_cfg_cb->pir, pA1->shaping_cfg_cb->cbs, pA1->shaping_cfg_cb->pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        if ((pA1->schedule_cfg_cb) && (0 != pA1->schedule_cfg_cb->mode)) /*调度策略*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwWfqWeightSet(0, pcb->pw_id, pA1->schedule_cfg_cb->weight);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_hqos_arad_set_pw_weight_error_log(pcb->pw_id, pA1->schedule_cfg_cb->weight,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        for (i = 0; i < pA2->que_shape_num; i++)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosTrafficBandwidthSet(UNIT_0, pcb->pw_id,pA2->que_shaping_cfg_cb[i].queue_id,
                pA2->que_shaping_cfg_cb[i].cir,
                pA2->que_shaping_cfg_cb[i].pir, pA2->que_shaping_cfg_cb[i].cbs,
                pA2->que_shaping_cfg_cb[i].pbs);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_hqos_arad_set_traffic_bandwidth_error_log(pcb->pw_id, 
                    pA2->que_shaping_cfg_cb[i].queue_id,
                    pA2->que_shaping_cfg_cb[i].cir,
                    pA2->que_shaping_cfg_cb[i].pir,
                    pA2->que_shaping_cfg_cb[i].cbs,
                    pA2->que_shaping_cfg_cb[i].pbs,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }
        for (i = 0; i < pA2->que_schedul_num; i++)
        {
            offset = (pA2->que_schedul_cfg_cb[i].queue_id) % 4;
            if (pA2->que_schedul_cfg_cb[i].queue_id < 4)
            {
                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    que4_1 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            else
            {
                if (0 != pA2->que_schedul_cfg_cb[i].mode)
                {
                    que8_5 |= ((pA2->que_schedul_cfg_cb[i].weight) << (8 * offset));
                }
            }
            qflag = ATG_DCI_RC_OK;
        }
        if (ATG_DCI_RC_OK == qflag)
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosTrafficScheModeSet(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, que8_5);
#endif
            if (ATG_DCI_RC_OK != ret)
            {
                spm_hqos_arad_set_trafficsche_error_log(port + PTN_690_PORT_OFFSET, pcb->pw_id, que4_1, 
                    que8_5,__FUNCTION__,__LINE__,ret NBB_CCXT);
                flag = ATG_DCI_RC_UNSUCCESSFUL;
            }
        }

        //A_NODE_LABEL:
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*有一个子tlv配置错误则删除LSP*/
        {
        
            /* 删除已经创建的voq全连接 */
            spm_hqos_del_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
#if defined (SPU) || defined (SRC)
            ApiAradHqosPwDelete(0, pcb->pw_id, plsp->lsp_id);
#endif
            if (ATG_DCI_RC_OK == plsp->port)
            {
#if defined (SPU) || defined (SRC)
                ApiHqosLspDelete(0, port, plsp->lsp_id);
#endif
            }
            spm_free_hqos_log_usr_cb(pcb NBB_CCXT);
            ret = ATG_DCI_RC_ADD_FAILED;

            /*异常跳出*/
            goto EXIT_LABEL;

        }

        /*只对出口盘做引用计数，如果配置失败，其他单盘无法下发配置*/
        (pB1->cnt)++;
        (pB2->cnt)++;
        (pA1->cnt)++;
        (pA2->cnt)++;
        (plsp->pw_cnt)++;
        
        /*for (i = 0; i < pA2->que_congest_num; i++)
        {
            ret = spm_connect_hqos_wred_cnt(&(pA2->que_congst_cfg_cb[i]) NBB_CCXT);

            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
        }*/
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_lsp(NBB_USHORT slot,
    NBB_USHORT port,
    SPM_HQOS_LSP_TX_CB *pcb,
    NBB_ULONG node_index,
    NBB_ULONG nhi NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    if ((NULL == pcb) || (0 == slot))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /*调用删除B节点函数*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosLspDelete(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*调用驱动删除B节点错误*/
    {
        spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,rv NBB_CCXT);
        spm_hqos_arad_del_lsp_error_log(port + PTN_690_PORT_OFFSET,
            pcb->lsp_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
        ret = rv;
    }
    AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pcb->spm_hqos_lsp_tx_node);
    rv = spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*释放B节点内存错误*/
    {
        ret = rv;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_vc(NBB_USHORT slot, NBB_USHORT port, 
    SPM_HQOS_LSP_TX_CB *pB, SPM_HQOS_VC_CB *pcb,NBB_LONG node_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pB != NULL);

    /*指针为空*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /* 删除已经创建的voq全连接 */
    spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

    /*调用删除A节点函数*/
    if (fapid == slot)
    { 
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosPwDelete(UNIT_0, pcb->pw_id, pB->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*调用驱动删除B节点错误*/
    {
        printf("**QOS ERROR**%s,%d ret = %ld ApiAradHqosPwDelete\n",
                    __FUNCTION__,__LINE__,rv);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiAradHqosPwDelete",rv,
						"slot","port","lsp_id","pw id",
						slot,port + PTN_690_PORT_OFFSET,pB->lsp_id,pcb->pw_id));
        ret = rv;
    }
    rv = spm_free_hqos_vc_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*释放A节点内存错误*/
    {
        ret = rv;
    }
    if (fapid == slot)
    {
        if (0 == pB->pw_cnt)
        {
            printf("**QOS ERROR**%s,%d pw cnt==0\n",
                    __FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS pw cnt==0",ret,
						"slot","port","lsp_id","pw id",
						slot,port + PTN_690_PORT_OFFSET,pB->lsp_id,pcb->pw_id));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_vrf(NBB_USHORT slot, NBB_USHORT port,
    SPM_HQOS_LSP_TX_CB *pB, SPM_HQOS_VRF_CB *pcb,NBB_LONG node_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pB != NULL);

    /*指针为空*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /*此时已经从树中删除，或未插入树当中所以判断是否为0*/
    if(0 == spm_hqos_vrftree_counter(pB,&(pcb->vrf_key) NBB_CCXT))
    {
        /* 只在出口盘执行,删除出口盘和入口盘之间的连接关系 */
        spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

        /* 删除A节点:删除指定的PW ID */
        if (fapid == slot)
        {
#if defined (SPU) || defined (SRC)
            rv = ApiAradHqosPwDelete(0, pcb->pw_id, pB->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != rv) /*调用驱动删除B节点错误*/
        {
            printf("**QOS ERROR**ApiHqosPwDelete ret=%ld %s,%d\n",rv,__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    						"HQOS del pw",rv,
    						"slot","port","lsp_id","pw id",
    						slot,port + PTN_690_PORT_OFFSET,pB->lsp_id,pcb->pw_id));
            ret = rv;
        }
    }

    rv = spm_free_hqos_vrf_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*释放A节点内存错误*/
    {
        ret = rv;
    }
    if (fapid == slot)
    {
        if (0 == pB->pw_cnt)
        {
            printf("**QOS ERROR**pw cnt==0%s,%d\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS del pw",ATG_DCI_RC_UNSUCCESSFUL,
						"slot","port","lsp_id","pw id",
						slot,port,pB->lsp_id,pcb->pw_id));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_clear_vrf_mem(SPM_QOS_VRF_INSTANSE_KEY *key,
    ATG_DCI_VRF_INSTANCE_UP_VPN_QOS *data NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_LONG flag = ATG_DCI_RC_OK; 
    SPM_HQOS_LSP_TX_CB *lsp_cb = NULL;
    SPM_HQOS_LSP_TX_CB *next_lsp_cb = NULL;
    SPM_HQOS_VRF_CB *vrf_cb = NULL;
    SPM_HQOS_VRF_CB *next_vrf_cb = NULL;
    NBB_USHORT i = 0;
    NBB_USHORT j = 0;
    NBB_BYTE fapid = 0;
    NBB_LONG voq = 0;
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG baseVCId = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == key) || (NULL == data))
    {
        printf("**QOS ERROR**%s,%d,指针为空\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"QOS 指针为空",ATG_DCI_RC_UNSUCCESSFUL,
					"","","","",
					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_vrf_cfg_print)
    {
        OS_PRINTF("%s %s,%d vrf_id=%d,peer_ip=0x%lx,label=%ld\n\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            key->vrf_id,key->peer_ip,key->label);
        OS_SPRINTF(ucMessage,"###############%s %s,%d vrf_id=%d,peer_ip=0x%lx,label=%ld"
            "###############\n\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            key->vrf_id,key->peer_ip,key->label);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);   
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
        
    /* 判断slot是否开启hqos的开关 */
	ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
	if (ATG_DCI_RC_OK != ret)
	{
	    ret = ATG_DCI_RC_OK;

        /*异常跳出*/
	    goto EXIT_LABEL;
	}

    for(j = 0; j < MAX_SLOT_NUM;j++)
    {
        
        /* 判断slot是否开启hqos的开关 */
        ret = spm_hqos_checkslot(j + 1, &slot_index NBB_CCXT);
    	if (ATG_DCI_RC_OK != ret)
    	{
    	    ret = ATG_DCI_RC_OK;
    	    continue;
    	}

        for (i = 0; i < MAX_PHYSIC_PORT_NUM; i++)
        {
            lsp_cb = (SPM_HQOS_LSP_TX_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[j][i].lsp_tree);
            while (NULL != lsp_cb)
            {
                next_lsp_cb = (SPM_HQOS_LSP_TX_CB *)AVLL_NEXT(v_spm_shared->qos_port_cb[j][i].lsp_tree,
                                    lsp_cb->spm_hqos_lsp_tx_node);
                
                vrf_cb = (SPM_HQOS_VRF_CB*) AVLL_FIRST(lsp_cb->vrf_tree);
                while (vrf_cb != NULL)
                {
                    next_vrf_cb = (SPM_HQOS_VRF_CB*) 
                        AVLL_NEXT(lsp_cb->vrf_tree,vrf_cb->spm_hqos_vrf_node);
                    if((vrf_cb->vrf_key.vrf_id == key->vrf_id) &&
                        (vrf_cb->vrf_key.peer_ip == key->peer_ip))
                    {
                        flag = ATG_DCI_RC_UNSUCCESSFUL;
                        
                        if(1 == spm_hqos_vrftree_counter(lsp_cb,&(vrf_cb->vrf_key) NBB_CCXT))
                        {
                            if((j + 1) == v_spm_shared->local_slot_id)
                            {
                                spm_disconnect_policy_cnt(vrf_cb->vrf_data.qos_policy_index,
                                ATG_DCI_RC_OK NBB_CCXT);
                                spm_disconnect_policy_cnt(vrf_cb->vrf_data.flow_queue_qos_policy_index,
                                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
                            }

                            /* 删除已经创建的voq全连接 */
                            spm_hqos_del_voq(j + 1,i,vrf_cb->vrf_data.node_index, vrf_cb->pw_id NBB_CCXT);

                            voq = slot_index* NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
                            baseVCId = fapid_index * NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;

#if defined (SPU) || defined (SRC)

                            /* 删除入口盘和出口盘之间的voq全连接 */
                            rv = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                            if (ATG_DCI_RC_OK != rv)
                            {
                                spm_qos_vrf_key_pd(key,HQOS_VRF_ERROR,rv NBB_CCXT);
                                spm_hqos_arad_voq_del_error_log(voq,j + 1,i + PTN_690_PORT_OFFSET,
                                    baseVCId,__FUNCTION__,__LINE__,rv NBB_CCXT);
                            }
    						
                            if((j + 1) == v_spm_shared->local_slot_id)
                            {
#if defined (SPU) || defined (SRC)
                                rv = ApiAradHqosPwDelete(UNIT_0,vrf_cb->pw_id,lsp_cb->lsp_id);
#endif
                                if(ATG_DCI_RC_OK != rv)
                                {
                                    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                						"HQOS vrf pw delete",rv,
                						"node_index","flow_id","lsp_id","pw_id",
                						vrf_cb->vrf_data.node_index,vrf_cb->flow_id,
                						lsp_cb->lsp_id,vrf_cb->pw_id));
                                }
                            }
                        }
                        
                        AVLL_DELETE(lsp_cb->vrf_tree, vrf_cb->spm_hqos_vrf_node);
                        spm_free_hqos_vrf_cb(vrf_cb NBB_CCXT);
                    }
                    vrf_cb = next_vrf_cb;          
                }
                if (NULL == AVLL_FIRST(lsp_cb->vc_tree) && NULL == AVLL_FIRST(lsp_cb->vrf_tree)
                    && (0 == lsp_cb->node_index))
                {
                   AVLL_DELETE((SHARED.qos_port_cb[j][i]).lsp_tree, lsp_cb->spm_hqos_lsp_tx_node);
                   spm_free_hqos_lsp_tx_cb(lsp_cb NBB_CCXT);
                }
                lsp_cb = next_lsp_cb;
            }
        } 
    }

    if(ATG_DCI_RC_OK == flag)/*如果没有找到配置则错误的配置*/
    {
        ret = ATG_DCI_RC_OK;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_log_group(NBB_USHORT slot, NBB_USHORT port, 
    SPM_HQOS_LOG_GROUP_CB *pcb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    if ((0 >= slot) || (MAX_SLOT_NUM < slot))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    
    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /*调用删除B节点函数*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosLspDelete(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*调用驱动删除B节点错误*/
    {
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiAradHqosLspDelete",rv,
						"slot","port","lsp_id","",
						slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,0));
        spm_hqos_arad_del_lsp_error_log(port + PTN_690_PORT_OFFSET, pcb->lsp_id,
                    __FUNCTION__,__LINE__,rv NBB_CCXT);
        ret = rv;
    }
    AVLL_DELETE(SHARED.qos_port_cb[slot - 1][port].group_tree, pcb->spm_hqos_group_node);
    rv = spm_free_hqos_log_group_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*释放B节点内存错误*/
    {
        ret = rv;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_log_usr(NBB_USHORT slot, NBB_USHORT port,
    SPM_HQOS_LOG_GROUP_CB *pB, SPM_HQOS_LOG_USR_CB *pcb,NBB_LONG node_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pcb != NULL);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pB != NULL);

    /*指针为空*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /* 删除已经创建的voq全连接 */
    spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

    /*调用删除A节点函数*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosPwDelete(0, pcb->pw_id, pB->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*调用驱动删除B节点错误*/
    {
        printf("**QOS ERROR**%s,%d ret = %ld ApiAradHqosPwDelete\n",
                    __FUNCTION__,__LINE__,rv);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiAradHqosPwDelete",rv,
						"slot","port","lsp_id","pw id",
						slot,port + PTN_690_PORT_OFFSET,pB->lsp_id,pcb->pw_id));
    }
    rv = spm_free_hqos_log_usr_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*释放A节点内存错误*/
    {
        ret = rv;
    }
    if (fapid == slot)
    {
        if (0 == pB->pw_cnt)
        {
            printf("**QOS ERROR**pw cnt==0%s,%d\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS del pw",ATG_DCI_RC_UNSUCCESSFUL,
						"slot","port","lsp_id","pw id",
						slot,port,pB->lsp_id,pcb->pw_id));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


#if 1

/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_vrf_nhi(SUB_PORT *sub_port,
    NBB_ULONG posid,
    NBB_ULONG logic_key,
    NBB_ULONG group_key NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LOG_USR_CB *pcb = NULL;
    SPM_HQOS_LOG_GROUP_CB *pB = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;
    NBB_LONG unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 输入参数指针必须有效 */
    NBB_ASSERT(logic_key != 0);

    if ((0 == group_key) || (0 == logic_key) || (NULL == sub_port))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(sub_port->slot, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(fapid, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*查找是否存在B节点*/
    pB = AVLL_FIND(SHARED.qos_port_cb[sub_port->slot - 1][sub_port->port].group_tree, &(sub_port->port));

    /* 如果条目不存在则分配默认B节点和LSP ID*/
    if (pB == NULL)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;

    }
    pcb = AVLL_FIND(pB->usr_tree, &logic_key);

    /* 如果条目不存在则分配A节点内存和pw id*/
    if (pcb == NULL)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    for(unit = 0; unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)
        ret += ApiC3SetL3UniHqos(unit, posid,NULL, 1, pcb->flow_id);
        if (ATG_DCI_RC_OK != ret)
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif  
    } 

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_del_vrf_nhi(NBB_ULONG nhi NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    for(unit = 0; unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)
        ret += ApiC3SetL3UniHqos(unit, nhi,NULL, 0,0);
#endif
    }
    if (ATG_DCI_RC_OK != ret)
    {
        goto EXIT_LABEL;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_set_l3uni(NBB_ULONG logic_key,NBB_ULONG posid,
            ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *data NBB_CCXT_T NBB_CXT)
{
    NBB_INT ret = ATG_DCI_RC_OK;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    NBB_LONG unit = 0;
    NBB_LONG slot_index = 0;
    NBB_LONG voq = 0;
    SPM_PORT_INFO_CB stPortInfo = {0};
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == data) || (0 == posid) || (0 == logic_key))
    {
        printf("%s %s,LINE=%d : data==null,logic_key==0 or posid==0.\n",HQOS_USR_ERROR,__FUNCTION__,__LINE__);
        OS_SPRINTF(ucMessage,"%s %s,LINE=%d : data==null,logic_key==0 or posid==0.\n", 
                   HQOS_USR_ERROR,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    ret = spm_get_portid_from_logical_port_index(logic_key, &stPortInfo NBB_CCXT);
    if(0 != ret)
    {
        printf("%s %s,LINE=%d spm_get_portid_from_logical_port_index logic_key=%ld,ret=%d\n",
            HQOS_USR_ERROR,__FUNCTION__,__LINE__,logic_key,ret);
        OS_SPRINTF(ucMessage,"%s %s,LINE=%d spm_get_portid_from_logical_port_index logic_key=%ld,ret=%d\n",
            HQOS_USR_ERROR,__FUNCTION__,__LINE__,logic_key,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(stPortInfo.slot_id, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,LINE=%d logic_key=%ld,node_index=%ld,\n"
            "user_qos_policy_index=%ld,prio_queue_qos_policy_index=%ld\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            logic_key,data->node_index,data->user_qos_policy_index,
            data->prio_queue_qos_policy_index);
        OS_SPRINTF(ucMessage,"%s %s,LINE=%d logic_key=%ld,node_index=%ld,\n"
            "user_qos_policy_index=%ld,prio_queue_qos_policy_index=%ld\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            logic_key,data->node_index,data->user_qos_policy_index,
            data->prio_queue_qos_policy_index);
       BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }

    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    for(unit = 0; unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        /*创建下话l3uni的hqos:将hqos使能开关和flowid传递给c3驱动*/
        ret = ApiC3SetL3UniHqos(unit,posid,NULL, 1, voq);
        if (ATG_DCI_RC_OK != ret)
        {
            printf("%s %s,LINE=%d posid=%ld,voq=%ld,ret=%d\n"
                "ApiC3SetL3UniHqos\n",
                HQOS_USR_ERROR,__FUNCTION__,__LINE__,posid,voq,ret);
            OS_SPRINTF(ucMessage,"%s %s,LINE=%d posid=%ld,voq=%ld,ret=%d\n"
                "ApiC3SetL3UniHqos\n",
                HQOS_USR_ERROR,__FUNCTION__,__LINE__,posid,voq,ret);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif 
    }
    
    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}




/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_group_node(NBB_USHORT slot, NBB_LONG port, 
    NBB_ULONG index, NBB_ULONG log_key NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LOG_GROUP_CB *pcb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;
    SPM_QOS_USER_GROUP_CB *p_group = NULL;
    SPM_HQOS_LOG_GROUP_CB *group_cb = NULL;
    NBB_ULONG policy_index = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if(0 == slot)
    {
        printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(slot, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(fapid, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK; 

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,LINE=%d slot=%d,port=%ld,usr gourp index=%ld,log_key=%ld \n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            slot,port,index,log_key);
        OS_SPRINTF(ucMessage,"%s %s,%d slot=%d,port=%ld,usr gourp index=%ld,log_key=%ld \n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            slot,port,index,log_key);
       BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
    
    if(port >= 0x80)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    
    if (0 == index)
    {
        policy_index = index;
        for (group_cb = (SPM_HQOS_LOG_GROUP_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[slot - 1][port].group_tree);
            group_cb != NULL;
            group_cb = (SPM_HQOS_LOG_GROUP_CB *)AVLL_NEXT(v_spm_shared->qos_port_cb[slot - 1][port].group_tree, 
                        group_cb->spm_hqos_group_node))
        {
            if((group_cb->slot == slot) && (group_cb->port == port))
            {
                pcb = group_cb;
                break;
            }
        }
    }
    else
    {
        /* 检查用户组节点的配置是否为空 */
        p_group = spm_check_group_down_cb(index,slot, port, &policy_index NBB_CCXT);
        if ((NULL == p_group) || (0 == policy_index))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* group_tree:出端口出槽位上所绑定的下话用户组QoS配置的树; */
        /*            其中,该树的key值为用户组节点的索引值index.   */
        pcb = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].group_tree, &index);
    }

    /******************************创建B(LSP)节点************************************/
    if ((pcb == NULL) && (0 != index))
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_ADD NBB_CCXT);
        pcb = spm_alloc_hqos_log_group_cb(index NBB_CCXT);
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_log_group(slot, port, pcb, ATG_DCI_RC_UNSUCCESSFUL, policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            spm_free_hqos_log_group_cb(pcb NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (NULL == p_group)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            spm_free_hqos_log_group_cb(pcb NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (slot == fapid)
        {      
            (p_group->cnt)++;    
        }
        pcb->policy_index = policy_index;
        pcb->group_index = index;
        pcb->slot = slot;
        pcb->port = port;
        p_group->port = port;
        p_group->slot = slot;
        p_group->lsp_id = pcb->lsp_id;
        
        rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].group_tree, pcb->spm_hqos_group_node);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 != index) && (pcb->group_index != index)) /*强制更新*/
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_UPD NBB_CCXT);
#if 0
        ret = spm_hqos_add_log_group(slot, port, pcb, ATG_DCI_RC_OK, index, nhi);

        if (ATG_DCI_RC_OK != ret) /*更新失败*/
        {
            NBB_TRC_FLOW((NBB_FORMAT "**ERROR** update lsp policy failed"));
            goto EXIT_LABEL;
        }

        if (slot == fapid)
        {
            (p_group->cnt)++;
        }
        pcb->policy_index = policy_index;
        pcb->group_index = index;
        pcb->slot = slot;
        p_group->port = port;

        p_group = spm_find_group_cb(pcb->group_index);
        p_group->port = 0;

        if (slot == fapid) /*引用计数只在出口盘*/
        {
            ret = spm_disconnect_policy_cnt(pcb->policy_index);

            if (ATG_DCI_RC_OK != ret) /*引用计数减1失败*/
            {
                goto EXIT_LABEL;
            }

            ret = spm_disconnect_usr_group_cnt(pcb->group_index);

            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
        }
#endif
        printf("**QOS ERROR**%s,LINE=%d different group old=%ld,new=%ld\n",
                __FUNCTION__,__LINE__,pcb->group_index,index);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS group hqos config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
					"old group","new group","slot","port",
					pcb->group_index,index,slot,port));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 != index) && (pcb->group_index == index)) /*一样的配置*/
    {
        printf("**QOS ERROR**%s,LINE=%d the same group cfg old=%ld,new=%ld\n",
                __FUNCTION__,__LINE__,pcb->group_index,index);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS the same group cfg",ret,
					"old group","new group","slot","port",
					pcb->group_index,index,slot,port));

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 == index)) /*删除B节点配置*/
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_DEL NBB_CCXT);
        
        /*如果A节点还存在引用则不允许删除B节点*/
        if (NULL != AVLL_FIRST(pcb->usr_tree))
        {
            printf("**QOS ERROR**%s,LINE=%d usr tree is in used can't del group\n",
                __FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS usr tree is in used can't del group",ret,
    					"old group","new group","slot","port",
    					pcb->group_index,index,slot,port));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*引用计数只在出口盘*/
        {
            spm_disconnect_policy_cnt(pcb->policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            spm_disconnect_usr_group_cnt(pcb->group_index NBB_CCXT);
        }
        p_group = spm_find_group_cb(pcb->group_index NBB_CCXT);
        if((NULL != p_group) && (0 == p_group->cnt))
        {
            p_group->port = 0;
            p_group->slot = 0;
        }

        /*调用删除B节点函数*/
        ret = spm_hqos_del_log_group(slot, port, pcb NBB_CCXT); 

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((pcb == NULL) && (0 == index))
    {
        printf("**QOS ERROR**%s,LINE=%d group has been deleted\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS group has been deleted",ret,
    					"pcb==null","new group","slot","port",
    					0,index,slot,port));

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else
    {
        printf("**QOS ERROR**%s,LINE=%d policy index config missmatch\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS policy index config missmatch",ret,
    					"","new group","slot","port",
    					0,index,slot,port));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_lsp_node(NBB_USHORT slot,
    NBB_LONG port,
    SPM_QOS_TUNNEL_KEY *pkey,
    ATG_DCI_LSP_TX_UP_TNNL_QOS *pstUpTnnlQos,
    NBB_ULONG nhi NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LSP_TX_CB *pcb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_BYTE fapid = 0;
    NBB_LONG nflag = ATG_DCI_RC_OK;
    NBB_ULONG node_index = 0;
    NBB_BYTE mode = 0;
    NBB_ULONG index = 0;
    SPM_TE_SHAPING shap = {0};
    SPM_TE_SHAPING *shaping = NULL;
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_ULONG voq = 0;
    NBB_ULONG baseVCId = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == pkey) || (NULL == pstUpTnnlQos) || (0 == slot))
    {
        printf("**QOS ERROR**%s,LINE=%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_txlsp_cfg_print)
    {
        printf("%s %s,LINE=%d slot=%d,port=%ld,ingress=0x%lx,egress=0x%lx,\n"
            "lspid=%ld tunnelid=%ld,vrf id=%d,fec=%ld,mask=%d,type=%s,flag=%s\n"
            "mode=%s,node_index=%ld,qos_policy_index=%ld,cir=%ld,pir=%ld\n\n",
            HQOS_LSP_CFG,__FUNCTION__,__LINE__,
            slot,port,pkey->tx_lsp.ingress,pkey->tx_lsp.egress,
            pkey->tx_lsp.lspid,pkey->tx_lsp.tunnelid,
            pkey->ftn.vrfid,pkey->ftn.fec,pkey->ftn.mask,
            (0 == pkey->type)?"FTN":"CRLSP",(1 == pkey->flag)?"主用":"备用",
            (pstUpTnnlQos->mode)?"TE":"模板",
            pstUpTnnlQos->node_index,pstUpTnnlQos->qos_policy_index,
            pstUpTnnlQos->cir,pstUpTnnlQos->pir);
    }
    OS_SPRINTF(ucMessage,"%s %s,%d slot=%d,port=%ld,ingress=0x%lx,egress=0x%lx,\n"
            "lspid=%ld tunnelid=%ld,vrf id=%d,fec=%ld,mask=%d,type=%s,flag=%s\n"
            "mode=%s,node_index=%ld,qos_policy_index=%ld,cir=%ld,pir=%ld\n\n",
            HQOS_LSP_CFG,__FUNCTION__,__LINE__,
            slot,port,pkey->tx_lsp.ingress,pkey->tx_lsp.egress,
            pkey->tx_lsp.lspid,pkey->tx_lsp.tunnelid,
            pkey->ftn.vrfid,pkey->ftn.fec,pkey->ftn.mask,
            (0 == pkey->type)?"FTN":"CRLSP",(1 == pkey->flag)?"主用":"备用",
            (pstUpTnnlQos->mode)?"TE":"模板",
            pstUpTnnlQos->node_index,pstUpTnnlQos->qos_policy_index,
            pstUpTnnlQos->cir,pstUpTnnlQos->pir);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    node_index = pstUpTnnlQos->node_index;
    mode = pstUpTnnlQos->mode;
    shap.cir = pstUpTnnlQos->cir;
    shap.cbs = pstUpTnnlQos->cbs;
    shap.pir = pstUpTnnlQos->pir;
    shap.pbs = pstUpTnnlQos->pbs;
    index = pstUpTnnlQos->qos_policy_index;
    shaping = &shap;

    /* 查找是否存在,关键字pkey为该lsp的Type、Flag及Tx_Lsp的key值(ingress、egress、tunnelid及lspid) */
    pcb = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pkey);
    if(NULL == pcb)
    {
        nflag = ATG_DCI_RC_UNSUCCESSFUL;
    }
    if ((NULL != pcb) && (0 != pcb->node_index) && (pcb->node_index != node_index))
    {
        spm_check_hqos_lsp_node_cfg_log(pkey,pcb->node_index,node_index,
            __FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 模式的切换不支持有配置的情况下切换 */
    if ((NULL != pcb) && (0 != pcb->node_index) && (pcb->mode != mode)) 
    {
        spm_check_hqos_lsp_mode_cfg_log(pkey,pcb->mode,mode,
            __FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 如果lsp和vpn都需要配置hqos时,必须先配置lsp的hqos再配置vpn的hqos;若先配置了vpn的   */
    /* hqos再配置lsp的hqos时,需要先去绑定vpn的hqos,然后配置lsp的hqos最后再配置vpn的hqos. */
    if((NULL != pcb) && (pcb->lsp_id <= MAX_PHYSIC_PORT_NUM))
    {
        spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_ADD NBB_CCXT);
        spm_check_hqos_lsp_lspid_err_log(pcb->lsp_id,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    voq = slot_index* NUM_COS + offset * node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * node_index + HQOS_OFFSET;

    /* TE模式 */
    if ((0 != mode) && (NULL != shaping))
    {
        /* 增加 */
        if ((pcb == NULL) && (0 != shaping->pir))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_ADD NBB_CCXT);
            pcb = spm_alloc_hqos_lsp_tx_cb(pkey,ATG_DCI_RC_OK NBB_CCXT);
            if (NULL == pcb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            if (slot == fapid)
            {
#if defined (SPU) || defined (SRC)
                ret = ApiAradHqosLspCreate(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
                if (ATG_DCI_RC_OK != ret)
                {
                    spm_qos_txlsp_key_pd(pkey,HQOS_LSP_ERROR,ret NBB_CCXT);
                    spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET,
                        pcb->lsp_id,__FUNCTION__,__LINE__ ,ret NBB_CCXT);
                    spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);

                    /*异常跳出*/
                    goto EXIT_LABEL;
                }
#if defined (SPU) || defined (SRC)
                ret = ApiAradHqosLspBandwidthSet(UNIT_0, pcb->lsp_id, shaping->cir,
                      shaping->pir - shaping->cir, shaping->cbs, shaping->pbs);
#endif
                if (ATG_DCI_RC_OK != ret)
                {
                    spm_qos_txlsp_key_pd(pkey,HQOS_LSP_ERROR,ret NBB_CCXT);
                    spm_hqos_arad_set_lsp_bandwidth_error_log(pcb->lsp_id, shaping->cir,
                        shaping->pir, shaping->cbs, shaping->pbs,
                        __FUNCTION__,__LINE__,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)
                    ApiAradHqosLspDelete(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
                    spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);

                    /*异常跳出*/
                    goto EXIT_LABEL;
                }
            }
            pcb->mode = mode;
            pcb->node_index = node_index;
            OS_MEMCPY(&(pcb->shaping), shaping, sizeof(SPM_TE_SHAPING));

            rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pcb->spm_hqos_lsp_tx_node); 

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* 更新 */
        else if((NULL != pcb) && (0 != shaping->pir))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_UPD NBB_CCXT);
            if (slot == fapid)
            {
#if defined (SPU) || defined (SRC)
                ret = ApiAradHqosLspBandwidthSet(UNIT_0, pcb->lsp_id, shaping->cir,
                    shaping->pir - shaping->cir, shaping->cbs, shaping->pbs);
#endif
                if (ATG_DCI_RC_OK != ret)
                {
                    spm_qos_txlsp_key_pd(pkey,HQOS_LSP_ERROR,ret NBB_CCXT);
                    spm_hqos_arad_set_lsp_bandwidth_error_log(pcb->lsp_id,
                        shaping->cir, shaping->pir - shaping->cir, 
                        shaping->cbs, shaping->pbs,
                        __FUNCTION__,__LINE__,ret NBB_CCXT);

                    /*异常跳出*/
                    goto EXIT_LABEL;
                }
            }
            pcb->mode = mode;
            pcb->node_index = node_index;
            OS_MEMCPY(&(pcb->shaping), shaping, sizeof(SPM_TE_SHAPING));

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* 删除 */
        else if ((NULL != pcb) && (0 == shaping->pir))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_DEL NBB_CCXT);
            if (NULL != AVLL_FIRST(pcb->vc_tree) || NULL != AVLL_FIRST(pcb->vrf_tree))
            {
                spm_check_hqos_lsp_del_cfg_log(pkey,__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*异常跳出*/
                goto EXIT_LABEL;
            }

            /* 调用删除B节点函数 */
            ret = spm_hqos_del_lsp(slot, port, pcb, node_index, nhi NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* 错误的配置 */
        else
        {
            spm_check_hqos_lsp_cfg_err_log(pkey,nflag,__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }
    else if (0 == mode)
    {
        if ((pcb == NULL) && (0 != index))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_ADD NBB_CCXT);
            pcb = spm_alloc_hqos_lsp_tx_cb(pkey,ATG_DCI_RC_OK NBB_CCXT);
            if (NULL == pcb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            ret = spm_hqos_add_lsp(slot, port, pcb, ATG_DCI_RC_UNSUCCESSFUL,
                  node_index, index, nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            pcb->policy_index = index;
            pcb->mode = mode;
            pcb->node_index = node_index;
            
            rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pcb->spm_hqos_lsp_tx_node);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 != index) && (pcb->policy_index != index)) /*强制更新*/
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_UPD NBB_CCXT);
            ret = spm_hqos_add_lsp(slot,port,pcb,ATG_DCI_RC_OK,node_index,index,nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {

                /*异常跳出*/
                goto EXIT_LABEL;
            }

            /* 引用计数只在出口盘 */
            if (slot == fapid)
            {
                spm_disconnect_policy_cnt(pcb->policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            }
            pcb->policy_index = index;
            pcb->node_index = node_index;

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* 在txlsp上新增hqos配置 */
        else if ((NULL != pcb) && (0 == pcb->policy_index) && (0 != index))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_UPD NBB_CCXT);
            ret = spm_hqos_add_lsp(slot,port,pcb,ATG_DCI_RC_OK,node_index,index,nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            pcb->policy_index = index;
            pcb->node_index = node_index;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 != index) && (pcb->policy_index == index)) /*一样的配置*/
        {
            spm_check_hqos_lsp_cfg_same_log(pkey,index,__FUNCTION__,__LINE__ NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }

        /* 删除B节点配置 */
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 == index))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_DEL NBB_CCXT);
            
            /* 如果A节点还存在引用则不允许删除B节点 */
            if (NULL != AVLL_FIRST(pcb->vc_tree) || NULL != AVLL_FIRST(pcb->vrf_tree))
            {
                spm_check_hqos_lsp_del_cfg_log(pkey,__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*异常跳出*/
                goto EXIT_LABEL;
            }

            /* 引用计数只在出口盘 */
            if (slot == fapid)
            {
               spm_disconnect_policy_cnt(pcb->policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            }

            /* 调用删除B节点函数 */
            ret = spm_hqos_del_lsp(slot, port, pcb, node_index, nhi NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        else
        {
            spm_check_hqos_lsp_cfg_err_log(pkey,nflag,__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_vc_node(NBB_USHORT slot, NBB_LONG port, NBB_ULONG posid, NBB_BYTE proflag,
    SPM_QOS_TUNNEL_KEY *skey, ATG_DCI_VC_KEY *pkey,
    ATG_DCI_VC_UP_VPN_QOS_POLICY *data NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_VC_CB *pcb = NULL;
    SPM_HQOS_LSP_TX_CB *pB = NULL;
    CRTXLSP_KEY lsptx_key = {0};
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_ULONG old_index = 0;
    NBB_ULONG index = 0;
    NBB_ULONG flag = 0;
    NBB_BYTE fapid = 0;
    NBB_LONG voq = 0;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG unit = 0;
#endif
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG baseVCId = 0;
    
    //NBB_ULONG port_index = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == pkey) || (NULL == skey) || (NULL == data) ||
        (0 == posid) || (0 == slot))
    {
       /* printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__); */
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_vc_cfg_print)
    {
        OS_PRINTF("%s %s,%d slot=%d,port=%ld,vc_id=%ld,peer_ip=0x%lx \n"
                  "vc_type=%d,posid=%ld,flag=%d ingress=0x%lx,egress=0x%lx,\n"
                  "lspid=%ld,tunnelid=%ld,vrfid=%d,fec=%ld,mask=%d,lsp type=%s,flag=%s\n"
                  "node_index=%ld,policy index=%ld,que policy index=%ld\n\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            slot,port,pkey->vc_id,pkey->peer_ip,pkey->vc_type,posid,proflag,
            skey->tx_lsp.ingress,skey->tx_lsp.egress,skey->tx_lsp.lspid,
            skey->tx_lsp.tunnelid,skey->ftn.vrfid,skey->ftn.fec,skey->ftn.mask,
            (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"主用":"备用",
            data->node_index,data->qos_policy_index,
            data->flow_queue_qos_policy_index);
        OS_SPRINTF(ucMessage,"%s %s,%d slot=%d,port=%ld,vc_id=%ld,peer_ip=0x%lx \n"
                  "vc_type=%d,posid=%ld,flag=%d ingress=0x%lx,egress=0x%lx,\n"
                  "lspid=%ld,tunnelid=%ld,vrfid=%d,fec=%ld,mask=%d,lsp type=%s,flag=%s\n"
                  "node_index=%ld,policy index=%ld,que policy index=%ld\n\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            slot,port,pkey->vc_id,pkey->peer_ip,pkey->vc_type,posid,proflag,
            skey->tx_lsp.ingress,skey->tx_lsp.egress,skey->tx_lsp.lspid,
            skey->tx_lsp.tunnelid,skey->ftn.vrfid,skey->ftn.fec,skey->ftn.mask,
            (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"主用":"备用",
            data->node_index,data->qos_policy_index,
            data->flow_queue_qos_policy_index);
       BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);
        
    /* 判断slot是否开启hqos的开关 */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(port > 0x80)
    {
    OS_SPRINTF(ucMessage,"%s,%d slot=%d,port=%ld,vc_id=%ld,peer_ip=0x%lx : "
               "LAG NONSUPPORT HQOS, JUST PRINT THE CONFIGURATION.\n\n",
               __FUNCTION__,__LINE__,slot,port,pkey->vc_id,pkey->peer_ip);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if((0 != skey->type) && (0 == skey->tx_lsp.lspid)) /*LSP类型为CRLSP_TX*/
    {
        OS_MEMCPY(&lsptx_key,&(skey->tx_lsp),sizeof(CRTXLSP_KEY));
        ret = spm_l3_gettxlspid(&lsptx_key, &(skey->tx_lsp.lspid) NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            OS_SPRINTF(ucMessage,"%s,%d spm_l3_rsvpforoam err=%ld\n\n",
                       __FUNCTION__,__LINE__,ret);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        			HQOS_VC_ERROR,ret,
        			ucMessage,__FUNCTION__,"LINE","spm_l3_rsvpforoam",
        			0,0,__LINE__,0));

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }
    
    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * data->node_index + HQOS_OFFSET;

    /*查找是否存在B节点*/
    pB = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].lsp_tree, skey);

    /* 如果条目不存在则分配默认B节点和LSP ID*/
    if (pB == NULL)
    {
        if((0 == data->node_index) ||
           (0 == data->qos_policy_index) ||
           (0 == data->flow_queue_qos_policy_index))
        {
            printf("**QOS ERROR**%s,LINE=%d,param=NULL\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
    						"","","","",
    						0,0,0,0));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_ADD NBB_CCXT);
        pcb = spm_alloc_hqos_vc_cb(pkey NBB_CCXT);

        /*申请内存失败*/
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB = spm_alloc_hqos_lsp_tx_cb(skey,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);

        /*申请内存失败*/
        if (NULL == pB)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB->lsp_id = port + 1;
#if defined (SPU) || defined (SRC)

        /* 建立入口盘和出口盘之间的全连接 */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                baseVCId, __FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vc_cb(pcb NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*调用驱动返回错误*/
        {
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {
            if(QOS_MAIN == proflag)
            {
                ret += ApiC3SetVpHqos(unit, posid, 0, 1, voq);
            }
            else
            {
                ret += ApiC3SetVpHqos(unit, posid, 1, 1, voq);
            }
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_vp_error_log(voq,1,posid,proflag,
                __FUNCTION__,__LINE__,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vc(slot, port, pB, pcb,data->node_index NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif
#ifdef PTN690
        spm_hqos_add_vc_pmline(pkey,voq NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_addline_hqos(data->node_index,slot,voq);
        if (ATG_DCI_RC_OK != rv)
        {
            spm_qos_vc_key_pd(pkey,HQOS_BMU_ERROR,rv NBB_CCXT);
            spm_hqos_almpm_addline_error_log(voq,data->node_index,slot,
                __FUNCTION__,__LINE__,rv NBB_CCXT);
        }
#endif
#endif
        OS_MEMCPY(&(pcb->vc_data), data, sizeof(ATG_DCI_VC_UP_VPN_QOS_POLICY));
        pcb->flow_id = voq;
        
        rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pB->spm_hqos_lsp_tx_node);
        rv = AVLL_INSERT(pB->vc_tree, pcb->spm_hqos_vc_node);

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /************************先建LSP节点*****************************/   
    pcb = AVLL_FIND(pB->vc_tree, pkey);

    /* 如果条目不存在则分配A节点内存和pw id*/
    if (pcb == NULL)
    {
        pcb = spm_alloc_hqos_vc_cb(pkey NBB_CCXT);

        /*申请内存失败*/
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }
    if ((0 != pcb->vc_data.node_index) && (pcb->vc_data.node_index != data->node_index))
    {
        spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
        spm_check_hqos_vc_node_index_cfg_log(pkey,pcb->vc_data.node_index,
            data->node_index,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    old_index = (pcb->vc_data.qos_policy_index) | (pcb->vc_data.flow_queue_qos_policy_index);
    index = (data->qos_policy_index) | (data->flow_queue_qos_policy_index);
    flag = ((pcb->vc_data.qos_policy_index != data->qos_policy_index) || 
          (pcb->vc_data.flow_queue_qos_policy_index != data->flow_queue_qos_policy_index));
    if ((0 == old_index) && (0 != index)) /*新增*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_ADD NBB_CCXT);
#if defined (SPU) || defined (SRC)

        /* 建立入口盘和出口盘之间的全连接 */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                baseVCId, __FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vc_cb(pcb NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*调用驱动返回错误*/
        {
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_vc_cb(pcb NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {
            if(QOS_MAIN == proflag)
            {
                ret += ApiC3SetVpHqos(unit, posid, 0, 1, voq);
            }
            else
            {
                ret += ApiC3SetVpHqos(unit, posid, 1, 1, voq);
            }
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_vp_error_log(voq,1,posid,proflag,
                __FUNCTION__,__LINE__,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vc(slot, port, pB, pcb,data->node_index NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif
#ifdef PTN690
        spm_hqos_add_vc_pmline(pkey,voq NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_addline_hqos(data->node_index,slot,voq);
        if (ATG_DCI_RC_OK != rv)
        {
            spm_qos_vc_key_pd(pkey,HQOS_BMU_ERROR,rv NBB_CCXT);
            spm_hqos_almpm_addline_error_log(voq,data->node_index,slot,
                __FUNCTION__,__LINE__,rv NBB_CCXT);
        }
#endif
#endif     
        OS_MEMCPY(&(pcb->vc_data), data, sizeof(ATG_DCI_VC_UP_VPN_QOS_POLICY));
        pcb->flow_id = voq;
        
        //coverity[no_effect_test]
        rv = AVLL_INSERT(pB->vc_tree, pcb->spm_hqos_vc_node);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 != flag)) /*更新A节点policy index*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_UPD NBB_CCXT);
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*调用驱动返回错误*/
        {

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*引用计数只在出口盘*/
        {
            ret = spm_disconnect_policy_cnt(pcb->vc_data.qos_policy_index,
                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*引用计数减1失败*/
            {

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            ret = spm_disconnect_policy_cnt(pcb->vc_data.flow_queue_qos_policy_index,
                ATG_DCI_RC_OK NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*引用计数减1失败*/
            {

                /*异常跳出*/
                goto EXIT_LABEL;
            }
        }
        OS_MEMCPY(&(pcb->vc_data), data, sizeof(ATG_DCI_VC_UP_VPN_QOS_POLICY));

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 == flag)) /*同样的配置*/
    {
    /*
        spm_qos_vc_key_pd(pkey,HQOS_BMU_ERROR,ret NBB_CCXT);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS pw the same index",ATG_DCI_RC_OK,
						"index","old_index","flag","",
						index,old_index,flag,0));
    */  
    
        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 == index)) /*删除A节点*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_DEL NBB_CCXT);
        if (slot == fapid) /*引用计数只在出口盘*/
        {
            spm_disconnect_policy_cnt(pcb->vc_data.qos_policy_index,
                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            spm_disconnect_policy_cnt(pcb->vc_data.flow_queue_qos_policy_index,
                ATG_DCI_RC_OK NBB_CCXT);

        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {
            if(QOS_MAIN == proflag)
            {
                rv += ApiC3SetVpHqos(unit, posid, 0, 0, pcb->flow_id);
            }
            else
            {
                rv += ApiC3SetVpHqos(unit, posid, 1, 0, pcb->flow_id);
            }
        }
        if (ATG_DCI_RC_OK != rv)
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,rv NBB_CCXT);
            spm_hqos_vp_error_log(voq,0,posid,proflag,
                __FUNCTION__,__LINE__,rv NBB_CCXT);
            ret = rv;
        }
#endif

#if defined (SPU) || defined (SRC)

        /* 删除入口盘和出口盘之间的voq全连接 */
        rv = ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != rv)
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,rv NBB_CCXT);
            spm_hqos_arad_voq_del_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                baseVCId,__FUNCTION__,__LINE__,rv NBB_CCXT);
            ret = rv;
        }
#ifdef PTN690
        spm_hqos_del_vc_pmline(pkey NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_delline_hqos(data->node_index,slot);
        if (ATG_DCI_RC_OK != rv)
        {
            printf("**QOS ERROR**%s,LINE=%d almpm_delline_hqos,node_index=%ld",
                __FUNCTION__,__LINE__,data->node_index);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS almpm_delline_hqos",rv,
						"baseQueue","slot","port","baseVCId",
						voq,slot,port,baseVCId));
            ret = rv;
        }
#endif
#endif
        AVLL_DELETE(pB->vc_tree, pcb->spm_hqos_vc_node);
        rv = spm_hqos_del_vc(slot, port, pB, pcb,data->node_index NBB_CCXT);
        if (ATG_DCI_RC_OK != rv) /*返回错误*/
        {
            ret = rv;
        }
        if (NULL == AVLL_FIRST(pB->vc_tree) && NULL == AVLL_FIRST(pB->vrf_tree)
            && (0 == pB->node_index))
        {
           AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pB->spm_hqos_lsp_tx_node);
           spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);
        }

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((0 == old_index) && (0 == index))
    { 
	       OS_SPRINTF(ucMessage,"QOS VC NODE DUPLICATE ELIMINATION : NO FAULT, JUST PRINT THE INFORMATION.\n"
				       "qos_policy_index=%ld,flow_queue_qos_policy_indexdata=%ld,node_index=%ld,posid=%ld\n\n",
				       data->qos_policy_index,data->flow_queue_qos_policy_index,data->node_index,posid);
	        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
		 spm_free_hqos_vc_cb(pcb NBB_CCXT);
		 goto EXIT_LABEL;
    }
    else /*非法配置*/
    {
        /*printf("**QOS ERROR**%s,%d vc hqos config missmatch\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS vc pw config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
						"qos_policy_index","flow_queue_qos_policy_indexdata",
						"node_index","posid",
						data->qos_policy_index,data->flow_queue_qos_policy_index,
						data->node_index,posid));*/
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*异常跳出*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_hqos_add_vrf_node(NBB_USHORT slot,
    NBB_LONG port,
    NBB_ULONG posid,
    SPM_QOS_TUNNEL_KEY *skey,
    SPM_QOS_VRF_INSTANSE_KEY *pkey,
    ATG_DCI_VRF_INSTANCE_UP_VPN_QOS *data NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_VRF_CB *pcb = NULL;
    SPM_HQOS_LSP_TX_CB *pB = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_ULONG old_index = 0;
    NBB_ULONG index = 0;
    NBB_ULONG flag = 0;
    NBB_BYTE fapid = 0;
    NBB_LONG voq = 0;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG unit = 0;
#endif
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG baseVCId = 0;
    
    //NBB_ULONG port_index = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    NBB_LONG vrf_pw_flag = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* vrf振荡过 */
    if(0 == posid)
    {
        ret = spm_hqos_clear_vrf_mem(pkey,data NBB_CCXT);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if ((NULL == pkey) || (NULL == skey) || (NULL == data)
         || (0 == posid) || (0 == slot))
    {
        printf("**QOS ERROR**%s,LINE=%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_vrf_cfg_print)
    {
        OS_PRINTF("%s %s,LINE=%d slot=%d,port=%ld,vrf_id=%d,peer_ip=0x%lx label=%ld,posid=%ld,\n"
                  "ingress=0x%lx,egress=0x%lx,lspid=%ld,tunnelid=%ld\n"
                  "vrfid=%d,fec=%ld,mask=%d,lsp type=%s,flag=%s\n"
                  "node index=%ld,policy index=%ld,que policy index=%ld\n\n",
                  QOS_CFG_STRING,__FUNCTION__,__LINE__,
                  slot,port,pkey->vrf_id,pkey->peer_ip,pkey->label,posid,
                  skey->tx_lsp.ingress,skey->tx_lsp.egress,skey->tx_lsp.lspid,
                  skey->tx_lsp.tunnelid,skey->ftn.vrfid,skey->ftn.fec,skey->ftn.mask,
                  (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"主用":"备用",
                  data->node_index,data->qos_policy_index,
                  data->flow_queue_qos_policy_index);
        OS_SPRINTF(ucMessage,"%s %s,LINE=%d slot=%d,port=%ld,vrf_id=%d,peer_ip=0x%lx label=%ld,posid=%ld,\n"
                   "ingress=0x%lx,egress=0x%lx,lspid=%ld,tunnelid=%ld\n"
                   "vrfid=%d,fec=%ld,mask=%d,lsp type=%s,flag=%s\n"
                   "node index=%ld,policy index=%ld,que policy index=%ld\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__,
                   slot,port,pkey->vrf_id,pkey->peer_ip,pkey->label,posid,
                   skey->tx_lsp.ingress,skey->tx_lsp.egress,skey->tx_lsp.lspid,
                   skey->tx_lsp.tunnelid,skey->ftn.vrfid,skey->ftn.fec,skey->ftn.mask,
                   (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"主用":"备用",
                   data->node_index,data->qos_policy_index,
                   data->flow_queue_qos_policy_index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }

    /* 判断目的槽位是否已开启hqos开关 */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;
        OS_SPRINTF(ucMessage,"%s %s,%d : Objective slot=%d don't open HQOS switch.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__,slot);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 获取本槽位的槽位号 */
    spm_hardw_getslot(&fapid);

    /* 判断本槽位是否已开启hqos开关 */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;
        OS_SPRINTF(ucMessage,"%s %s,%d : This slot=%d don't open HQOS switch.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__,fapid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    if(port > 0x80)
    {
        OS_SPRINTF(ucMessage,"%s,%d slot=%d,port=%ld,label=%ld,vrf_id=%d,peer_ip=0x%lx : "
        "LAG NONSUPPORT HQOS, JUST PRINT THE CONFIGURATION.\n\n",
                   __FUNCTION__,__LINE__,slot,port,pkey->label,pkey->vrf_id,pkey->peer_ip);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_OK;

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /*
      slot_index  : 目的槽位索引值
      fapid_index : 本槽位索引值
    */
    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * data->node_index + HQOS_OFFSET;

    /* 查找目的槽位的目的端口是否存在B节点,即lsp节点 */
    pB = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].lsp_tree, skey);

    /* 如果条目不存在则分配默认B节点和LSP ID */
    if (pB == NULL)
    {
        /* 不允许为0 */
        if((0 == data->node_index) ||
           (0 == data->qos_policy_index) ||
           (0 == data->flow_queue_qos_policy_index))
        {
            printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
    						"","","","",0,0,0,0));
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS param is NULL.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_ADD NBB_CCXT);

        /* 申请B节点 */
        pB = spm_alloc_hqos_lsp_tx_cb(skey,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (NULL == pB)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_alloc_hqos_lsp_tx_cb failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        pB->lsp_id = port + 1;

        /* 将vrf与hqos的配置绑定在同一个结构体中,在申请B节点后,插入到B节点的vrf树中 */
        pcb = spm_alloc_hqos_vrf_cb(pkey,ATG_DCI_RC_OK  NBB_CCXT);
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_alloc_hqos_vrf_cb failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)

        /* 建立入口盘和出口盘之间的全连接 */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                        baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }    

        /* 将vrf的hqos配置写入驱动 */
        ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* 在txpw软表中写入flowid,即在ihmt头中加入flowid字段 */
            ret += ApiC3SetL3TxPwHqos(unit, posid, 1, voq);
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_l3txpw_error_log(voq,1,posid,__FUNCTION__,__LINE__,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vrf(slot, port, pB, pcb,data->node_index NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif
#ifdef PTN690
        spm_hqos_add_vrf_pmline(pkey,voq NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_addline_hqos(data->node_index,slot,voq);
        if (ATG_DCI_RC_OK != rv)
        {
            spm_qos_vrf_key_pd(pkey,HQOS_BMU_ERROR,rv NBB_CCXT);
            spm_hqos_almpm_addline_error_log(voq,data->node_index,slot,
                                    __FUNCTION__,__LINE__,rv NBB_CCXT);
        }
#endif
#endif
        OS_MEMCPY(&(pcb->vrf_data), data, sizeof(ATG_DCI_VRF_INSTANCE_UP_VPN_QOS));
        pcb->flow_id = voq;
        rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pB->spm_hqos_lsp_tx_node);
        rv = AVLL_INSERT(pB->vrf_tree, pcb->spm_hqos_vrf_node);

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(pB->vrf_tree, pkey);

    /* 如果条目不存在则分配A节点内存和pw id */
    if (pcb == NULL)
    {
    	/* 防止重复注册,因为一个vrf实例中可能存在多个标签,此时只需要注册一次 */
        if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
            vrf_pw_flag = ATG_DCI_RC_OK;
        }
        else
        {
           vrf_pw_flag = ATG_DCI_RC_UNSUCCESSFUL;
        } 
        pcb = spm_alloc_hqos_vrf_cb(pkey,vrf_pw_flag NBB_CCXT);
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_alloc_hqos_vrf_cb failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_find_vrf_pwid(pB,pcb NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);

            /*异常跳出*/
            goto EXIT_LABEL;
        }
    }

    /* 相同vrfid更新hqos的配置时,node_index必须一致否则直接报错 */
    if ((0 != pcb->vrf_data.node_index) && (pcb->vrf_data.node_index != data->node_index))
    {
        spm_check_hqos_vrf_node_index_cfg_log(pkey,pcb->vrf_data.node_index,
                           data->node_index,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        
        /*跳出可能挂掉*/

        //goto EXIT_LABEL;
    }

    /* flag为0表示hqos的配置未更新,与老配置相同 */
    old_index = (pcb->vrf_data.qos_policy_index) | (pcb->vrf_data.flow_queue_qos_policy_index);
    index = (data->qos_policy_index) | (data->flow_queue_qos_policy_index);
    flag = ((pcb->vrf_data.qos_policy_index != data->qos_policy_index) || 
           (pcb->vrf_data.flow_queue_qos_policy_index != data->flow_queue_qos_policy_index));
    if ((0 == old_index) && (0 != index)) /*新增*/
    {
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_ADD NBB_CCXT);

        /* 防止重复注册,因为一个vrf实例中可能存在多个标签,此时只需要注册一次 */
        if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
#if defined (SPU) || defined (SRC)

            /* 建立入口盘和出口盘之间的全连接 */
            ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            if (ATG_DCI_RC_OK != ret)       
            {
                spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                                 baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                spm_free_hqos_vrf_cb(pcb NBB_CCXT);

                /*异常跳出*/
                goto EXIT_LABEL;
            }
            ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*调用驱动返回错误*/
            {
#if defined (SPU) || defined (SRC)

                /* 删除入口盘和出口盘之间的voq全连接 */
                ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif

                /*异常跳出*/
                goto EXIT_LABEL;
            }
        }

#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* 在txpw软表中写入flowid,即在ihmt头中加入flowid字段 */
            ret += ApiC3SetL3TxPwHqos(unit, posid, 1, voq);
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_l3txpw_error_log(voq,1,posid,__FUNCTION__,__LINE__,ret NBB_CCXT);

            if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
            {
#if defined (SPU) || defined (SRC)

                /* 删除入口盘和出口盘之间的voq全连接 */
                ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
                spm_hqos_del_vrf(slot, port, pB, pcb,data->node_index NBB_CCXT);
            }

            /*异常跳出*/
            goto EXIT_LABEL;
        }
#endif

        /* 防止重复注册,因为一个vrf实例中可能存在多个标签,此时只需要注册一次 */
        if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
#ifdef PTN690
	        spm_hqos_add_vrf_pmline(pkey,voq NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
	        rv = almpm_addline_hqos(data->node_index,slot,voq);
	        if (ATG_DCI_RC_OK != rv)
	        {
	            spm_qos_vrf_key_pd(pkey,HQOS_BMU_ERROR,rv NBB_CCXT);
	            spm_hqos_almpm_addline_error_log(voq,data->node_index,slot,
	                                    __FUNCTION__,__LINE__,rv NBB_CCXT);
	        }
#endif
#endif
		}
        OS_MEMCPY(&(pcb->vrf_data), data, sizeof(ATG_DCI_VRF_INSTANCE_UP_VPN_QOS));
        pcb->flow_id = voq;
        rv = AVLL_INSERT(pB->vrf_tree, pcb->spm_hqos_vrf_node);

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* flag为0表示vrf中hqos的原配置与新配置相同 */
    else if ((0 != old_index) && (0 != index) && (0 != flag)) /*更新A节点policy index*/
    {

        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_UPD NBB_CCXT);
#if 0
        ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*调用驱动返回错误*/
        {
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_hqos_add_vrf failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*引用计数只在出口盘*/
        {
            ret = spm_disconnect_policy_cnt(pcb->vrf_data.qos_policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*引用计数减1失败*/
            {
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_disconnect_policy_cnt failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
            ret = spm_disconnect_policy_cnt(pcb->vrf_data.flow_queue_qos_policy_index,ATG_DCI_RC_OK NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*引用计数减1失败*/
            {
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_disconnect_policy_cnt failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
        }
        OS_MEMCPY(&(pcb->vrf_data), data, sizeof(ATG_DCI_VRF_INSTANCE_UP_VPN_QOS));
#endif

        /*异常跳出*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 == flag)) /*同样的配置*/
    {
	     OS_SPRINTF(ucMessage,"##### VRF:SAME HQOS CONFIGURATION (vrf_id=%d,peer_ip=0x%lx,"
	        		"label=%ld,index=%ld,old_index=%ld) #####\n",
	                pkey->vrf_id,pkey->peer_ip,pkey->label,index,old_index);
	     BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage); 
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* 在txpw软表中写入flowid,即在ihmt头中加入flowid字段 */
            ret += ApiC3SetL3TxPwHqos(unit, posid, 1, voq);
        }
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR**%s,%d ret=%ld ApiC3SetL3TxPwHqos voq=%ld posid=%ld\n",
                __FUNCTION__,__LINE__,ret,voq, posid);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiC3SetL3TxPwHqos",ret,
						"baseQueue","slot","port","posid",voq,slot,port,posid));
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR ApiC3SetL3TxPwHqos failed.\n"
                       "ret=%ld, baseQueue=%ld, slot=%d, port=%ld, posid=%ld.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__,ret,voq,slot,port,posid);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        }
#endif

         /*异常跳出*/
         goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 == index)) /* 删除A节点 */
    {
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_DEL NBB_CCXT);
        if ((1 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT)) && (slot == fapid)) /* 删除vrf节点时,引用计数只在出口盘 */
        {
            spm_disconnect_policy_cnt(pcb->vrf_data.qos_policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            spm_disconnect_policy_cnt(pcb->vrf_data.flow_queue_qos_policy_index,ATG_DCI_RC_OK NBB_CCXT);
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* 在txpw软表中写入flowid,即在ihmt头中加入flowid字段 */
            rv += ApiC3SetL3TxPwHqos(unit, posid, 0, 0);
        }
        if (ATG_DCI_RC_OK != rv)
        {
            printf("**QOS ERROR**%s,%d ApiC3SetL3TxPwHqos=%ld\n",__FUNCTION__,__LINE__,rv);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiC3SetL3TxPwHqos",rv,
						"baseQueue","","","",pcb->flow_id,0,0,0));
            OS_SPRINTF(ucMessage,"%s %s,%d : ApiC3SetL3TxPwHqos failed rv=%ld, baseQueue=%ld.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__,rv,pcb->flow_id);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            ret = rv;
        }
#endif

		/* 防止重复注册,删除时只在最后一次才注册;添加时只在第一次才注册 */
        if(1 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
#if defined (SPU) || defined (SRC)

            /* 删除入口盘和出口盘之间的voq全连接 */
            rv = ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            if (ATG_DCI_RC_OK != rv)
            {
                spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,rv NBB_CCXT);
                spm_hqos_arad_voq_del_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                                      baseVCId,__FUNCTION__,__LINE__,rv NBB_CCXT);
                ret = rv;
            }        
#ifdef PTN690
			spm_hqos_del_vrf_pmline(pkey NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
	        rv = almpm_delline_hqos(data->node_index,slot);
	        if (ATG_DCI_RC_OK != rv)
	        {
	            printf("**QOS ERROR**%s,%d almpm_delline_hqos,node_index=%ld",
	                      __FUNCTION__,__LINE__,data->node_index);
	            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
							   "HQOS almpm_delline_hqos",rv,
							   "baseQueue","slot","port","baseVCId",
							   voq,slot,port,baseVCId));
	        }
#endif
#endif
        }
        AVLL_DELETE(pB->vrf_tree, pcb->spm_hqos_vrf_node);
        rv = spm_hqos_del_vrf(slot, port, pB, pcb,data->node_index NBB_CCXT);
        if (ATG_DCI_RC_OK != rv)
        {
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_hqos_del_vrf failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            ret = rv;
        }

        /* 若此时lsp节点上未挂树,即vc和vrf的树均为空,则删掉目的槽位端口上的lsp树节点 */
        if (NULL == AVLL_FIRST(pB->vc_tree) && NULL == AVLL_FIRST(pB->vrf_tree)
            && (0 == pB->node_index))
        {
           AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pB->spm_hqos_lsp_tx_node);
           spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);
        }

        /*异常跳出*/
        goto EXIT_LABEL;
    }

    /* 非法配置 */
    else
    {
	     OS_SPRINTF(ucMessage,"##### VRF:ERROR HQOS CONFIGURATION (vrf_id=%d,peer_ip=0x%lx,label=%ld) #####\n"
	  			    "node_index=%ld,qos_policy_index=%ld,flow_queue_qos_policy_index=%ld,posid=%ld\n\n",
                    pkey->vrf_id,pkey->peer_ip,pkey->label,data->node_index,data->qos_policy_index,
				    data->flow_queue_qos_policy_index,posid);
	     BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);  
         ret = ATG_DCI_RC_OK;

         /*异常跳出*/
         goto EXIT_LABEL;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_usr_group_cnt
   功能描述  : 释放HQOS树的节点并释放内存
   输入参数  : upflag=0代表更新
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_hqos_clear_all(NBB_CXT_T NBB_CXT)
{
    /*************************************************************************/
    /* Local Variables                                                       */
    /*************************************************************************/
    SPM_HQOS_LSP_TX_CB *lsp_cb = NULL;
    SPM_HQOS_VRF_CB *vrf_cb = NULL;
    SPM_HQOS_VC_CB *vc_cb = NULL;
    SPM_HQOS_LOG_GROUP_CB *group_cb = NULL;
    SPM_HQOS_LOG_USR_CB *usr_cb = NULL;
    NBB_USHORT i = 0;
    NBB_USHORT j = 0;
    NBB_LONG ret = 0;
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_LONG slot = 0;
    NBB_LONG voq = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG baseVCId = 0;

    for(j = 0; j < MAX_SLOT_NUM;j++)
    {
        /* 判断slot是否开启hqos的开关 */
        ret = spm_hqos_checkslot(j + 1, &slot_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)/*不支持HQOS的槽位跳过*/
        {
            continue;
        }
        
        /* 判断slot是否开启hqos的开关 */
        ret = spm_hqos_checkslot(v_spm_shared->local_slot_id, &fapid_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)/*不支持HQOS的槽位跳过*/
        {
            continue;
        }

        /* 清楚目的槽位目的端口上lsp树中hqos相关的配置信息 */
        for (i = 0; i < MAX_PHYSIC_PORT_NUM; i++)
        {
            for (lsp_cb = (SPM_HQOS_LSP_TX_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[j][i].lsp_tree);
                lsp_cb != NULL;
                lsp_cb = (SPM_HQOS_LSP_TX_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[j][i].lsp_tree))
            {
                for (vrf_cb = (SPM_HQOS_VRF_CB *)AVLL_FIRST(lsp_cb->vrf_tree);
                    vrf_cb != NULL;
                    vrf_cb = (SPM_HQOS_VRF_CB *)AVLL_FIRST(lsp_cb->vrf_tree))
                {
                    /* 当同一个vrf中有多条路由时,只在最后一条路由时才删除整个vrf的hqos配置信息 */
                    if(1 == spm_hqos_vrftree_counter(lsp_cb,&(vrf_cb->vrf_key) NBB_CCXT))
                    {
                        voq = slot_index* NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
                        baseVCId = fapid_index * NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

                        /* 删除入口盘和出口盘之间的voq全连接 */
                        ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                        if (ATG_DCI_RC_OK != ret)
                        {
                            spm_qos_vrf_key_pd(&(vrf_cb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                            spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                                  baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                        }

                        /* 删除已经创建的voq全连接 */
                        spm_hqos_del_voq(j + 1, i, vrf_cb->vrf_data.node_index, vrf_cb->pw_id NBB_CCXT);
                        if((j + 1) == v_spm_shared->local_slot_id)
                        {
#if defined (SPU) || defined (SRC)
                            ret = ApiAradHqosPwDelete(UNIT_0,vrf_cb->pw_id,lsp_cb->lsp_id);
#endif
                            if(ATG_DCI_RC_OK != ret)
                            {
                                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            						"HQOS vrf pw delete",ret,
            						"node_index","flow_id","lsp_id","pw_id",
            						vrf_cb->vrf_data.node_index,vrf_cb->flow_id,
            						lsp_cb->lsp_id,vrf_cb->pw_id));
                            }
                        }
                    }
                    AVLL_DELETE(lsp_cb->vrf_tree, vrf_cb->spm_hqos_vrf_node);
                    spm_free_hqos_vrf_cb(vrf_cb NBB_CCXT);
                }
                for (vc_cb = (SPM_HQOS_VC_CB *)AVLL_FIRST(lsp_cb->vc_tree);
                    vc_cb != NULL;
                    vc_cb = (SPM_HQOS_VC_CB *)AVLL_FIRST(lsp_cb->vc_tree))
                {
                    voq = slot_index* NUM_COS + offset * vc_cb->vc_data.node_index + HQOS_OFFSET;
                    baseVCId = fapid_index * NUM_COS + offset * vc_cb->vc_data.node_index + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

                    /* 删除入口盘和出口盘之间的voq全连接 */
                    ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                    if (ATG_DCI_RC_OK != ret)
                    {
                        spm_qos_vc_key_pd(&(vc_cb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                        spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                              baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                    }

                    /* 删除已经创建的voq全连接 */
                    spm_hqos_del_voq(j + 1, i, vc_cb->vc_data.node_index, vc_cb->pw_id NBB_CCXT);
                    if((j + 1) == v_spm_shared->local_slot_id)
                    {
#if defined (SPU) || defined (SRC)
                        ret = ApiAradHqosPwDelete(UNIT_0,vc_cb->pw_id,lsp_cb->lsp_id);
#endif
                        if(ATG_DCI_RC_OK != ret)
                        {
                            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        						"HQOS vc pw delete",ret,
        						"node_index","flow_id","lsp_id","pw_id",
        						vc_cb->vc_data.node_index,vc_cb->flow_id,
        						lsp_cb->lsp_id,vc_cb->pw_id));
                        }
                    }
                    AVLL_DELETE(lsp_cb->vc_tree, vc_cb->spm_hqos_vc_node);
                    spm_free_hqos_vc_cb(vc_cb NBB_CCXT);
                }

                /* 在出口盘上删除hqos配置过程中创建的lspid */
                if(((j + 1) == v_spm_shared->local_slot_id)
                    && (lsp_cb->lsp_id > MAX_PHYSIC_PORT_NUM))
                {
#if defined (SPU) || defined (SRC)
                    ret = ApiAradHqosLspDelete(UNIT_0,i,lsp_cb->lsp_id);
#endif
                    if (ATG_DCI_RC_OK != ret) /*失败*/
                    {
                        spm_qos_txlsp_key_pd(&(lsp_cb->lsp_tx_key),HQOS_LSP_ERROR,ret NBB_CCXT);
                        spm_hqos_arad_create_lsp_error_log(i + PTN_690_PORT_OFFSET, lsp_cb->lsp_id,
                                __FUNCTION__,__LINE__,ret NBB_CCXT);
                    }
                }
                AVLL_DELETE((v_spm_shared->qos_port_cb[j][i]).lsp_tree, lsp_cb->spm_hqos_lsp_tx_node);
                spm_free_hqos_lsp_tx_cb(lsp_cb NBB_CCXT);
            }
            for (group_cb = (SPM_HQOS_LOG_GROUP_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[j][i].group_tree);
                group_cb != NULL;
                group_cb = (SPM_HQOS_LOG_GROUP_CB *)AVLL_FIRST(v_spm_shared->qos_port_cb[j][i].group_tree))
            {
                for (usr_cb = (SPM_HQOS_LOG_USR_CB *)AVLL_FIRST(group_cb->usr_tree);
                    usr_cb != NULL;
                    usr_cb = (SPM_HQOS_LOG_USR_CB *)AVLL_FIRST(group_cb->usr_tree))
                {
                    voq = slot_index* NUM_COS + offset * usr_cb->usr_data.node_index + HQOS_OFFSET;
                    baseVCId = fapid_index * NUM_COS + offset * usr_cb->usr_data.node_index + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

                    /* 删除入口盘和出口盘之间的voq全连接 */
                    ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                    if (ATG_DCI_RC_OK != ret)
                    {
                        spm_qos_log_key_pd(usr_cb->index,HQOS_USR_ERROR,ret NBB_CCXT);
                        spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                              baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                    }

                    /* 删除已经创建的voq全连接 */
                    spm_hqos_del_voq(j + 1, i, usr_cb->usr_data.node_index, usr_cb->pw_id NBB_CCXT);
                    if((j + 1) == v_spm_shared->local_slot_id)
                    {
#if defined (SPU) || defined (SRC)
                        ret = ApiAradHqosPwDelete(UNIT_0,usr_cb->pw_id,group_cb->lsp_id);
#endif
                        if(ATG_DCI_RC_OK != ret)
                        {
                            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        						"HQOS vc pw delete",ret,
        						"node_index","flow_id","lsp_id","pw_id",
        						usr_cb->usr_data.node_index,usr_cb->flow_id,
        						group_cb->lsp_id,usr_cb->pw_id));
                        }
                    }
                    AVLL_DELETE(group_cb->usr_tree, usr_cb->spm_hqos_usr_node);
                    spm_free_hqos_log_usr_cb(usr_cb NBB_CCXT);
                }
                if((j + 1) == v_spm_shared->local_slot_id)
                {
#if defined (SPU) || defined (SRC)
                    ret = ApiAradHqosLspDelete(UNIT_0,i,group_cb->lsp_id);
#endif
                    if (ATG_DCI_RC_OK != ret) /*失败*/
                    {
                        spm_hqos_arad_create_lsp_error_log(i + PTN_690_PORT_OFFSET, group_cb->lsp_id,
                                __FUNCTION__,__LINE__,ret NBB_CCXT);
                    }
                }
                AVLL_DELETE(v_spm_shared->qos_port_cb[j][i].group_tree, group_cb->spm_hqos_group_node);
                spm_free_hqos_log_group_cb(group_cb NBB_CCXT);
            }
        } 
    }

    return;
}

#endif
