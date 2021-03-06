/******************************************************************************

                  版权所有 (C), 1999-2013, 烽火通信科技股份有限公司

******************************************************************************
   文 件 名   : spm_qos_action.c
   版 本 号   : 初稿
   作    者   : zenglu
   生成日期   : 2012年9月18日
   最近修改   :
   功能描述   : QOS处理
   函数列表   :
   修改历史   :
   日    期   : 2012年9月18日
   作    者   : zenglu
   修改内容   : 创建文件

******************************************************************************/
#define SHARED_DATA_TYPE SPM_SHARED_LOCAL

#include <nbase.h>
#include <spmincl.h>

/* 全局打印变量acl_pri_setting */
extern NBB_BYTE acl_pri_setting;

/* 全局打印变量qos_acl_action_print */
extern NBB_BYTE qos_acl_action_print;

/* 全局打印变量qos_log_cfg_print */
extern NBB_BYTE qos_log_cfg_print;

/* 全局打印变量qos_txlsp_cfg_print */
extern NBB_BYTE qos_txlsp_cfg_print;

/* 全局打印变量qos_pw_cfg_print */
extern NBB_BYTE qos_pw_cfg_print;


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_ACTION_CB *spm_qos_find_action_cb(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_ACTION_CB *pcb = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (0 == index)
    {
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(SHARED.qos_action_tree, &index);
    if (NULL == pcb)
    {
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return pcb;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_disconnect_action_cnt(NBB_ULONG index,NBB_BYTE wred_flag NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_ACTION_CB *pcb = NULL;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_action_cnt err : ret=%ld, "
                  "action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_action_cnt err : ret=%ld, "
                   "action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(SHARED.qos_action_tree, &index);
    if (NULL == pcb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_action_cnt err : can't find action cfg. "
                  "ret = %ld, action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_action_cnt err : can't find action cfg. "
                   "ret = %ld, action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        goto EXIT_LABEL;
    }
    
    /*for (i = 0; (i < pcb->que_congest_num)&&(ATG_DCI_RC_OK == wred_flag); i++)
    {
        ret = spm_disconnect_hqos_wred_cnt(&(pcb->que_congst_cfg_cb[i]) NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
    }*/
    if (0 == pcb->cnt)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_action_cnt err : action cnt is 0. "
                  "ret = %ld, action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_action_cnt err : action cnt is 0. "
                   "ret = %ld, action_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        goto EXIT_LABEL;
    }
    (pcb->cnt)--;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_connect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_connect_action_cnt(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_ACTION_CB *pcb = NULL;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_connect_action_cnt err : ret = %ld, "
                  "behavior_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_connect_action_cnt err : ret = %ld, "
                   "behavior_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(SHARED.qos_action_tree, &index);
    if (NULL == pcb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_connect_action_cnt err : can't find action cfg. "
                  "ret = %ld, behavior_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_connect_action_cnt err : can't find action cfg. "
                   "ret = %ld, behavior_index = %lu\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }
    (pcb->cnt)++;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_apply_meter_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    //NBB_ULONG cnt = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    for (id = MAX_PHYSIC_PORT_NUM * 3; id < MAX_METER_NUM; id++)
    {
        if (0 == SHARED.flow_meter_id[id]) /* 有空闲ID */
        {
            *pid = id + 1;
            SHARED.flow_meter_id[id] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_free_meter_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_UNSUCCESSFUL;

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if (*pid < MAX_PHYSIC_PORT_NUM * 3 + 1)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if (0 != SHARED.flow_meter_id[*pid - 1])
    {
        SHARED.flow_meter_id[*pid - 1] = 0;
        *pid = 0;
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }
    else
    {
        ret = ATG_DCI_RC_DEL_FAILED;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/***************************************************************************/
/*   help function                                                        */
/***************************************************************************/
#if 1

/*****************************************************************************
   函 数 名  : spm_qos_apply_color_action_id
   功能描述  : 申请色感知动作模板的id
   输入参数  : 模板id
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_apply_color_action_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    //NBB_ULONG cnt = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 色感知动作模板最多有4000个 */
    for (id = 0; id < MAX_COLOR_ACTION_NUM; id++)
    {
        /* 有空闲ID */
        if (0 == v_spm_shared->color_action_id[id])
        {
            *pid = id + 1;
            v_spm_shared->color_action_id[id] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;
            goto EXIT_LABEL;
        }
    }
    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
    ret = ATG_DCI_RC_UNSUCCESSFUL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_free_color_action_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_UNSUCCESSFUL;
    
    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((*pid > MAX_COLOR_ACTION_NUM) || (*pid < 1))
    {
        goto EXIT_LABEL;
    }
    if (0 != v_spm_shared->color_action_id[*pid - 1])
    {
        v_spm_shared->color_action_id[*pid - 1] = 0;
        *pid = 0;
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }
    
    /*ID已经被释放*/ 
    
    /*else
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_DEL_FAILED;
        goto EXIT_LABEL;
    }*/

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#ifdef PTN690

/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_apply_meter_bucket_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];
    //NBB_ULONG cnt = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 申请meter桶的模板id,目前最多只能申请512个meter桶 */
    for (id = 0; id < MAX_METER_BUKECT_NUM; id++)
    {
        if (0 == v_spm_shared->meter_bukect_id[id]) /* 有空闲ID */
        {
            *pid = id + 1;
            v_spm_shared->meter_bukect_id[id] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;
            goto EXIT_LABEL;
        }
    }
    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
    ret = ATG_DCI_RC_UNSUCCESSFUL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}





/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : action引用计数减1
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_free_meter_bucket_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_UNSUCCESSFUL;

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((*pid > MAX_METER_BUKECT_NUM) || (*pid < 1))
    {
        goto EXIT_LABEL;
    }
    if (0 != v_spm_shared->meter_bukect_id[*pid - 1])
    {
        v_spm_shared->meter_bukect_id[*pid - 1] = 0;
        *pid = 0;
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }
    
    /*ID已经被释放*/ 
    
    /*else
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_DEL_FAILED;
        goto EXIT_LABEL;
    }*/

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}





/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 不申请桶ID
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_POLICY_SUPERVISE_CB* spm_alloc_policy_supervise_cb(
    SPM_QOS_POLICY_SUPERVISE_KEY* pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_POLICY_SUPERVISE_CB *pcb = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pkey)
    {
        goto EXIT_LABEL;
    }
    pcb = (SPM_QOS_POLICY_SUPERVISE_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_POLICY_SUPERVISE_CB),
              NBB_NORETRY_ACT,MEM_SPM_ACTION_POLICY_SUPERVISE_CB);
    if(NULL == pcb)
    {
        goto EXIT_LABEL;
    }
    
    OS_MEMSET(pcb, 0, sizeof(SPM_QOS_POLICY_SUPERVISE_CB));
    OS_MEMCPY(&(pcb->policy_supervise_key), pkey, sizeof(SPM_QOS_POLICY_SUPERVISE_KEY));
    AVLL_INIT_NODE(pcb->policy_supervise_node);
    
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pcb);
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_policy_supervise_cb(SPM_QOS_POLICY_SUPERVISE_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    
    //SPM_QOS_POLICY_SUPERVISE_CB *pcb = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_POLICY_SUPERVISE_CB), MEM_SPM_ACTION_POLICY_SUPERVISE_CB);

    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/
    spm_qos_free_meter_bucket_id(&(pstTbl->meter_bucket_id)); 
    NBB_MM_FREE(pstTbl, MEM_SPM_ACTION_POLICY_SUPERVISE_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_del_policy_supervise_drive
   功能描述  : 释放所申请的bukect桶id
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_del_policy_supervise_drive(SPM_QOS_ACTION_CB *pcb NBB_CCXT_T NBB_CXT)
{
    SPM_QOS_POLICY_SUPERVISE_CB *cfg_cb = NULL;
    NBB_USHORT unit = 0;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG ret = ATG_DCI_RC_OK;
#endif

    NBB_TRC_ENTRY(__FUNCTION__);
    
    if(NULL != pcb)
    {

       /*********************************************************** 
        * 流量监管树(spm_qos_supervise_tree)中节点所存放的信息有: *
        * 1.与该behavior模块相关的meter桶的id号;                  *
        * 2.与该behavior模块相关的policy模板的id号;               *
        * 3.与该behavior模块相关的classify模板的id号;             *
        * 4.与该behavior模块相关的policy模板所对应的aclid号;      *
        ***********************************************************/
        for(cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_FIRST(pcb->spm_qos_supervise_tree); cfg_cb != NULL;
            cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_NEXT(pcb->spm_qos_supervise_tree,cfg_cb->policy_supervise_node))
        {
            if(0 != cfg_cb->meter_bucket_id)
            {
                for(unit = 0;unit < SHARED.c3_num;unit++)
                {
#if defined (SPU) || defined (PTN690_CES)
                    ret = fhdrv_qos_clear_policer_bucket_template(unit,cfg_cb->meter_bucket_id);
                    if(ATG_DCI_RC_OK != ret)
                    {
                        spm_clear_policer_bucket_template_error_log(unit,cfg_cb->meter_bucket_id,
                            __FUNCTION__,__LINE__,ret NBB_CCXT);
                    }
#endif
                }
            }
        }
    }

    NBB_TRC_EXIT();
    return;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_del_policy_supervise_mem(SPM_QOS_ACTION_CB *pcb NBB_CCXT_T NBB_CXT)
{
    SPM_QOS_POLICY_SUPERVISE_CB *cfg_cb = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);
    
    if(NULL != pcb)
    {
        for(cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_FIRST(pcb->spm_qos_supervise_tree); 
            cfg_cb != NULL;
            cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_FIRST(pcb->spm_qos_supervise_tree))
        {
            AVLL_DELETE(pcb->spm_qos_supervise_tree, cfg_cb->policy_supervise_node);
            spm_free_policy_supervise_cb(cfg_cb NBB_CCXT); 
        }
    }

    NBB_TRC_EXIT();
    return;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_del_policy_supervise(SPM_QOS_ACTION_CB *pcb NBB_CCXT_T NBB_CXT)
{
    NBB_TRC_ENTRY(__FUNCTION__);
    
    spm_del_policy_supervise_drive(pcb NBB_CCXT);
    spm_del_policy_supervise_mem(pcb NBB_CCXT);
    
    NBB_TRC_EXIT();
    return;
}

#endif


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_ACTION_CB *spm_alloc_action_cb(NBB_ULONG ulkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_ACTION_CB *pstTbl = NULL;
    NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY(__FUNCTION__);

    pstTbl = (SPM_QOS_ACTION_CB*)NBB_MM_ALLOC(sizeof(SPM_QOS_ACTION_CB),
              NBB_NORETRY_ACT,MEM_SPM_ACTION_CB);
    if (pstTbl == NULL)
    {
        goto EXIT_LABEL;
    }
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_ACTION_CB));
    pstTbl->action_key = ulkey;
    AVLL_INIT_NODE(pstTbl->spm_action_node);

#ifdef PTN690
    avll_key_offset = NBB_OFFSETOF(SPM_QOS_POLICY_SUPERVISE_CB, policy_supervise_key);
    AVLL_INIT_TREE(pstTbl->spm_qos_supervise_tree, spm_qos_policy_supervise_comp,
            (NBB_USHORT)avll_key_offset,
            (NBB_USHORT)NBB_OFFSETOF(SPM_QOS_POLICY_SUPERVISE_CB,policy_supervise_node));
#endif

    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_action_cb(SPM_QOS_ACTION_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_ACTION_CB), MEM_SPM_ACTION_CB);

    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/


    /***************************************************************************/
    /* 释放基本配置数据块。                                                */
    /***************************************************************************/
    if (NULL != pstTbl->basic_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->basic_cfg_cb, MEM_SPM_ACTION_BASIC_CB);
        pstTbl->basic_cfg_cb = NULL;
    }
    
#ifdef PTN690

   /************************************************* 
    * 在删除behavior模块之前,需要先删除behavior模块 *
    * 中流量监管树(spm_qos_supervise_tree)中的节点. *
    *************************************************/
    spm_del_policy_supervise_mem(pstTbl NBB_CCXT);
#endif

    if (NULL != pstTbl->supervise_cfg_cb)
    {           
        NBB_MM_FREE(pstTbl->supervise_cfg_cb, MEM_SPM_ACTION_SUPERVISE_CB);
        pstTbl->supervise_cfg_cb = NULL;
    }
    if (NULL != pstTbl->shaping_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->shaping_cfg_cb, MEM_SPM_ACTION_SHAPING_CB);
        pstTbl->shaping_cfg_cb = NULL;
    }
    if (NULL != pstTbl->schedule_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->schedule_cfg_cb, MEM_SPM_ACTION_SCHEDULE_CB);
        pstTbl->schedule_cfg_cb = NULL;
    }
    if (NULL != pstTbl->flow_act_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->flow_act_cfg_cb, MEM_SPM_ACTION_FLOW_ACT_CB);
        pstTbl->flow_act_cfg_cb = NULL;
    }
    if (NULL != pstTbl->flow_samping_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->flow_samping_cfg_cb, MEM_SPM_ACTION_FLOW_SAMPL_CB);
        pstTbl->flow_samping_cfg_cb = NULL;
    }
    if (NULL != pstTbl->redirect_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->redirect_cfg_cb, MEM_SPM_ACTION_REDIRECT_CB);
        pstTbl->redirect_cfg_cb = NULL;
    }
    if (NULL != pstTbl->policy_nhi_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->policy_nhi_cfg_cb, MEM_SPM_ACTION_POLCY_RT_CB);
        pstTbl->policy_nhi_cfg_cb = NULL;
    }
    if (NULL != pstTbl->phb_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->phb_cfg_cb, MEM_SPM_ACTION_PHB_CB);
        pstTbl->phb_cfg_cb = NULL;
    }
    if (NULL != pstTbl->domain_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->domain_cfg_cb, MEM_SPM_ACTION_DOMAIN_CB);
        pstTbl->domain_cfg_cb = NULL;
    }
    if (NULL != pstTbl->urpf_cfg_cb)
    {
        NBB_MM_FREE(pstTbl->urpf_cfg_cb, MEM_SPM_ACTION_URPF_CB);
        pstTbl->urpf_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_ACTION_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#endif


#if 2

/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_shaping_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_SHAPPING **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_SHAPPING *ptemp = NULL;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_SHAPING_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_SHAPPING *)buf;
            if ((ptemp->cir) > (ptemp->pir))
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                printf("%s line = %d spm_set_action_shaping_cfg err : cir > pir. "
                       "ret = %ld, cir = %lu, pir = %lu\n\n",__FUNCTION__,__LINE__,ret,ptemp->cir,ptemp->pir);
                OS_SPRINTF(ucMessage,"%s line = %d spm_set_action_shaping_cfg err : cir > pir. "
                           "ret = %ld, cir = %lu, pir = %lu\n\n",__FUNCTION__,__LINE__,ret,ptemp->cir,ptemp->pir);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            			       QOS_ERROR_STRING,ret,
            			       ucMessage,__FUNCTION__,"LINE","",
            			       0,0,__LINE__,0)); 
                goto EXIT_LABEL;
            }

            ptemp->pir -= ptemp->cir;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_SHAPPING *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_SHAPPING),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_SHAPING_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                } 
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_SHAPPING));
            ptemp->pir += ptemp->cir;
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : BEHAVIOR中流量监管用来给复杂流分类限速;
               流量整型用来给hqos中tunnel限速
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_meter(ATG_DCI_QOS_BEHAVIOR_SUPERVISE *pstCb,
    NBB_ULONG *meter_id NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    METER_CFG_T meter = {0};
    NBB_BYTE MeterFlag = 0;
    NBB_USHORT unit = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG rv = ATG_DCI_RC_OK;
#endif

    //NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    /* 第一次申请meter,若已经申请了meter_id,此次仅更新流量监管的配置则不需要重复申请 */
    if(0 == *meter_id)
    {
        MeterFlag = 1;
        ret = spm_qos_apply_meter_id(meter_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_apply_meter_error_log(meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }
    meter.cir = pstCb->cir;
    meter.pireir = pstCb->pir;
    meter.cbs = pstCb->cbs * 1024;
    meter.pbsebs = pstCb->pbs * 1024;
    meter.meterId = *meter_id;
    meter.dropRed = 1;

#if 0
    RFC_2697,          /* RFC_2697*/
    RFC_2698,          /* RFC_2698*/
    RFC_4115,          /* RFC_4115*/
    MEF                   /* MEF*/
#endif

    if(3 == pstCb->mode)
    {
         meter.eMeterType = RFC_2697;
    }
    else if(2 == pstCb->mode)
    {
        meter.eMeterType = RFC_2698;
    }
    else
    {
        meter.eMeterType = MEF;
    }
    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3SetMeter(unit,&meter);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_set_meter_error_log(unit,&meter,__FUNCTION__,__LINE__,ret NBB_CCXT);
            if(1 == MeterFlag)
            {
                rv = spm_qos_free_meter_id(meter_id NBB_CCXT);
                if(ATG_DCI_RC_OK != rv)
                {
                    spm_qos_free_meter_error_log(meter_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
                }
            }
            goto EXIT_LABEL;
        }
#endif
    }
            
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;           
}

#ifdef PTN690

/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_verify_policy_supervise_mem(SPM_QOS_ACTION_CB *pcb NBB_CCXT_T NBB_CXT)
{
    SPM_QOS_POLICY_SUPERVISE_CB *cfg_cb = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);
    
    if(NULL != pcb)
    {
        for(cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_FIRST(pcb->spm_qos_supervise_tree); cfg_cb != NULL;
            cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_NEXT(pcb->spm_qos_supervise_tree,cfg_cb->policy_supervise_node))
        {
            NBB_TRC_FLOW((NBB_FORMAT "Verify policy supervise_cfg_cb cb %p", cfg_cb));
            NBB_VERIFY_MEMORY(cfg_cb, sizeof(SPM_QOS_POLICY_SUPERVISE_CB),
                              MEM_SPM_ACTION_POLICY_SUPERVISE_CB);
        }
    }

    NBB_TRC_EXIT();
    return;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_policy_supervise_drive(
    SPM_QOS_ACTION_CB *pcb, ATG_DCI_QOS_BEHAVIOR_SUPERVISE *pcfg NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_SUPERVISE_CB *cfg_cb = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);
    
    if((NULL != pcb) && (NULL != pcfg))
    {
       /************************************************************************************ 
        * 1.此时在更新流量监管子模块的内容之前,需要更新流量监管树(spm_qos_supervise_tree)  *
        *   节点中存放的behavior模板对应的meter桶的id号(meter_bucket_id)所对应的桶的信息.  *
        * 2.桶的信息有: meterid(桶id)、cir、pir、cbs、pbs、colorBlind、dropRed、eMeterType.*
        * 3.流量监管树节点(cfg_cb)中存放的信息有:                                          *
        *   (1)policy模块的id号,用于说明behavior模块被哪些policy模块所引用;                *
        *   (2)classify模块的id号,用于说明behavior模块与哪些classify模块一起组合成cb对;    *
        *   (3)acl模块的id号,用于说明引用behavior模块的policy被绑定时分配的aclid;          *
        ************************************************************************************/
        for(cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_FIRST(pcb->spm_qos_supervise_tree); cfg_cb != NULL;
            cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_NEXT(pcb->spm_qos_supervise_tree,cfg_cb->policy_supervise_node))
        {
            if(0 != cfg_cb->meter_bucket_id)
            {
                ret = spm_qos_set_bucket(cfg_cb->meter_bucket_id,pcfg NBB_CCXT);
                if(ATG_DCI_RC_OK != ret)
                {
                    goto EXIT_LABEL;
                }
            }
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_supervise_cfg(NBB_ULONG oper, SPM_QOS_ACTION_CB *pcb, 
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_SUPERVISE *ptemp = NULL;
    
    //NBB_USHORT unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (NULL == pcb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if(NULL != AVLL_FIRST(pcb->spm_qos_supervise_tree))
        {
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            goto EXIT_LABEL;
        }
        
        //spm_del_policy_supervise(pcb NBB_CCXT);
        if (NULL != pcb->supervise_cfg_cb)
        {   
        
            /*释放内存*/
            NBB_MM_FREE(pcb->supervise_cfg_cb,MEM_SPM_ACTION_SUPERVISE_CB);
            pcb->supervise_cfg_cb = NULL;
        }
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_SUPERVISE *)buf;

            /*添加操作*/
            if (NULL == pcb->supervise_cfg_cb)
            {
                pcb->supervise_cfg_cb = (ATG_DCI_QOS_BEHAVIOR_SUPERVISE *)NBB_MM_ALLOC( \
                                         sizeof(ATG_DCI_QOS_BEHAVIOR_SUPERVISE),
                                         NBB_NORETRY_ACT, MEM_SPM_ACTION_SUPERVISE_CB);
                if (NULL == pcb->supervise_cfg_cb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            
            /* 更新操作:若此时policy已被绑定,则更新policy中meter桶的信息 */
            else
            {
                ret = spm_set_policy_supervise_drive(pcb,ptemp NBB_CCXT);
                if(ATG_DCI_RC_OK != ret)
                {
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(pcb->supervise_cfg_cb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_SUPERVISE));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#else


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_supervise_cfg(NBB_ULONG oper, SPM_QOS_ACTION_CB *pcb, 
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_SUPERVISE *ptemp = NULL;
    NBB_USHORT unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (NULL == pcb)
    {
        printf("BUF is NULL.");
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:

        if (NULL == pcb->supervise_cfg_cb)
        {
            printf("supervise_cfg_cb is NULL.");
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        if(0 != pcb->supervise_cfg_cb->mode)
        {
            for(unit = 0;unit < SHARED.c3_num;unit++)
            {
#if defined (SPU) || defined (PTN690_CES)

                /* meter是配置流量监管过程中产生的 */
                
                //coverity[dead_error_condition]
                ret = ApiC3DelMeter(unit,pcb->meter_id);
                if(ATG_DCI_RC_OK != ret)
                {
                    spm_api_c3_del_meter_error_log(unit,pcb->meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
                    goto EXIT_LABEL;
                }
#endif
            }
            ret = spm_qos_free_meter_id(&(pcb->meter_id) NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_free_meter_error_log(&(pcb->meter_id),__FUNCTION__,__LINE__,ret NBB_CCXT);
            }
        }
        NBB_MM_FREE(pcb->supervise_cfg_cb,MEM_SPM_ACTION_SUPERVISE_CB);
        pcb->supervise_cfg_cb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_SUPERVISE *)buf;
            if(0 != ptemp->mode)
            {
                ret = spm_set_action_meter(ptemp,&(pcb->meter_id) NBB_CCXT);
                if(ATG_DCI_RC_OK != ret)
                {
                    goto EXIT_LABEL;
                }
            }

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == pcb->supervise_cfg_cb)
            {
                pcb->supervise_cfg_cb = (ATG_DCI_QOS_BEHAVIOR_SUPERVISE *)NBB_MM_ALLOC(  \
                                         sizeof(ATG_DCI_QOS_BEHAVIOR_SUPERVISE),
                                         NBB_NORETRY_ACT, MEM_SPM_ACTION_SUPERVISE_CB);
                if (NULL == pcb->supervise_cfg_cb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(pcb->supervise_cfg_cb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_SUPERVISE));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}




#endif


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_schedule_cfg(NBB_ULONG oper, 
    ATG_DCI_QOS_BEHAVIOR_SCHEDULE **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_SCHEDULE *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_SCHEDULE_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_SCHEDULE *)buf;
            if ((0 == ptemp->mode) || (0 == ptemp->weight)) /* 只支持WFQ模式 */
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_SCHEDULE *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_SCHEDULE),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_SCHEDULE_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_SCHEDULE));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_flow_act_cfg(NBB_ULONG oper, 
    ATG_DCI_QOS_BEHAVIOR_FLOW_ACT **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_FLOW_ACT *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_FLOW_ACT_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_FLOW_ACT *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_FLOW_ACT *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_FLOW_ACT),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_FLOW_ACT_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_FLOW_ACT));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_flow_samp_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_FLOW_SAMPL_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_FLOW_SAMPL_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_FLOW_SAMPL));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_redirect_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_REDIRECT **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_REDIRECT *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_REDIRECT_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_REDIRECT *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_REDIRECT *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_REDIRECT),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_REDIRECT_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_REDIRECT));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_policy_nhi_cfg(NBB_ULONG oper, 
    ATG_DCI_QOS_BEHAVIOR_POLICY_RT **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_POLICY_RT *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:
        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_POLCY_RT_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_POLICY_RT *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_POLICY_RT *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_POLICY_RT),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_POLCY_RT_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_POLICY_RT));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_phb_cfg(NBB_ULONG oper, ATG_DCI_QOS_BEHAVIOR_PHB **pcb,
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_PHB *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:

        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_PHB_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_PHB *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_PHB *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_PHB),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_PHB_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_PHB));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_domain_cfg(NBB_ULONG oper, 
    ATG_DCI_QOS_BEHAVIOR_DOMAIN **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_DOMAIN *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:

        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_DOMAIN_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_DOMAIN *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_DOMAIN *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_DOMAIN),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_DOMAIN_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_DOMAIN));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_urpf_cfg(NBB_ULONG oper, 
    ATG_DCI_QOS_BEHAVIOR_URPF **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_URPF *ptemp = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* ips消息操作类型 */
    switch (oper)
    {
        /* 更新 */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* 删除 */
        case  ATG_DCI_OPER_DEL:

        if (NULL == (*pcb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        NBB_MM_FREE(*pcb,MEM_SPM_ACTION_URPF_CB);
        *pcb = NULL;
        break;

        /* 增加 */
        case  ATG_DCI_OPER_ADD:

        /* ips消息内容为空 */
        if (NULL == buf)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 不为空 */
        else
        {
            ptemp = (ATG_DCI_QOS_BEHAVIOR_URPF *)buf;

            /***************************************************************************/
            /*                          存配置                                         */
            /***************************************************************************/
            if (NULL == *pcb)
            {
                *pcb = (ATG_DCI_QOS_BEHAVIOR_URPF *)NBB_MM_ALLOC(sizeof(ATG_DCI_QOS_BEHAVIOR_URPF),
                        NBB_NORETRY_ACT, MEM_SPM_ACTION_URPF_CB);
                if (NULL == *pcb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            OS_MEMCPY(*pcb, ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_URPF));
        }
        break;

        default:
        break;
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_que_shape_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_QUE_SHAPE pcb[ATG_DCI_QOS_BEHAVIOR_SHAPE_NUM],
    NBB_USHORT num,
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_QUE_SHAPE *ptemp = NULL;
    NBB_USHORT i = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (ATG_DCI_QOS_BEHAVIOR_SHAPE_NUM < num)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    for (i = 0; i < num; i++)
    {
        if(NULL == buf)
        {
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        ptemp = (ATG_DCI_QOS_BEHAVIOR_QUE_SHAPE *)
            (buf + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_SHAPE))) * i);

        /* ips消息操作类型 */
        switch (oper)
        {
            case  ATG_DCI_OPER_ADD:
            break;

            case  ATG_DCI_OPER_DEL:
            break;

            case  ATG_DCI_OPER_UPDATE:
            
            //coverity[dead_error_condition]
            if (NULL == buf)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }
            else
            {
                if ((ptemp->cir) > (ptemp->pir))
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
                ptemp->pir -= ptemp->cir;

                /***************************************************************************/
                /*                          存配置                                         */
                /***************************************************************************/
                OS_MEMCPY(&pcb[i], ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_SHAPE));
                ptemp->pir += ptemp->cir;
            }
            break;

            default:
            break;
        }
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_que_schedul_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_QUE_SCHEDL pcb[ATG_DCI_QOS_BEHAVIOR_SCHEDL_NUM],
    NBB_USHORT num,
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_QUE_SCHEDL *ptemp = NULL;
    NBB_USHORT i = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (ATG_DCI_QOS_BEHAVIOR_SCHEDL_NUM < num)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    for (i = 0; i < num; i++)
    {
        if(NULL == buf)
        {
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        ptemp = (ATG_DCI_QOS_BEHAVIOR_QUE_SCHEDL *)
            (buf + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_SCHEDL))) * i);

        /* ips消息操作类型 */
        switch (oper)
        {
            /* 更新 */
            case  ATG_DCI_OPER_ADD:
            break;

            /* 删除 */
            case  ATG_DCI_OPER_DEL:
            break;

            /* 增加 */
            case  ATG_DCI_OPER_UPDATE:

            /* ips消息内容为空 */
            //coverity[dead_error_condition]
            if (NULL == buf)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }

            /* 不为空 */
            else
            {
                /***************************************************************************/
                /*                          存配置                                         */
                /***************************************************************************/
                OS_MEMCPY(&pcb[i], ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_SCHEDL));
            }
            break;

            default:
            break;
        }
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_action_que_congst_cfg(NBB_ULONG oper,
    ATG_DCI_QOS_BEHAVIOR_QUE_CONGST pcb[ATG_DCI_QOS_BEHAVIOR_CONGST_NUM],
    NBB_USHORT num,
    NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    ATG_DCI_QOS_BEHAVIOR_QUE_CONGST *ptemp = NULL;
    NBB_USHORT i = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if (ATG_DCI_QOS_BEHAVIOR_CONGST_NUM < num)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    for (i = 0; i < num; i++)
    {
        if(NULL == buf)
        {
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        ptemp = (ATG_DCI_QOS_BEHAVIOR_QUE_CONGST *)
            (buf + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_CONGST))) * i);

        /* ips消息操作类型 */
        switch (oper)
        {
            /* 更新 */
            case  ATG_DCI_OPER_ADD:
            break;

            /* 删除 */
            case  ATG_DCI_OPER_DEL:
            break;

            /* 增加 */
            case  ATG_DCI_OPER_UPDATE:

            /* ips消息内容为空 */
            //coverity[dead_error_condition]
            if (NULL == buf)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }

            /* 不为空 */
            else
            {

                /***************************************************************************/
                /*                          存配置                                         */
                /***************************************************************************/
                OS_MEMCPY(&pcb[i], ptemp, sizeof(ATG_DCI_QOS_BEHAVIOR_QUE_CONGST));
            }
            break;

            default:
            break;
        }
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_del_action_driver(SPM_QOS_ACTION_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
#ifndef PTN690
    NBB_USHORT unit = 0;
#endif
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
#ifdef PTN690
    spm_del_policy_supervise_drive(pstTbl NBB_CCXT);
#else

    /*************删除驱动配置不检查错误****************/
    if(0 != pstTbl->meter_id)
    {
    
        /* meter是在配置流量监管的过程中产生的,用于给复杂流分类限速;流量整型用于给hqos限速 */
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)

            //coverity[dead_error_condition]
            ret = ApiC3DelMeter(unit,pstTbl->meter_id);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_api_c3_del_meter_error_log(unit,pstTbl->meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
                goto EXIT_LABEL;
            }
#endif
        }
        ret = spm_qos_free_meter_id(&(pstTbl->meter_id) NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_free_meter_error_log(&(pstTbl->meter_id),__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }
#endif
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}



