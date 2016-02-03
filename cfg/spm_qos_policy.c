/******************************************************************************

                  ��Ȩ���� (C), 1999-2013, ���ͨ�ſƼ��ɷ����޹�˾

******************************************************************************
   �� �� ��   : spm_qos_pm.c
   �� �� ��   : ����
   ��    ��   : zenglu
   ��������   : 2012��9��18��
   ����޸�   :
   ��������   : QOS����
   �����б�   :
   �޸���ʷ   :
   1.��    ��   : 2012��9��18��
    ��    ��   : zenglu
    �޸�����   : �����ļ�
******************************************************************************/
#define SHARED_DATA_TYPE SPM_SHARED_LOCAL

#include <nbase.h>
#include <spmincl.h>

/* ȫ�ִ�ӡ����qos_vc_cfg_print */
extern NBB_BYTE acl_pri_setting;

/* ȫ�ִ�ӡ����qos_vc_cfg_print */
extern NBB_BYTE qos_acl_action_print;

/* ȫ�ִ�ӡ����qos_vc_cfg_print */
extern NBB_BYTE qos_log_cfg_print;

/***************************************************************************/
/*   gloabl function                                                       */
/***************************************************************************/

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_QOS_POLICY_CB *spm_qos_find_policy_cb(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_POLICY_CB *pcb = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    pcb = AVLL_FIND(SHARED.qos_policy_tree, &index);
    if (NULL == pcb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return pcb;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_disconnect_policy_cnt(NBB_ULONG index,NBB_BYTE wred_flag NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pcb = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b = NULL;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_policy_cnt err : ret = %ld, "
                  "policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_policy_cnt err : ret = %ld, "
                   "policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(SHARED.qos_policy_tree, &index);
    if (NULL == pcb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_policy_cnt err : can't find policy cfg. "
                  "ret=%ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_policy_cnt err : can't find policy cfg. "
                   "ret=%ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    for (c2b = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pcb->c2b_tree); c2b != NULL;
         c2b = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pcb->c2b_tree, c2b->spm_policy_c2b_node))
    {
        ret = spm_disconnect_action_cnt(c2b->c2b_key.qos_behavior_index,wred_flag NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        if ((NULL != pcb->basic_cfg_cb) && (0 != pcb->basic_cfg_cb->mode))
        {
            ret = spm_disconnect_classify_cnt(c2b->c2b_key.qos_classify_index NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
        }
    }
    if (0 == pcb->cnt)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_disconnect_policy_cnt err : policy cnt is 0. "
                  "ret=%ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_disconnect_policy_cnt err : policy cnt is 0. "
                   "ret=%ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    (pcb->cnt)--;

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_connect_policy_cnt(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pcb = NULL;
    SPM_QOS_POLICY_C2B_CB *c2b = NULL;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if (0 == index)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_connect_policy_cnt err : ret = %ld, "
                  "policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_connect_policy_cnt err : ret = %ld, "
                   "policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(SHARED.qos_policy_tree, &index);
    if (NULL == pcb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_connect_policy_cnt err : can't find policy cfg. "
                  "ret = %ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        OS_SPRINTF(ucMessage,"%s line=%d spm_connect_policy_cnt err : can't find policy cfg. "
                   "ret = %ld, policy_index = %ld\n\n",__FUNCTION__,__LINE__,ret,index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    for (c2b = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pcb->c2b_tree); c2b != NULL;
         c2b = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pcb->c2b_tree, c2b->spm_policy_c2b_node))
    {
        ret = spm_connect_action_cnt(c2b->c2b_key.qos_behavior_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        if ((NULL != pcb->basic_cfg_cb) && (0 != pcb->basic_cfg_cb->mode))
        {
            ret = spm_connect_classify_cnt(c2b->c2b_key.qos_classify_index NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
        }
    }
    (pcb->cnt)++;

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}




/***************************************************************************/
/*   help function                                                         */
/***************************************************************************/
#if 1

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_apply_classify_high_acl_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    for (id = CLASSIFY_HIGH_ACL_ID; id < CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM; id++)
    {
        if (ATG_DCI_RC_OK == SHARED.g_classify_high_acl_id[id - CLASSIFY_HIGH_ACL_ID]) /*�п���ID*/
        {
            *pid = id;
            SHARED.g_classify_high_acl_id[id - CLASSIFY_HIGH_ACL_ID] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;
    printf("%s line=%d spm_apply_classify_high_acl_id err: ret=%ld, "
              "g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    OS_SPRINTF(ucMessage,"%s line=%d spm_apply_classify_high_acl_id err: ret=%ld, "
               "g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
			       QOS_ERROR_STRING,ret,
			       ucMessage,__FUNCTION__,"LINE","",
			       0,0,__LINE__,0)); 

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_apply_classify_port_acl_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    for (id = CLASSIFY_PORT_ACL_ID; id < CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM; id++)
    {
        if (ATG_DCI_RC_OK == SHARED.g_classify_port_acl_id[id - CLASSIFY_PORT_ACL_ID]) /*�п���ID*/
        {
            *pid = id;
            SHARED.g_classify_port_acl_id[id - CLASSIFY_PORT_ACL_ID] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;
    printf("%s line=%d spm_apply_classify_port_acl_id err: ret=%ld, "
              "g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    OS_SPRINTF(ucMessage,"%s line=%d spm_apply_classify_port_acl_id err: ret=%ld, "
               "g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
			       QOS_ERROR_STRING,ret,
			       ucMessage,__FUNCTION__,"LINE","",
			       0,0,__LINE__,0)); 

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_apply_classify_eth_acl_id(NBB_ULONG *pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    for (id = CLASSIFY_ETH_ACL_ID; id < CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM; id++)
    {
        if (ATG_DCI_RC_OK == SHARED.g_classify_eth_acl_id[id - CLASSIFY_ETH_ACL_ID]) /*�п���ID*/
        {
            *pid = id;
            SHARED.g_classify_eth_acl_id[id - CLASSIFY_ETH_ACL_ID] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;
    printf("%s line=%d spm_apply_classify_eth_acl_id err: ret=%ld, "
              "g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    OS_SPRINTF(ucMessage,"%s line=%d spm_apply_classify_eth_acl_id err: ret=%ld, "
               "g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,*pid);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
			       QOS_ERROR_STRING,ret,
			       ucMessage,__FUNCTION__,"LINE","",
			       0,0,__LINE__,0)); 

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_release_classify_high_acl_id(NBB_ULONG pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((CLASSIFY_HIGH_ACL_ID > pid) || (pid >= (CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM)))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_high_acl_id err:out of range. "
                  "ret=%ld, g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_high_acl_id err:out of range. "
                   "ret=%ld, g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    if (ATG_DCI_RC_OK != SHARED.g_classify_high_acl_id[pid - CLASSIFY_HIGH_ACL_ID])
    {
        SHARED.g_classify_high_acl_id[pid - CLASSIFY_HIGH_ACL_ID] = ATG_DCI_RC_OK;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    else
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_high_acl_id err:high classify acl id is not in use. "
                  "ret=%ld, g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_high_acl_id err:high classify acl id is not in use. "
                   "ret=%ld, g_classify_high_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_release_classify_port_acl_id(NBB_ULONG pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((CLASSIFY_PORT_ACL_ID > pid) || (pid >= (CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM)))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_port_acl_id err:out of range. "
                  "ret=%ld, g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_port_acl_id err:out of range. "
                   "ret=%ld, g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    if (ATG_DCI_RC_OK != SHARED.g_classify_port_acl_id[pid - CLASSIFY_PORT_ACL_ID])
    {
        SHARED.g_classify_port_acl_id[pid - CLASSIFY_PORT_ACL_ID] = ATG_DCI_RC_OK;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    else
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_port_acl_id err:port classify acl id is not in use. "
                  "ret=%ld, g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_port_acl_id err:port classify acl id is not in use. "
                   "ret=%ld, g_classify_port_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_release_classify_eth_acl_id(NBB_ULONG pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((CLASSIFY_ETH_ACL_ID > pid) || (pid >= (CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM)))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_eth_acl_id err:out of range. "
                  "ret=%ld, g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_eth_acl_id err:out of range. "
                   "ret=%ld, g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    if (ATG_DCI_RC_OK != SHARED.g_classify_eth_acl_id[pid - CLASSIFY_ETH_ACL_ID])
    {
        SHARED.g_classify_eth_acl_id[pid - CLASSIFY_ETH_ACL_ID] = ATG_DCI_RC_OK;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    else
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_release_classify_eth_acl_id err:eth classify acl id is not in use. "
                  "ret=%ld, g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        OS_SPRINTF(ucMessage,"%s line=%d spm_release_classify_eth_acl_id err:eth classify acl id is not in use. "
                   "ret=%ld, g_classify_eth_acl_id=%ld\n\n",__FUNCTION__,__LINE__,ret,pid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    			       QOS_ERROR_STRING,ret,
    			       ucMessage,__FUNCTION__,"LINE","",
    			       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_release_policy_acl_id(NBB_ULONG acl_id NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG ret = ATG_DCI_RC_OK;
    
    if((acl_id >= CLASSIFY_PORT_ACL_ID) && (acl_id < CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM))
    {
        spm_release_classify_port_acl_id(acl_id NBB_CCXT);    
    }
    else if((acl_id >= CLASSIFY_ETH_ACL_ID) && (acl_id < CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM))
    {
        spm_release_classify_eth_acl_id(acl_id NBB_CCXT); 
    } 
    else if((acl_id >= CLASSIFY_HIGH_ACL_ID) && (acl_id < CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM))
    {
        spm_release_classify_high_acl_id(acl_id NBB_CCXT);    
    }

    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_QOS_POLICY_C2B_CB* spm_alloc_c2b_cb(ATG_DCI_QOS_POLICY_CB2QOS_DATA *ulkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_POLICY_C2B_CB *pstC2B = NULL;
    

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == ulkey)
    {
    
        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    pstC2B = (SPM_QOS_POLICY_C2B_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_POLICY_C2B_CB),
              NBB_NORETRY_ACT,MEM_SPM_POLICY_C2B_CB);
    if(NULL == pstC2B)
    {

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    
    OS_MEMSET(pstC2B, 0, sizeof(SPM_QOS_POLICY_C2B_CB));
    OS_MEMCPY(&(pstC2B->c2b_key), ulkey, sizeof(ATG_DCI_QOS_POLICY_CB2QOS_DATA));
    AVLL_INIT_NODE(pstC2B->spm_policy_c2b_node);

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstC2B);
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_QOS_POLICY_CB *spm_alloc_policy_cb(NBB_ULONG ulkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_POLICY_CB *pstTbl = NULL;
    NBB_BUF_SIZE avll_key_offset;
    NBB_USHORT i = 0;
    

    NBB_TRC_ENTRY(__FUNCTION__);

    pstTbl = (SPM_QOS_POLICY_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_POLICY_CB),
              NBB_NORETRY_ACT,MEM_SPM_POLICY_CB);
    if (pstTbl == NULL)
    {

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_POLICY_CB));
    pstTbl->policy_key = ulkey;

    AVLL_INIT_NODE(pstTbl->spm_policy_node);

    /* c2b���ĳ�ʼ�� */
    avll_key_offset = NBB_OFFSETOF(SPM_QOS_POLICY_C2B_CB, c2b_key);
    AVLL_INIT_TREE(pstTbl->c2b_tree,spm_qos_c2b_comp,(NBB_USHORT)avll_key_offset,
            (NBB_USHORT)NBB_OFFSETOF(SPM_QOS_POLICY_C2B_CB, spm_policy_c2b_node));

    /* ����classify������߼��˿����ĳ�ʼ�� */
    avll_key_offset = NBB_OFFSETOF(SPM_QOS_FLOW_CLASSIFY_CB, key);
    AVLL_INIT_TREE(pstTbl->instance_tree,compare_ulong,(NBB_USHORT)avll_key_offset,
            (NBB_USHORT)NBB_OFFSETOF(SPM_QOS_FLOW_CLASSIFY_CB, spm_classify_id_node));
    
#ifdef PTN690
    for(i = 0;i < POLICY_C2B_INSTANCE_MAX;i++)
    {
        avll_key_offset = NBB_OFFSETOF(SPM_QOS_FLOW_CLASSIFY_CB, key);
        AVLL_INIT_TREE(pstTbl->c2b_instance_tree[i].instance_tree,spm_qos_logic_key_compare,(NBB_USHORT)avll_key_offset,
            (NBB_USHORT)NBB_OFFSETOF(SPM_QOS_FLOW_CLASSIFY_CB, spm_classify_id_node));
    }  
#endif

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_policy_c2b_cb(SPM_QOS_POLICY_C2B_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    
    //SPM_QOS_POLICY_C2B_CB *pcb = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_POLICY_C2B_CB), MEM_SPM_POLICY_C2B_CB);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_POLICY_C2B_CB);
    pstTbl = NULL;

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_policy_cb(SPM_QOS_POLICY_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *pcb = NULL;
    
    //SPM_QOS_FLOW_CLASSIFY_CB *cfg_cb = NULL;
    //NBB_USHORT i = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pstTbl);

    if (NULL == pstTbl)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_POLICY_CB), MEM_SPM_POLICY_CB);

    /***************************************************************************/
    /* ɾ�����ƿ�ľ����                                                      */
    /***************************************************************************/
    if (NULL != pstTbl->basic_cfg_cb)
    {         
        NBB_MM_FREE(pstTbl->basic_cfg_cb, MEM_SPM_POLICY_BASIC_CB);
        pstTbl->basic_cfg_cb = NULL;
    }
#if 0
#ifdef PTN690

    /* ��ɾ��policy����ģ��֮ǰ,�ȼ��ǹ���ģʽpolicy������(c2b_instance_tree)�Ƿ�Ϊ�� */
    for(i = 0;i < POLICY_C2B_INSTANCE_MAX;i++)
    {
        for(cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_FIRST(pstTbl->c2b_instance_tree[i].instance_tree);
            NULL != cfg_cb;
            cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_FIRST(pstTbl->c2b_instance_tree[i].instance_tree))
        {
            AVLL_DELETE(pstTbl->c2b_instance_tree[i].instance_tree,cfg_cb->spm_classify_id_node);
            
            /* ���ڴ�?*/
        }
    } 
#endif

    /* ��ɾ��policy����ģ��֮ǰ,�ȼ�鹲��ģʽpolicy������(instance_tree)�Ƿ�Ϊ�� */
    for(cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_FIRST(pstTbl->instance_tree);
        NULL != cfg_cb;
        cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_FIRST(pstTbl->instance_tree))
    {
        AVLL_DELETE(pstTbl->instance_tree,cfg_cb->spm_classify_id_node);
    }   
#endif

    /* ɾ��policyģ���е�cb����Ϣ */
    for(pcb = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pstTbl->c2b_tree); pcb != NULL;
        pcb = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pstTbl->c2b_tree))
    {
        SHARED.g_classify_id_instance[pcb->c2b_key.qos_classify_index].match_type = MATCH_ERROR;
        AVLL_DELETE(pstTbl->c2b_tree, pcb->spm_policy_c2b_node);
        NBB_MM_FREE(pcb, MEM_SPM_POLICY_C2B_CB);
    }

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_POLICY_CB);
    pstTbl = NULL;

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#endif


/***************************************************************************/
/*   ips handle function                                                   */
/***************************************************************************/
#if 2

#ifdef PTN690

/*ATG_DCI_RC_OK��ʾû�б�ռ��*/

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_check_policy_instanced(SPM_QOS_POLICY_CB *pstCb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_ULONG i = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL != pstCb)
    {
        if(0 != pstCb->cnt)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

       /************************************************************* 
        * POLICY_C2B_INSTANCE_MAX:��ʾpolicy�����������󶨵�����  *
        * ��(128)�ı���.һ��meterͰ�����Ա���128,���һ��policy *
        * �������󶨵�������Ϊ128*POLICY_C2B_INSTANCE_MAX��.      *
        *************************************************************/
        for(i = 0; i < POLICY_C2B_INSTANCE_MAX;i++)
        {
            /* �ǹ���ģʽpolicy�Ƿ񱻰� */
            if(NULL != AVLL_FIRST(pstCb->c2b_instance_tree[i].instance_tree))
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
        }

        /* ����ģʽpolicy�Ƿ񱻰� */
        if(NULL != AVLL_FIRST(pstCb->instance_tree))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
    } 

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*ɾ���������ӹ�ϵ*/
#if 0
NBB_VOID spm_del_policy_instance_tree_node(SPM_QOS_POLICY_CB *pcb NBB_CCXT_T NBB_CXT)
{
    SPM_QOS_FLOW_CLASSIFY_CB *cfg_cb = NULL;
    NBB_ULONG i = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);
    
    if(NULL != pcb)
    {
        for(i = 0; i < POLICY_C2B_INSTANCE_MAX;i++)
        {
            for(cfg_cb =(SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_FIRST(
                pcb->c2b_instance_tree[i].instance_tree);
                cfg_cb != NULL;cfg_cb = (SPM_QOS_POLICY_SUPERVISE_CB*)AVLL_NEXT(
                pcb->c2b_instance_tree[i].instance_tree,cfg_cb->spm_classify_id_node))
            {
                AVLL_DELETE(pcb->c2b_instance_tree[i].instance_tree,cfg_cb->spm_classify_id_node);
            }
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return;
}

NBB_LONG spm_del_policy_c2b_drive(NBB_ULONG oper,
    ATG_DCI_QOS_POLICY_BASIC_DATA **pcb, NBB_BYTE *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pstCb = NULL;
    SPM_QOS_POLICY_C2B_CB *pstC2B = NULL;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    SPM_QOS_POLICY_CLASSIFY_CB *pstPlicyClassify = NULL;
    SPM_QOS_ACL_CB* pstAcl = NULL;
    NBB_USHORT unit = 0;
    NBB_LONG acl_flag = 0;
    NBB_ULONG classify_index = 0;
    NBB_ULONG action_index = 0;


    NBB_TRC_ENTRY(__FUNCTION__);

    
            
    for(pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pstCb->c2b_tree); pstC2B != NULL;
                pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node))
    {
        classify_index = pstC2B->c2b_key.qos_classify_index;
        action_index = pstC2B->c2b_key.qos_behavior_index;

        /* ɾ��ACL���������ACL��ɾ��ֻ���action�����İ�δ�� */
        for (pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((SHARED.g_classify_id_instance[classify_index].classify_group));
             pstClassify != NULL; pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(pstClassify->spm_classify_group_node))
        {
            for(pstPlicyClassify = (SPM_QOS_POLICY_CLASSIFY_CB*)AVLL_FIRST(pstClassify->spm_classify_policy_tree);
                pstPlicyClassify != NULL;
                pstPlicyClassify = (SPM_QOS_POLICY_CLASSIFY_CB *)AVLL_NEXT(
                    pstClassify->spm_classify_policy_tree, pstPlicyClassify->tree_node))
            {
                
            }
            acl.mAclId = pstCb->acl_id;
            acl.mRuleId = pstClassify->rule_id;
            if(NULL != pstClassify->basic_cfg_cb)
            {   
               /*********************************************************************** 
                * ��ʱClassify�����ƥ��ģʽ��ƥ��AclId;��Policy��������Classify����  *
                * ����Acl,���Policy������ֻ��Ψһ��һ��C2B��,��C2B����ClassifyIdֻ�� *
                * Ψһ��һ��Classify����,�ù���������Acl��.                           *
                ***********************************************************************/
                if(MATCH_ACL_ID == pstClassify->basic_cfg_cb->match_type) 
                {
                    for(pstAcl = (SPM_QOS_ACL_CB*)NBB_NEXT_IN_LIST((SHARED.g_acl_id_instance[pstClassify->basic_cfg_cb->acl_id].acl_group));
                        pstAcl != NULL; pstAcl= (SPM_QOS_ACL_CB *)NBB_NEXT_IN_LIST(pstAcl->spm_acl_group_node))
                    {
                        acl.mAclId = pstAcl->acl_key.acl_id;
                        acl.mRuleId = pstAcl->acl_key.rule_id;

                        /* �ָ����� */
                        if(0 == pstAcl->basic_cfg_cb->action)
                        {
                            acl.eAclAction = ACL_ACTION_DROP;
                        }
                        else if(1 == pstAcl->basic_cfg_cb->action)
                        {
                            acl.eAclAction = ACL_ACTION_PASS;
                        }
                        for(unit = 0; unit < SHARED.c3_num;unit++)
                        {
#if defined (SPU) || defined (PTN690_CES)
                            /* ��C2B����Classify������Acl��,��ɾ��Policyʱ��Ҫ��Acl��ԭ */
                            //coverity[dead_error_condition]
                            ret = ApiC3SetAcl(unit,&acl);
                            if(ATG_DCI_RC_OK != ret)
                            {
                                spm_api_c3_set_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                goto EXIT_LABEL;
                            }
#endif
                        } 
                    }
                    acl_flag = ATG_DCI_RC_UNSUCCESSFUL;
                }

                /* ��ʱclassify��ƥ��ģʽ����ƥ��AclId,������������ƥ������(��̫��ͷ/IP�ȵ�)��� */
                else
                {
                    for(unit = 0; unit < SHARED.c3_num;unit++)
                    {
#if defined (SPU) || defined (PTN690_CES)
                        //coverity[dead_error_condition]
                        ret = ApiC3DelAcl(unit,&acl);
                        if(ATG_DCI_RC_OK != ret)
                        {
                            spm_api_c3_del_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            goto EXIT_LABEL;
                        }
#endif
                    } 
                }
            }
        }
    } 

    /* ��c2b��ģʽ,ͬʱclassify�����ƥ��ģʽ�Ƿ�����AclId��ģʽ:�ͷ�Ϊpolicy�����aclid */
    if((0 != pstCb->basic_cfg_cb->mode)&&(ATG_DCI_RC_OK == acl_flag))
    {
        
        if((pstCb->acl_id >= CLASSIFY_PORT_ACL_ID) && (pstCb->acl_id < CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM))
        {
            spm_release_classify_port_acl_id(pstCb->acl_id NBB_CCXT);    
        }
        else if((pstCb->acl_id >= CLASSIFY_ETH_ACL_ID) && (pstCb->acl_id < CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM))
        {
            spm_release_classify_eth_acl_id(pstCb->acl_id NBB_CCXT); 
        } 
        else if((pstCb->acl_id >= CLASSIFY_HIGH_ACL_ID) &&( pstCb->acl_id < CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM))
        {
            spm_release_classify_high_acl_id(pstCb->acl_id NBB_CCXT);    
        }
    }

            
    

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


NBB_VOID spm_del_policy_drive(SPM_QOS_POLICY_CB *pstCb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pstCb = NULL;
    SPM_QOS_POLICY_C2B_CB *pstC2B = NULL;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    SPM_QOS_POLICY_CLASSIFY_CB *pstPlicyClassify = NULL;
    NBB_ULONG classify_index = 0;
    NBB_ULONG action_index = 0;


    NBB_TRC_ENTRY(__FUNCTION__);

    
            
    for(pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pstCb->c2b_tree); pstC2B != NULL;
                pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node))
    {
        classify_index = pstC2B->c2b_key.qos_classify_index;
        action_index = pstC2B->c2b_key.qos_behavior_index;

        /* ɾ��ACL���������ACL��ɾ��ֻ���action�����İ�δ�� */
        for (pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((SHARED.g_classify_id_instance[classify_index].classify_group));
             pstClassify != NULL; pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(pstClassify->spm_classify_group_node))
        {
            for(pstPlicyClassify = (SPM_QOS_POLICY_CLASSIFY_CB*)AVLL_FIRST(pstClassify->spm_classify_policy_tree);
                pstPlicyClassify != NULL;
                pstPlicyClassify = (SPM_QOS_POLICY_CLASSIFY_CB *)AVLL_NEXT(
                    pstClassify->spm_classify_policy_tree, pstPlicyClassify->tree_node))
            {
                
            }
            acl.mAclId = pstCb->acl_id;
            acl.mRuleId = pstClassify->rule_id;
            if(NULL != pstClassify->basic_cfg_cb)
            {   
               /*********************************************************************** 
                * ��ʱClassify�����ƥ��ģʽ��ƥ��AclId;��Policy��������Classify����  *
                * ����Acl,���Policy������ֻ��Ψһ��һ��C2B��,��C2B����ClassifyIdֻ�� *
                * Ψһ��һ��Classify����,�ù���������Acl��.                           *
                ***********************************************************************/
                if(MATCH_ACL_ID == pstClassify->basic_cfg_cb->match_type) 
                {
                    for(pstAcl = (SPM_QOS_ACL_CB*)NBB_NEXT_IN_LIST((SHARED.g_acl_id_instance[pstClassify->basic_cfg_cb->acl_id].acl_group));
                        pstAcl != NULL; pstAcl= (SPM_QOS_ACL_CB *)NBB_NEXT_IN_LIST(pstAcl->spm_acl_group_node))
                    {
                        acl.mAclId = pstAcl->acl_key.acl_id;
                        acl.mRuleId = pstAcl->acl_key.rule_id;

                        /* �ָ����� */
                        if(0 == pstAcl->basic_cfg_cb->action)
                        {
                            acl.eAclAction = ACL_ACTION_DROP;
                        }
                        else if(1 == pstAcl->basic_cfg_cb->action)
                        {
                            acl.eAclAction = ACL_ACTION_PASS;
                        }
                        for(unit = 0; unit < SHARED.c3_num;unit++)
                        {
#if defined (SPU) || defined (PTN690_CES)
                            /* ��C2B����Classify������Acl��,��ɾ��Policyʱ��Ҫ��Acl��ԭ */
                            //coverity[dead_error_condition]
                            ret = ApiC3SetAcl(unit,&acl);
                            if(ATG_DCI_RC_OK != ret)
                            {
                                spm_api_c3_set_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                goto EXIT_LABEL;
                            }
#endif
                        } 
                    }
                    acl_flag = ATG_DCI_RC_UNSUCCESSFUL;
                }

                /* ��ʱclassify��ƥ��ģʽ����ƥ��AclId,������������ƥ������(��̫��ͷ/IP�ȵ�)��� */
                else
                {
                    for(unit = 0; unit < SHARED.c3_num;unit++)
                    {
#if defined (SPU) || defined (PTN690_CES)
                        //coverity[dead_error_condition]
                        ret = ApiC3DelAcl(unit,&acl);
                        if(ATG_DCI_RC_OK != ret)
                        {
                            spm_api_c3_del_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            goto EXIT_LABEL;
                        }
#endif
                    } 
                }
            }
        }
    } 

    /* ��c2b��ģʽ,ͬʱclassify�����ƥ��ģʽ�Ƿ�����AclId��ģʽ:�ͷ�Ϊpolicy�����aclid */
    if((0 != pstCb->basic_cfg_cb->mode)&&(ATG_DCI_RC_OK == acl_flag))
    {
        
        if((pstCb->acl_id >= CLASSIFY_PORT_ACL_ID) && (pstCb->acl_id < CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM))
        {
            spm_release_classify_port_acl_id(pstCb->acl_id NBB_CCXT);    
        }
        else if((pstCb->acl_id >= CLASSIFY_ETH_ACL_ID) && (pstCb->acl_id < CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM))
        {
            spm_release_classify_eth_acl_id(pstCb->acl_id NBB_CCXT); 
        } 
        else if((pstCb->acl_id >= CLASSIFY_HIGH_ACL_ID) && (pstCb->acl_id < CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM))
        {
            spm_release_classify_high_acl_id(pstCb->acl_id NBB_CCXT);    
        }
    }

            
    

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}
#endif

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_set_policy_basic_cfg(SPM_QOS_POLICY_CB *pstCb,NBB_ULONG oper, 
            ATG_DCI_QOS_POLICY_BASIC_DATA *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstCb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* ips��Ϣ���� */
    switch (oper)
    {
        /* ���� */
        case  ATG_DCI_OPER_UPDATE:
        break;

        /* ɾ�� */
        case  ATG_DCI_OPER_DEL:

        break;

        /* ���� */
        case  ATG_DCI_OPER_ADD:

        /* ��Ϣ����Ϊ�� */
        if (NULL == buf)
        {
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        /* ��Ϣ���ݲ�Ϊ�� */
        else
        {
            /* �ж�policy����ģ���Ƿ����� */
            ret  = spm_check_policy_instanced(pstCb NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }

            /***************************************************************************/
            /*                          ������                                         */
            /***************************************************************************/
            if (NULL == pstCb->basic_cfg_cb)
            {
                pstCb->basic_cfg_cb = (ATG_DCI_QOS_POLICY_BASIC_DATA *)NBB_MM_ALLOC(    \
                        sizeof(ATG_DCI_QOS_POLICY_BASIC_DATA), NBB_NORETRY_ACT, MEM_SPM_POLICY_BASIC_CB);
                if (NULL == pstCb->basic_cfg_cb)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;

                    /* �쳣�˳� */
                    goto EXIT_LABEL;
                } 
            }
            OS_MEMCPY(pstCb->basic_cfg_cb, buf, sizeof(ATG_DCI_QOS_POLICY_BASIC_DATA));
        }
        break;

        default:
        break;
    }

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_set_policy_c2b_cfg(SPM_QOS_POLICY_CB *pstCb,NBB_ULONG oper,
    ATG_DCI_QOS_POLICY_CB2QOS_DATA *buf NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *pstC2B = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstCb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    
    //coverity[var_deref_model]
    pstC2B = AVLL_FIND(pstCb->c2b_tree, buf);

    /* ips��Ϣ���� */
    switch (oper)
    {
        /* ���� */
        case  ATG_DCI_OPER_UPDATE:
        break;

        case  ATG_DCI_OPER_DEL:
        if (NULL == buf)
        {
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        spm_dbg_record_qos_policy_c2b_head(buf,SPM_OPER_DEL NBB_CCXT);

        /* ���������ɾ������ */
        if(NULL != pstC2B)
        {
            /* Ŀǰ�ݲ�֧�����Ѿ��󶨵�policy������ɾ�����е�cb�� */
            ret  = spm_check_policy_instanced(pstCb NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            else
            {
                AVLL_DELETE(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node);
                spm_free_policy_c2b_cb(pstC2B NBB_CCXT);
                pstC2B = NULL;
            } 
        }
        break;
        
        case  ATG_DCI_OPER_ADD:
        if (NULL == buf)
        {
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        spm_dbg_record_qos_policy_c2b_head(buf,SPM_OPER_ADD NBB_CCXT);

        /* ����CB�� */
        if(NULL == pstC2B)
        {

            /* Ŀǰ�ݲ�֧�����Ѿ��󶨵�policy�����������µ�cb�� */
            ret  = spm_check_policy_instanced(pstCb NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            else
            {
                pstC2B = spm_alloc_c2b_cb(buf NBB_CCXT);
                if(NULL == pstC2B)
                {
                    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                    ret = ATG_DCI_RC_UNSUCCESSFUL;

                    /* �쳣�˳� */
                    goto EXIT_LABEL;
                }

                rv = AVLL_INSERT(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node);
            }
        }

        /* ��ͬ������ */
        else
        {
            spm_dbg_record_qos_policy_c2b_head(buf,SPM_OPER_UPD NBB_CCXT);
        }   
        break;

        default:
        break;
    }

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID spm_rcv_dci_set_policy(ATG_DCI_SET_QOS_POLICY *pstSetIps NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ulkey = 0;
    SPM_QOS_POLICY_CB *pstCb = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    ATG_DCI_QOS_POLICY_BASIC_DATA *pstBasicData = NULL;
    ATG_DCI_QOS_POLICY_CB2QOS_DATA *pstC2bData = NULL;
    NBB_BYTE *pucBasicDataStart = NULL;
    NBB_BYTE *pucC2bDataStart = NULL;
    NBB_ULONG ulOperBasic = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperC2b = ATG_DCI_OPER_NULL;
    NBB_USHORT c2bnum = 0;
    NBB_USHORT i = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstSetIps)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    pstSetIps->return_code = ATG_DCI_RC_OK;

    ulkey = pstSetIps->key;
    c2bnum = pstSetIps->cb2qos_num; 

    /* ��ȡ���õĲ����� */
    ulOperBasic = pstSetIps->oper_basic;
    ulOperC2b = pstSetIps->oper_cb2qos;

    /* ��ȡ���õ�ƫ�Ƶ�ַ */
    pucBasicDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->basic_data);
    pucC2bDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->cb2qos_data);
    pstBasicData = (ATG_DCI_QOS_POLICY_BASIC_DATA *)pucBasicDataStart;
    pstC2bData = (ATG_DCI_QOS_POLICY_CB2QOS_DATA*)pucC2bDataStart;
        
    pstCb = AVLL_FIND(SHARED.qos_policy_tree, &ulkey);

    /* ips��ϢΪɾ�� */
    if (TRUE == pstSetIps->delete_struct)
    {
        spm_dbg_record_qos_policy_head(&ulkey,SPM_OPER_DEL NBB_CCXT);
        
        /* �����Ŀ������,��ɾ�� ����OK*/
        if (NULL == pstCb)
        {

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        
        /* ����,ɾ�� */
        else
        {
            /* �ж��Ƿ����� */
            ret  = spm_check_policy_instanced(pstCb NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            AVLL_DELETE(SHARED.qos_policy_tree, pstCb->spm_policy_node);
            spm_free_policy_cb(pstCb NBB_CCXT);
            pstCb = NULL;
        }
    }
    
    /* ���ӻ��߸��� */
    else
    {
        /* ���� */
        if (NULL == pstCb)
        {
            spm_dbg_record_qos_policy_head(&ulkey,SPM_OPER_ADD NBB_CCXT);
            pstCb = spm_alloc_policy_cb(ulkey NBB_CCXT);
            if (NULL == pstCb)
            {
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            
            //coverity[no_effect_test]
            rv = AVLL_INSERT(SHARED.qos_policy_tree, pstCb->spm_policy_node);
        }

        /* ���� */
        else
        {
            spm_dbg_record_qos_policy_head(&ulkey,SPM_OPER_UPD NBB_CCXT);
        }

        /* �������� */
        ret = spm_set_policy_basic_cfg(pstCb,ulOperBasic,pstBasicData NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            pstSetIps->basic_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        /* C2B���� */
        for (i = 0; ((i < c2bnum) && (NULL != pucC2bDataStart)); i++)
        {
            pstC2bData = (ATG_DCI_QOS_POLICY_CB2QOS_DATA *)
               (pucC2bDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_QOS_POLICY_CB2QOS_DATA))) * i);
            ret = spm_set_policy_c2b_cfg(pstCb,ulOperC2b,pstC2bData NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
            }
            else
            {
                if(ATG_DCI_OPER_DEL == ulOperC2b)
                {
                    (pstCb->c2b_num)--; 
                }
                else
                {
                    (pstCb->c2b_num)++; 
                }
            }
        }
    }

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return;
}

#else

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_set_policy_by_acl(SPM_QOS_POLICY_CB *pstPolicy,
    SPM_QOS_ACTION_CB *pstAction NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_ACL_CB *pstAcl = NULL;
    ACL_T acl = {0};
    NBB_USHORT unit = 0;
    NBB_ULONG nhi = 0;
    SPM_PORT_INFO_CB stPortInfo = {0};
    NBB_USHORT i = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstPolicy)
    {
         printf("pstPolicy == NULL\n");
         NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
             			"QOS pstPolicy == NULL",ATG_DCI_RC_UNSUCCESSFUL,
             			"","","","",0,0,0,0));
         ret = ATG_DCI_RC_UNSUCCESSFUL;

         /* �쳣�˳� */
         goto EXIT_LABEL; 
    }
    if(NULL == pstAction)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line = %d spm_set_policy_by_acl err : pstAction "
                  "is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line = %d spm_set_policy_by_acl err : pstAction "
                   "is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        		       QOS_ERROR_STRING,ret,
        		       ucMessage,__FUNCTION__,"LINE","",
        		       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL; 
    }
    for(pstAcl = (SPM_QOS_ACL_CB*)NBB_NEXT_IN_LIST((SHARED.g_acl_id_instance[pstPolicy->acl_id].acl_group));
        pstAcl != NULL; pstAcl = (SPM_QOS_ACL_CB *)NBB_NEXT_IN_LIST(pstAcl->spm_acl_group_node))
    {
        acl.mAclId = pstAcl->acl_key.acl_id;
        acl.mRuleId = pstAcl->acl_key.rule_id;
        if(NULL == pstAction->flow_act_cfg_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line=%d spm_set_policy_by_acl err : flow_act_cfg_cb "
                      "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
            OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_by_acl err : flow_act_cfg_cb "
                       "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            		       QOS_ERROR_STRING,ret,
            		       ucMessage,__FUNCTION__,"LINE","",
            		       0,0,__LINE__,0));

            /* �쳣�˳� */
            goto EXIT_LABEL; 
        }
        if(0 == pstAction->flow_act_cfg_cb->action)
        {
            acl.eAclAction = ACL_ACTION_DROP;
        }
        else if(1 == pstAction->flow_act_cfg_cb->action)
        {
            acl.eAclAction = ACL_ACTION_PASS;
        }

        /* �ض��� */
        else if(3 == pstAction->flow_act_cfg_cb->action)
        {
            if(NULL == pstAction->redirect_cfg_cb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                printf("%s line=%d spm_set_policy_by_acl err : redirect_cfg_cb "
                          "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                          __LINE__, ret, pstAction->action_key);
                OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_by_acl err : redirect_cfg_cb "
                           "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                           __LINE__, ret, pstAction->action_key);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                		       QOS_ERROR_STRING,ret,
                		       ucMessage,__FUNCTION__,"LINE","",
                		       0,0,__LINE__,0)); 

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }

            /* �ر��ض��� */
            if(0 == pstAction->redirect_cfg_cb->action)
            {
                acl.eAclAction = ACL_ACTION_PASS;
            }

            /* IP�����ض���VRF */
            else if(1 == pstAction->redirect_cfg_cb->action)
            {
                acl.eAclAction = ACL_ACTION_VRF;
                acl.vrfId = pstAction->redirect_cfg_cb->vrf_d;
            }

            /* ԭʼ�����ض��򵽶˿� */
            else if(2 == pstAction->redirect_cfg_cb->action)
            {
                if(0 == pstAction->redirect_cfg_cb->out_port_index)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    printf("%s line=%d spm_set_policy_by_acl err : redirect_cfg_cb "
                              "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                              __FUNCTION__, __LINE__, ret, pstAction->action_key);
                    OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_by_acl err : redirect_cfg_cb "
                               "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                               __FUNCTION__, __LINE__, ret, pstAction->action_key);
                    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                    		       QOS_ERROR_STRING,ret,
                    		       ucMessage,__FUNCTION__,"LINE","",
                    		       0,0,__LINE__,0)); 

                    /* �쳣�˳� */
                    goto EXIT_LABEL;
                }
                ret = spm_get_portid_from_logical_port_index(   \
                            pstAction->redirect_cfg_cb->out_port_index,&stPortInfo NBB_CCXT);
                if(ATG_DCI_RC_OK != ret)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    spm_get_portid_from_logical_port_index_error_log(pstAction->redirect_cfg_cb->out_port_index,
                            __FUNCTION__,__LINE__,ret NBB_CCXT);

                    /* �쳣�˳� */
                    goto EXIT_LABEL;
                }
                acl.slot = stPortInfo.slot_id;
                acl.cardPort = stPortInfo.port_id;
                acl.eAclAction = ACL_ACTION_REDIRECTION;
            }
        }

        /* PolicyRouting����·�� */
        else if(4 == pstAction->flow_act_cfg_cb->action)
        {
            acl.eAclAction = ACL_ACTION_NHI;
            if(NULL == pstAction->policy_nhi_cfg_cb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                printf("%s line=%d spm_set_policy_by_acl err : policy_nhi_cfg_cb "
                          "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                          __LINE__, ret, pstAction->action_key);
                OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_by_acl err : policy_nhi_cfg_cb "
                           "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                           __LINE__, ret, pstAction->action_key);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                		       QOS_ERROR_STRING,ret,
                		       ucMessage,__FUNCTION__,"LINE","",
                		       0,0,__LINE__,0)); 

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            for(i = 0;i < pstAction->policy_nhi_cfg_cb->nxhop_num;i++)
            {
                /* IPV4 */
                if(0 == pstAction->policy_nhi_cfg_cb->nxhp[i].ip_type)
                {
                    ret = spm_l3_lookupuni(0, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[3]),
                    pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
                }

                /* IPV6 */
                else
                {
                    ret = spm_l3_lookupuni(1, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[0]),
                    pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
                }
                if(ATG_DCI_RC_OK != ret)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    spm_l3_lookupuni_error_log(acl.mAclId,acl.mRuleId,pstAction->action_key,__FUNCTION__,
                           __LINE__,pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,ret NBB_CCXT);

                    /* �쳣�˳� */
                    goto EXIT_LABEL;
                }
                acl.nhIdx = nhi;
            }
        }   

        /* ���ö��� */
        if(0 != pstAction->meter_id)
        {
            acl.meterIdx = pstAction->meter_id;
            acl.flags |= ACL_METER_ON;

            if(NULL == pstAction->supervise_cfg_cb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            if(1 == pstAction->supervise_cfg_cb->cm_color)
            {
                acl.flags |= ACL_COLOR_AWARE;
            }
            if(2 == pstAction->supervise_cfg_cb->green_action.action)
            {
                acl.greenPhb = pstAction->supervise_cfg_cb->green_action.markphb;
                acl.flags |= ACL_GREEN_PHB;
            }
            if(2 == pstAction->supervise_cfg_cb->yellow_action.action)
            {
                acl.yellowPhb = pstAction->supervise_cfg_cb->yellow_action.markphb;
                acl.flags |= ACL_YELLOW_PHB;
            }
            if(2 == pstAction->supervise_cfg_cb->red_action.action)
            {
                acl.redPhb = pstAction->supervise_cfg_cb->red_action.markphb;
                acl.flags |= ACL_RED_PHB;
            }
            else if(1 == pstAction->supervise_cfg_cb->red_action.action)
            {
                acl.flags |= ACL_DROP_RED;
            }
        }
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
            ret = ApiC3SetAcl(unit, &acl);
            if(ATG_DCI_RC_OK != ret)
            {
                 spm_api_c3_set_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);

                 /* �쳣�˳� */
                 goto EXIT_LABEL;
            }
#endif
        }
        
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;   
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_set_policy_classify_acl(SPM_QOS_CLASSIFY_CB *pstClassify,
    SPM_QOS_ACTION_CB *pstAction,ACL_T *pstAcl NBB_CCXT_T NBB_CXT) 
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    NBB_ULONG aclid = 0;
    SPM_PORT_INFO_CB stPortInfo = {0};
    NBB_ULONG nhi = 0;
    NBB_USHORT i = 0;
    NBB_BYTE tcp_flag = 0;
    SPM_PORT_INFO_CB pstPortInfo = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pstClassify) || (NULL == pstAcl) || (NULL == pstAction))
    {
        printf("NULL == pstClassify or pstAction");
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    if(NULL == pstClassify->basic_cfg_cb)
    {
        printf("NULL == pstClassify->basic_cfg_cb");
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }
    switch(pstClassify->basic_cfg_cb->match_type)
    {

        /* classify����Ŀǰ��ȡ��ƥ��˿ڵĹ��� */
        case MATCH_PORT:
        if(NULL == pstClassify->Port_cfg_cb)
        {
           printf("NULL == pstClassify->Eth_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;

           /* �쳣�˳� */
           goto EXIT_LABEL;
        }
        OS_MEMSET(&pstPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
        ret = spm_get_portid_from_logical_port_index(pstClassify->Port_cfg_cb->index, &pstPortInfo NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_get_portid_from_logical_port_index_error_log(pstClassify->Port_cfg_cb->index,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        /* VE�� */
        if((0 == pstPortInfo.slot_id) && (3 == pstPortInfo.port_type))
        {
            /* ����VE�� */
            if(0 == pstPortInfo.svlan)
            {
               pstAcl->tAclRule.tAclKey.ePortType = ACL_L2VE;
               pstAcl->tAclRule.tAclKey.port = pstPortInfo.port_id;
               pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
               pstAcl->tAclRule.tAclMask.port      = 0xffff;
            }

            /* ����VE�� */
            else
            {
               pstAcl->tAclRule.tAclKey.ePortType = ACL_L3VE;
               pstAcl->tAclRule.tAclKey.port = pstPortInfo.port_id;
               pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
               pstAcl->tAclRule.tAclMask.port      = 0xffff;
            }
        }

        /* ��ͨlogic��lag�� */
        else
        {
            if(0 != pstPortInfo.svlan)
            {
                pstAcl->tAclRule.tAclKey.oVid  = pstPortInfo.svlan;
                pstAcl->tAclRule.tAclMask.oVid = 0x0fff;
            }
            if(0 != pstPortInfo.cvlan)
            {
                pstAcl->tAclRule.tAclKey.iVid  = pstPortInfo.cvlan;
                pstAcl->tAclRule.tAclMask.iVid = 0x0fff;
            }
            pstAcl->tAclRule.tAclKey.ePortType  = ACL_PORT;
            pstAcl->tAclRule.tAclKey.port       = pstPortInfo.port_id;
            pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
            pstAcl->tAclRule.tAclMask.port      = 0xffff;   
#ifdef R8000
            if(0 != pstPortInfo.slot_id)
            {
                pstAcl->tAclRule.tAclKey.slot       = pstPortInfo.slot_id;
                pstAcl->tAclRule.tAclMask.slot      = 0xffff;
            }  
#endif
        }
        break;

        case MATCH_ETH_HEADER:
        if(NULL == pstClassify->Eth_cfg_cb)
        {
           printf("NULL == pstClassify->Eth_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;

           /* �쳣�˳� */
           goto EXIT_LABEL;
        }
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.smac[0]), &(pstClassify->Eth_cfg_cb->src_mac[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.dmac[0]), &(pstClassify->Eth_cfg_cb->dst_mac[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.smac[0]), &(pstClassify->Eth_cfg_cb->src_mac_mask[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.dmac[0]), &(pstClassify->Eth_cfg_cb->dst_mac_mask[0]), ATG_DCI_MAC_LEN);
        pstAcl->tAclRule.tAclKey.ovlan_pri = pstClassify->Eth_cfg_cb->vlan_prio;
        pstAcl->tAclRule.tAclMask.ovlan_pri = pstClassify->Eth_cfg_cb->vlan_prio_mask;
#ifdef R8000_V3R2
        pstAcl->tAclRule.tAclKey.ivlan_pri = pstClassify->Eth_cfg_cb->cvlan_prio;
        pstAcl->tAclRule.tAclMask.ivlan_pri = pstClassify->Eth_cfg_cb->cvlan_prio_mask;
#endif
        pstAcl->tAclRule.tAclKey.ethType  = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IPV4:
        if(NULL == pstClassify->ipv4_cfg_cb)
        {
           printf("NULL == pstClassify->ipv4_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;

           /* �쳣�˳� */
           goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipv4_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipv4_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipv4_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipv4_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipv4_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipv4_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = pstClassify->ipv4_cfg_cb->l3_protocol;
        pstAcl->tAclRule.tAclMask.l3Protocol = pstClassify->ipv4_cfg_cb->l3_protocol_mask;
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.ipHeaderMF = pstClassify->ipv4_cfg_cb->mf;
        pstAcl->tAclRule.tAclMask.ipHeaderMF = pstClassify->ipv4_cfg_cb->mf_mask;
        pstAcl->tAclRule.tAclKey.ipHeaderDF = pstClassify->ipv4_cfg_cb->df;
        pstAcl->tAclRule.tAclMask.ipHeaderDF = pstClassify->ipv4_cfg_cb->df_mask;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipv4_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipv4_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipv4_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipv4_cfg_cb->dst_port_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        if((1 == pstClassify->ipv4_cfg_cb->l3_protocol) 
            && (0xff == pstClassify->ipv4_cfg_cb->l3_protocol_mask) 
            && (0 != pstClassify->ipv4_cfg_cb->payload_mask[0])) 
        {
            pstAcl->tAclRule.tAclKey.icmpType = pstClassify->ipv4_cfg_cb->payload[0];
            pstAcl->tAclRule.tAclMask.icmpType = pstClassify->ipv4_cfg_cb->payload_mask[0];
        }
        if((1 == pstClassify->ipv4_cfg_cb->l3_protocol) 
            && (0xff == pstClassify->ipv4_cfg_cb->l3_protocol_mask)  
            && (0 != pstClassify->ipv4_cfg_cb->payload_mask[1])) 
        { 
            pstAcl->tAclRule.tAclKey.icmpCode = pstClassify->ipv4_cfg_cb->payload[1];
            pstAcl->tAclRule.tAclMask.icmpCode = pstClassify->ipv4_cfg_cb->payload_mask[1];
        }
        break;

        case MATCH_IP_TCP:
        if(NULL == pstClassify->ipTcp_cfg_cb)
        {
            printf("NULL == pstClassify->ipTcp_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipTcp_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipTcp_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipTcp_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipTcp_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipTcp_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipTcp_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = 6;
        pstAcl->tAclRule.tAclMask.l3Protocol = 0xff;
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipTcp_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipTcp_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipTcp_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipTcp_cfg_cb->dst_port_mask; 
        OS_MEMCPY(&tcp_flag,&(pstClassify->ipTcp_cfg_cb->tcp_flag),1);
        pstAcl->tAclRule.tAclKey.tcp_flag = tcp_flag;
        pstAcl->tAclRule.tAclMask.tcp_flag = pstClassify->ipTcp_cfg_cb->tcp_flag_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IP_UDP:
        if(NULL == pstClassify->ipUdp_cfg_cb)
        {
            printf("NULL == pstClassify->ipUdp_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipUdp_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipUdp_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipUdp_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipUdp_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipUdp_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipUdp_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = 0x11;
        pstAcl->tAclRule.tAclMask.l3Protocol = 0xff;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipUdp_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipUdp_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipUdp_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipUdp_cfg_cb->dst_port_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IPV6:
        if(NULL == pstClassify->ipv6_cfg_cb)
        {
            printf("NULL == pstClassify->ipv6_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = 0x86dd;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        OS_MEMCPY(pstAcl->tAclRule.tAclKey.dipv6,pstClassify->ipv6_cfg_cb->dst_ip,ATG_DCI_IPV6_LEN * sizeof(NBB_ULONG));
        ret = spm_get_ipv6_mask(pstClassify->ipv6_cfg_cb->dst_ip_mask_len,
            (NBB_ULONG*)&(pstAcl->tAclRule.tAclMask.dipv6) NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        OS_MEMCPY(pstAcl->tAclRule.tAclKey.sipv6,pstClassify->ipv6_cfg_cb->src_ip,ATG_DCI_IPV6_LEN * sizeof(NBB_ULONG));
        ret = spm_get_ipv6_mask(pstClassify->ipv6_cfg_cb->src_ip_mask_len,
            (NBB_ULONG*)&(pstAcl->tAclRule.tAclMask.sipv6) NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.l3Protocol = pstClassify->ipv6_cfg_cb->l3_protocol;
        pstAcl->tAclRule.tAclMask.l3Protocol = pstClassify->ipv6_cfg_cb->l3_protocol_mask;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipv6_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipv6_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipv6_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipv6_cfg_cb->dst_port_mask;
        pstAcl->tAclRule.tAclKey.tos = pstClassify->ipv6_cfg_cb->tos;
        pstAcl->tAclRule.tAclMask.tos = pstClassify->ipv6_cfg_cb->tos_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_NOIP:
        if(NULL == pstClassify->noIp_cfg_cb)
        {
            printf("NULL == pstClassify->noIp_cfg_cb\n");
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = pstClassify->noIp_cfg_cb->etype;
        pstAcl->tAclRule.tAclMask.ethType = pstClassify->noIp_cfg_cb->etype_mask;
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData6),&(pstClassify->noIp_cfg_cb->payload[0]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData5),&(pstClassify->noIp_cfg_cb->payload[4]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData4),&(pstClassify->noIp_cfg_cb->payload[8]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData6),&(pstClassify->noIp_cfg_cb->payload_mask[0]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData5),&(pstClassify->noIp_cfg_cb->payload_mask[4]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData4),&(pstClassify->noIp_cfg_cb->payload_mask[8]),4);
        pstAcl->flags |= ACL_COUNTER_ON;
        break;
#ifdef R8000_V3R2
        case MATCH_PORT_VLAN:
        if(NULL == pstClassify->portVlan_cfg_cb)
        {
            printf("NULL == pstClassify->portVlan_cfg_cb\n");
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.oVid = pstClassify->portVlan_cfg_cb->svlan_id;
        pstAcl->tAclRule.tAclMask.oVid = pstClassify->portVlan_cfg_cb->svlan_mask;
        pstAcl->tAclRule.tAclKey.iVid = pstClassify->portVlan_cfg_cb->cvlan_id;
        pstAcl->tAclRule.tAclMask.iVid = pstClassify->portVlan_cfg_cb->cvlan_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;
#endif      
        default:
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
        break;
    }

    /* C2B������classify��Ե�actionģ����,����������Ķ���flow_act_cfg_cb������Ϊ�� */
    if(NULL == pstAction->flow_act_cfg_cb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_set_policy_classify_acl err : flow_act_cfg_cb "
                  "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : flow_act_cfg_cb "
                   "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        		       QOS_ERROR_STRING,ret,
        		       ucMessage,__FUNCTION__,"LINE","",
        		       0,0,__LINE__,0)); 

        /* �쳣�˳� */
        goto EXIT_LABEL; 
    }
    if(0 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_DROP;
    }
    else if(1 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_PASS;
    }
    else if(3 == pstAction->flow_act_cfg_cb->action)
    {
        /* ���������ض���ʱ,�ض������ÿ鲻����Ϊ�� */
        if(NULL == pstAction->redirect_cfg_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                      "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                      __LINE__, ret, pstAction->action_key);
            OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                       "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                       __LINE__, ret, pstAction->action_key);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            		       QOS_ERROR_STRING,ret,
            		       ucMessage,__FUNCTION__,"LINE","",
            		       0,0,__LINE__,0)); 

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        if(0 == pstAction->redirect_cfg_cb->action)
        {
            pstAcl->eAclAction = ACL_ACTION_PASS;
        }
        else if(1 == pstAction->redirect_cfg_cb->action)
        {
            pstAcl->eAclAction = ACL_ACTION_VRF;
            pstAcl->vrfId = pstAction->redirect_cfg_cb->vrf_d;
        }
        else if(2 == pstAction->redirect_cfg_cb->action)
        {
            if(0 == pstAction->redirect_cfg_cb->out_port_index)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                printf("%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                          "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                          __FUNCTION__, __LINE__, ret, pstAction->action_key);
                OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                           "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                           __FUNCTION__, __LINE__, ret, pstAction->action_key);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                		       QOS_ERROR_STRING,ret,
                		       ucMessage,__FUNCTION__,"LINE","",
                		       0,0,__LINE__,0)); 

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            ret = spm_get_portid_from_logical_port_index(       \
                        pstAction->redirect_cfg_cb->out_port_index,&stPortInfo NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                spm_get_portid_from_logical_port_index_error_log(pstAction->redirect_cfg_cb->out_port_index,
                        __FUNCTION__,__LINE__,ret NBB_CCXT);

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            pstAcl->slot = stPortInfo.slot_id;
            pstAcl->cardPort = stPortInfo.port_id; 
            pstAcl->eAclAction = ACL_ACTION_REDIRECTION;
        }
    }
    else if(4 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_NHI;
        if(NULL == pstAction->policy_nhi_cfg_cb)
        {   
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line=%d spm_set_policy_classify_acl err : policy_nhi_cfg_cb "
                      "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", 
                      __FUNCTION__, __LINE__, ret, pstAction->action_key);
            OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : policy_nhi_cfg_cb "
                       "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", 
                       __FUNCTION__, __LINE__, ret, pstAction->action_key);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            		       QOS_ERROR_STRING,ret,
            		       ucMessage,__FUNCTION__,"LINE","",
            		       0,0,__LINE__,0)); 

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        for(i = 0;i < pstAction->policy_nhi_cfg_cb->nxhop_num;i++)
        {
            if(0 == pstAction->policy_nhi_cfg_cb->nxhp[i].ip_type)
            {
                ret = spm_l3_lookupuni(0, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[3]),
                pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
            }
            else
            {
                ret = spm_l3_lookupuni(1, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[0]),
                pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
            }
            if(ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                spm_l3_lookupuni_error_log(pstAcl->mAclId,pstAcl->mRuleId,pstAction->action_key,__FUNCTION__,
                       __LINE__,pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,ret NBB_CCXT);

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            pstAcl->nhIdx = nhi;
        }
    }

    /* ���ö��� */
    if(0 != pstAction->meter_id)
    {
        pstAcl->meterIdx = pstAction->meter_id;
        pstAcl->flags |= ACL_METER_ON;

        if(NULL == pstAction->supervise_cfg_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
        if(1 == pstAction->supervise_cfg_cb->cm_color)
        {
            pstAcl->flags |= ACL_COLOR_AWARE;
        }
        if(2 == pstAction->supervise_cfg_cb->green_action.action)
        {
            pstAcl->greenPhb = pstAction->supervise_cfg_cb->green_action.markphb;
            pstAcl->flags |= ACL_GREEN_PHB;
        }
        if(2 == pstAction->supervise_cfg_cb->yellow_action.action)
        {
            pstAcl->yellowPhb = pstAction->supervise_cfg_cb->yellow_action.markphb;
            pstAcl->flags |= ACL_YELLOW_PHB;
        }
        if(2 == pstAction->supervise_cfg_cb->red_action.action)
        {
            pstAcl->redPhb = pstAction->supervise_cfg_cb->red_action.markphb;
            pstAcl->flags |= ACL_RED_PHB;
        }
        else if(1 == pstAction->supervise_cfg_cb->red_action.action)
        {
            pstAcl->flags |= ACL_DROP_RED;
        }
    }
    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3SetAcl(unit, pstAcl);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_set_acl_error_log(unit, pstAcl, __FUNCTION__, __LINE__, ret NBB_CCXT);

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }
#endif
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;   
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_set_policy_c2b_rule(SPM_QOS_POLICY_CB *pstPolicy,
    NBB_ULONG classify_index,NBB_ULONG action_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    SPM_QOS_ACL_CB *pstAcl = NULL;
    ACL_T acl = {0};
    NBB_USHORT unit = 0;
    NBB_ULONG aclid = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstPolicy || classify_index >= MAX_FLOW_CLASSIFY_PRI)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line = %d spm_set_policy_c2b_rule err : policy "
                  "cfg is NULL. ret = %ld\n\n", __FUNCTION__, __LINE__, ret);
        OS_SPRINTF(ucMessage,"%s line = %d spm_set_policy_c2b_rule err : policy "
                   "cfg is NULL. ret = %ld\n\n", __FUNCTION__, __LINE__, ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        		       QOS_ERROR_STRING,ret,
        		       ucMessage,__FUNCTION__,"LINE","",
        		       0,0,__LINE__,0)); 

        /* �쳣�˳� */
         goto EXIT_LABEL; 
    }
    pstAction = AVLL_FIND(SHARED.qos_action_tree,&action_index);
    if(NULL == pstAction)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line = %d spm_set_policy_c2b_rule err : can't find "
                  "action cfg. ret = %ld, action_id = %ld\n\n", __FUNCTION__, __LINE__, 
                  ret, action_index);
        OS_SPRINTF(ucMessage,"%s line = %d spm_set_policy_c2b_rule err : can't find "
                   "action cfg. ret = %ld, action_id = %ld\n\n", __FUNCTION__, __LINE__, 
                   ret, action_index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        		       QOS_ERROR_STRING,ret,
        		       ucMessage,__FUNCTION__,"LINE","",
        		       0,0,__LINE__,0));

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* �������в��ClassifyId�����������й��� */
    for (pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((     \
                                SHARED.g_classify_id_instance[classify_index].classify_group)); 
         pstClassify != NULL; 
         pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(pstClassify->spm_classify_group_node))
    {
    
        /* ÿ��classify����Ļ������ò�����Ϊ��,��ָ���˸ù����ƥ������(��̫��ͷ/TCP_IP��)�� */
        if(NULL == (pstClassify->basic_cfg_cb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line = %d  spm_set_policy_c2b_rule  err : classfiy basic "
                      "cfg is NULL. ret = %ld, ClassfiyId = %ld, if_match_id = %ld\n\n", __FUNCTION__,
                      __LINE__, ret, classify_index, pstClassify->classify_key.if_match_id);
            OS_SPRINTF(ucMessage,"%s line = %d  spm_set_policy_c2b_rule  err : classfiy basic "
                       "cfg is NULL. ret = %ld, ClassfiyId = %ld, if_match_id = %ld\n\n", __FUNCTION__,
                       __LINE__, ret, classify_index, pstClassify->classify_key.if_match_id);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            	           QOS_ERROR_STRING,ret,
            	           ucMessage,__FUNCTION__,"LINE","",
            	           0,0,__LINE__,0)); 

            /* �쳣�˳� */
            goto EXIT_LABEL; 
        }
        
       /******************************************************************** 
        * 1.Policy�����е�C2B������Aclʱ�������AclId;����Policy������˵,  *
        *   ���C2B����ĳ��Classify����������Acl��,��ô��Classifyid��ֻ��  *
        *   ������һ��Classify����,����������N������,�����������ϲ�������  *
        *   ʵ�ֵ�;                                                        *
        * 2.���ĳ��AclId��Policy������C2B�Ե�Classify����������,��ô��Acl *
        *   �������󶨵�intf��,��һ��Ҳ�����ϲ���������֤��;               *
        * 3.��ɾ��Policy����ʱ,���C2B��������Acl��,��ʱ����Ҫ��ԭAcl����, *
        *   ��ԭ���Acl���԰���intf��,����Aclģ���и��»�ɾ����;         *
        * 4.֮������Ҫ��ԭAcl����ΪC2B������Aclʱ���ὫC2B���е�Action���� *
        *   д��Acl��,��ɾ��Policyʱ��Ҫ���ⲿ��Action����������ɾ����     *
        ********************************************************************/
        if(MATCH_ACL_ID == pstClassify->basic_cfg_cb->match_type)
        {
            pstPolicy->acl_id = pstClassify->basic_cfg_cb->acl_id;

            /* ����C2B���������õ�AclId���������:��Action�е�����Ҳд��C3��Acl�� */
            ret = spm_set_policy_by_acl(pstPolicy,pstAction NBB_CCXT);

            /* �쳣�˳� */
            goto EXIT_LABEL;
        }

        /**********************************************************************
         * 1.������Acl�����,��һ�δ洢Policy����ʱ��Ҫ��Policy����һ��AclId; *
         * 2.ÿ��policy����ֻ��һ��aclid,��Ȼpolicy���ж��cb��,���Ӧ�Ŷ���  *
         *   classify����,����Щclassify���򶼹���һ��aclid;                  *
         * 3.ÿ��classify����ϵͳ��Ϊ�����һ��ȫ��Ψһ��ruleid��ָ���ù���.  *
         **********************************************************************/
        if( 0 == pstPolicy->acl_id)
        {
            /* Classif������ƥ��˿� */
            if(MATCH_PORT == pstClassify->basic_cfg_cb->match_type || 
               MATCH_PORT_VLAN == pstClassify->basic_cfg_cb->match_type)
            {
                ret = spm_apply_classify_port_acl_id(&aclid NBB_CCXT);    
            }

            /* Classif��������̫��ͷ */
            else if(MATCH_ETH_HEADER == pstClassify->basic_cfg_cb->match_type)
            {
                ret = spm_apply_classify_eth_acl_id(&aclid NBB_CCXT);    
            }

            /* Classif������ƥ��IP���� */
            else
            {
                ret = spm_apply_classify_high_acl_id(&aclid NBB_CCXT);
            }
            if(ATG_DCI_RC_OK != ret)
            {

                /* �쳣�˳� */
                goto EXIT_LABEL;
            }
            pstPolicy->acl_id = aclid;
        }
        acl.mAclId = pstPolicy->acl_id;
        acl.mRuleId = pstClassify->rule_id;

        /*ACL�����ȼ�*/
        if(0 != acl_pri_setting)
        {
            acl.priority = pstClassify->rule_id;
        }

        ret = spm_set_policy_classify_acl(pstClassify,pstAction,&acl NBB_CCXT);

        /* ��������һ��ʧ��,��Ҫ��ˢ�ɹ���ACLû���� */
        if(ATG_DCI_RC_OK != ret)
        { 

            if(MATCH_PORT == pstClassify->basic_cfg_cb->match_type ||
                MATCH_PORT_VLAN == pstClassify->basic_cfg_cb->match_type)
            {
                spm_release_classify_port_acl_id(pstPolicy->acl_id NBB_CCXT);    
            }
            else if(MATCH_ETH_HEADER == pstClassify->basic_cfg_cb->match_type)
            {
                spm_release_classify_eth_acl_id(pstPolicy->acl_id NBB_CCXT);    
            }
            else
            {
                spm_release_classify_high_acl_id(pstPolicy->acl_id NBB_CCXT);
            }
            pstPolicy->acl_id = 0;
            
            /* �쳣�˳� */
            goto EXIT_LABEL;      
        }
    }

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;    
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID spm_rcv_dci_set_policy(ATG_DCI_SET_QOS_POLICY *pstSetIps NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_BYTE ucIfExist = QOS_EXIST;
    NBB_ULONG ulkey = 0;
    SPM_QOS_POLICY_CB *pstCb = NULL;
    SPM_QOS_POLICY_C2B_CB *pstC2B = NULL;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    NBB_LONG ips_ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    ACL_T acl = {0};
    SPM_QOS_ACL_CB *pstAcl = NULL;
    NBB_ULONG classify_index = 0;
    NBB_BYTE acl_flag = ATG_DCI_RC_OK; /*����ACL��־λ*/
    ATG_DCI_QOS_POLICY_BASIC_DATA *pstBasicData = NULL;
    ATG_DCI_QOS_POLICY_CB2QOS_DATA *pstC2bData = NULL;
    NBB_BYTE *pucBasicDataStart = NULL;
    NBB_BYTE *pucC2bDataStart = NULL;
    NBB_ULONG ulOperBasic = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperC2b = ATG_DCI_OPER_NULL;
    NBB_USHORT c2bnum = 0;
    NBB_SHORT c2bret[ATG_DCI_QOS_POLICY_CB2QOS_NUM] = {0};
    NBB_USHORT i = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pstSetIps)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        goto EXIT_LABEL;
    }

    pstSetIps->return_code = ATG_DCI_RC_OK;
    OS_MEMSET(&acl, 0, sizeof(ACL_T));
    ulkey = pstSetIps->key;
    pstCb = AVLL_FIND(SHARED.qos_policy_tree, &ulkey);
    if (pstCb == NULL)
    {
        ucIfExist = QOS_UNEXIST;
    }
    c2bnum = pstSetIps->cb2qos_num;
    ulOperBasic = pstSetIps->oper_basic;
    ulOperC2b = pstSetIps->oper_cb2qos;
    pucBasicDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->basic_data);
    pucC2bDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetIps, &pstSetIps->cb2qos_data);
    pstBasicData = (ATG_DCI_QOS_POLICY_BASIC_DATA *)pucBasicDataStart;

    /* ips��ϢΪɾ��:�����Ŀ������,��ɾ��;������,��ɾ�� */
    if (TRUE == pstSetIps->delete_struct)
    {
        if (ucIfExist == QOS_UNEXIST)
        {
            pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            goto EXIT_LABEL;
        }
        else
        {
            if(NULL == pstCb)
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                goto EXIT_LABEL;
            }
            if(0 != pstCb->cnt) /* ������ */
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                goto EXIT_LABEL;
            }
            if(NULL != AVLL_FIRST(pstCb->instance_tree)) /* ���������౻���� */
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                goto EXIT_LABEL;
            }
            for(pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pstCb->c2b_tree); pstC2B != NULL;
                pstC2B = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node))
            {
                classify_index = pstC2B->c2b_key.qos_classify_index;

                /* ɾ��ACL���������ACL��ɾ��ֻ���action�����İ�δ�� */
                for (pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((     \
                                    SHARED.g_classify_id_instance[classify_index].classify_group)); 
                     pstClassify != NULL; 
                     pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(pstClassify->spm_classify_group_node))
                {
                    acl.mAclId = pstCb->acl_id;
                    acl.mRuleId = pstClassify->rule_id;
                    if(NULL != pstClassify->basic_cfg_cb)
                    {   
                    
                       /*********************************************************************** 
                        * ��ʱClassify�����ƥ��ģʽ��ƥ��AclId;��Policy��������Classify����  *
                        * ����Acl,���Policy������ֻ��Ψһ��һ��C2B��,��C2B����ClassifyIdֻ�� *
                        * Ψһ��һ��Classify����,�ù���������Acl��.                           *
                        ***********************************************************************/
                        if(MATCH_ACL_ID == pstClassify->basic_cfg_cb->match_type) 
                        {
                            for(pstAcl = (SPM_QOS_ACL_CB*)NBB_NEXT_IN_LIST((        \
                                           SHARED.g_acl_id_instance[pstClassify->basic_cfg_cb->acl_id].acl_group));
                                pstAcl != NULL; 
                                pstAcl = (SPM_QOS_ACL_CB *)NBB_NEXT_IN_LIST(pstAcl->spm_acl_group_node))
                            {
                                acl.mAclId = pstAcl->acl_key.acl_id;
                                acl.mRuleId = pstAcl->acl_key.rule_id;
                                if(0 == pstAcl->basic_cfg_cb->action) /* �ָ����� */
                                {
                                    acl.eAclAction = ACL_ACTION_DROP;
                                }
                                else if(1 == pstAcl->basic_cfg_cb->action)
                                {
                                    acl.eAclAction = ACL_ACTION_PASS;
                                }
                                for(unit = 0; unit < SHARED.c3_num;unit++)
                                {
#if defined (SPU) || defined (PTN690_CES)

                                    /* ��C2B����Classify������Acl��,��ɾ��Policyʱ��Ҫ��Acl��ԭ */
                                    ret = ApiC3SetAcl(unit,&acl);
                                    if(ATG_DCI_RC_OK != ret)
                                    {
                                        spm_api_c3_set_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                                        pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                    }
#endif
                                } 
                            }
                            acl_flag = ATG_DCI_RC_UNSUCCESSFUL;
                        }

                        /* ��ʱclassify��ƥ��ģʽ����ƥ��AclId,������������ƥ������(��̫��ͷ/IP�ȵ�)��� */
                        else
                        {
                            for(unit = 0; unit < SHARED.c3_num;unit++)
                            {
#if defined (SPU) || defined (PTN690_CES)
                                ret = ApiC3DelAcl(unit,&acl);
                                if(ATG_DCI_RC_OK != ret)
                                {
                                    spm_api_c3_del_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                }
#endif
                            } 
                        }
                    }
                }
            } 

            /* ��c2b��ģʽ,ͬʱclassify�����ƥ��ģʽ�Ƿ�����AclId��ģʽ:�ͷ�Ϊpolicy�����aclid */
            if((0 != pstCb->basic_cfg_cb->mode) && (ATG_DCI_RC_OK == acl_flag))
            {
                if((pstCb->acl_id >= CLASSIFY_PORT_ACL_ID) && 
                   (pstCb->acl_id < CLASSIFY_PORT_ACL_ID + MAX_CLASSIFY_PORT_ACL_ID_NUM))
                {
                    spm_release_classify_port_acl_id(pstCb->acl_id NBB_CCXT);    
                }
                else if((pstCb->acl_id >= CLASSIFY_ETH_ACL_ID) && 
                        (pstCb->acl_id < CLASSIFY_ETH_ACL_ID + MAX_CLASSIFY_ETH_ACL_ID_NUM))
                {
                    spm_release_classify_eth_acl_id(pstCb->acl_id NBB_CCXT); 
                } 
                else if((pstCb->acl_id >= CLASSIFY_HIGH_ACL_ID) && 
                        (pstCb->acl_id < CLASSIFY_HIGH_ACL_ID + MAX_CLASSIFY_HIGH_ACL_ID_NUM))
                {
                    spm_release_classify_high_acl_id(pstCb->acl_id NBB_CCXT);    
                }
            }
            AVLL_DELETE(SHARED.qos_policy_tree, pstCb->spm_policy_node);
            ret = spm_free_policy_cb(pstCb NBB_CCXT); /* �ͷ�POLICY��������ڴ�ռ� */
            if (ATG_DCI_RC_OK != ret)
            {
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }
            pstCb = NULL;
        }
    } 
    else /* ���ӻ��߸��� */
    {
        if (ucIfExist != QOS_UNEXIST)
        {
            if (0 != pstCb->cnt)  /* ������ */
            {
                pstSetIps->return_code = ATG_DCI_RC_ADD_FAILED;
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                goto EXIT_LABEL;
            }
            if(NULL != AVLL_FIRST(pstCb->instance_tree)) /* c2b�е�Classify�����ò��ܸ��� */
            {
                pstSetIps->return_code = ATG_DCI_RC_DEL_FAILED;
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                goto EXIT_LABEL;
            }
        }    
        switch (ulOperBasic)  /* ��policy�Ļ�������д���ڴ��� */
        {
            case  ATG_DCI_OPER_UPDATE:
            pstSetIps->basic_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
            break;

            case  ATG_DCI_OPER_DEL:
            pstSetIps->basic_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
            break;

            case  ATG_DCI_OPER_ADD:
            if(NULL == pstBasicData)
            {
                pstSetIps->basic_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;  
            }
            if (ucIfExist == QOS_UNEXIST) /* �����Ŀ������,�������ڴ�ռ䱣������ */
            {
                pstCb = spm_alloc_policy_cb(ulkey NBB_CCXT); /* ����һ������Ŀ���ڴ�ռ� */
                if (NULL == pstCb)
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }
            if(NULL == pstCb->basic_cfg_cb) /* policyģʽ�е��Ƚڵ�/��C2B��ģʽ */
            {
                pstCb->basic_cfg_cb = (ATG_DCI_QOS_POLICY_BASIC_DATA *)NBB_MM_ALLOC(    \
                                       sizeof(ATG_DCI_QOS_POLICY_BASIC_DATA), NBB_NORETRY_ACT, MEM_SPM_POLICY_BASIC_CB);
                if (NULL == pstCb->basic_cfg_cb)
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
            }  
            OS_MEMCPY(pstCb->basic_cfg_cb,pstBasicData,sizeof(ATG_DCI_QOS_POLICY_BASIC_DATA));
            break;

            default:
            break;
        }
        if (ATG_DCI_QOS_POLICY_CB2QOS_NUM <= c2bnum) /* CB�Դ��� */
        {
            /* ��64��C2B�Եķ���ֵȫ����ֵΪʧ�� */
            OS_MEMSET(&(pstSetIps->cb2qos_return_code[0]),ATG_DCI_RC_UNSUCCESSFUL,
                        sizeof(NBB_SHORT)*ATG_DCI_QOS_POLICY_CB2QOS_NUM);
            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
            if(QOS_UNEXIST == ucIfExist)
            {
                spm_free_policy_cb(pstCb NBB_CCXT);
                pstCb = NULL;
            } 
            goto EXIT_LABEL;
        }
        for (i = 0; ((i < c2bnum) && (NULL != pucC2bDataStart)); i++) /* ��policy�����е�C2B�Խ��в���:ɾ��/���� */
        {
            pstC2bData = (ATG_DCI_QOS_POLICY_CB2QOS_DATA *)
               (pucC2bDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_QOS_POLICY_CB2QOS_DATA))) * i);

            if(NULL == pstC2bData)
            {
                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                if(QOS_UNEXIST == ucIfExist)
                {
                    spm_free_policy_cb(pstCb NBB_CCXT);
                    pstCb = NULL;
                }
                goto EXIT_LABEL;
            }

           /**************************************************************************************** 
            *  C2B�ԵĲ�������:����(��ʱ��Ч)��ɾ��������;��Ȼһ��policy��������������64��C2B��, *
            *  ������ͬһ��policy������˵,���������е�C2B�Բ�����������ͬ��:ҪôͬΪɾ��,ҪôͬΪ  *
            *  ���Ӳ���.��ͬһ��policy�����в����ܳ���ĳ��/ЩC2B�ԵĲ�������Ϊ����,����һ��/ЩC2B  *
            *  �ԵĲ�������Ϊɾ�������.                                                           *
            ****************************************************************************************/
            switch (ulOperC2b)
            {
                case  ATG_DCI_OPER_UPDATE:
                break;
                case  ATG_DCI_OPER_DEL: /* ɾ��C2B�� */
                if(NULL == pstCb)
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
                pstC2B = AVLL_FIND(pstCb->c2b_tree, pstC2bData);
                if (NULL == pstC2B)
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                    goto EXIT_LABEL;
                }
                pstAction = AVLL_FIND(SHARED.qos_action_tree,&(pstC2B->c2b_key.qos_behavior_index));
                if(NULL == pstAction)
                {
                    ret = ATG_DCI_RC_UNSUCCESSFUL;
                    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                    goto EXIT_LABEL;
                }
                if(0 != pstCb->basic_cfg_cb->mode) /* policy����Ϊ��C2B��ģʽ */
                {
                    /* C2B���е�Cָ����ClassifyId,ÿ��ClassifyId����������N��Classify���� */
                    for (pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((     \
                             SHARED.g_classify_id_instance[pstC2B->c2b_key.qos_classify_index].classify_group));
                         pstClassify != NULL; 
                         pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(pstClassify->spm_classify_group_node))
                    {
                        
                        if(NULL == pstClassify->basic_cfg_cb) /* classify�����ƥ�����͵Ļ������ò���Ϊ�� */
                        {
                            pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL; 
                            pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                            spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                            goto EXIT_LABEL;
                        }

                        /* ÿ��policy����ֻ��һ��aclid,��������C2B���е�classify�����ø�aclid */
                        acl.mAclId = pstCb->acl_id;

                     /************************************************************************** 
                      * ϵͳ��Ϊÿ��classify�������һ��Ψһ��������RuleId;��Ȼpolicy����C2B�� * 
                      * �����е�classify��aclid��ͬ,��RuleId��Ψһ��,��������ÿ��classify����  * 
                      **************************************************************************/
                        acl.mRuleId = pstClassify->rule_id;
                        for(unit = 0;unit < SHARED.c3_num;unit++)
                        {                      
#if defined (SPU) || defined (PTN690_CES)
                            ret += ApiC3DelAcl(unit,&acl);
                            if(ATG_DCI_RC_OK != ret)
                            {
                                spm_api_c3_del_acl_error_log(unit, &acl, __FUNCTION__, __LINE__, ret NBB_CCXT);
                                pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                                goto EXIT_LABEL;
                            }
#endif  
                        }
                    } 
                }

             /****************************************************************** 
              * ɾ��C3������ClassifyId�����Ĺ����,��ClassifyId��ƥ����������; *
              * ͬһ��ClassifyId�е����й����������ͬ��ƥ������:��̫��ͷ/IP�� *
              ******************************************************************/
                SHARED.g_classify_id_instance[pstC2B->c2b_key.qos_classify_index].match_type = MATCH_ERROR;
                AVLL_DELETE(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node);
                NBB_MM_FREE(pstC2B,MEM_SPM_POLICY_C2B_CB);
                pstC2B = AVLL_FIRST(pstCb->c2b_tree); /* ���CB��ʱ�����ȫ�����Ҫ�ͷ�ACL��Դ */
                if(NULL == pstC2B)
                {
                    spm_release_policy_acl_id(pstCb->acl_id NBB_CCXT);
                    pstCb->acl_id = 0;
                }
                break;

                case  ATG_DCI_OPER_ADD: /* ����C2B�� */

               /****************************************************
                * ��policy����Ϊ��C2B��ģʽʱ,classifyid������Ϊ��;*
                * ��policy����Ϊ���Ƚڵ�ģʽʱ,classifyid�ֶ�Ϊ�㡣*
                ****************************************************/
                if ((NULL != pstCb) && (NULL != pstCb->basic_cfg_cb) && 
                    (0 == pstC2bData->qos_classify_index) &&  
                    (0 != pstCb->basic_cfg_cb->mode))
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                    if(QOS_UNEXIST == ucIfExist)
                    {
                        spm_free_policy_cb(pstCb NBB_CCXT);
                        pstCb = NULL;
                    }
                    goto EXIT_LABEL;
                }

                /* ����Policy�����Ǻ���ģʽ,C2B���е�BehaviorId��������Ϊ�� */
                if (0 == pstC2bData->qos_behavior_index)
                {
                    pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                    if(QOS_UNEXIST == ucIfExist)
                    {
                        spm_free_policy_cb(pstCb NBB_CCXT);
                        pstCb = NULL;
                    }
                    goto EXIT_LABEL;
                }
                if((NULL != pstCb) && (NULL != pstCb->basic_cfg_cb) 
                    && (0 != pstCb->basic_cfg_cb->mode)) /* ��C2B��ģʽ */
                {
                    ret = spm_set_policy_c2b_rule(pstCb, pstC2bData->qos_classify_index,
                                                  pstC2bData->qos_behavior_index NBB_CCXT);
                    if(ATG_DCI_RC_OK != ret)
                    {
                        pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                        if(QOS_UNEXIST == ucIfExist)
                        {
                            spm_free_policy_cb(pstCb NBB_CCXT);
                            pstCb = NULL;
                        }
                        goto EXIT_LABEL;
                    }
                }
                else /*���Ƚڵ�ģʽ*/
                {
                    pstAction = AVLL_FIND(SHARED.qos_action_tree,&(pstC2bData->qos_behavior_index));
                    if(NULL == pstAction)
                    {
                        pstSetIps->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                        if(QOS_UNEXIST == ucIfExist)
                        {
                            spm_free_policy_cb(pstCb NBB_CCXT);
                            pstCb = NULL;
                        }
                        goto EXIT_LABEL;  
                    }

                  /**************************************************************** 
                   * Policy�����е�meter����behavior�е��������,���ڵ��Ƚڵ㴦�� *
                   * ������������;behavior�е���������,������hqos�ж�������������.*
                   ****************************************************************/
                    //coverity[var_deref_op]
                    pstCb->meter_id = pstAction->meter_id; 
                }
                
                //coverity[var_deref_model]
                pstC2B = AVLL_FIND(pstCb->c2b_tree, pstC2bData);
                if(NULL == pstC2B)
                {
                    /* �����������C2B��,�򽫸�C2B����Ϊ�ڵ���뵽policy��C2B������ */
                    pstC2B = spm_alloc_c2b_cb(pstC2bData NBB_CCXT);
                    if(NULL != pstC2B)
                    {
                        rv = AVLL_INSERT(pstCb->c2b_tree, pstC2B->spm_policy_c2b_node);
                    }
                }  
                break;

                default:
                break;
            }
            pstSetIps->cb2qos_return_code[i] = ATG_DCI_RC_OK;
        }

        /* policy�����ж�C2B�ԵĴ������̽���,��C2B�Ե���������policy���Խṹ���� */
        if(NULL != pstCb)
        {
            pstCb->c2b_num = pstSetIps->cb2qos_num;
        }
        if ((QOS_UNEXIST == ucIfExist) && (NULL != pstCb)) /* �������������Ŀ,���뵽���� */
        {
            pstCb->policy_key = ulkey;
            rv = AVLL_INSERT(SHARED.qos_policy_tree, pstCb->spm_policy_node);
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return;
}