/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_rcv_dci_set_action(ATG_DCI_SET_QOS_BEHAVIOR *pstSetIps NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_BYTE ucIfExist = QOS_EXIST;
    NBB_ULONG ulkey = 0;
    SPM_QOS_ACTION_CB *pstCb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_LONG ips_ret = ATG_DCI_RC_OK;
    NBB_USHORT shape_num = 0;
    NBB_USHORT schedule_num = 0;
    NBB_USHORT congest_num = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    /* IPS消息偏移的首地址 */
    NBB_BYTE *pucShapingDataStart = NULL;
    NBB_BYTE *pucSuperviseDataStart = NULL;
    NBB_BYTE *pucScheduleDataStart = NULL;
    NBB_BYTE *pucFlow_actDataStart = NULL;
    NBB_BYTE *pucFlow_sampingDataStart = NULL;
    NBB_BYTE *pucRedirectDataStart = NULL;
    NBB_BYTE *pucPolicy_nhiDataStart = NULL;
    NBB_BYTE *pucPhbDataStart = NULL;
    NBB_BYTE *pucDomainDataStart = NULL;
    NBB_BYTE *pucUrpfDataStart = NULL;
    NBB_BYTE *pucQue_shapingDataStart = NULL;
    NBB_BYTE *pucQue_schedulDataStart = NULL;
    NBB_BYTE *pucQue_congstDataStart = NULL;
    NBB_ULONG ulOperShaping = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperSupervise = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperSchedule = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperFlow_act = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperFlow_samping = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperRedirect = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperPolicy_nhi = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperPhb = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperDomain = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperUrpf = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperQue_shaping = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperQue_schedul = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperQue_congst = ATG_DCI_OPER_NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstSetIps)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_rcv_dci_set_action err : ATG_DCI_SET_QOS_BEHAVIOR "
                  "*pstSetIps is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_rcv_dci_set_action err : ATG_DCI_SET_QOS_BEHAVIOR "
                   "*pstSetIps is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }
    pstSetIps->return_code = ATG_DCI_RC_OK;
    ulkey = pstSetIps->key;
    pstCb = AVLL_FIND(v_spm_shared->qos_action_tree, &ulkey);

    /* 如果条目不存在 */
    if (pstCb == NULL)
    {
        ucIfExist = QOS_UNEXIST;
    }
    shape_num = pstSetIps->queue_shape_num;        /* 队列整型 */
    schedule_num = pstSetIps->queue_schedule_num;  /* 队列调度 */
    congest_num = pstSetIps->queue_congestion_num; /* 队列拥塞 */

    /* 获取子配置操作 */
    ulOperShaping = pstSetIps->oper_shapping;
    ulOperSupervise = pstSetIps->oper_supervise;
    ulOperSchedule = pstSetIps->oper_schedule;
    ulOperFlow_act = pstSetIps->oper_flow_action;
    ulOperFlow_samping = pstSetIps->oper_flow_sampling;
    ulOperRedirect = pstSetIps->oper_redirect;
    ulOperPolicy_nhi = pstSetIps->oper_policy_route;
    ulOperPhb = pstSetIps->oper_phb;
    ulOperDomain = pstSetIps->oper_domain;
    ulOperUrpf = pstSetIps->oper_urpf;
    ulOperQue_shaping = pstSetIps->oper_queue_shape;
    ulOperQue_schedul = pstSetIps->oper_queue_schedule;
    ulOperQue_congst = pstSetIps->oper_queue_congestion;

    /* 获取子配置的偏移地址 */
    pucShapingDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->shapping_data);
    pucSuperviseDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->supervise_data);
    pucScheduleDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->schedule_data);
    pucFlow_actDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->flow_action_data);
    pucFlow_sampingDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->flow_sampling_data);
    pucRedirectDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->redirect_data);
    pucPolicy_nhiDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->policy_route_data);
    pucPhbDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->phb_data);
    pucDomainDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->domain_data);
    pucUrpfDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->urpf_data);
    pucQue_shapingDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->queue_shape_data);
    pucQue_schedulDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->queue_schedule_data);
    pucQue_congstDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->queue_congestion_data);

    /* 消息类型为删除 */
    if (TRUE == pstSetIps->delete_struct)
    {
        /* 如果条目不存在,不删除 */
        if (ucIfExist == QOS_UNEXIST)
        {
            pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
            printf("%s line = %d spm_rcv_dci_set_action del err : no action "
                      "cfg. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__, 
                      pstSetIps->return_code, ulkey);
            OS_SPRINTF(ucMessage,"%s line = %d spm_rcv_dci_set_action del err : no action "
                       "cfg. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__, 
                       pstSetIps->return_code, ulkey);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        			       QOS_ERROR_STRING,ret,
        			       ucMessage,__FUNCTION__,"LINE","",
        			       0,0,__LINE__,0)); 
            goto EXIT_LABEL;
        }

        /* 存在,删除 */
        else
        {
            if(NULL == pstCb)
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                goto EXIT_LABEL;
            }

            /* 引用计数不为0 */
            if (0 != pstCb->cnt)
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                printf("%s line = %d  spm_rcv_dci_set_action del err : action is "
                          "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__,
                          pstSetIps->return_code, ulkey);
                OS_SPRINTF(ucMessage,"%s line = %d  spm_rcv_dci_set_action del err : action is "
                           "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__,
                           pstSetIps->return_code, ulkey);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            			       QOS_ERROR_STRING,ret,
            			       ucMessage,__FUNCTION__,"LINE","",
            			       0,0,__LINE__,0)); 
                goto EXIT_LABEL;
            }
#ifdef PTN690

            /* 流量监管的树为空,表示该behavior模板所对应的policy模板未被实例化 */
            if(NULL != AVLL_FIRST(pstCb->spm_qos_supervise_tree))
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                printf("%s line = %d  spm_rcv_dci_set_action del err : action is "
                          "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__,
                          pstSetIps->return_code, ulkey);
                OS_SPRINTF(ucMessage,"%s line = %d  spm_rcv_dci_set_action del err : action is "
                           "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, __LINE__,
                           pstSetIps->return_code, ulkey);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            			       QOS_ERROR_STRING,ret,
            			       ucMessage,__FUNCTION__,"LINE","",
            			       0,0,__LINE__,0)); 
                goto EXIT_LABEL;
            }
#endif

           /****************************************************************
            * 删除流量监管时的meter,删除驱动信息不判断返回值;BEHAVIOR中    *
            * 流量监管用来给复杂流分类限速；流量整型用来给hqos中tunnel限速 * 
            ****************************************************************/
            spm_del_action_driver(pstCb NBB_CCXT);
            AVLL_DELETE(SHARED.qos_action_tree, pstCb->spm_action_node);
            ret = spm_free_action_cb(pstCb NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }
        }
    }

    /* 增加或者更新 */
    else
    {
        /* 如果条目不存在,新申请内存空间保存数据 */
        if (ucIfExist == QOS_UNEXIST)
        {
            /* 申请一个新条目的内存空间 */
            pstCb = spm_alloc_action_cb(ulkey NBB_CCXT);
            if (NULL == pstCb)
            {
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }
        }

        /* 被引用 */
        if ((NULL != pstCb) && (0 != pstCb->cnt))
        {
            pstSetIps->return_code = ATG_DCI_RC_ADD_FAILED;
            printf("%s line = %d  spm_rcv_dci_set_action err : action is "
                   "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, 
                   __LINE__, pstSetIps->return_code, ulkey);
            OS_SPRINTF(ucMessage,"%s line = %d  spm_rcv_dci_set_action err : action is "
                       "being used. ret = %ld, behavior_id = %lu\n\n", __FUNCTION__, 
                       __LINE__, pstSetIps->return_code, ulkey);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        			       QOS_ERROR_STRING,ret,
        			       ucMessage,__FUNCTION__,"LINE","",
        			       0,0,__LINE__,0)); 
            goto EXIT_LABEL;
        }

        if(NULL == pstCb)
        {
            goto EXIT_LABEL;
        }

        /***************************************************************************/
        /*                             流量整形                                    */
        /***************************************************************************/
        ret = spm_set_action_shaping_cfg(ulOperShaping,&(pstCb->shaping_cfg_cb),pucShapingDataStart NBB_CCXT);
        pstSetIps->shapping_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                             流量监管                                    */
        /***************************************************************************/
        ret = spm_set_action_supervise_cfg(ulOperSupervise,pstCb,pucSuperviseDataStart NBB_CCXT);
        pstSetIps->supervise_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                             调度策略                                    */
        /***************************************************************************/
        ret = spm_set_action_schedule_cfg(ulOperSchedule, &(pstCb->schedule_cfg_cb) ,pucScheduleDataStart NBB_CCXT);
        pstSetIps->schedule_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                               流动作                                    */
        /***************************************************************************/
        ret = spm_set_action_flow_act_cfg(ulOperFlow_act,&(pstCb->flow_act_cfg_cb),pucFlow_actDataStart NBB_CCXT);
        pstSetIps->flow_action_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                              流采样                                     */
        /***************************************************************************/
        ret = spm_set_action_flow_samp_cfg(ulOperFlow_samping,      \
               &(pstCb->flow_samping_cfg_cb),pucFlow_sampingDataStart NBB_CCXT);
        pstSetIps->flow_sampling_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                             重定向                                      */
        /***************************************************************************/
        ret = spm_set_action_redirect_cfg(ulOperRedirect,   \
                &(pstCb->redirect_cfg_cb),pucRedirectDataStart NBB_CCXT);
        pstSetIps->redirect_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                             策略路由                                    */
        /***************************************************************************/
        ret = spm_set_action_policy_nhi_cfg(ulOperPolicy_nhi,   \
                &(pstCb->policy_nhi_cfg_cb),pucPolicy_nhiDataStart NBB_CCXT);
        pstSetIps->policy_route_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                                  PHB                                    */
        /***************************************************************************/
        ret = spm_set_action_phb_cfg(ulOperPhb,     \
                &(pstCb->phb_cfg_cb),pucPhbDataStart NBB_CCXT);
        pstSetIps->phb_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                                      域                                 */
        /***************************************************************************/
        ret = spm_set_action_domain_cfg(ulOperDomain,       \
                &(pstCb->domain_cfg_cb),pucDomainDataStart NBB_CCXT);
        pstSetIps->domain_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                                  urpf                                   */
        /***************************************************************************/
        ret = spm_set_action_urpf_cfg(ulOperUrpf,       \
                &(pstCb->urpf_cfg_cb),pucUrpfDataStart NBB_CCXT);
        pstSetIps->urpf_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }

        /***************************************************************************/
        /*                                  队列整形                               */
        /***************************************************************************/
        ret = spm_set_action_que_shape_cfg(ulOperQue_shaping,       \
                (pstCb->que_shaping_cfg_cb),shape_num,pucQue_shapingDataStart NBB_CCXT);
        pstSetIps->queue_shape_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }
        else if(ATG_DCI_OPER_UPDATE == ulOperQue_shaping)
        {
            pstCb->que_shape_num = shape_num;
        }

        /***************************************************************************/
        /*                                  队列调度                               */
        /***************************************************************************/
        ret = spm_set_action_que_schedul_cfg(ulOperQue_schedul,     \
                (pstCb->que_schedul_cfg_cb),schedule_num,pucQue_schedulDataStart NBB_CCXT);
        pstSetIps->queue_schedule_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }
        else if(ATG_DCI_OPER_UPDATE == ulOperQue_schedul)
        {
            pstCb->que_schedul_num = schedule_num;
        }

        /***************************************************************************/
        /*                                  队列拥塞                               */
        /***************************************************************************/
        ret = spm_set_action_que_congst_cfg(ulOperQue_congst,       \
                (pstCb->que_congst_cfg_cb),congest_num, pucQue_congstDataStart NBB_CCXT);
        pstSetIps->queue_congestion_return_code = ret;
        if (ATG_DCI_RC_OK != ret)
        {
            ips_ret = ret;
        }
        else if(ATG_DCI_OPER_UPDATE == ulOperQue_congst)
        {
           pstCb->que_congest_num = congest_num; 
        }

        pstSetIps->return_code = ips_ret;

        /* 如果是新增的条目,插入到树中 */
        if(QOS_UNEXIST == ucIfExist)
        {
        
            //coverity[no_effect_test]
            rv = AVLL_INSERT(SHARED.qos_action_tree, pstCb->spm_action_node);
        }
    }

    /* 异常跳出 */
    EXIT_LABEL: NBB_TRC_EXIT();
    return;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_qos_clear_all_action(NBB_CXT_T NBB_CXT)
{
    SPM_QOS_ACTION_CB *cfg_cb = NULL;


    for (cfg_cb = (SPM_QOS_ACTION_CB*) AVLL_FIRST(v_spm_shared->qos_action_tree); cfg_cb != NULL;
         cfg_cb = (SPM_QOS_ACTION_CB*) AVLL_FIRST(v_spm_shared->qos_action_tree))
     {
        AVLL_DELETE(v_spm_shared->qos_action_tree, cfg_cb->spm_action_node);
        spm_free_action_cb(cfg_cb NBB_CCXT);   
     }


    return;   
}