#endif

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID spm_qos_clear_all_policy(NBB_CXT_T NBB_CXT)
{
    SPM_QOS_POLICY_CB *cfg_cb = NULL;

    for (cfg_cb = (SPM_QOS_POLICY_CB*) AVLL_FIRST(v_spm_shared->qos_policy_tree);
         cfg_cb != NULL;
         cfg_cb = (SPM_QOS_POLICY_CB*) AVLL_FIRST(v_spm_shared->qos_policy_tree))
     {
        AVLL_DELETE(v_spm_shared->qos_policy_tree, cfg_cb->spm_policy_node);
        spm_free_policy_cb(cfg_cb NBB_CCXT);   
     }

    return;   
}

#endif





/*Ӧ�ú���*/
#if 3

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_QOS_FLOW_CLASSIFY_CB *spm_alloc_qos_flow_classify_cb(
    SPM_QOS_LOGIC_INTF_KEY *pkey NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_CLASSIFY_CB *pstTbl = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pkey);

    if (NULL == pkey)
    {
        printf("**QOS ERROR**%s,%d,key==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_alloc_qos_flow_classify_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* ����һ���µ��߼����ÿ�qos��������Ŀ */
    pstTbl = (SPM_QOS_FLOW_CLASSIFY_CB *)NBB_MM_ALLOC(sizeof(SPM_QOS_FLOW_CLASSIFY_CB),
              NBB_NORETRY_ACT, MEM_SPM_QOS_FLOW_CLASSIFY_CB);
    if (NULL == pstTbl)
    {
    
        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /* ��ʼ�߼����ÿ�������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_QOS_FLOW_CLASSIFY_CB));
    OS_MEMCPY(&(pstTbl->key), pkey, sizeof(SPM_QOS_LOGIC_INTF_KEY));
    AVLL_INIT_NODE(pstTbl->spm_qos_flow_classify_node);
    AVLL_INIT_NODE(pstTbl->spm_classify_id_node);

    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return pstTbl;
}

/*****************************************************************************
   �� �� ��  : 
   ��������  : �ͷ��߼��˿���INTF DS�ڵ�
   �������  : �߼��˿�ָ��
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_flow_classify_cb(SPM_QOS_FLOW_CLASSIFY_CB *pstTbl NBB_CCXT_T NBB_CXT)
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
					   "QOS spm_free_flow_classify_cb",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /* �쳣�˳� */
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_QOS_FLOW_CLASSIFY_CB), MEM_SPM_QOS_FLOW_CLASSIFY_CB);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_QOS_FLOW_CLASSIFY_CB);
    pstTbl = NULL;


    /* �쳣�˳� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


#if defined (CR8000_SMART) || defined (PTN690)
/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_qos_set_color_action(NBB_ULONG *id,ATG_DCI_QOS_BEHAVIOR_SUPERVISE *cfg NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    POLICER_ACTION_TEMPLATE_T color_action;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == cfg) || (NULL == id))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }  

    NBB_MEMSET(&color_action, 0, sizeof(POLICER_ACTION_TEMPLATE_T));

    /* ���� */
    if(0 == *id)
    {
       /* ����ɫ��֪����ģ���ģ��id,Ŀǰ���ֻ������4000��ɫ��֪����ģ�� */
        ret = spm_qos_apply_color_action_id(id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        color_action.policer_action_id = *id;

        /* action : 0 / 1 / 2 = pass / drop / markphb */
        if(2 == cfg->green_action.action)
        {
            color_action.chg_gre_pri_ena = 1;
            color_action.gre_pri = cfg->green_action.markphb;
        }
        else if(1 == cfg->green_action.action)
        {
            color_action.chg_gre_pri_ena = 0;
            color_action.gre_drop_ena = 1;
        }
        else if(0 == cfg->green_action.action)
        {
            color_action.chg_gre_pri_ena = 0;
            color_action.gre_drop_ena = 0;
        }

        /* action : 0 / 1 / 2 = pass / drop / markphb */
        if(2 == cfg->yellow_action.action)
        {
            color_action.chg_yel_pri_ena = 1;
            color_action.yel_pri = cfg->yellow_action.markphb;
        }
        else if(1 == cfg->yellow_action.action)
        {
            color_action.chg_yel_pri_ena = 0;
            color_action.yel_drop_ena = 1;
        }
        else if(0 == cfg->yellow_action.action)
        {
            color_action.chg_yel_pri_ena = 0;
            color_action.yel_drop_ena = 0;
        }

        /* action : 0 / 1 / 2 = pass / drop / markphb */
        if(2 == cfg->red_action.action)
        {
            color_action.chg_red_pri_ena = 1;
            color_action.red_pri = cfg->red_action.markphb;
        }
        else if(1 == cfg->red_action.action)
        {
            color_action.chg_red_pri_ena = 0;
            color_action.red_drop_ena = 1;
        }
        else if(0 == cfg->red_action.action)
        {
            color_action.chg_red_pri_ena = 0;
            color_action.red_drop_ena = 0;
        }

        /* ��ɫ��֪����ģ��д��C3������ */
        for(unit = 0; unit < v_spm_shared->c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
            ret = fhdrv_qos_set_policer_action_template(unit,&color_action);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_qos_free_color_action_id(id NBB_CCXT);
                spm_set_policer_action_template_error_log(unit,&color_action,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
                goto EXIT_LABEL;
            }
#endif  
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_qos_del_color_action(NBB_ULONG *id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    
    //POLICER_ACTION_TEMPLATE_T color_action;

    if((NULL != id) && (0 != *id))
    {
        for(unit = 0; unit < v_spm_shared->c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
            ret = fhdrv_qos_clear_policer_action_template(unit,*id);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_clear_policer_color_action_template_error_log(unit,*id,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
            }
#endif
        }
        spm_qos_free_color_action_id(id NBB_CCXT);
    }

    return ret;
}

#endif


#ifdef PTN690

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_qos_set_bucket(NBB_ULONG id,ATG_DCI_QOS_BEHAVIOR_SUPERVISE* cfg NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    METER_CFG_T bucket;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == cfg)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }  

    bucket.cbs = cfg->cbs * 1024;
    bucket.pbsebs = cfg->pbs * 1024;
    bucket.cir = cfg->cir ;
    bucket.pireir = cfg->pir;

   /********************************* 
    * meterͰ��ģ��id,��meterʵ��id *
    * ʵ��id = ģ��id + offset      *
    *********************************/
    bucket.meterId = id;
    bucket.dropRed = 1;

    if(0 == cfg->cm_color)
    {
        bucket.colorBlind = 0;
    }
    else
    {
        bucket.colorBlind = 1;
    }
    
    if(3 == cfg->mode)
    {
         bucket.eMeterType = RFC_2697;
    }
    else if(2 == cfg->mode)
    {
        bucket.eMeterType = RFC_2698;
    }
    else
    {
        bucket.eMeterType = MEF;
    }

    for(unit = 0; unit < v_spm_shared->c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        /* ��meterͰ����Ϣд��C3������ */
        ret = fhdrv_qos_set_policer_bucket_template(unit,&bucket);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_set_policer_bucket_template_error_log(unit,&bucket,
                __FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }
#endif
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_qos_del_bucket(NBB_ULONG *id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;

    if((NULL != id) && (0 != *id))
    {
        for(unit = 0; unit < v_spm_shared->c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
            ret = fhdrv_qos_clear_policer_bucket_template(unit,*id);
            if(ATG_DCI_RC_OK != ret)
            {
                spm_clear_policer_bucket_template_error_log(unit,*id,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
            }
#endif
        }

        //spm_qos_free_meter_bucket_Id(id NBB_CCXT);
    }  

    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_apply_policy_acl_id
   ��������  : ����policyģ������Ӧ��acl_id
   �������  : policyģ�����õ�ָ��
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_apply_policy_acl_id(SPM_QOS_POLICY_CB *pcb NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG acl_id = 0;
    NBB_LONG ret = 0;
    SPM_QOS_POLICY_C2B_CB *c2b = NULL;
    NBB_ULONG classify_id = 0;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;

    if(NULL != pcb)
    {
       /*************************************************
        * ����policy����ģ�����Ƿ���cb�Ե�����;Ĭ��ͬ *
        * һpolicyģ��������cb�Ե�classifyƥ��������ͬ. *
        *************************************************/
        c2b = AVLL_FIRST(pcb->c2b_tree);
        if(NULL != c2b)
        {
            classify_id = c2b->c2b_key.qos_classify_index;
            pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST(
                (SHARED.g_classify_id_instance[classify_id].classify_group));
            if(NULL != pstClassify)
            {
                if(MATCH_PORT == pstClassify->match_type)
                {
                    ret = spm_apply_classify_port_acl_id(&acl_id NBB_CCXT);   
                }
                else if(MATCH_ETH_HEADER == pstClassify->match_type)
                {
                    ret = spm_apply_classify_eth_acl_id(&acl_id NBB_CCXT); 
                }
                else if(MATCH_IP_TCP == pstClassify->match_type)
                {
                    ret = spm_apply_classify_high_acl_id(&acl_id NBB_CCXT); 
                }
                else
                {
                    spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
                    acl_id = 0;
                }
                if(ATG_DCI_RC_OK != ret)
                {
                    acl_id = 0;
                }
            }
        }      
    }

    return acl_id;
}


/*****************************************************************************
   �� �� ��  : spm_del_c2b_acl
   ��������  : ����policyģ����cb�������Ƿ�Ϊ��,Ȼ���������
               ���ڴ��б������cb���йص���Ϣ�� 
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_del_c2b_acl(NBB_ULONG policy_id,NBB_ULONG acl_id,
    NBB_ULONG classify_id,NBB_ULONG action_id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    ACL_T acl = {0};
    NBB_USHORT unit = 0;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    SPM_QOS_POLICY_SUPERVISE_CB *policy_supevise = NULL;
    SPM_QOS_POLICY_SUPERVISE_KEY policy_supervise_key;
    SPM_QOS_POLICY_CLASSIFY_CB *policy_classify = NULL;
    SPM_QOS_POLICY_CLASSIFY_KEY policy_classify_key;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    /* ����behavior��classifyģ�������Ƿ�Ϊ�� */
    pstAction = AVLL_FIND(SHARED.qos_action_tree,&(action_id));
    pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((SHARED.g_classify_id_instance[classify_id].classify_group));
    if((NULL == pstAction) || (NULL == pstClassify))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    
    NBB_MEMSET(&acl,0,sizeof(ACL_T));

    /* policy����ģ���ʱ�������acl_id */
    acl.mAclId = acl_id;

   /********************************************************************* 
    * 1.ɾ��METERͰ�Լ���behaviorģ���в������������֮��Ĳ����ϵ.    *
    * 2.��policy�󶨵�intf��ʱ,�Ὣpolicy��cb����Ϣд���ڴ��������.    *
    * 3.��д��ɹ���,�ὫMETERͰ�����Ϣ(policy_id��classify_id��acl_id *
    *   ��meter_bucket_id)����behaviorģ��Ĳ��������������.           *
    * 4.��һ��policy����ģ����,һ��behaviorģ����ܻ��Ӧ�Ŷ��classify *
    *   ģ��,���behaviorģ���в��������������key��Ҫ����classify_id,  *
    *   ����ָ����ʱ��policy��behavior�ֱ�����Щclassifyģ����ϳ�cb��. *
    *********************************************************************/
    if(NULL != pstAction)
    {
       /***************************************************************************** 
        * ����������ܽڵ��ŵ���Ϣ:meterͰ��ģ��id��behaviorģ�屻���õ�policyid��*
        * ��behaviorģ��һ����ϳ�cb�Ե�classifyid��policyģ�屻���������acl_id. *
        *****************************************************************************/
        policy_supervise_key.policy_id   = policy_id;
        policy_supervise_key.classify_id = classify_id;
        policy_supervise_key.acl_id      = acl_id;
        policy_supevise = AVLL_FIND(pstAction->spm_qos_supervise_tree,
                            &policy_supervise_key);
        if(NULL != policy_supevise)
        {
            spm_qos_del_bucket(&(policy_supevise->meter_bucket_id) NBB_CCXT);
            AVLL_DELETE(pstAction->spm_qos_supervise_tree,
                  policy_supevise->policy_supervise_node);
            spm_free_policy_supervise_cb(policy_supevise NBB_CCXT);
            policy_supevise = NULL;
        }
    }

   /********************************************************************* 
    * 1.ɾ��ACL RULE�Լ���classifyģ���в���classify��֮��Ĳ����ϵ.   *
    * 2.��policy�󶨵�intf��ʱ,�Ὣpolicy��cb����Ϣд���ڴ��������.    *
    * 3.��д��ɹ���,�Ὣ��classify�����Ϣ(policy_id��acl_id��rule_id��*
    *   posid)����classifyģ���в���classify����.                       *
    * 4.��һ��policy����ģ����,һ��classifyģ��ֻ���ܶ�Ӧ��һ��behavior *
    *   ģ�弴һ��policy��N��cb����ÿ��classifyid����Ψһ��.���classify*
    *   ģ���в���classify����key����classify_id,���еĽڵ��ʾclassify *
    *   ģ�屻��Щpolicy����ģ��(policyģ����ܶ�Ӧ��ͬ��aclid)������.  *
    *********************************************************************/
    for( ; pstClassify != NULL;  
            pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(
            pstClassify->spm_classify_group_node))
    {
       /********************************************************************* 
        * 1.����classify�ڵ��ŵ���Ϣ:if-match�������õ�policyid��policy *
        *   ģ�屻���������acl_id���ɹ�д��acl������������ص�posid������*
        *   if-match��������Ӧ��acl�����rule_id.                           *
        * 2.ͬһ��classifyģ����ܻᱻ���policy����������,��ͬһ��policy�� *
        *   ��������һ��behavior��Ӧ���classify,���ǲ��������behavior��Ӧ *
        *   ͬһ��classify,��һ��policy��N��cb����ÿ��classifyid����Ψһ��. * 
        *********************************************************************/
        policy_classify_key.policy_id = policy_id;
        policy_classify_key.acl_id    = acl_id;
        policy_classify = AVLL_FIND(pstClassify->spm_classify_policy_tree,
                                   &policy_classify_key);
        if(NULL != policy_classify)
        {
            acl.mRuleId = policy_classify->rule_id;
            for(unit = 0;unit < SHARED.c3_num;unit++)
            {
#if defined (SPU) || defined (PTN690_CES)

                //coverity[dead_error_condition]
                ret = ApiC3DelAcl(unit, &acl);
                if(ATG_DCI_RC_OK != ret)
                {
                    spm_api_c3_del_acl_error_log(unit, &acl, __FUNCTION__, 
                              __LINE__, ret NBB_CCXT);
                }
#endif
            }
            AVLL_DELETE(pstClassify->spm_classify_policy_tree,
                              policy_classify->tree_node);
            spm_free_policy_classify_cb(policy_classify NBB_CCXT);
            policy_classify = NULL;
        }
    }

   /******************************************************************* 
    * 1.����ͷ�ɫ��֪����ģ��,behaviorģ��Ĳ�����������������нڵ� *
    *   ����ͬһ��ɫ��֪����ģ��.                                     *
    * 2.ֻ����behaviorģ��Ĳ��������������û���κνڵ��,�Ž�ɫ��֪ *
    *   ����ģ��ɾ��.                                                 *
    * 3.�ڵ���Э����,ɫ��֪����ģ��ֻ��������������ÿ��е�һ����,���*
    *   behaviorģ��Ĳ������������Ϊ��,��ʾ������������ÿ�������� *
    *   �����ݱ����,��ô��ʱҲ��Ҫ��ɫ��֪����ģ��ɾ��.              *
    *******************************************************************/
    if(NULL == AVLL_FIRST(pstAction->spm_qos_supervise_tree))
    {
       spm_qos_del_color_action(&(pstAction->color_action_id) NBB_CCXT );
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;  
}

/*****************************************************************************
   �� �� ��  : spm_del_acl_by_policy
   ��������  : ����policyģ��ɾ��cb�����ڴ�������е���Ϣ
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_del_acl_by_policy(SPM_QOS_POLICY_CB *pcb,NBB_ULONG acl_id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *c2b_cb = NULL;
    NBB_ULONG classify_id = 0;
    NBB_ULONG action_id = 0;
    NBB_ULONG policy_id = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pcb))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    policy_id = pcb->policy_key;

    for(c2b_cb = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pcb->c2b_tree); c2b_cb != NULL;
        c2b_cb = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pcb->c2b_tree, c2b_cb->spm_policy_c2b_node))
    {
        classify_id = c2b_cb->c2b_key.qos_classify_index;
        action_id = c2b_cb->c2b_key.qos_behavior_index;
        
       /*********************************************************** 
        * ɾ��cb�����ڴ�������е���Ϣ,����:meterͰģ�塢ɫ��֪�� *
        * ��ģ�塢����������ܽڵ㡢����classify�ڵ㡢acl�����.  *
        ***********************************************************/
        ret = spm_del_c2b_acl(policy_id,acl_id,classify_id,action_id  NBB_CCXT);
    }
    
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;  
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_create_acl_rule_by_c2b(SPM_QOS_CLASSIFY_CB *pstClassify,
    SPM_QOS_ACTION_CB *pstAction,NBB_ULONG aclid,NBB_ULONG rule_id,
    NBB_ULONG bucket_id,NBB_ULONG *posid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT unit = 0;
    SPM_PORT_INFO_CB stPortInfo = {0};
    NBB_ULONG nhi = 0;
    NBB_USHORT i = 0;
    NBB_BYTE tcp_flag = 0;
    NBB_USHORT UsSvlan = 0;
    ACL_T acl;
    ACL_T *pstAcl = NULL;
    
    //POLICER_ACTION_TEMPLATE_T color_action;
    SPM_PORT_INFO_CB pstPortInfo = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pstClassify) || (NULL == pstAction))
    {
        printf("NULL == pstClassify or pstAction");
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(NULL == pstClassify->basic_cfg_cb)
    {
        printf("NULL == pstClassify->basic_cfg_cb");
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    NBB_MEMSET(&acl,0,sizeof(ACL_T));
    pstAcl = &acl;

    pstAcl->mAclId = aclid;
    pstAcl->mRuleId = rule_id;
    if(0 != acl_pri_setting)
    {
        pstAcl->priority = rule_id;
    }
    
    switch(pstClassify->basic_cfg_cb->match_type)
    {
        case MATCH_PORT:
        if(NULL == pstClassify->Port_cfg_cb)
        {
           printf("NULL == pstClassify->Port_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        OS_MEMSET(&pstPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
        ret = spm_get_portid_from_logical_port_index(pstClassify->Port_cfg_cb->index, &pstPortInfo NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_get_portid_from_logical_port_index_error_log(pstClassify->Port_cfg_cb->index,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);
            goto EXIT_LABEL;
        }

        /* VE�� */
        if((0 == pstPortInfo.slot_id) && (3 == pstPortInfo.port_type))
        {
            /* ����VE�� */
            if(0 == pstPortInfo.svlan)
            {
               pstAcl->tAclRule.tAclKey.ePortType = ACL_L2VE;
               pstAcl->tAclRule.tAclKey.port = pstPortInfo.port_id;
               pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
               pstAcl->tAclRule.tAclMask.port      = 0xffff;
            }

            /* ����VE�� */
            else
            {
               pstAcl->tAclRule.tAclKey.ePortType = ACL_L3VE;
               pstAcl->tAclRule.tAclKey.port = pstPortInfo.port_id;
               pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
               pstAcl->tAclRule.tAclMask.port      = 0xffff;
            }
        }

        /* ��ͨlogic��lag�� */
        else
        {
            /* ����� */
            if(0 == pstPortInfo.svlan)
            {
                ret = spm_get_vlan_from_logical_port_index(pstClassify->Port_cfg_cb->index, &UsSvlan);
                if(ATG_DCI_RC_OK != ret)
                {
                    spm_get_vlan_from_logical_port_index_error_log(pstClassify->Port_cfg_cb->index,
                            __FUNCTION__,__LINE__,ret NBB_CCXT);
                    goto EXIT_LABEL;
                }
                if(0 != UsSvlan)
                {
                    pstAcl->tAclRule.tAclKey.oVid  = UsSvlan;
                    pstAcl->tAclRule.tAclMask.oVid = 0x0fff;
                }
            }

            /* ����� */
            else
            {
                pstAcl->tAclRule.tAclKey.oVid  = pstPortInfo.svlan;
                pstAcl->tAclRule.tAclMask.oVid = 0x0fff;
            }
            pstAcl->tAclRule.tAclKey.ePortType  = ACL_PORT;
            pstAcl->tAclRule.tAclKey.port       = pstPortInfo.port_id;
            pstAcl->tAclRule.tAclMask.ePortType = 0xffff;
            pstAcl->tAclRule.tAclMask.port      = 0xffff;     
        }
        break;

        case MATCH_ETH_HEADER:
        if(NULL == pstClassify->Eth_cfg_cb)
        {
           printf("NULL == pstClassify->Eth_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.smac[0]), &(pstClassify->Eth_cfg_cb->src_mac[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.dmac[0]), &(pstClassify->Eth_cfg_cb->dst_mac[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.smac[0]), &(pstClassify->Eth_cfg_cb->src_mac_mask[0]), ATG_DCI_MAC_LEN);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.dmac[0]), &(pstClassify->Eth_cfg_cb->dst_mac_mask[0]), ATG_DCI_MAC_LEN);
        pstAcl->tAclRule.tAclKey.ovlan_pri = pstClassify->Eth_cfg_cb->vlan_prio;
        pstAcl->tAclRule.tAclMask.ovlan_pri = pstClassify->Eth_cfg_cb->vlan_prio_mask;
        pstAcl->tAclRule.tAclKey.ethType  = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IPV4:
        if(NULL == pstClassify->ipv4_cfg_cb)
        {
           printf("NULL == pstClassify->ipv4_cfg_cb");
           ret = ATG_DCI_RC_UNSUCCESSFUL;
           goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipv4_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipv4_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipv4_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipv4_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipv4_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipv4_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = pstClassify->ipv4_cfg_cb->l3_protocol;
        pstAcl->tAclRule.tAclMask.l3Protocol = pstClassify->ipv4_cfg_cb->l3_protocol_mask;
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.ipHeaderMF = pstClassify->ipv4_cfg_cb->mf;
        pstAcl->tAclRule.tAclMask.ipHeaderMF = pstClassify->ipv4_cfg_cb->mf_mask;
        pstAcl->tAclRule.tAclKey.ipHeaderDF = pstClassify->ipv4_cfg_cb->df;
        pstAcl->tAclRule.tAclMask.ipHeaderDF = pstClassify->ipv4_cfg_cb->df_mask;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipv4_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipv4_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipv4_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipv4_cfg_cb->dst_port_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        if((1 == pstClassify->ipv4_cfg_cb->l3_protocol) 
            && (0xff == pstClassify->ipv4_cfg_cb->l3_protocol_mask) 
            && (0 != pstClassify->ipv4_cfg_cb->payload_mask[0])) 
        {
            pstAcl->tAclRule.tAclKey.icmpType = pstClassify->ipv4_cfg_cb->payload[0];
            pstAcl->tAclRule.tAclMask.icmpType = pstClassify->ipv4_cfg_cb->payload_mask[0];
        }
        if((1 == pstClassify->ipv4_cfg_cb->l3_protocol) 
            && (0xff == pstClassify->ipv4_cfg_cb->l3_protocol_mask) 
            && (0 != pstClassify->ipv4_cfg_cb->payload_mask[1])) 
        {
            pstAcl->tAclRule.tAclKey.icmpCode = pstClassify->ipv4_cfg_cb->payload[1];
            pstAcl->tAclRule.tAclMask.icmpCode = pstClassify->ipv4_cfg_cb->payload_mask[1];
        }
        break;

        case MATCH_IP_TCP:
        if(NULL == pstClassify->ipTcp_cfg_cb)
        {
            printf("NULL == pstClassify->ipTcp_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipTcp_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipTcp_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipTcp_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipTcp_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipTcp_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipTcp_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = 6;
        pstAcl->tAclRule.tAclMask.l3Protocol = 0xff;
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipTcp_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipTcp_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipTcp_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipTcp_cfg_cb->dst_port_mask; 
        OS_MEMCPY(&tcp_flag,&(pstClassify->ipTcp_cfg_cb->tcp_flag),1);
        pstAcl->tAclRule.tAclKey.tcp_flag = tcp_flag;
        pstAcl->tAclRule.tAclMask.tcp_flag = pstClassify->ipTcp_cfg_cb->tcp_flag_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IP_UDP:
        if(NULL == pstClassify->ipUdp_cfg_cb)
        {
            printf("NULL == pstClassify->ipUdp_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = 0x0800;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        pstAcl->tAclRule.tAclKey.dip = pstClassify->ipUdp_cfg_cb->dst_ip;
        pstAcl->tAclRule.tAclMask.dip = pstClassify->ipUdp_cfg_cb->dst_ip_mask;
        pstAcl->tAclRule.tAclKey.sip = pstClassify->ipUdp_cfg_cb->src_ip;
        pstAcl->tAclRule.tAclMask.sip = pstClassify->ipUdp_cfg_cb->src_ip_mask;
        pstAcl->tAclRule.tAclKey.tos = (pstClassify->ipUdp_cfg_cb->dscp);
        pstAcl->tAclRule.tAclMask.tos = (pstClassify->ipUdp_cfg_cb->dscp_mask);
        pstAcl->tAclRule.tAclKey.l3Protocol = 0x11;
        pstAcl->tAclRule.tAclMask.l3Protocol = 0xff;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipUdp_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipUdp_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipUdp_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipUdp_cfg_cb->dst_port_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_IPV6:
        if(NULL == pstClassify->ipv6_cfg_cb)
        {
            printf("NULL == pstClassify->ipv6_cfg_cb");
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = 0x86dd;
        pstAcl->tAclRule.tAclMask.ethType = 0xffff;
        OS_MEMCPY(pstAcl->tAclRule.tAclKey.dipv6,pstClassify->ipv6_cfg_cb->dst_ip,ATG_DCI_IPV6_LEN * sizeof(NBB_ULONG));
        ret = spm_get_ipv6_mask(pstClassify->ipv6_cfg_cb->dst_ip_mask_len,
            (NBB_ULONG*)&(pstAcl->tAclRule.tAclMask.dipv6) NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        OS_MEMCPY(pstAcl->tAclRule.tAclKey.sipv6,pstClassify->ipv6_cfg_cb->src_ip,ATG_DCI_IPV6_LEN * sizeof(NBB_ULONG));
        ret = spm_get_ipv6_mask(pstClassify->ipv6_cfg_cb->src_ip_mask_len,
            (NBB_ULONG*)&(pstAcl->tAclRule.tAclMask.sipv6) NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.l3Protocol = pstClassify->ipv6_cfg_cb->l3_protocol;
        pstAcl->tAclRule.tAclMask.l3Protocol = pstClassify->ipv6_cfg_cb->l3_protocol_mask;
        pstAcl->tAclRule.tAclKey.l4SrcPort = pstClassify->ipv6_cfg_cb->src_port;
        pstAcl->tAclRule.tAclMask.l4SrcPort = pstClassify->ipv6_cfg_cb->src_port_mask;
        pstAcl->tAclRule.tAclKey.l4DstPort = pstClassify->ipv6_cfg_cb->dst_port;
        pstAcl->tAclRule.tAclMask.l4DstPort = pstClassify->ipv6_cfg_cb->dst_port_mask;
        pstAcl->tAclRule.tAclKey.tos = pstClassify->ipv6_cfg_cb->tos;
        pstAcl->tAclRule.tAclMask.tos = pstClassify->ipv6_cfg_cb->tos_mask;
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        case MATCH_NOIP:
        if(NULL == pstClassify->noIp_cfg_cb)
        {
            printf("NULL == pstClassify->noIp_cfg_cb\n");
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        pstAcl->tAclRule.tAclKey.ethType = pstClassify->noIp_cfg_cb->etype;
        pstAcl->tAclRule.tAclMask.ethType = pstClassify->noIp_cfg_cb->etype_mask;
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData6),&(pstClassify->noIp_cfg_cb->payload[0]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData5),&(pstClassify->noIp_cfg_cb->payload[4]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclKey.customerData4),&(pstClassify->noIp_cfg_cb->payload[8]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData6),&(pstClassify->noIp_cfg_cb->payload_mask[0]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData5),&(pstClassify->noIp_cfg_cb->payload_mask[4]),4);
        OS_MEMCPY(&(pstAcl->tAclRule.tAclMask.customerData4),&(pstClassify->noIp_cfg_cb->payload_mask[8]),4);
        pstAcl->flags |= ACL_COUNTER_ON;
        break;

        default:
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
        break;
    }

    /* C2B������classify��Ե�actionģ����,����������Ķ���flow_act_cfg_cb������Ϊ�� */
    if(NULL == pstAction->flow_act_cfg_cb)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        printf("%s line=%d spm_set_policy_classify_acl err : flow_act_cfg_cb "
                  "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : flow_act_cfg_cb "
                   "of pstAction is NULL. ret = %ld\n\n",__FUNCTION__,__LINE__,ret);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        		       QOS_ERROR_STRING,ret,
        		       ucMessage,__FUNCTION__,"LINE","",
        		       0,0,__LINE__,0)); 
        goto EXIT_LABEL; 
    }
    if(0 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_DROP;
    }
    else if(1 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_PASS;
    }
    else if(3 == pstAction->flow_act_cfg_cb->action)
    {
        /* ���������ض���ʱ,�ض������ÿ鲻����Ϊ�� */
        if(NULL == pstAction->redirect_cfg_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                      "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                      __LINE__, ret, pstAction->action_key);
            OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                       "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", __FUNCTION__,
                       __LINE__, ret, pstAction->action_key);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            		       QOS_ERROR_STRING,ret,
            		       ucMessage,__FUNCTION__,"LINE","",
            		       0,0,__LINE__,0)); 

            goto EXIT_LABEL;
        }
        if(0 == pstAction->redirect_cfg_cb->action)
        {
            pstAcl->eAclAction = ACL_ACTION_PASS;
        }
        else if(1 == pstAction->redirect_cfg_cb->action)
        {
            pstAcl->eAclAction = ACL_ACTION_VRF;
            pstAcl->vrfId = pstAction->redirect_cfg_cb->vrf_d;
        }
        else if(2 == pstAction->redirect_cfg_cb->action)
        {
            if(0 == pstAction->redirect_cfg_cb->out_port_index)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                printf("%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                          "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                          __FUNCTION__, __LINE__, ret, pstAction->action_key);
                OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : redirect_cfg_cb "
                           "of pstAction out_port_index is 0. ret = %ld, action_id = %ld\n\n", 
                           __FUNCTION__, __LINE__, ret, pstAction->action_key);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                		       QOS_ERROR_STRING,ret,
                		       ucMessage,__FUNCTION__,"LINE","",
                		       0,0,__LINE__,0)); 
                goto EXIT_LABEL;
            }
            ret = spm_get_portid_from_logical_port_index(   \
                            pstAction->redirect_cfg_cb->out_port_index,&stPortInfo NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                spm_get_portid_from_logical_port_index_error_log(pstAction->redirect_cfg_cb->out_port_index,
                        __FUNCTION__,__LINE__,ret NBB_CCXT);
                goto EXIT_LABEL;
            }
            pstAcl->slot = stPortInfo.slot_id;
            pstAcl->cardPort = stPortInfo.port_id;
            pstAcl->eAclAction = ACL_ACTION_REDIRECTION;
        }
    }
    else if(4 == pstAction->flow_act_cfg_cb->action)
    {
        pstAcl->eAclAction = ACL_ACTION_NHI;
        if(NULL == pstAction->policy_nhi_cfg_cb)
        {   
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            printf("%s line=%d spm_set_policy_classify_acl err : policy_nhi_cfg_cb "
                      "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", 
                      __FUNCTION__, __LINE__, ret, pstAction->action_key);
            OS_SPRINTF(ucMessage,"%s line=%d spm_set_policy_classify_acl err : policy_nhi_cfg_cb "
                       "of pstAction is NULL. ret = %ld, action_id = %ld\n\n", 
                       __FUNCTION__, __LINE__, ret, pstAction->action_key);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
            		       QOS_ERROR_STRING,ret,
            		       ucMessage,__FUNCTION__,"LINE","",
            		       0,0,__LINE__,0)); 
            goto EXIT_LABEL;
        }
        for(i = 0;i < pstAction->policy_nhi_cfg_cb->nxhop_num;i++)
        {
            if(0 == pstAction->policy_nhi_cfg_cb->nxhp[i].ip_type)
            {
                ret = spm_l3_lookupuni(0, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[3]),
                pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
            }
            else
            {
                ret = spm_l3_lookupuni(1, &(pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_ip[0]),
                pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,&nhi NBB_CCXT);
            }
            if(ATG_DCI_RC_OK != ret)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;
                spm_l3_lookupuni_error_log(pstAcl->mAclId,pstAcl->mRuleId,pstAction->action_key,__FUNCTION__,
                       __LINE__,pstAction->policy_nhi_cfg_cb->nxhp[i].nxhop_port_index,ret NBB_CCXT);
                goto EXIT_LABEL;
            }
            pstAcl->nhIdx = nhi;
        }
    }

    /* ����meterͰ�Ͷ���*/
    if(NULL != pstAction->supervise_cfg_cb)
    {
        /* meterͰģ�� */
        pstAcl->meterIdx = bucket_id;
        pstAcl->flags |= ACL_METER_ON;
        if(0 != pstAction->color_action_id)
        {
            /* ɫ��֪����ģ�� */
            pstAcl->policer_action_id = pstAction->color_action_id;
        }
    }
    for(unit = 0;unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)

        //coverity[dead_error_condition]
        ret = ApiC3SetAcl(unit, pstAcl);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_api_c3_set_acl_error_log(unit, pstAcl, __FUNCTION__, __LINE__, ret NBB_CCXT);
            goto EXIT_LABEL;
        }
        else
        {
            /* д��ɹ��򷵻ظ���acl rule��������Ӧ��posid */
            *posid = pstAcl->posId;
        }
#endif
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;   
}

/*****************************************************************************
   �� �� ��  : spm_create_c2b_acl
   ��������  : ����һ��cb���е���Ϣ��д��������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_create_c2b_acl(NBB_ULONG policy_id,NBB_ULONG acl_id,NBB_ULONG *rule_id,
    NBB_ULONG classify_id,NBB_ULONG action_id NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_ULONG bucket_id = 0;
    NBB_ULONG posid = 0;
    SPM_QOS_CLASSIFY_CB *pstClassify = NULL;
    SPM_QOS_ACTION_CB *pstAction = NULL;
    SPM_QOS_POLICY_SUPERVISE_KEY policy_supervise_key;
    SPM_QOS_POLICY_CLASSIFY_KEY policy_classify_key;
    SPM_QOS_POLICY_SUPERVISE_CB *policy_supevise = NULL;
    SPM_QOS_POLICY_CLASSIFY_CB *policy_classify = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if(classify_id >= MAX_FLOW_CLASSIFY_PRI)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ����behaviorģ�����ú�classify���������е�classify�������� */
    pstAction = AVLL_FIND(SHARED.qos_action_tree,&(action_id));
    pstClassify = (SPM_QOS_CLASSIFY_CB*)NBB_NEXT_IN_LIST((SHARED.g_classify_id_instance[classify_id].classify_group));
    if((NULL == pstAction) || (NULL == pstClassify) || (NULL == rule_id))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

   /********************************************************** 
    * (��ʼ����cb����behaviorģ�������)                     *
    * ��behaviorģ����������������ÿ������д��������,����: *
    * ɫ��֪����ģ�塢meterͰģ�塢�Լ�������������ܽڵ��  *
    * ��behaviorģ��ļ����(spm_qos_supervise_tree)��.      *
    **********************************************************/
    if((NULL != pstAction->supervise_cfg_cb) && (0 != pstAction->supervise_cfg_cb->mode))
    {
       /************************************************************** 
        * ����ɫ��֪����ģ���ģ��id,����ɫ��֪����ģ��д��C3������; *
        * ����:Ŀǰ����������Դ����,���ֻ������4000��ɫ��֪����ģ�� *
        **************************************************************/
        ret = spm_qos_set_color_action(&(pstAction->color_action_id),
              pstAction->supervise_cfg_cb NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }
        
       /************************************************************* 
        * ����meterͰ��ģ��id,Ŀǰ���ֻ������512��meterͰ;�·����� *
        * behavior����ģ����Ϊ��ɫ��֪����ģ���meterͰģ��������.*
        *************************************************************/
        ret = spm_qos_apply_meter_bucket_id(&bucket_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }

        /* ����C3������cir��pir��ֵд��meterͰ��ģ��id�� */
        ret = spm_qos_set_bucket(bucket_id,pstAction->supervise_cfg_cb NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_qos_free_meter_bucket_id(&bucket_id NBB_CCXT);
            goto EXIT_LABEL;
        }

        /* ����������ȷ,��������� */
        else
        {
            policy_supervise_key.policy_id = policy_id;
            policy_supervise_key.classify_id = classify_id;
            policy_supervise_key.acl_id = acl_id;

           /***************************************************************************** 
            * ����������ܽڵ��ŵ���Ϣ:meterͰ��ģ��id��behaviorģ�屻���õ�policyid��*
            * ��behaviorģ��һ����ϳ�cb�Ե�classifyid��policyģ�屻���������acl_id. *
            *****************************************************************************/
            policy_supevise = spm_alloc_policy_supervise_cb(&policy_supervise_key NBB_CCXT);
            if(NULL != policy_supevise)
            {
                policy_supevise->meter_bucket_id = bucket_id;

                /* ������������ܽڵ����behaviorģ��ļ����(spm_qos_supervise_tree)�� */
                //coverity[no_effect_test]
                rv = AVLL_INSERT(pstAction->spm_qos_supervise_tree,
                        policy_supevise->policy_supervise_node);
            }                
        }        
    }

    /* ��ʼ����cb����classifyģ������� */
    for ( ; pstClassify != NULL; pstClassify = (SPM_QOS_CLASSIFY_CB *)NBB_NEXT_IN_LIST(
            pstClassify->spm_classify_group_node))
    {   
        /* rule_id��Ӧ��policy��cb�����е�if-match���� */
        (*rule_id)++;

         /* posid�ǳɹ�д��acl����������ķ���ֵ */
         ret = spm_create_acl_rule_by_c2b(pstClassify,pstAction,acl_id,
                (*rule_id),bucket_id,&posid NBB_CCXT);
         if(ATG_DCI_RC_OK != ret)
         {
            goto EXIT_LABEL;
         }
         else
         {
            policy_classify_key.policy_id = policy_id;
            policy_classify_key.acl_id    = acl_id;

           /********************************************************************* 
            * 1.����classify�ڵ��ŵ���Ϣ:if-match�������õ�policyid��policy *
            *   ģ�屻���������acl_id���ɹ�д��acl������������ص�posid������*
            *   if-match��������Ӧ��acl�����rule_id.                           *
            * 2.ͬһ��classifyģ����ܻᱻ���policy����������,��ͬһ��policy�� *
            *   ��������һ��behavior��Ӧ���classify,���ٲ��������behavior��Ӧ *
            *   ͬһ��classify,��һ��policy��N��cb����ÿ��classifyid����Ψһ��. * 
            *********************************************************************/
            policy_classify = spm_alloc_policy_classify_cb(&policy_classify_key NBB_CCXT);
            if(NULL != policy_classify)
            {
               policy_classify->posid = posid;
               policy_classify->rule_id = *rule_id;
               rv = AVLL_INSERT((pstClassify->spm_classify_policy_tree),
                                     (policy_classify->tree_node));
            } 
         }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_create_acl_by_policy
   ��������  : ����acl_id����policy����ģ����cb�Ե���Ϣд��������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG  spm_create_acl_by_policy(SPM_QOS_POLICY_CB *pcb,
    NBB_ULONG *acl_id NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG id = 0;
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_C2B_CB *c2b_cb = NULL;
    NBB_ULONG classify_id = 0;
    NBB_ULONG action_id = 0;
    NBB_ULONG policy_id = 0;
    NBB_ULONG old_rule = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pcb) || (NULL == acl_id))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ����policy��������Ӧ��acl_id,Ϊ0��ʾ����ʧ�� */
    id = spm_apply_policy_acl_id(pcb);
    if(0 == id)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    policy_id = pcb->policy_key;
    old_rule  = pcb->rule_resouce;
    for(c2b_cb = (SPM_QOS_POLICY_C2B_CB *)AVLL_FIRST(pcb->c2b_tree); c2b_cb != NULL;
        c2b_cb = (SPM_QOS_POLICY_C2B_CB *)AVLL_NEXT(pcb->c2b_tree, c2b_cb->spm_policy_c2b_node))
    {
        classify_id = c2b_cb->c2b_key.qos_classify_index;
        action_id   = c2b_cb->c2b_key.qos_behavior_index;

        /* ��classify��Ӧ�Ĺ�����behavior��Ӧ��meter����һ��д��C3��acl������ */
        ret = spm_create_c2b_acl(policy_id,id,&(pcb->rule_resouce),
                    classify_id, action_id NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
           /*********************************************************** 
            * ɾ��cb�����ڴ�������е���Ϣ,����:meterͰģ�塢ɫ��֪�� *
            * ��ģ�塢����������ܽڵ㡢����classify�ڵ㡢acl�����.  *
            ***********************************************************/
            spm_del_acl_by_policy(pcb,id NBB_CCXT);
            spm_release_policy_acl_id(id NBB_CCXT);

            /* cb��д������ʧ�ܺ�,��rule_resouce��ԭΪ�ɹ�д�������е�acl������Ŀ�� */
            pcb->rule_resouce = old_rule;
            goto EXIT_LABEL;
        }
    }

    *acl_id = id;
         
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*����1-128*/

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_ULONG  spm_apply_policy_intf_offset(NBB_BYTE *resoure NBB_CCXT_T NBB_CXT)
{
    NBB_ULONG i = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL != resoure)
    {
        for(i = 1;i < POLICY_C2B_INSTANCE_NUM + 1;i++)
        {
            if(ATG_DCI_RC_OK == resoure[i])
            {
                resoure[i] = ATG_DCI_RC_UNSUCCESSFUL;
                goto EXIT_LABEL;
            }
        }
    }
    
    EXIT_LABEL: NBB_TRC_EXIT();
    return i;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID  spm_release_policy_intf_offset(NBB_BYTE *resoure,NBB_ULONG id NBB_CCXT_T NBB_CXT)
{   
    //NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL != resoure) && (id < POLICY_C2B_INSTANCE_NUM + 1))
    {
        if(ATG_DCI_RC_OK != resoure[id])
        {
            resoure[id] = ATG_DCI_RC_OK;
        }
    }
    
    //EXIT_LABEL: NBB_TRC_EXIT();
    return;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_apply_policy_acl_drive(
    SPM_QOS_POLICY_CB *pcb,NBB_ULONG *acl_id,NBB_USHORT *intf_offset,
    NBB_USHORT *sz_offset NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_USHORT i = 0;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pcb) || (NULL == pcb->basic_cfg_cb) || 
        (NULL == acl_id) || (NULL == intf_offset) || (NULL == sz_offset))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* �ǹ���ģʽ */
    if(0 == pcb->basic_cfg_cb->share)
    {
        /* POLICY_C2B_INSTANCE_MAX:һ��polic����ģ���������󶨵�������(128��)�ı��� */
        for(i = 0; i < POLICY_C2B_INSTANCE_MAX;i++)
        {
            /* Ӳ����Դ�Ѿ�����,��ʾ��policy�����Ѿ���intf�󶨹��� */
            if((0 != pcb->c2b_instance_tree[i].acl_id) && 
               (pcb->c2b_instance_tree[i].cnt < POLICY_C2B_INSTANCE_NUM))
            {
               *acl_id = pcb->c2b_instance_tree[i].acl_id;

               /* ����intf_offset,������������ڴ�����κβ��� */
               *intf_offset = spm_apply_policy_intf_offset(
                    &(pcb->c2b_instance_tree[i].offset[0]) NBB_CCXT);

               /* cnt:��ʾ��policy����ģ��������ĵ�i��acl_id��intf�󶨵Ĵ��� */
               (pcb->c2b_instance_tree[i].cnt)++;
               *sz_offset = i;
               goto EXIT_LABEL;
            }
        }
        
        /* �����µ�Ӳ����Դ */
        for(i = 0; i < POLICY_C2B_INSTANCE_MAX;i++)
        {
           /************************************************************* 
            * 1.��һ��policy����ģ�屻������128��ʱ,ֻ��Ӧһ��acl_id; *
            *   ���󶨴�������128��ʱ,�ͻ������µ�acl_id.���һ��policy *
            *   ����ģ����������󶨴����Ķ��ٿ��ܻ��Ӧ���acl_idֵ.   *
            * 2.POLICY_C2B_INSTANCE_MAX��һ��polic����ģ���������󶨵�  *
            *   ������(128��)�ı���,�༴һ��policy����ģ�������ܶ�  *
            *   Ӧ��POLICY_C2B_INSTANCE_MAX��acl_idֵ.                  *
            *************************************************************/
            if(0 == pcb->c2b_instance_tree[i].acl_id)
            {
               /*************************************************************************
                * 1.��ʱacl_id���ϻ�����������ڵ���intf�󶨵�policyģ������Ӧ��acl_id. *
                * 2.���øú�����Ŀ��������acl_id,����policy����ģ���е�����cb����Ϣ���� *
                *   �ڴ��C3оƬ������.                                                 *
                * 3.д���ڴ�������ɹ���,�ٽ���ص�������Ϣ����behaviorģ��Ĳ��������� *
                *   ������classifyģ���еĲ���classify����.���ڱ�ʾcb����classifyģ��id *
                *   ��behaviorģ��id��policy_id��acl_id��meterͰģ�塢acl����posid����Ϣ*
                *   ֮��Ķ�Ӧ��ϵ.                                                     *
                *************************************************************************/
                ret = spm_create_acl_by_policy(pcb,acl_id NBB_CCXT);
                if(ATG_DCI_RC_OK != ret)
                {
                   goto EXIT_LABEL;
                }

                /* �������ݺ�д�������� */
                else
                {
                   /************************************************* 
                    * ����policy����ģ���ʱ��meterͰģ���offset *
                    * ͬһ��policy����Ӧ��acl_id�����������128��.*
                    *************************************************/
                    *intf_offset = spm_apply_policy_intf_offset(
                        &(pcb->c2b_instance_tree[i].offset[0]) NBB_CCXT);

                   /***************************************************************
                    * intf_offset:��intf��policyʱ,ʹ�õ�meterͰģ���offsetֵ; *
                    * sz_offset  :��intf��policyʱ,ʹ�õ���policy����ģ�������� *
                    *             �ĵڼ���acl_id��.һ��policy����ģ�����ֻ�ܶ�Ӧ *
                    *             POLICY_C2B_INSTANCE_MAX��acl_id��.              *
                    * acl_id     :��intf��policyʱ,policy����ģ���������acl_id.*
                    ***************************************************************/
                    *sz_offset = i;
                    pcb->c2b_instance_tree[i].acl_id = *acl_id;

                    /* policy����ģ���������acl_id��intf�󶨵Ĵ��� */
                    (pcb->c2b_instance_tree[i].cnt)++;
                    goto EXIT_LABEL;
                }
            }
        }
        
        /* ��Դ���� */
        ret = ATG_DCI_RC_NO_RESOURCE;
        goto EXIT_LABEL;
    }
    else
    {
        if(0 == pcb->acl_id)
        {
            ret = spm_create_acl_by_policy(pcb, acl_id NBB_CCXT);
            if(ATG_DCI_RC_OK != ret)
            {
               goto EXIT_LABEL;
            }
            else
            {
                *intf_offset = 1;
                *sz_offset   = 0;
                pcb->acl_id  = *acl_id;
                goto EXIT_LABEL;
            }
        }
        else
        {
            *intf_offset = 1;
            *sz_offset = 0;
            *acl_id = pcb->acl_id;
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;  
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID spm_release_policy_acl_drive(
    SPM_QOS_POLICY_CB *pcb,NBB_ULONG acl_id,NBB_ULONG intf_offset,
    NBB_ULONG sz_offset NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_FLOW_CLASSIFY_CB *cfg = NULL;
    
    NBB_TRC_ENTRY(__FUNCTION__);

    if((NULL == pcb) || (NULL == pcb->basic_cfg_cb) || 
        (sz_offset >= POLICY_C2B_INSTANCE_MAX) || 
        (intf_offset > POLICY_C2B_INSTANCE_NUM))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* �ǹ���ģʽ */
    if(0 == pcb->basic_cfg_cb->share)
    {
         /* �ͷ�offset��Դ,�������κ����� */
        spm_release_policy_intf_offset(&(pcb->c2b_instance_tree[sz_offset].offset[0]),
                        intf_offset NBB_CCXT);

        /* cnt:��policy��������Ӧ�ĵ�sz_offset��acl_id��intf���󶨵Ĵ��� */
        if(pcb->c2b_instance_tree[sz_offset].cnt > 0)
        {
            (pcb->c2b_instance_tree[sz_offset].cnt)--;
        }

        /* ���Ҵ�ʱpolicy����ģ���Ƿ�intf�� */
        cfg = AVLL_FIRST(pcb->c2b_instance_tree[sz_offset].instance_tree);

       /*********************************************************** 
        * ��ʱpolicy����ģ�岢û�б��κ�intf����.����ʱpolicy�� *
        * ��ģ�廹������intf��,�򲻶��ڴ���������κ�ɾ������.  * 
        ***********************************************************/
        if(NULL == cfg)
        {
            /* ��ʱpolicy�������ü���cnt����Ϊ0,��Ȼ���������� */
            if(0 == pcb->c2b_instance_tree[sz_offset].cnt)
            {
               /*********************************************************** 
                * ɾ��cb�����ڴ�������е���Ϣ,����:meterͰģ�塢ɫ��֪�� *
                * ��ģ�塢����������ܽڵ㡢����classify�ڵ㡢acl�����.  *
                ***********************************************************/
                spm_del_acl_by_policy(pcb,acl_id NBB_CCXT);

                /* �ͷ�policy�����������acl_id,�����������ڴ�����κβ��� */
                spm_release_policy_acl_id(acl_id NBB_CCXT);
                pcb->c2b_instance_tree[sz_offset].acl_id = 0;
            }
            else
            {
                spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
            }
        } 
        goto EXIT_LABEL;
    }

    /* ����ģʽ */
    else
    {
        cfg = AVLL_FIRST(pcb->instance_tree);
        if(NULL == cfg)/* ԭ��û�а󶨹�ϵ */
        {
           spm_del_acl_by_policy(pcb,acl_id NBB_CCXT);
           spm_release_policy_acl_id(acl_id NBB_CCXT);
           pcb->acl_id = 0;
        } 
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return;  
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_add_flow_classify_drive(
    SPM_QOS_FLOW_CLASSIFY_CB* pcb NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pcb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ����policy����ģ������,����ģʽ�����Ƕ�cb��ģʽ */
    pB = spm_qos_find_policy_cb(pcb->qos_cfg.qos_policy_index NBB_CCXT);
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb) || 
        (0 == pB->basic_cfg_cb->mode))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

   /******************************************************************** 
    * pcb->intf_offset:��intf��policyʱ,ʹ�õ�meterͰģ���offsetֵ; *
    * pcb->sz_offset  :��intf��policyʱ,ʹ�õ���policy����ģ�������� *
    *                  �ĵڼ���acl_id��.һ��policy����ģ�����ֻ�ܶ�Ӧ *
    *                  POLICY_C2B_INSTANCE_MAX��acl_id��.              *
    * pcb->acl_id     :��intf��policyʱ,policy����ģ���������acl_id.*
    ********************************************************************/
    ret = spm_apply_policy_acl_drive(pB,&(pcb->acl_id),&(pcb->intf_offset),
                &(pcb->sz_offset) NBB_CCXT);
    if(ATG_DCI_RC_OK != ret)
    {
        goto EXIT_LABEL;
    }

#if defined (SPU) || defined (PTN690_CES)

    /* ��meterͰģ���offsetֵ��intf��posid��policy����ģ���������acl_id�� */
    ret = fhdrv_intf_set_policer_bucket_offset(pcb->sub_port.unit,
          pcb->sub_port.posid,pcb->intf_offset,pcb->acl_id);

    /* ����ʧ��,��ǰ��д�������е�policy������cb�Ե���Ϣȫ����� */
    if(ATG_DCI_RC_OK != ret)
    {
        spm_release_policy_acl_drive(pB,(pcb->acl_id),(pcb->intf_offset),
            (pcb->sz_offset) NBB_CCXT);
        spm_intf_set_policer_bucket_offset_error_log(pcb->sub_port.unit,
            pcb->sub_port.posid,pcb->intf_offset,pcb->acl_id,
            __FUNCTION__,__LINE__,ret NBB_CCXT);
        goto EXIT_LABEL;
    }
#endif

    /* ���������ɹ��Ժ�����intf��policy����ģ����֮��İ󶨹�ϵ */

    /* �ǹ���ģʽ */
    if(0 == pB->basic_cfg_cb->share)
    {
        /* ��Ѱpolicy����ģ�屻��Щintf���� */
        if(NULL == AVLL_FIND(pB->c2b_instance_tree[pcb->sz_offset].instance_tree,
                             &(pcb->key)))
        {
            rv = AVLL_INSERT(pB->c2b_instance_tree[pcb->sz_offset].instance_tree,
                             pcb->spm_classify_id_node);
        }
    }

    /* ����ģʽ */
    else
    {
        if(NULL == AVLL_FIND(pB->instance_tree,&(pcb->key)))
        {
            rv = AVLL_INSERT(pB->instance_tree,pcb->spm_classify_id_node);
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_del_flow_classify_drive
   ��������  : ɾ��intf�ϻ�����������ڵ���Ϣ
   �������  : SPM_QOS_FLOW_CLASSIFY_CB:intf�ϻ�����������ڵ���Ϣ
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_del_flow_classify_drive(
    SPM_QOS_FLOW_CLASSIFY_CB* pcb NBB_CCXT_T NBB_CXT)
{
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(NULL == pcb)
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    pB = spm_qos_find_policy_cb(pcb->qos_cfg.qos_policy_index NBB_CCXT);
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb) || 
        (0 == pB->basic_cfg_cb->mode))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

#if defined (SPU) || defined (PTN690_CES)

    /* ɾ��intf��acl��bucket_offset֮��İ󶨹�ϵ */
    ret = fhdrv_intf_set_policer_bucket_offset(pcb->sub_port.unit,
          pcb->sub_port.posid,0,0);
    if(ATG_DCI_RC_OK != ret)
    {
        spm_intf_set_policer_bucket_offset_error_log(pcb->sub_port.unit,
            pcb->sub_port.posid,0,0,
            __FUNCTION__,__LINE__,ret NBB_CCXT);
    }
#endif   

    /* �ǹ���ģʽ,ɾ��policy����ģ����intf�ϻ��������������İ󶨹�ϵ */
    if(0 == pB->basic_cfg_cb->share)
    {
        if(NULL != AVLL_FIND(pB->c2b_instance_tree[pcb->sz_offset].instance_tree,
               &(pcb->key)))
        {
            AVLL_DELETE(pB->c2b_instance_tree[pcb->sz_offset].instance_tree,
                pcb->spm_classify_id_node);
        }
    }

    /* ����ģʽ */
    else
    {
        if(NULL != AVLL_FIND(pB->instance_tree,&(pcb->key)))
        {
            AVLL_DELETE(pB->instance_tree,pcb->spm_classify_id_node);
        }
    }

   /********************************************************* 
    * ɾ��policy����ģ����cb�Ե���Ϣ,����: �ͷ�intf_offset��*
    * ���ü���ֵ�Լ���ɾ��������behavior��classify�����õ�. *
    *********************************************************/
    spm_release_policy_acl_drive(pB,(pcb->acl_id),(pcb->intf_offset),
            (pcb->sz_offset) NBB_CCXT);

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_add_logic_flow_classify_node(NBB_ULONG key,
    ATG_DCI_LOG_PORT_INCLASSIFY_QOS *classify_qos, SUB_PORT *sub_port NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_CLASSIFY_CB *join_node = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    
    //SPM_QOS_POLICY_CB *pB = NULL;
    //SPM_QOS_POLICY_CB *pB_old = NULL;
    NBB_ULONG index = 0;
    SPM_QOS_LOGIC_INTF_KEY log_key = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == key) || (NULL == classify_qos) || (NULL == sub_port))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,%d log_key=%ld,unit=%d,slot=%d,port=%d,posid=%ld,policy_index=%ld,"
               "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
               key,sub_port->unit,sub_port->slot,sub_port->port,sub_port->posid,
               classify_qos->qos_policy_index,classify_qos->svlan);
        OS_SPRINTF(ucMessage,"%s %s,%d log_key=%ld,unit=%d,slot=%d,port=%d,posid=%ld,policy_index=%ld,"
                   "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,key,sub_port->unit,
                   sub_port->slot,sub_port->port,sub_port->posid,classify_qos->qos_policy_index,
                   classify_qos->svlan);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
    if((0 != sub_port->slot) && (sub_port->slot != v_spm_shared->local_slot_id))
    {
        goto EXIT_LABEL;
    }
    index = classify_qos->qos_policy_index;
    log_key.svlan = classify_qos->svlan;
    log_key.cvlan = classify_qos->cvlan;
    log_key.index = key;

    /* ���Ҹ�intf���Ƿ��Ѿ�����policy���� */
    join_node = AVLL_FIND(v_spm_shared->qos_flow_classify_tree, &log_key);

    /* ���� */
    if(NULL == join_node)
    {
        /* ɾ�������ڵ����� */
        if(0 == index)
        {
            goto EXIT_LABEL;
        }
        
       /************************************************************************** 
        * ����intf�ϻ�����������ڵ�.��intf��policy����ģ��󶨳ɹ���,�Ὣ�ýڵ� *
        * �ֱ����intf���ϻ������������������(qos_flow_classify_tree)��policy�� *
        * ��ģ���������(c2b_instance_tree[i].instance_tree)��.                  *
        **************************************************************************/
        join_node = spm_alloc_qos_flow_classify_cb(&log_key NBB_CCXT);
        if (NULL == join_node)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        OS_MEMCPY(&(join_node->sub_port),sub_port,sizeof(SUB_PORT));
        OS_MEMCPY(&(join_node->qos_cfg),classify_qos,sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));

        /* ��policy����ģ��д���ڴ��������,д��ɹ�������intf�� */
        ret = spm_add_flow_classify_drive(join_node NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_free_flow_classify_cb(join_node NBB_CCXT);
            join_node = NULL;
            goto EXIT_LABEL;
        }

       /******************************************************************** 
        * policy����ģ����intf�󶨳ɹ���,���������ذ���Ϣ���ϻ�������  *
        * ����ڵ�(SPM_QOS_FLOW_CLASSIFY_CB)���뵽intf���ϻ�������������� *
        * ����(qos_flow_classify_tree)��,��������policy���Եİ󶨹��̽���. *
        ********************************************************************/

        //coverity[no_effect_test]
        rv = AVLL_INSERT(SHARED.qos_flow_classify_tree, join_node->spm_qos_flow_classify_node);    
    }
    else 
    {
        /* policy����ģ������ֵΪ0,��ʾɾ������ */
        if(0 == index)
        {
            spm_del_flow_classify_drive(join_node NBB_CCXT);
            AVLL_DELETE(SHARED.qos_flow_classify_tree, join_node->spm_qos_flow_classify_node);
            spm_free_flow_classify_cb(join_node NBB_CCXT);
            join_node = NULL;
        }

        /* ������ͬ */
        else if(join_node->qos_cfg.qos_policy_index == index)
        {
        }

        /* ���ò�ͬ,���²��� */
        else
        {
            spm_del_flow_classify_drive(join_node NBB_CCXT);
            OS_MEMCPY(&(join_node->qos_cfg),classify_qos,
                sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
            spm_add_flow_classify_drive(join_node NBB_CCXT);
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#else


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_add_flow_classify(SPM_QOS_POLICY_CB *pB, SUB_PORT *sub_port NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_BYTE unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != sub_port);
    NBB_ASSERT(NULL != pB);

    if ((NULL == sub_port) || (NULL == pB))
    {
        printf("**QOS ERROR**%s,%d,key==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_add_flow_classify",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0));        
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ģʽ���� */
    if (0 == (pB->basic_cfg_cb->mode))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }
    if(0 != sub_port->slot)
    {
#if defined (SPU) || defined (PTN690_CES)
       ret = ApiC3SetIntfIngressAcl(sub_port->unit,sub_port->posid,1,pB->acl_id);
       if (ATG_DCI_RC_OK != ret)
       {
            printf("**ERROR**acl id=%ld, ret =%ld ApiC3SetIntfIngressAcl",pB->acl_id, ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS set acl",ret,"AclId","unit","posid","policy index",
    					   pB->acl_id,sub_port->unit,sub_port->posid,pB->policy_key));
            goto EXIT_LABEL;
       } 
#endif
    }
    else
    {
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
           ret = ApiC3SetIntfIngressAcl(unit,sub_port->posid,1,pB->acl_id);
           if (ATG_DCI_RC_OK != ret)
           {
                printf("**ERROR**acl id=%ld, ret =%ld ApiC3SetIntfIngressAcl",pB->acl_id, ret);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        					   "QOS set acl",ret,"AclId","unit","posid","policy index",
        					   pB->acl_id,unit,sub_port->posid,pB->policy_key));
                goto EXIT_LABEL;
           } 
#endif
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_del_flow_classify(SPM_QOS_POLICY_CB *pB, SUB_PORT *sub_port NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_BYTE unit = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    NBB_ASSERT(NULL != pB);

    if (NULL == pB)
    {
        printf("**QOS ERROR**%s,%d,cfg==null\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "QOS spm_del_flow_classify",ATG_DCI_RC_UNSUCCESSFUL,
					   "","","","",0,0,0,0)); 
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    /* ģʽ���� */
    if (0 == pB->basic_cfg_cb->mode)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    if(0 != sub_port->slot)
    {
#if defined (SPU) || defined (PTN690_CES)
       ret = ApiC3SetIntfIngressAcl(sub_port->unit,sub_port->posid,0,pB->acl_id);
       if (ATG_DCI_RC_OK != ret)
       {
            printf("**ERROR**del flow classify acl id=%ld, ret =%ld ApiC3SetIntfIngressAcl",pB->acl_id, ret);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS set del flow classify",ret,
    					   "AclId","unit","posid","policy index",
    					   pB->acl_id,sub_port->unit,sub_port->posid,pB->policy_key));
            goto EXIT_LABEL;
       } 
#endif
    }
    else
    {
        for(unit = 0;unit < SHARED.c3_num;unit++)
        {
#if defined (SPU) || defined (PTN690_CES)
           ret = ApiC3SetIntfIngressAcl(unit,sub_port->posid,0,pB->acl_id);
           if (ATG_DCI_RC_OK != ret)
           {
                printf("**ERROR**del flow classify acl id=%ld, ret =%ld ApiC3SetIntfIngressAcl",pB->acl_id, ret);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
                			   "QOS set del flow classify",ret,
                			   "AclId","unit","posid","policy index",
                			   pB->acl_id,unit,sub_port->posid,pB->policy_key));
                goto EXIT_LABEL;
           }
#endif
        }
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_add_logic_flow_classify_node(NBB_ULONG key,
    ATG_DCI_LOG_PORT_INCLASSIFY_QOS *classify_qos, SUB_PORT *sub_port NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_CLASSIFY_CB *join_node = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv  = ATG_DCI_RC_OK;
    SPM_QOS_POLICY_CB *pB = NULL;
    SPM_QOS_POLICY_CB *pB_old = NULL;
    NBB_ULONG index = 0;
    SPM_QOS_LOGIC_INTF_KEY log_key = {0};
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == key) || (NULL == classify_qos) || (NULL == sub_port))
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
        printf("%s %s,%d log key=%ld,port=%d,posid=%ld,qos_policy_index=%ld,"
                  "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
                  key,sub_port->port,sub_port->posid,classify_qos->qos_policy_index,
                  classify_qos->svlan);
        OS_SPRINTF(ucMessage,"%s %s,%d log key=%ld,port=%d,posid=%ld,qos_policy_index=%ld,"
                   "svlan=%d\n\n",QOS_CFG_STRING,__FUNCTION__,__LINE__,
                   key,sub_port->port,sub_port->posid,classify_qos->qos_policy_index,
                   classify_qos->svlan);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
    if((0 != sub_port->slot) && (sub_port->slot != v_spm_shared->local_slot_id))
    {
        goto EXIT_LABEL;
    }
    index = classify_qos->qos_policy_index;
    log_key.svlan = classify_qos->svlan;
    log_key.cvlan = classify_qos->cvlan;
    log_key.index = key;
    join_node = AVLL_FIND(SHARED.qos_flow_classify_tree, &log_key);

    /***************************************************************************/
    /*                                  ���Ӳ���                               */
    /***************************************************************************/
    if ((NULL == join_node) && (0 != index))
    {
        join_node = spm_alloc_qos_flow_classify_cb(&log_key NBB_CCXT);
        if (NULL == join_node)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        pB = spm_qos_find_policy_cb(index NBB_CCXT);
        if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
        {
            spm_free_flow_classify_cb(join_node NBB_CCXT);
            join_node = NULL;
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        ret = spm_add_flow_classify(pB, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            ret = spm_free_flow_classify_cb(join_node NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                goto EXIT_LABEL;
            }
            join_node = NULL;
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        join_node->acl_id = pB->acl_id;
        OS_MEMCPY(&(join_node->sub_port),sub_port,sizeof(SUB_PORT));
        
        //coverity[no_effect_test]
        rv = AVLL_INSERT(SHARED.qos_flow_classify_tree, join_node->spm_qos_flow_classify_node);
        rv = AVLL_INSERT(pB->instance_tree, join_node->spm_classify_id_node);
        goto EXIT_LABEL;
    }

    /* ���� */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index != index) && (0 != index))
    {           
        pB = spm_qos_find_policy_cb(index NBB_CCXT);
        if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        ret = spm_add_flow_classify(pB, sub_port NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            goto EXIT_LABEL;
        }    
        pB_old = spm_qos_find_policy_cb(join_node->policy_index NBB_CCXT);
        if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        join_node->policy_index = index;
        join_node->acl_id = pB->acl_id;
        if(NULL != AVLL_FIND(pB_old->instance_tree, &log_key))
        {
            AVLL_DELETE(pB_old->instance_tree, join_node->spm_classify_id_node);
        } 
        
        //coverity[no_effect_test]
        rv = AVLL_INSERT(pB->instance_tree, join_node->spm_classify_id_node);
        goto EXIT_LABEL;
    }

    /* ��ͬ������ */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && 
            (join_node->policy_index == index) && (0 != index))
    {
        printf("**QOS ERROR**%s,%d,the same cfg\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					   "logic flow classify the same cfg",ATG_DCI_RC_OK,
					   "index","policy_index","key","",
					   index,join_node->policy_index,key,0));
        ret = ATG_DCI_RC_OK;
        goto EXIT_LABEL;
    }

    /* ɾ������ */
    else if ((NULL != join_node) && (0 != join_node->policy_index) && (0 == index))
    {
        pB = spm_qos_find_policy_cb(join_node->policy_index NBB_CCXT);
        if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }
        spm_del_flow_classify(pB,sub_port NBB_CCXT);
        AVLL_DELETE(SHARED.qos_flow_classify_tree, join_node->spm_qos_flow_classify_node);
        if(NULL != AVLL_FIND(pB->instance_tree, &log_key))
        {
            AVLL_DELETE(pB->instance_tree, join_node->spm_classify_id_node);
        }
        spm_free_flow_classify_cb(join_node NBB_CCXT);
        join_node = NULL;
        goto EXIT_LABEL;
    }

    /* ��������� */
    else 
    {
        printf("**QOS ERROR**%s,%d,logic flow classify config missmatch\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS logic flow classify config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
						"policy index","svlan","logic index",
						(NULL == join_node)?"no cfg cvlan=":"find cfg cvlan=",
						index,log_key.svlan,key,log_key.cvlan));
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        goto EXIT_LABEL;
    }

    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