#endif

#ifdef PTN690

#if 3

/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_set_txlsp_color_action(ATG_DCI_CR_LSP_TX_CAR_DATA *txlsp_car,
    NBB_ULONG *color_id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG rv = ATG_DCI_RC_OK;
#endif
    NBB_LONG ColorFlag = 0;
    POLICER_ACTION_TEMPLATE_T color_action = {0};
    NBB_USHORT unit = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == color_id)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }  

    /* 申请色感知动作模板的模板id,目前最多只能申请4000个色感知动作模板 */
    if(0 == *color_id)
    {
        /* 1表示color_id是由本次申请,当写驱动失败后需要释放该id资源 */
        ColorFlag = 1;
        ret = spm_qos_apply_color_action_id(color_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_apply_meter_error_log(color_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }

    NBB_MEMSET(&color_action, 0, sizeof(POLICER_ACTION_TEMPLATE_T));

    color_action.policer_action_id = *color_id;

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == txlsp_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 1;
        color_action.gre_pri = txlsp_car->green_action.markphb;
    }
    else if(1 == txlsp_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 0;
        color_action.gre_drop_ena    = 1;
    }
    else if(0 == txlsp_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 0;
        color_action.gre_drop_ena    = 0;
    }

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == txlsp_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 1;
        color_action.yel_pri = txlsp_car->yellow_action.markphb;
    }
    else if(1 == txlsp_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 0;
        color_action.yel_drop_ena    = 1;
    }
    else if(0 == txlsp_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 0;
        color_action.yel_drop_ena    = 0;
    }

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == txlsp_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 1;
        color_action.red_pri = txlsp_car->red_action.markphb;
    }
    else if(1 == txlsp_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 0;
        color_action.red_drop_ena    = 1;
    }
    else if(0 == txlsp_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 0;
        color_action.red_drop_ena    = 0;
    }

    /* 将色感知动作模板写入C3驱动中 */
    for(unit = 0; unit < v_spm_shared->c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)
        ret = fhdrv_qos_set_policer_action_template(unit,&color_action);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_set_policer_action_template_error_log(unit,&color_action,
                                     __FUNCTION__,__LINE__,ret NBB_CCXT);

            /* 表示color_id是此时申请的,写驱动失败时需要释放该id资源. */
            if(1 == ColorFlag)
            {
                rv = spm_qos_free_color_action_id(color_id NBB_CCXT);
                if(ATG_DCI_RC_OK != rv)
                {
                    spm_qos_free_meter_error_log(color_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
                }
            }
            goto EXIT_LABEL;
        }
#endif  
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_TXLSP_CAR_CB *spm_qos_alloc_txlsp_car_cb(CRTXLSP_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_TXLSP_CAR_CB *pstTbl = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pkey);

    if (NULL == pkey)
    {
        printf("**QOS ERROR**%s,%d,key==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_qos_alloc_txlsp_car_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 分配一个新的逻辑配置块qos表配置条目 */
    pstTbl = (SPM_QOS_TXLSP_CAR_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_TXLSP_CAR_CB),
              NBB_NORETRY_ACT, MEM_SPM_QOS_TXLSP_CAR_CB);
    if (NULL == pstTbl)
    {
        goto EXIT_LABEL;
    }

    /* 初始逻辑配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_TXLSP_CAR_CB));
    OS_MEMCPY(&(pstTbl->txlsp_key), pkey, sizeof(CRTXLSP_KEY));
    AVLL_INIT_NODE(pstTbl->spm_qos_txlsp_car_node);

    EXIT_LABEL: NBB_TRC_EXIT();
    return pstTbl;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_free_txlsp_car_cb(SPM_QOS_TXLSP_CAR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        printf("**QOS ERROR**%s,%d,pstTbl==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_qos_free_txlsp_car_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_TXLSP_CAR_CB), MEM_SPM_QOS_TXLSP_CAR_CB);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_QOS_TXLSP_CAR_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_set_txlsp_meter(ATG_DCI_CR_LSP_TX_CAR_DATA *pstCb,
    NBB_ULONG *meter_id NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    METER_CFG_T meter  = {0};
    NBB_USHORT unit = 0;
    NBB_LONG MeterFlag = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG rv = ATG_DCI_RC_OK;
#endif
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == meter_id) || (NULL == pstCb) || (0 == pstCb->mode))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;  
        printf("%s line=%d spm_qos_set_txlsp_meter err : pstCb/meterid=null or"
               "meter_mode=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_qos_set_txlsp_meter err : pstCb/meterid=null or"
               "meter_mode=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }

    if((0 == *meter_id))
    {
        /* MeterFlag赋值为1表示meterid是此时申请的,在meter写入驱动失败时需要释放该id资源. */
        MeterFlag = 1;
        ret = spm_qos_apply_meter_id(meter_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_apply_meter_error_log(meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }

    meter.meterId = *meter_id;
    meter.cir     = pstCb->cir;
    meter.pireir  = pstCb->pir;
    meter.cbs     = pstCb->cbs * 1024;
    meter.pbsebs  = pstCb->pbs * 1024;
    meter.dropRed = 1;

    if(3 == pstCb->mode)
    {
         meter.eMeterType = RFC_2697;
    }
    else if(2 == pstCb->mode)
    {
        meter.eMeterType = RFC_2698;
    }
    else
    {
        meter.eMeterType = MEF;
    }

    /*色盲模式*/
    if(0 == pstCb->cm_color)
    {
         meter.colorBlind = 0;
    }
    else
    {
         meter.colorBlind = 1;
    }

    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3SetMeter(unit,&meter);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_set_meter_error_log(unit,&meter,__FUNCTION__,__LINE__,ret NBB_CCXT);

            /* 表示meterid是此时申请的,meter写入驱动失败,此时需要释放该id资源. */
            if(1 == MeterFlag)
            {
                rv = spm_qos_free_meter_id(meter_id NBB_CCXT);
                if(ATG_DCI_RC_OK != rv)
                {
                    spm_qos_free_meter_error_log(meter_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
                }
            }
            goto EXIT_LABEL;
        }
#endif
    }
            
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;           
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_add_txlsp_car(NBB_LONG txlsp_posid,NBB_ULONG meter_id,
     NBB_ULONG color_id  NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
  
    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == txlsp_posid) || (0 == meter_id) || (0 == color_id))
    {
        printf("**QOS ERROR**%s,%d,CFG==0\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_qos_add_txlsp_car",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)           
        ret = fhdrv_l3_set_txlsp_meter_policer_action(unit,txlsp_posid,1,meter_id,1,color_id);
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR**%s,%d,fhdrv_l3_set_txlsp_meter_policer_action\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    				       "QOS add txlsp car",ret,"txlsp_posid","meter id","color_action_id","",
    				       txlsp_posid,meter_id,color_id,0));
            goto EXIT_LABEL;
        }
#endif
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_del_txlsp_car(NBB_LONG txlsp_posid, NBB_ULONG meter_id, 
        NBB_ULONG color_actionid, NBB_LONG flag  NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == meter_id) || (0 == txlsp_posid))
    {
        printf("**QOS ERROR**%s,%d,CFG==0\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_qos_del_txlsp_car",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /*disable txlsp car function*/
    if(ATG_DCI_RC_OK != flag)
    {     
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)          
            ret = fhdrv_l3_set_txlsp_meter_policer_action(unit,txlsp_posid,0,0,0,0);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d,fhdrv_l3_set_txlsp_meter_policer_action\n",__FUNCTION__,__LINE__);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        				       "QOS del txlsp car",ret,"txlsp_posid","old meter id","old color id","",
        				       txlsp_posid,meter_id,color_actionid,0));
                goto EXIT_LABEL;
            }