#endif

/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_qos_clear_logic_classify(NBB_LONG logic_key NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    SPM_QOS_FLOW_CLASSIFY_CB *cfg_cb = NULL;
    SPM_QOS_FLOW_CLASSIFY_CB *next_cfg_cb = NULL;
    SPM_QOS_POLICY_CB *pB = NULL;

    //NBB_TRC_ENTRY(__FUNCTION__);

    cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*) AVLL_FIRST(SHARED.qos_flow_classify_tree);
    while(cfg_cb != NULL)
    {
        next_cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*)AVLL_NEXT(     \
                        SHARED.qos_flow_classify_tree,cfg_cb->spm_qos_flow_classify_node);

        if(cfg_cb->key.index != logic_key)
        {
            cfg_cb = next_cfg_cb;
            continue;
        }

        pB = spm_qos_find_policy_cb(cfg_cb->policy_index NBB_CCXT);
        if(NULL != pB)
        {
#ifdef PTN690
            spm_del_flow_classify_drive(cfg_cb NBB_CCXT);
#else
            if(NULL != AVLL_FIND(pB->instance_tree, &(cfg_cb->key)))
            {
                AVLL_DELETE(pB->instance_tree, cfg_cb->spm_classify_id_node);
            }
#endif
        }
        AVLL_DELETE(SHARED.qos_flow_classify_tree, cfg_cb->spm_qos_flow_classify_node);
        spm_free_flow_classify_cb(cfg_cb NBB_CCXT);  
        cfg_cb = next_cfg_cb;
    }

    //EXIT_LABEL: NBB_TRC_EXIT();
    return ret;  
}


/*****************************************************************************
   �� �� ��  : spm_find_policy_cb
   ��������  : ����policyģ���������
   �������  : policyģ���index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_VOID spm_qos_clear_all_logic_classify(NBB_CXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_FLOW_CLASSIFY_CB *cfg_cb = NULL;

#if 0
    SPM_QOS_POLICY_CB *pB = NULL;
#endif

    for(cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*) AVLL_FIRST(v_spm_shared->qos_flow_classify_tree);cfg_cb != NULL;
        cfg_cb = (SPM_QOS_FLOW_CLASSIFY_CB*) AVLL_FIRST(v_spm_shared->qos_flow_classify_tree))
    {
#if 0
        pB = spm_qos_find_policy_cb(cfg_cb->policy_index NBB_CCXT);
        if (NULL == pB)
        {
            printf("**QOS ERROR**%s line=%d logic_key=%d policy_index=%d can't find policy\n",
                      __FUNCTION__,__LINE__,cfg_cb->key.index,cfg_cb->policy_index);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					   "QOS can't find policy cfg",ATG_DCI_RC_UNSUCCESSFUL,
    					   "logic_key","policy index","posid","acl_id",
    					   cfg_cb->key.index,cfg_cb->policy_index,
    					   cfg_cb->sub_port.posid,cfg_cb->acl_id));
        }
        else
        {
#ifdef  PTN690
            if(AVLL_IN_TREE(cfg_cb->spm_classify_id_node))
            {
                /* �ǹ���ģʽ */
                if(0 == pB->basic_cfg_cb->share)
                {
                    AVLL_DELETE(pB->c2b_instance_tree[cfg_cb->sz_offset].instance_tree,
                                cfg_cb->spm_classify_id_node);
                }

                /* ����ģʽ */
                else
                {
                    AVLL_DELETE(pB->instance_tree, cfg_cb->spm_classify_id_node);
                }
            }
#else
            if(NULL != AVLL_IN_TREE(cfg_cb->spm_classify_id_node))
            {
                AVLL_DELETE(pB->instance_tree, cfg_cb->spm_classify_id_node);
            }
#endif
            else
            {
                printf("**QOS ERROR**%s line=%d logic_key=%d,policy_index=%d,policy_share=%d,sz_offset=%d:can't "
                       "find node in tree of instance_tree.\n",__FUNCTION__,__LINE__,cfg_cb->key.index,
                       cfg_cb->policy_index,pB->basic_cfg_cb->share,cfg_cb->sz_offset);
                NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
        					   "QOS can't find policy instance_tree node",ATG_DCI_RC_UNSUCCESSFUL,
        					   "logic_key","policy index","policy_share","sz_offset",
        					   cfg_cb->key.index,cfg_cb->policy_index,
        					   pB->basic_cfg_cb->share,cfg_cb->sz_offset));
                continue;
            }
        }
#endif      
        AVLL_DELETE(v_spm_shared->qos_flow_classify_tree, cfg_cb->spm_qos_flow_classify_node);
        spm_free_flow_classify_cb(cfg_cb NBB_CCXT);
    }

    return; 
}

#endif