#endif
        }
    }

    /* 删除驱动并释放color_action_id资源 */
    ret = spm_qos_del_color_action(&color_actionid NBB_CCXT );
    if(ATG_DCI_RC_OK != ret)
    {
        spm_qos_free_meter_error_log(&color_actionid,__FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }

    /* 删除驱动并释放meter_id资源 */
    ret = spm_qos_del_txlsp_pw_meter(&meter_id NBB_CCXT);
    if(ATG_DCI_RC_OK != ret)
    {
        spm_qos_free_meter_error_log(&meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_add_txlsp_meter_qos_node
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_txlsp_meter_qos_node(
     CRTXLSP_KEY *key, NBB_ULONG txlsp_posid, 
     ATG_DCI_CR_LSP_TX_CAR_DATA *txlsp_car NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_TXLSP_CAR_CB *join_node = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == txlsp_car) || (NULL == key))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_add_txlsp_meter_qos_node err :  QOS param is NULL. "
               "ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_add_txlsp_meter_qos_node err :  QOS param is NULL. "
                   "ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }

    if(ATG_DCI_RC_OK != qos_txlsp_cfg_print)
    {
        printf("%s %s,%d txlsp.ingress=0x%lx,txlsp.egress=0x%lx,txlsp.tunnelid=%lu,txlsp.lspid=%lu,\n"
               "txlsp_posid=%lu,meter.mode=%d,meter.cir=%ldMbps,meter.pir=%ldMbps,meter.cbs=%dKb,meter.pbs=%dKb.\n\n",
               QOS_CFG_STRING,__FUNCTION__,__LINE__,key->ingress,key->egress,key->tunnelid,key->lspid,
               txlsp_posid,txlsp_car->mode,(txlsp_car->cir/1000),(txlsp_car->pir/1000),txlsp_car->cbs,txlsp_car->pbs);
        OS_SPRINTF(ucMessage,"%s %s,%d txlsp.ingress=0x%lx,txlsp.egress=0x%lx,txlsp.tunnelid=%lu,txlsp.lspid=%lu,"
                   "txlsp_posid=%lu,meter.mode=%d,meter.cir=%ldMbps,meter.pir=%ldMbps,meter.cbs=%dKb,"
                   "meter.pbs=%dKb.\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,key->ingress,key->egress,
                   key->tunnelid,key->lspid,txlsp_posid,txlsp_car->mode,(txlsp_car->cir/1000),(txlsp_car->pir/1000),
                   txlsp_car->cbs,txlsp_car->pbs);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }

    /*lsp发生了振荡*/
    if(0 == txlsp_posid)
    {
        join_node = AVLL_FIND(SHARED.qos_txlsp_car_tree, key);
        if (NULL == join_node)
        {
            /* 未找到txlsp的car配置,直接退出 */
            ret = ATG_DCI_RC_OK;
            goto EXIT_LABEL;
        }
        ret = spm_qos_del_txlsp_car(join_node->txlsp_posid, join_node->meter_id, 
                            join_node->color_action_id, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        AVLL_DELETE(SHARED.qos_txlsp_car_tree, join_node->spm_qos_txlsp_car_node);
        ret = spm_qos_free_txlsp_car_cb(join_node NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        goto EXIT_LABEL;
    }
    
    join_node = AVLL_FIND(SHARED.qos_txlsp_car_tree, key);

    /* 该txlsp上没有绑定car功能 */
    if(NULL == join_node)
    {
        /* 未开启meter功能,配置出错,直接跳出 */
        if(0 == txlsp_car->mode)
        {
            printf("**QOS ERROR**%s,%d,txlsp car config missmatch.\n\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS txlsp car config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
    					   "","","","",0,0,0,0));
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* txlsp上绑定car功能 */
        else
        {
            join_node = spm_qos_alloc_txlsp_car_cb(key NBB_CCXT); 
            if (NULL == join_node)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }

            ret = spm_qos_set_txlsp_meter(txlsp_car,&(join_node->meter_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_free_txlsp_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            ret = spm_qos_set_txlsp_color_action(txlsp_car,&(join_node->color_action_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_del_txlsp_pw_meter(&(join_node->meter_id) NBB_CCXT);
                spm_qos_free_txlsp_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            ret = spm_qos_add_txlsp_car(txlsp_posid,join_node->meter_id,
                                   join_node->color_action_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_del_color_action(&(join_node->color_action_id) NBB_CCXT );
                spm_qos_del_txlsp_pw_meter(&(join_node->meter_id) NBB_CCXT);
                spm_qos_free_txlsp_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            join_node->txlsp_posid = txlsp_posid;
            OS_MEMCPY(&(join_node->txlsp_car),txlsp_car,sizeof(ATG_DCI_CR_LSP_TX_CAR_DATA));

            rv = AVLL_INSERT(SHARED.qos_txlsp_car_tree, join_node->spm_qos_txlsp_car_node);     
            goto EXIT_LABEL;
        }
    }

    /* 该txlsp上已经之前绑定了car的功能 */
    else
    {
        /* 删除该txlsp上绑定的car功能 */
        if(0 == txlsp_car->mode)
        {
            ret = spm_qos_del_txlsp_car(txlsp_posid,join_node->meter_id, 
                  join_node->color_action_id, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            AVLL_DELETE(SHARED.qos_txlsp_car_tree, join_node->spm_qos_txlsp_car_node);
            ret = spm_qos_free_txlsp_car_cb(join_node NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            goto EXIT_LABEL;  
        }

        /* 更新txlsp上绑定的car配置 */
        else
        {
            ret = spm_qos_set_txlsp_meter(txlsp_car,&(join_node->meter_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("%s line=%d spm_qos_set_txlsp_meter err : set new txlsp meter "
                       "failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                OS_SPRINTF(ucMessage,"%s line=%d spm_qos_set_txlsp_meter err : set new txlsp meter "
                       "failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }

            ret = spm_qos_set_txlsp_color_action(txlsp_car,&(join_node->color_action_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("%s line=%d spm_qos_set_txlsp_color_action err : set txlsp new color "
                       "action failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                OS_SPRINTF(ucMessage,"%s line=%d spm_qos_set_txlsp_color_action err : set txlsp new color "
                       "action failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
            OS_MEMCPY(&(join_node->txlsp_car),txlsp_car,sizeof(ATG_DCI_CR_LSP_TX_CAR_DATA));
            goto EXIT_LABEL;  
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_clear_all_txlsp_car(NBB_CXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_TXLSP_CAR_CB *cfg_cb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;

    for(cfg_cb = (SPM_QOS_TXLSP_CAR_CB*) AVLL_FIRST(v_spm_shared->qos_txlsp_car_tree); cfg_cb != NULL;
        cfg_cb = (SPM_QOS_TXLSP_CAR_CB*) AVLL_FIRST(v_spm_shared->qos_txlsp_car_tree))
    {
        AVLL_DELETE(v_spm_shared->qos_txlsp_car_tree, cfg_cb->spm_qos_txlsp_car_node);
        spm_qos_free_color_action_id(&(cfg_cb->color_action_id) NBB_CCXT);
        ret = spm_qos_free_meter_id(&(cfg_cb->meter_id) NBB_CCXT);
        spm_qos_free_txlsp_car_cb(cfg_cb NBB_CCXT);
    }

    return ret;
}

#endif


#endif


#if defined (CR8000_SMART) || defined (PTN690)
/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_set_pw_color_action(ATG_DCI_LOG_PORT_CAR_DATA *pw_car,
    NBB_ULONG *color_id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG rv = ATG_DCI_RC_OK;
#endif
    NBB_LONG ColorFlag = 0;
    POLICER_ACTION_TEMPLATE_T color_action = {0};
    NBB_USHORT unit = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == color_id) || (NULL == pw_car))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }  

    /* 申请色感知动作模板的模板id,目前最多只能申请4000个色感知动作模板 */
    if(0 == *color_id)
    {
        /* 1表示color_id是由本次申请,当写驱动失败后需要释放该id资源 */
        ColorFlag = 1;
        ret = spm_qos_apply_color_action_id(color_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_apply_meter_error_log(color_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }

    NBB_MEMSET(&color_action, 0, sizeof(POLICER_ACTION_TEMPLATE_T));

    color_action.policer_action_id = *color_id;

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == pw_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 1;
        color_action.gre_pri = pw_car->green_action.markphb;
    }
    else if(1 == pw_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 0;
        color_action.gre_drop_ena    = 1;
    }
    else if(0 == pw_car->green_action.action)
    {
        color_action.chg_gre_pri_ena = 0;
        color_action.gre_drop_ena    = 0;
    }

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == pw_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 1;
        color_action.yel_pri = pw_car->yellow_action.markphb;
    }
    else if(1 == pw_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 0;
        color_action.yel_drop_ena    = 1;
    }
    else if(0 == pw_car->yellow_action.action)
    {
        color_action.chg_yel_pri_ena = 0;
        color_action.yel_drop_ena    = 0;
    }

    /* action : 0 / 1 / 2 = pass / drop / markphb */
    if(2 == pw_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 1;
        color_action.red_pri = pw_car->red_action.markphb;
    }
    else if(1 == pw_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 0;
        color_action.red_drop_ena    = 1;
    }
    else if(0 == pw_car->red_action.action)
    {
        color_action.chg_red_pri_ena = 0;
        color_action.red_drop_ena    = 0;
    }

    /* 将色感知动作模板写入C3驱动中 */
    for(unit = 0; unit < v_spm_shared->c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)
        ret = fhdrv_qos_set_policer_action_template(unit,&color_action);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_set_policer_action_template_error_log(unit,&color_action,
                                     __FUNCTION__,__LINE__,ret NBB_CCXT);

            /* 表示color_id是此时申请的,写驱动失败时需要释放该id资源. */
            if(1 == ColorFlag)
            {
                rv = spm_qos_free_color_action_id(color_id NBB_CCXT);
                if(ATG_DCI_RC_OK != rv)
                {
                    spm_qos_free_meter_error_log(color_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
                }
            }
            goto EXIT_LABEL;
        }
#endif  
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_alloc_qos_pw_car_cb
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_PW_CAR_CB *spm_alloc_qos_pw_car_cb(NBB_ULONG log_key NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_PW_CAR_CB *pstTbl = NULL;
    
    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(0 != log_key);

    if (0 == log_key)
    {
        printf("**QOS ERROR**%s,%d,log_key==0\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_alloc_qos_pw_car_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 分配一个新的逻辑配置块qos表配置条目 */
    pstTbl = (SPM_QOS_PW_CAR_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_PW_CAR_CB),
              NBB_NORETRY_ACT, MEM_SPM_QOS_PW_CAR_CB);
    if (NULL == pstTbl)
    {
        goto EXIT_LABEL;
    }

    /* 初始逻辑配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_PW_CAR_CB));
    pstTbl->log_key = log_key;
    AVLL_INIT_NODE(pstTbl->spm_qos_pw_car_node);

    EXIT_LABEL: NBB_TRC_EXIT();
    return pstTbl;
}


/*****************************************************************************
   函 数 名  : spm_free_qos_pw_car_cb
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_qos_pw_car_cb(SPM_QOS_PW_CAR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        printf("**QOS ERROR**%s,%d,pstTbl==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_free_qos_pw_car_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_PW_CAR_CB), MEM_SPM_QOS_PW_CAR_CB);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_QOS_PW_CAR_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}




/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_del_txlsp_pw_meter(NBB_ULONG *meter_id NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_USHORT unit = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == meter_id) || (0 == *meter_id))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;  
        printf("%s line=%d spm_qos_del_txlsp_pw_meter err : meterid=null or "
               "*meterid!=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_qos_del_txlsp_pw_meter err : meterid=null or "
                   "*meterid!=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }

    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3DelMeter(unit,*meter_id);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_del_meter_error_log(unit,*meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
#endif
    }

    ret = spm_qos_free_meter_id(meter_id NBB_CCXT);
    if(ATG_DCI_RC_OK != ret)
    {
        spm_qos_free_meter_error_log(meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}



/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 释放内存和申请的bukect桶资源
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_set_qos_pw_meter(ATG_DCI_LOG_PORT_CAR_DATA *pstCb,
       NBB_ULONG *meter_id  NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    METER_CFG_T meter  = {0};
    NBB_USHORT unit = 0;
    NBB_USHORT MeterFlag = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG rv = ATG_DCI_RC_OK;
#endif
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == meter_id) || (NULL == pstCb) || (0 == pstCb->mode))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;  
        printf("%s line=%d spm_set_qos_pw_meter err : meter_id/pstCb=null "
               "or meter_mode=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_set_qos_pw_meter err : meter_id/pstCb=null "
               "or meter_mode=0. ret = %ld.\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }

    if (0 == *meter_id)
    {
        /* MeterFlag赋值为1表示meterid是此时申请的,在meter写入驱动失败时需要释放该id资源. */
        MeterFlag = 1;
        ret = spm_qos_apply_meter_id(meter_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_apply_meter_error_log(meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
    }

    meter.meterId = *meter_id;
    meter.cir     = pstCb->cir;
    meter.pireir  = pstCb->pir;
    meter.cbs     = pstCb->cbs * 1024;
    meter.pbsebs  = pstCb->pbs * 1024;
    meter.dropRed = 1;

    if(3 == pstCb->mode)
    {
         meter.eMeterType = RFC_2697;
    }
    else if(2 == pstCb->mode)
    {
        meter.eMeterType = RFC_2698;
    }
    else
    {
        meter.eMeterType = MEF;
    }

    /* 色盲模式 */
    if(0 == pstCb->cm_color)
    {
         meter.colorBlind = 0;
    }
    else
    {
         meter.colorBlind = 1;
    }

    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3SetMeter(unit,&meter);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_set_meter_error_log(unit,&meter,__FUNCTION__,__LINE__,ret NBB_CCXT);

            /* 表示meterid是此时申请的,meter写入驱动失败,此时需要释放该id资源. */
            if(1 == MeterFlag)
            {
                rv = spm_qos_free_meter_id(meter_id NBB_CCXT);
                if(ATG_DCI_RC_OK != rv)
                {
                    spm_qos_free_meter_error_log(meter_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
                }
            }
            goto EXIT_LABEL;
        }
#endif
    }
            
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;           
}


/*****************************************************************************
   函 数 名  : spm_add_pw_car_driver
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_pw_car_driver(NBB_ULONG log_id,NBB_ULONG meter_id,
       NBB_ULONG color_id NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;
    NBB_USHORT unit = 0;
  
    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == log_id) || (0 == meter_id) || (0 == color_id))
    {
        printf("**QOS ERROR**%s,%d,CFG param error\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_add_pw_car_driver",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    pstLogicalPort  = AVLL_FIND(SHARED.logical_port_tree, &log_id);
    if((NULL == pstLogicalPort) || (NULL == pstLogicalPort->phy_cfg_cb) || \
       (NULL == pstLogicalPort->basic_cfg_cb))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("**QOS ERROR**%s,%d,log port error.\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_add_pw_car_driver",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 该逻辑端口下所有的intf都需要绑定meter */
    for(pstTerminalIfCb = (SPM_TERMINAL_IF_CB*)AVLL_FIRST(pstLogicalPort->terminal_if_tree); 
        pstTerminalIfCb != NULL; pstTerminalIfCb = (SPM_TERMINAL_IF_CB*)AVLL_NEXT(\
        pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node))
    {
        /* intf上绑定meterid */
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)          
            ret = ApiC3SetIntfMeter(unit,pstTerminalIfCb->intf_pos_id,1,meter_id);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d,ApiC3SetIntfMeter\n",__FUNCTION__,__LINE__);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        				       "QOS add pw car",ret,"intf_posid","meter id","","",
        				       pstTerminalIfCb->intf_pos_id,meter_id,0,0));
                goto EXIT_LABEL;
            }
#endif
        } 

        /* intf上绑定color action id */
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
            ret = fhdrv_intf_set_policer_action(unit,pstTerminalIfCb->intf_pos_id,1,color_id);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d,fhdrv_intf_set_policer_action.\n",__FUNCTION__,__LINE__);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        				       "QOS add color action",ret,"intf_posid","color action id","","",
        				       pstTerminalIfCb->intf_pos_id,color_id,0,0));
                goto EXIT_LABEL;
            }
#endif
        } 
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_del_pw_car_driver
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_del_pw_car_driver(NBB_ULONG log_id, NBB_ULONG meter_id,
       NBB_ULONG color_id NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;
    NBB_USHORT unit = 0;
  
    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == log_id) || (0 == meter_id) || (0 == color_id))
    {
        printf("**QOS ERROR**%s,%d,CFG param error\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_del_pw_car_driver",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    pstLogicalPort  = AVLL_FIND(SHARED.logical_port_tree, &log_id);
    if((NULL == pstLogicalPort) || (NULL == pstLogicalPort->phy_cfg_cb) || \
       (NULL == pstLogicalPort->basic_cfg_cb))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("**QOS ERROR**%s,%d,log port error.\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_add_pw_car_driver",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 该逻辑端口下所有的intf都需要去绑定meter_id和color_action_id */
    for(pstTerminalIfCb = (SPM_TERMINAL_IF_CB*)AVLL_FIRST(pstLogicalPort->terminal_if_tree); 
        pstTerminalIfCb != NULL; pstTerminalIfCb = (SPM_TERMINAL_IF_CB*)AVLL_NEXT( \
        pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node))
    {
        /* intf上去绑定meter_id */
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)           
            ret = ApiC3SetIntfMeter(unit, pstTerminalIfCb->intf_pos_id, 0, 0);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d,ApiC3SetIntfMeter\n",__FUNCTION__,__LINE__);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        				       "QOS del pw meter",ret,"intf","meter id","disable","",
        				       pstTerminalIfCb->intf_pos_id,0,0,0));
                goto EXIT_LABEL;
            }
#endif
        }

        /* intf上去绑定color_action_id */
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)       
            ret = fhdrv_intf_set_policer_action(unit,pstTerminalIfCb->intf_pos_id,0,0);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("**QOS ERROR**%s,%d,fhdrv_intf_set_policer_action.\n",__FUNCTION__,__LINE__);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        				       "QOS del color action",ret,"intf","color id","disable","",
        				       pstTerminalIfCb->intf_pos_id,0,0,0));
                goto EXIT_LABEL;
            }
#endif
        } 
    }

    /* 删除驱动并释放color_action_id资源 */
    ret = spm_qos_del_color_action(&color_id NBB_CCXT );
    if(ATG_DCI_RC_OK != ret)
    {
        spm_qos_free_meter_error_log(&color_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }

    /* 删除驱动并释放meter_id资源 */
    ret = spm_qos_del_txlsp_pw_meter(&meter_id NBB_CCXT);
    if(ATG_DCI_RC_OK != ret)
    {
        spm_qos_free_meter_error_log(&meter_id,__FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }


    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_add_pw_meter_qos_node
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_pw_car_qos_node(NBB_ULONG log_id, ATG_DCI_VC_KEY *pw_key,
         SUB_PORT *sub_port,ATG_DCI_LOG_PORT_CAR_DATA *pw_car NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_PW_CAR_CB  *join_node = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((NULL == pw_car) || (NULL == pw_key) || \
        (0 == log_id) || (NULL == sub_port))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_add_pw_car_qos_node err :  QOS param is NULL. "
               "ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_add_pw_car_qos_node err :  QOS param is NULL. "
                   "ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 
        goto EXIT_LABEL;
    }

    if(ATG_DCI_RC_OK != qos_pw_cfg_print)
    {
        printf("%s %s,%d log_index=%lu,slot=%d,port=%d,vc_id=%lu,vc_type=%d,peer_ip=0x%lx,meter.mode=%d.\n"
               "meter.color=%d,meter.cir=%ldMbps,meter.pir=%ldMbps,meter.cbs=%dKb,meter.pbs=%dKb.\n\n",
               QOS_CFG_STRING,__FUNCTION__,__LINE__,log_id,sub_port->slot,sub_port->port,pw_key->vc_id,
               pw_key->vc_type,pw_key->peer_ip,pw_car->mode,pw_car->cm_color,(pw_car->cir/1000),
               (pw_car->pir/1000),pw_car->cbs,pw_car->pbs);
        OS_SPRINTF(ucMessage,"%s %s,%d log_index=%lu,slot=%d,port=%d,vc_id=%lu,vc_type=%d,peer_ip=0x%lx,"
               "meter.mode=%d.\nmeter.color=%d,meter.cir=%ldMbps,meter.pir=%ldMbps,meter.cbs=%dKb,"
               "meter.pbs=%dKb.\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,log_id,sub_port->slot,
               sub_port->port,pw_key->vc_id,pw_key->vc_type,pw_key->peer_ip,pw_car->mode,pw_car->cm_color,
               (pw_car->cir/1000),(pw_car->pir/1000),pw_car->cbs,pw_car->pbs);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }

    if((0 != sub_port->slot) && (sub_port->slot != v_spm_shared->local_slot_id))
    {
        goto EXIT_LABEL;
    }

    join_node = AVLL_FIND(SHARED.qos_pw_car_tree, &log_id);

    /* 该逻辑端口上没有绑定car功能 */
    if (NULL == join_node)
    {
        /* 未开启meter功能,配置出错,直接跳出 */
        if(0 == pw_car->mode)
        {
            printf("**QOS ERROR**%s,%d,pw car config missmatch.\n\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS pw car config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
    					   "","","","",0,0,0,0));
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /* 逻辑端口上绑定car功能 */
        else
        {
            join_node = spm_alloc_qos_pw_car_cb(log_id NBB_CCXT);
            if (NULL == join_node)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }

            ret = spm_set_qos_pw_meter(pw_car,&(join_node->meter_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_free_qos_pw_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            ret = spm_qos_set_pw_color_action(pw_car,&(join_node->color_action_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_del_txlsp_pw_meter(&(join_node->meter_id) NBB_CCXT);
                spm_free_qos_pw_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            ret = spm_add_pw_car_driver(log_id, join_node->meter_id, 
                               join_node->color_action_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_del_pw_car_driver(log_id, join_node->meter_id, 
                             join_node->color_action_id NBB_CCXT);
                spm_free_qos_pw_car_cb(join_node NBB_CCXT);
                goto EXIT_LABEL;
            }

            OS_MEMCPY(&(join_node->pw_car), pw_car, sizeof(ATG_DCI_LOG_PORT_CAR_DATA));
            OS_MEMCPY(&(join_node->vc_key), pw_key, sizeof(ATG_DCI_VC_KEY));
            rv = AVLL_INSERT(SHARED.qos_pw_car_tree, join_node->spm_qos_pw_car_node); 
            goto EXIT_LABEL;
        }
    }

    /* 该逻辑端口上之前已经绑定了car功能 */
    else
    {
        /* 删除操作,删除操作时不检查free/del函数的返回值,默认删除成功 */
        if(0 == pw_car->mode)
        {
            ret = spm_del_pw_car_driver(log_id,join_node->meter_id, 
                              join_node->color_action_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            AVLL_DELETE(SHARED.qos_pw_car_tree, join_node->spm_qos_pw_car_node);
            ret = spm_free_qos_pw_car_cb(join_node NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            goto EXIT_LABEL; 
        }

        /* 更新meter的配置内容 */
        else
        {
            ret = spm_set_qos_pw_meter(pw_car,&(join_node->meter_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("%s line=%d spm_set_qos_pw_meter err : set pw new meter "
                       "failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                OS_SPRINTF(ucMessage,"%s line=%d spm_set_qos_pw_meter err : set pw new meter "
                       "failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
            ret = spm_qos_set_pw_color_action(pw_car,&(join_node->color_action_id) NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                printf("%s line=%d spm_qos_set_pw_color_action err : set pw new color "
                       "action failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                OS_SPRINTF(ucMessage,"%s line=%d spm_qos_set_pw_color_action err : set pw new color "
                       "action failed. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
            OS_MEMCPY(&(join_node->pw_car),pw_car,sizeof(ATG_DCI_LOG_PORT_CAR_DATA));
            goto EXIT_LABEL; 
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_qos_clear_all_pw_car
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_qos_clear_all_pw_car(NBB_CXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_PW_CAR_CB *cfg_cb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;

    for(cfg_cb = (SPM_QOS_PW_CAR_CB*) AVLL_FIRST(v_spm_shared->qos_pw_car_tree); cfg_cb != NULL;
        cfg_cb = (SPM_QOS_PW_CAR_CB*) AVLL_FIRST(v_spm_shared->qos_pw_car_tree))
    {
        AVLL_DELETE(v_spm_shared->qos_pw_car_tree, cfg_cb->spm_qos_pw_car_node);
        spm_qos_free_color_action_id(&(cfg_cb->color_action_id) NBB_CCXT);
        ret = spm_qos_free_meter_id(&(cfg_cb->meter_id) NBB_CCXT);
        spm_free_qos_pw_car_cb(cfg_cb NBB_CCXT);
    }

    return; 
}

#endif


#if 4

/*****************************************************************************
   函 数 名  : spm_alloc_qos_up_usr_cb
   功能描述  : 
   输入参数  : 逻辑端口的key
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_UP_USR_CB *spm_alloc_qos_up_usr_cb(SPM_QOS_LOGIC_INTF_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_UP_USR_CB *pstTbl = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pkey);

    if (NULL == pkey)
    {
        printf("**QOS ERROR**%s,%d,key==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_alloc_qos_up_usr_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 分配一个新的逻辑配置块qos表配置条目 */
    pstTbl = (SPM_QOS_UP_USR_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_UP_USR_CB),
              NBB_NORETRY_ACT, MEM_SPM_QOS_UP_USR_CB);
    if (NULL == pstTbl)
    {
        goto EXIT_LABEL;
    }

    /* 初始逻辑配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_UP_USR_CB));
    OS_MEMCPY(&(pstTbl->key), pkey, sizeof(SPM_QOS_LOGIC_INTF_KEY));
    AVLL_INIT_NODE(pstTbl->spm_qos_up_usr_node);

    EXIT_LABEL: NBB_TRC_EXIT();
    return pstTbl;
}


/*****************************************************************************
   函 数 名  : spm_alloc_qos_flow_up_usr_cb
   功能描述  : 
   输入参数  : 逻辑端口的key
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
SPM_QOS_FLOW_UP_USR_CB *spm_alloc_qos_flow_up_usr_cb(SPM_QOS_LOGIC_FLOW_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_UP_USR_CB *pstTbl = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pkey);

    if (NULL == pkey)
    {
        printf("**QOS ERROR**%s,%d,key==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_alloc_qos_flow_up_usr_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        goto EXIT_LABEL;
    }

    /* 分配一个新的逻辑配置块qos表配置条目。*/
    pstTbl = (SPM_QOS_FLOW_UP_USR_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_FLOW_UP_USR_CB),
              NBB_NORETRY_ACT, MEM_SPM_QOS_FLOW_UP_USR_CB);
    if (NULL == pstTbl)
    {
        goto EXIT_LABEL;
    }

    /* 初始逻辑配置块配置条目 */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_FLOW_UP_USR_CB));
    OS_MEMCPY(&(pstTbl->key), pkey, sizeof(SPM_QOS_LOGIC_FLOW_KEY));
    AVLL_INIT_NODE(pstTbl->spm_qos_flow_up_usr_node);

    EXIT_LABEL: NBB_TRC_EXIT();
    return pstTbl;
}


/*****************************************************************************
   函 数 名  : spm_free_up_usr_cb
   功能描述  : 
   输入参数  : 逻辑端口指针
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_free_up_usr_cb(SPM_QOS_UP_USR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        printf("**QOS ERROR**%s,%d,pstTbl==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_free_up_usr_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_UP_USR_CB), MEM_SPM_QOS_UP_USR_CB);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_QOS_UP_USR_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_free_flow_up_usr_cb
   功能描述  : 
   输入参数  : 逻辑端口指针
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数

*****************************************************************************/
NBB_LONG spm_free_flow_up_usr_cb(SPM_QOS_FLOW_UP_USR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        printf("**QOS ERROR**%s,%d,pstTbl==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_free_flow_up_usr_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_FLOW_UP_USR_CB), MEM_SPM_QOS_FLOW_UP_USR_CB);

    /***************************************************************************/
    /* 现在释放单盘信息控制块。                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_QOS_FLOW_UP_USR_CB);
    pstTbl = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_del_up_usr(NBB_LONG unit,NBB_LONG posid,NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(0 != index);

    /* policy模板索引为0 */
    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    pB = spm_qos_find_policy_cb(index NBB_CCXT);
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* 模式错误 */
    if (0 != pB->basic_cfg_cb->mode)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(0 == pB->meter_id)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL; 
    }

#if defined (SPU) || defined (PTN690_CES)           
    ret = ApiC3SetIntfMeter(unit,posid,0,pB->meter_id);
    if (ATG_DCI_RC_OK != ret)
    {
        printf("**QOS ERROR**%s,%d,ApiC3SetIntfMeter\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
				       "QOS add up usr",ret,"posid","meter id","policy index","",
				       posid,pB->meter_id,pB->policy_key,0));
        goto EXIT_LABEL;
    }
#endif
    ret = spm_disconnect_policy_cnt(index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_up_usr(NBB_ULONG index,NBB_LONG posid, SUB_PORT *sub_port NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;
  
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != sub_port);
    NBB_ASSERT(0 != index);

    if ((NULL == sub_port) || (0 == index))
    {
        printf("**QOS ERROR**%s,%d,CFG==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_add_up_usr",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* policy模板索引为0 */
    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    pB = spm_qos_find_policy_cb(index NBB_CCXT);
    if (NULL == pB)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    if (NULL == pB->basic_cfg_cb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* 模式错误 */
    if (0 != (pB->basic_cfg_cb->mode)) 
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(0 == pB->meter_id)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL; 
    }

    /* 流量监管 */
    if (0 != pB->meter_id)
    {  
#if defined (SPU) || defined (PTN690_CES)           
        ret = ApiC3SetIntfMeter(sub_port->unit,posid,1,pB->meter_id);
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR**%s,%d,ApiC3SetIntfMeter\n",__FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					       "QOS add up usr",ret,"posid","meter id","policy_index","",
					       posid,pB->meter_id,pB->policy_key,0));
            goto EXIT_LABEL;
        }
#endif
    }
    ret = spm_connect_policy_cnt(index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_logic_up_usr_qos_node(
     NBB_ULONG key,                 //逻辑接口的key
     SUB_PORT *sub_port,
     ATG_DCI_LOG_UP_USER_QOS_POLICY *up_usr NBB_CCXT_T NBB_CXT)        //子接口信息
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_UP_USR_CB *join_node = NULL;
    NBB_BYTE fapid = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_LONG posid = 0;
    NBB_ULONG index = 0;
    SPM_QOS_LOGIC_INTF_KEY log_key = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == key) || (NULL == sub_port) || (NULL == up_usr))
    {
        printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,%d log key=%lu,port=%d,posid=%ld,qos_policy_index=%lu,"
                  "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
                  key,sub_port->port,sub_port->posid,up_usr->qos_policy_index,up_usr->svlan);
        OS_SPRINTF(ucMessage,"%s %s,%d log key=%lu,port=%d,posid=%ld,qos_policy_index=%lu,"
                   "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
                   key,sub_port->port,sub_port->posid,up_usr->qos_policy_index,up_usr->svlan);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
    spm_hardw_getslot(&fapid);
    if(sub_port->slot != fapid)
    {
        goto EXIT_LABEL;
    }

    posid = sub_port->posid;
    index = up_usr->qos_policy_index;
    log_key.index = key;
    log_key.svlan = up_usr->svlan;
    log_key.cvlan = up_usr->cvlan;
    join_node = AVLL_FIND(SHARED.qos_up_usr_tree, &log_key);

    /***************************************************************************/
    /*                                   增加操作                              */
    /***************************************************************************/
    if ((NULL == join_node) && (0 != index))
    {
        join_node = spm_alloc_qos_up_usr_cb(&log_key NBB_CCXT);
        if (NULL == join_node)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        ret = spm_add_up_usr(index,posid, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            ret = spm_free_up_usr_cb(join_node NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        OS_MEMCPY(&(join_node->sub_port),sub_port,sizeof(SUB_PORT));
        
        //coverity[no_effect_test]
        rv = AVLL_INSERT(SHARED.qos_up_usr_tree, join_node->spm_qos_up_usr_node);
        goto EXIT_LABEL;
    }

    /* 更新 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index != index) && (0 != index))
    {
        ret = spm_add_up_usr(index,posid, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        ret = spm_del_up_usr(sub_port->unit,posid,join_node->policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        goto EXIT_LABEL;
    }

    /* 相同的配置 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index == index) && (0 != index))
    {
        printf("**QOS ERROR**%s,%d,the same cfg\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "logic up usr the same cfg",ATG_DCI_RC_OK,
					   "policy_index","index","","",
					   join_node->policy_index,index,0,0));
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }

    /* 删除配置 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && (0 == index))
    {
        ret = spm_del_up_usr(sub_port->unit,posid,join_node->policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        AVLL_DELETE(SHARED.qos_up_usr_tree, join_node->spm_qos_up_usr_node);
        ret = spm_free_up_usr_cb(join_node NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        goto EXIT_LABEL;
    }

    /* 错误的配置 */
    else 
    {
        printf("**QOS ERROR**%s,%d,logic up usr config missmatch\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS logic up usr config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
    
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_add_flow_up_usr_qos_node(
     NBB_ULONG key,        /* 逻辑接口的key */
     SUB_PORT *sub_port,
     ATG_DCI_LOG_FLOW_UP_USER_QOS *up_usr NBB_CCXT_T NBB_CXT)  /* 子接口信息 */
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_UP_USR_CB *join_node = NULL;
    NBB_BYTE fapid = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_LONG posid = 0;
    NBB_ULONG index = 0;
    SPM_QOS_LOGIC_FLOW_KEY log_key = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY("spm_add_flow_up_usr_qos_node");

    if ((0 == key) || (NULL == sub_port) || (NULL == up_usr))
    {
        printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,%d log key=%lu,port=%d,posid=%ld,qos_policy_index=%lu,"
                  "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
                  key,sub_port->port,sub_port->posid,up_usr->qos_policy_index,up_usr->svlan);
    }
    OS_SPRINTF(ucMessage,"%s %s,%d log key=%lu,port=%d,posid=%ld,qos_policy_index=%lu,"
               "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
               key,sub_port->port,sub_port->posid,up_usr->qos_policy_index,up_usr->svlan);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    spm_hardw_getslot(&fapid);
    if(sub_port->slot != fapid)
    {
        goto EXIT_LABEL;
    }

    posid = sub_port->posid;
    index = up_usr->qos_policy_index;
    log_key.index = key;
    log_key.svlan = up_usr->svlan;
    log_key.cvlan = up_usr->cvlan;
    join_node = AVLL_FIND(SHARED.qos_flow_up_usr_tree, &log_key);
    
    /***************************************************************************/
    /*                                   增加操作                              */
    /***************************************************************************/
    if ((NULL == join_node) && (0 != index))
    {
        join_node = spm_alloc_qos_flow_up_usr_cb(&log_key NBB_CCXT);
        if (NULL == join_node)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        ret = spm_add_up_usr(index,posid, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            ret = spm_free_flow_up_usr_cb(join_node NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        OS_MEMCPY(&(join_node->sub_port),sub_port,sizeof(SUB_PORT));
        
        //coverity[no_effect_test]
        rv = AVLL_INSERT(SHARED.qos_flow_up_usr_tree, join_node->spm_qos_flow_up_usr_node);
        goto EXIT_LABEL;
    }

    /* 更新 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index != index) && (0 != index))
    {
        ret = spm_add_up_usr(index,posid, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        ret = spm_del_up_usr(sub_port->unit,posid,join_node->policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        goto EXIT_LABEL;
    }

    /* 相同的配置 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index == index) && (0 != index))
    {
        printf("**QOS ERROR**%s,%d,the same cfg\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "logic up usr the same cfg",ATG_DCI_RC_OK,
					   "policy_index","index","","",
					   join_node->policy_index,index,0,0));
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }

    /* 删除配置 */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && (0 == index))
    {
        ret = spm_del_up_usr(sub_port->unit,posid,join_node->policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }

        AVLL_DELETE(SHARED.qos_flow_up_usr_tree, join_node->spm_qos_flow_up_usr_node);
        ret = spm_free_flow_up_usr_cb(join_node NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        goto EXIT_LABEL;
    }

    /* 错误的配置 */
    else
    {
        printf("**QOS ERROR**%s,%d,logic up usr config missmatch\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS logic up usr config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_LONG spm_qos_clear_logic_up_usr(NBB_LONG logic_key NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_UP_USR_CB *cfg_cb = NULL;
    SPM_QOS_UP_USR_CB *next_cfg_cb = NULL;

    //SPM_QOS_POLICY_CB *pB = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    cfg_cb = (SPM_QOS_UP_USR_CB*) AVLL_FIRST(SHARED.qos_up_usr_tree);
    while(cfg_cb != NULL)
    {
        next_cfg_cb = (SPM_QOS_UP_USR_CB*) AVLL_NEXT(SHARED.qos_up_usr_tree,cfg_cb->spm_qos_up_usr_node);
        if(cfg_cb->key.index != logic_key)
        {
            cfg_cb = next_cfg_cb;
            continue;
        }
        
/*
        pB = spm_qos_find_policy_cb(cfg_cb->policy_index NBB_CCXT);
        if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
        {
            printf("**QOS ERROR**%s line=%d logic_key=%d policy_index=%d can't find policy\n",
                      __FUNCTION__,__LINE__,logic_key,cfg_cb->policy_index);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS spm_qos_clear_logic_acl",ATG_DCI_RC_UNSUCCESSFUL,
    					   "logic_key","policy index","posid","",
    					   logic_key,cfg_cb->policy_index,cfg_cb->sub_port.posid,0));
            ret = ATG_DCI_RC_UNSUCCESSFUL;
        }
#if defined (SPU) || defined (PTN690_CES)
        //coverity[dead_error_condition]
        ret = ApiC3SetIntfMeter(cfg_cb->sub_port.unit,cfg_cb->sub_port.posid,0,pB->meter_id);
        if(ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR**ret=%d,%s line=%d logic_key=%d,posid=%d,meter id=%d\n",
                      ret,__FUNCTION__,__LINE__,logic_key,cfg_cb->sub_port.posid,pB->meter_id);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS spm_qos_clear_logic_acl",ret,
    					   "logic_key","policy index","posid","meter id",
    					   logic_key,cfg_cb->policy_index,cfg_cb->sub_port.posid,pB->meter_id));
            goto EXIT_LABEL;
        }
#endif
*/
        AVLL_DELETE(SHARED.qos_up_usr_tree, cfg_cb->spm_qos_up_usr_node);
        spm_free_up_usr_cb(cfg_cb NBB_CCXT);
        cfg_cb = next_cfg_cb;
     }

    NBB_TRC_EXIT();
    return ret;  
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_qos_clear_all_logic_up_usr(NBB_CXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_UP_USR_CB *cfg_cb = NULL;

    for(cfg_cb = (SPM_QOS_UP_USR_CB*) AVLL_FIRST(v_spm_shared->qos_up_usr_tree); cfg_cb != NULL;
        cfg_cb = (SPM_QOS_UP_USR_CB*) AVLL_FIRST(v_spm_shared->qos_up_usr_tree))
    {
        AVLL_DELETE(v_spm_shared->qos_up_usr_tree, cfg_cb->spm_qos_up_usr_node);
        spm_free_up_usr_cb(cfg_cb NBB_CCXT);
    }


    return; 
}


/*****************************************************************************
   函 数 名  : spm_disconnect_action_cnt
   功能描述  : 当flag不为0时,则删除txlsp的car功能
   输入参数  : action的index
   输出参数  :
   返 回 值  :
   调用函数  :
   被调函数  :
   修改历史  :
   日    期  : 2013年1月15日 星期二
   作    者  : zenglu
   修改内容  : 新生成函数
*****************************************************************************/
NBB_VOID spm_qos_clear_all_flow_up_usr(NBB_CXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_UP_USR_CB *cfg_cb = NULL;


    for(cfg_cb = (SPM_QOS_FLOW_UP_USR_CB*) AVLL_FIRST(v_spm_shared->qos_flow_up_usr_tree); cfg_cb != NULL;
        cfg_cb = (SPM_QOS_FLOW_UP_USR_CB*) AVLL_FIRST(v_spm_shared->qos_flow_up_usr_tree))
    {
        AVLL_DELETE(v_spm_shared->qos_flow_up_usr_tree, cfg_cb->spm_qos_flow_up_usr_node);
        spm_free_flow_up_usr_cb(cfg_cb NBB_CCXT);
    }

    return; 
}

#endif

