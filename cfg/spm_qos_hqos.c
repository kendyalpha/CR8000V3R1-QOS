/******************************************************************************

                  ��Ȩ���� (C), 1999-2013, ���ͨ�ſƼ��ɷ����޹�˾

******************************************************************************
   �� �� ��   : spm_qos_proc.c
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
#include <TmApi.h>

/* ȫ�ִ�ӡ����qos_vc_cfg_print */
extern NBB_BYTE qos_vc_cfg_print;

/* ȫ�ִ�ӡ����qos_vrf_cfg_print */
extern NBB_BYTE qos_vrf_cfg_print;

/* ȫ�ִ�ӡ����qos_txlsp_cfg_print */
extern NBB_BYTE qos_txlsp_cfg_print;

/* ȫ�ִ�ӡ����qos_log_cfg_print */
extern NBB_BYTE qos_log_cfg_print;


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ���������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_checkslot(NBB_LONG slot, NBB_LONG *slot_index
    NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_BYTE i = 0;

    for (i = 0; i < MAX_HQOS_SLOT_NUM; i++)
    {
        /*hqos_port_index[i]:֧��hqos�Ĳ�λ��*/
        if (v_spm_shared->hqos_port_index[i] == slot)
        {
            if (NULL != slot_index)
            {
                *slot_index = i;
            }

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;

    /*�쳣����*/
    EXIT_LABEL:
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ���������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_create_voq(NBB_USHORT slot, NBB_LONG port,
    NBB_ULONG node_index, NBB_ULONG pwid NBB_CCXT_T NBB_CXT)
{
    NBB_BYTE fapid = 0; /*����λ*/
    NBB_LONG fapid_index = 0; /*��0��ʼ*/
    NBB_LONG baseQueue = 0;
    NBB_LONG baseVCId = 0;
    int rv = ATG_DCI_RC_OK;
    NBB_LONG slot_index = 0; /*��0��ʼ*/
#if defined (SPU) || defined (SRC)
    NBB_LONG unit = 0;
#endif

    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ò���ֻ�ڳ�����ִ��*/
    if(slot != SHARED.local_slot_id)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
    
    /*��鱾���ڵĶ˿��Ƿ���Ч:��Ч-1,��Ч-0*/
#if defined (SPU) || defined (SRC)
    ApiAradCheckPortValid(unit, port + PTN_690_PORT_OFFSET, &rv);
    if (0 == rv) /*�˿ڲ���������*/
    {
        printf("**QOS ERROR** %s %d ApiAradCheckPortValid port =%ld is unvalid\n",
                    __FUNCTION__,__LINE__,port);
        rv = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
#endif

    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    rv = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if(ATG_DCI_RC_OK != rv)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    rv = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if(ATG_DCI_RC_OK != rv)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    for(fapid_index = 0; fapid_index < MAX_HQOS_SLOT_NUM; fapid_index++)
    {
        if(0 == SHARED.hqos_port_index[fapid_index])
        {
            continue; /*��HQOS��λ*/
        }
        
        /*�˿�8���ȼ���ʼ����ʱ���Ѿ���������*/
#if 0
#if defined (SPU) || defined (SRC)
        rv = ApiSetupBaseSeEtm(0, port);
#endif

        if (rv != 0)         /*ִ�н���ж�*/
        {
            NBB_TRC_FLOW(("Error! ApiSetupBaseSeEtm err: %d(port=%ld)\n", rv, port));
            goto EXIT_LABEL;
        }
#endif
        baseQueue = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;
        baseVCId = offset * node_index + fapid_index * NUM_COS + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

        /*�ڳ��ڶ˴���VC���Ϳ�����hqos���صĵ�����ڶ�VOQ��������*/
        rv = ApiAradHqosTrafficSet(unit, baseQueue, NUM_COS, SHARED.hqos_port_index[fapid_index], 
                    port + PTN_690_PORT_OFFSET, baseVCId, pwid);
#endif

        if (rv != 0)    /*ִ�н���ж�*/
        {
            spm_hqos_arad_traffic_set_error_log(baseQueue,
                SHARED.hqos_port_index[fapid_index], port + PTN_690_PORT_OFFSET,
                baseVCId, pwid,__FUNCTION__,__LINE__,rv NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return rv;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_del_voq(NBB_USHORT slot, NBB_USHORT port,
    NBB_ULONG node_index, NBB_LONG pw_id NBB_CCXT_T NBB_CXT)
{
    NBB_BYTE fapid = 0; /*����λ*/
    NBB_LONG fapid_index = 0; /*��0��ʼ*/
    NBB_ULONG baseQueue = 0;
    NBB_ULONG baseVCId = 0;
    int rv = 0;
    NBB_LONG slot_index = 0; /*��0��ʼ*/
#if defined (SPU) || defined (SRC)
    NBB_LONG unit = 0;
#endif

    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;

    NBB_TRC_ENTRY(__FUNCTION__);

    if(slot != SHARED.local_slot_id)
    {
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
#if defined (SPU) || defined (SRC)
    ApiAradCheckPortValid(unit, port + PTN_690_PORT_OFFSET, &rv);
    if (0 == rv) /*�˿ڲ���������*/
    {
        rv = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
#endif

    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    rv = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != rv)
    {
        rv = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    rv = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != rv)
    {
        rv = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    for(fapid_index = 0; fapid_index < MAX_HQOS_SLOT_NUM; fapid_index++)
    {
        if(0 == SHARED.hqos_port_index[fapid_index])
        {
            continue;/*��HQOS��λ*/
        }
        
        /*�˿�8���ȼ���ʼ����ʱ���Ѿ���������*/
#if 0
#if defined (SPU) || defined (SRC)
        rv = ApiSetupBaseSeEtm(0, port);
#endif

        if (rv != 0)         /*ִ�н���ж�*/
        {
            NBB_TRC_FLOW(("Error! ApiSetupBaseSeEtm err: %d(port=%ld)\n", rv, port));
            goto EXIT_LABEL;
        }
#endif
        baseQueue = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;
        baseVCId = offset * node_index + fapid_index * NUM_COS + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

        /*�ڳ��ڶ�ɾ��������VC��ɾ������ڶ�VOQ�����Ĺ���*/
        rv = ApiAradHqosTrafficDelete(unit, baseQueue, NUM_COS, SHARED.hqos_port_index[fapid_index], 
                    port + PTN_690_PORT_OFFSET, baseVCId, pw_id);
#endif
        if (rv != 0)    /*ִ�н���ж�*/
        {
           spm_hqos_arad_traffic_del_error_log(baseQueue,
            SHARED.hqos_port_index[fapid_index],
            port + PTN_690_PORT_OFFSET, baseVCId, pw_id,
            __FUNCTION__,__LINE__,rv NBB_CCXT);

           /*�쳣����*/
           goto EXIT_LABEL;
        }
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return rv;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_apply_lsp_id(NBB_ULONG *pid)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*����LSP ID*/
    for (id = MAX_PHYSIC_PORT_NUM; id < HQOS_MAX_LSP_ID; id++)
    {
        if (ATG_DCI_RC_OK == SHARED.hqos_lsp_pool[id]) /*�п���ID*/
        {
            *pid = id + 1;
            SHARED.hqos_lsp_pool[id] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;
    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS apply lspid",ATG_DCI_RC_UNSUCCESSFUL,
						"id","","","",
						id,0,0,0));

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_apply_pw_id(NBB_ULONG *pid)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG id = 0;
    NBB_ULONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*����PW ID*/
    for (id = 0; id < HQOS_MAX_PW_ID; id++)
    {
        if (ATG_DCI_RC_OK == SHARED.hqos_pw_pool[id]) /*�п���ID*/
        {
            *pid = id + 1;
            SHARED.hqos_pw_pool[id] = ATG_DCI_RC_UNSUCCESSFUL;
            ret = ATG_DCI_RC_OK;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    ret = ATG_DCI_RC_UNSUCCESSFUL;
    NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS apply pwid",ATG_DCI_RC_UNSUCCESSFUL,
						"id","","","",
						id,0,0,0));

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_release_lsp_id(NBB_ULONG pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*��Դ��ռ�ÿ����ͷ�*/
    if (MAX_PHYSIC_PORT_NUM > pid)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (ATG_DCI_RC_OK != SHARED.hqos_lsp_pool[pid - 1])
    {
        SHARED.hqos_lsp_pool[pid - 1] = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else
    {
        printf("**QOS ERROR** %s %d lsp id is not in use\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS release lspid",ATG_DCI_RC_UNSUCCESSFUL,
						"id","","","",
						pid,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_release_pw_id(NBB_ULONG pid NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_ULONG ret = ATG_DCI_RC_OK;

    NBB_TRC_ENTRY(__FUNCTION__);

    /*��Դ��ռ�ÿ����ͷ�*/
    if (0 == pid)
    {
        
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS release pwid",ATG_DCI_RC_UNSUCCESSFUL,
						"id","","","",
						pid,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (ATG_DCI_RC_OK != SHARED.hqos_pw_pool[pid - 1])
    {
        SHARED.hqos_pw_pool[pid - 1] = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else
    {
        printf("**QOS ERROR** %s %d pw id is not in use\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS release pwid",ATG_DCI_RC_UNSUCCESSFUL,
						"id","","","",
						pid,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return p_group;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : ɾ�������̵�VC
   �������  : usr group��index
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_QOS_USER_GROUP_CB * spm_check_group_down_cb(NBB_ULONG index,
    NBB_USHORT slot, NBB_USHORT port, NBB_ULONG *policy_index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_QOS_USER_GROUP_CB *p_group = NULL;

    NBB_TRC_ENTRY("spm_find_group_cb");

    if (0 == index) /*�Ƿ�*/
    {
        printf("**QOS ERROR** %s %d group index = 0\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    p_group = AVLL_FIND(SHARED.qos_user_group_tree, &index);
    if (NULL == p_group)
    {
        printf("**QOS ERROR** %s %d can't find USR GROUP CFG =%ld\n",
                    __FUNCTION__,__LINE__,index);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (NULL == p_group->down_cfg_cb)
    {
        printf("**QOS ERROR** %s %d can't find USR GROUP DOWN CFG =%ld\n",
                    __FUNCTION__,__LINE__,index);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if ((0 != p_group->port) && (port != p_group->port))
    {
        printf("**QOS ERROR** %s %d group port is not the same\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if ((0 != p_group->slot) && (slot != p_group->slot))
    {
        printf("**QOS ERROR** %s %d group slot is not the same\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    *policy_index = p_group->down_cfg_cb->qos_policy_index;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return p_group;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �����ڴ沢����lsp id ��ʼ��HQOS��
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK == defaulte)
    {
        /*������е�lsp_id*/
        ret = spm_hqos_apply_lsp_id(&id NBB_CCXT);

        /*����lspidʧ��*/
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d spm_hqos_apply_lsp_id failed\n",
                        __FUNCTION__,__LINE__);

            /*�쳣����*/
            goto EXIT_LABEL;
        }   
    }

    /* ����һ���µ�lsp_tx���ÿ�qos��������Ŀ��*/
    pstTbl = (SPM_HQOS_LSP_TX_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_LSP_TX_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_LSP_TX_CB);

    /*�����ڴ�ʧ��*/
    if (pstTbl == NULL)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ʼlsp_tx���ÿ�������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_LSP_TX_CB));
    OS_MEMCPY(&pstTbl->lsp_tx_key, pkey, sizeof(SPM_QOS_TUNNEL_KEY));
    pstTbl->lsp_id = id;

    /*QOS vc ����ʼ��*/
    avll_key_offset = NBB_OFFSETOF(SPM_HQOS_VC_CB, vc_key);
    AVLL_INIT_TREE(pstTbl->vc_tree, spm_vc_key_compare,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_HQOS_VC_CB, spm_hqos_vc_node));

    /*QOS vrf ����ʼ��*/
    avll_key_offset = NBB_OFFSETOF(SPM_HQOS_VRF_CB, vrf_key);
    AVLL_INIT_TREE(pstTbl->vrf_tree, spm_qos_vrf_comp,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_HQOS_VRF_CB, spm_hqos_vrf_node));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_lsp_tx_node);

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �����ڴ��ͬʱ����PW ID
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }
    ret = spm_hqos_apply_pw_id(&id NBB_CCXT);

    /*����pwidʧ��*/
    if (ATG_DCI_RC_OK != ret)
    {
        printf("**QOS ERROR** %s %d spm_hqos_apply_pw_id failed\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ����һ���µ��߼����ÿ�qos��������Ŀ��*/
    pstTbl = (SPM_HQOS_VC_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_VC_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_VC_CB);

    /*�����ڴ�ʧ��*/
    if (pstTbl == NULL)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ʼ�߼����ÿ�������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_VC_CB));
    OS_MEMCPY(&(pstTbl->vc_key), pkey, sizeof(ATG_DCI_VC_KEY));
    pstTbl->pw_id = id;

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_vc_node);

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �����ڴ沢����lsp id ��ʼ��HQOS��
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK == defaulte)
    {
        ret = spm_hqos_apply_pw_id(&id NBB_CCXT);

        /*����pwidʧ��*/
        if (ATG_DCI_RC_OK != ret)
        {
            printf("**QOS ERROR** %s %d spm_hqos_apply_pw_id failed\n",
                        __FUNCTION__,__LINE__);

            /*�쳣����*/
            goto EXIT_LABEL;
        }   
    }
    

    /* ����hqos vrf ��A�ڵ�*/
    pstTbl = (SPM_HQOS_VRF_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_VRF_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_VRF_CB);

    /*�����ڴ�ʧ��*/
    if (pstTbl == NULL)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ʼhqos vrf vpn������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_VRF_CB));
    OS_MEMCPY(&(pstTbl->vrf_key), pkey, sizeof(SPM_QOS_VRF_INSTANSE_KEY));
    pstTbl->pw_id = id;

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_vrf_node);

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �����ڴ沢����lsp id ��ʼ��HQOS��
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_HQOS_LOG_GROUP_CB *spm_alloc_hqos_log_group_cb(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LOG_GROUP_CB *pstTbl = NULL;
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_ULONG id = 0;
    NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* �������ָ�������Ч */
    NBB_ASSERT(index != 0);

    ret = spm_hqos_apply_lsp_id(&id NBB_CCXT);

    /*����lspidʧ��*/
    if (ATG_DCI_RC_OK != ret)
    {
        printf("**QOS ERROR** %s %d spm_hqos_apply_lsp_id failed\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ����һ���µ�log group���ÿ�qos��������Ŀ��*/
    pstTbl = (SPM_HQOS_LOG_GROUP_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_LOG_GROUP_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_LOG_GROUP_CB);

    /*�����ڴ�ʧ��*/
    if (pstTbl == NULL)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ʼlog group���ÿ�������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_LOG_GROUP_CB));
    pstTbl->index = index;
    pstTbl->lsp_id = id;

    /*QOS USR ����ʼ��*/
    avll_key_offset = NBB_OFFSETOF(SPM_HQOS_LOG_USR_CB, index);
    AVLL_INIT_TREE(pstTbl->usr_tree, compare_ulong,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_HQOS_LOG_USR_CB, spm_hqos_usr_node));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTbl->spm_hqos_group_node);


    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �����ڴ��ͬʱ����PW ID
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
SPM_HQOS_LOG_USR_CB *spm_alloc_hqos_log_usr_cb(NBB_ULONG index NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LOG_USR_CB *pstTbl = NULL;
    NBB_ULONG ret = ATG_DCI_RC_OK;
    NBB_ULONG id = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    /* �������ָ�������Ч */
    NBB_ASSERT(index != 0);

    ret = spm_hqos_apply_pw_id(&id NBB_CCXT);

    /*����pwidʧ��*/
    if (ATG_DCI_RC_UNSUCCESSFUL == ret)
    {
        printf("**QOS ERROR** %s %d spm_hqos_apply_pw_id failed\n",
                    __FUNCTION__,__LINE__);

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ����һ���µ��߼����ÿ�qos��������Ŀ��*/
    pstTbl = (SPM_HQOS_LOG_USR_CB *)NBB_MM_ALLOC(sizeof(SPM_HQOS_LOG_USR_CB),
        NBB_NORETRY_ACT, MEM_SPM_HQOS_LOG_USR_CB);

    /*�����ڴ�ʧ��*/
    if (pstTbl == NULL)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ʼ�߼����ÿ�������Ŀ */
    OS_MEMSET(pstTbl, 0, sizeof(SPM_HQOS_LOG_USR_CB));
    pstTbl->index = index;
    pstTbl->pw_id = id;
    AVLL_INIT_NODE(pstTbl->spm_hqos_usr_node);

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return(pstTbl);
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_hqos_lsp_tx_cb(SPM_HQOS_LSP_TX_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ͷ��ڴ�ʧ��*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free lsp",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LSP_TX_CB), MEM_SPM_HQOS_LSP_TX_CB);
    ret = spm_hqos_release_lsp_id(pstTbl->lsp_id NBB_CCXT);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LSP_TX_CB);
    pstTbl = NULL;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_hqos_vc_cb(SPM_HQOS_VC_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ͷ��ڴ�ʧ��*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free vc",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_VC_CB), MEM_SPM_HQOS_VC_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_VC_CB);
    pstTbl = NULL;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_hqos_vrf_cb(SPM_HQOS_VRF_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ͷ��ڴ�ʧ��*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free vrf",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_VRF_CB), MEM_SPM_HQOS_VRF_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);


    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_VRF_CB);
    pstTbl = NULL;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_hqos_log_group_cb(SPM_HQOS_LOG_GROUP_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ͷ��ڴ�ʧ��*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free group",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LOG_GROUP_CB), MEM_SPM_HQOS_LOG_GROUP_CB);
    spm_hqos_release_lsp_id(pstTbl->lsp_id NBB_CCXT);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LOG_GROUP_CB);
    pstTbl = NULL;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_free_hqos_log_usr_cb(SPM_HQOS_LOG_USR_CB *pstTbl NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_TRC_ENTRY(__FUNCTION__);

    /*�ͷ��ڴ�ʧ��*/
    if (NULL == pstTbl)
    {
        printf("**QOS ERROR** %s %d pstTbl == NULL\n",
                    __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS free usr",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /***************************************************************************/
    /* �����ƿ����ȷ�ԡ�                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTbl, sizeof(SPM_HQOS_LOG_USR_CB), MEM_SPM_HQOS_LOG_USR_CB);
    spm_hqos_release_pw_id(pstTbl->pw_id NBB_CCXT);

    /***************************************************************************/
    /* �����ͷŵ�����Ϣ���ƿ顣                                                */
    /***************************************************************************/
    NBB_MM_FREE(pstTbl, MEM_SPM_HQOS_LOG_USR_CB);
    pstTbl = NULL;

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        ret = spm_connect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        ret = spm_connect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_connect_wred_cnt(cfg->yellow_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_connect_wred_cnt(cfg->red_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : defaulte = ATG_DCI_RC_OK:����LSP ID
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        ret = spm_disconnect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {
        
            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        ret = spm_disconnect_wred_cnt(cfg->green_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {
        
            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_disconnect_wred_cnt(cfg->yellow_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_disconnect_wred_cnt(cfg->red_wred_id NBB_CCXT);

        /*���ò�ͬ�ķ���ֵ*/
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : slot -- ���ڲ�λ
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    voq = slot_index * NUM_COS + offset * node_index + HQOS_OFFSET;

    /*��β����*/
    if (WRED_MODE_QUE_DROP == cfg->mode)
    {
#if defined (SPU) || defined (SRC)
        ApiAradSetQueueTailDrop(UNIT_0, voq, cfg->queue_id, -1, 10 * 1024 * 1024);
        ret = ApiSetQueueWred(UNIT_0,voq,cfg->queue_id,0,-1,100,100,0);
#endif
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*ɫäģʽ*/
    else if (WRED_MODE_COL_BIND == cfg->mode)
    {
        green_cb = spm_qos_find_wred_cb(cfg->green_wred_id NBB_CCXT);
        if (NULL == green_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (NULL == green_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find green wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /*������ɫwred*/
    else if (WRED_MODE_COLOR == cfg->mode)
    {
        green_cb = spm_qos_find_wred_cb(cfg->green_wred_id NBB_CCXT);
        if (NULL == green_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (NULL == green_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find green wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        yellow_cb = spm_qos_find_wred_cb(cfg->yellow_wred_id NBB_CCXT);
        if (NULL == yellow_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (NULL == yellow_cb->basic_cfg_cb)
        {
            printf("**QOS ERROR** %s %d not find yellow wred basic cfg\n",
                    __FUNCTION__,__LINE__);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        red_cb = spm_qos_find_wred_cb(cfg->red_wred_id NBB_CCXT);
        if (NULL == red_cb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }


    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : slot -- ���ڲ�λ
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /*�쳣����*/
    NBB_TRC_EXIT();
    return Counter;
}

/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : slot -- ���ڲ�λ
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

                    /*�쳣����*/
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

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : slot -- ���ڲ�λ
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(index != 0);

    /*ָ��Ϊ��*/
    if (0 == index)
    {
        printf("**QOS ERROR**%s,%d policy index ==0\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS policy index ==0",ATG_DCI_RC_UNSUCCESSFUL,
    					"","","","",
    					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pB = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(index NBB_CCXT);

    /*policy���ò�����*/
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
    {
        printf("**QOS ERROR**%s,%d can't find policy or basic_cfg\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS can't find policy or basic_cfg",ATG_DCI_RC_UNSUCCESSFUL,
    					"policy index","","","",
    					index,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (0 != pB->basic_cfg_cb->mode) /*�ǵ��Ƚڵ�*/
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pA = spm_qos_find_action_cb(c2b->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION������*/
    if (NULL == pA)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*����Ǳ���λ����н���*/
    {
        if (ATG_DCI_RC_OK != upflag) /*�״δ���*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspCreate(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != ret) /*ʧ��*/
        {
            spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,ret NBB_CCXT);
            spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET, pcb->lsp_id,
                    __FUNCTION__,__LINE__,ret NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (pA->shaping_cfg_cb) /*��������*/
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
        if ((pA->schedule_cfg_cb) && (0 != pA->schedule_cfg_cb->mode)) /*���Ȳ���*/
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
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*��һ����tlv���ô�����ɾ��LSP*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }

        /*ֻ�Գ����������ü������������ʧ�ܣ����������޷��·�����*/
        (pB->cnt)++;
        (pA->cnt)++;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(plsp != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(data != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->flow_queue_qos_policy_index NBB_CCXT);

    /*policy���ò�����*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�ǵ��Ƚڵ�*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*����policy������action*/
    c2b_1 = AVLL_FIRST(pB1->c2b_tree);
    c2b_2 = AVLL_FIRST(pB2->c2b_tree);
    if ((NULL == c2b_1) || (NULL == c2b_2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pA1 = spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /* ACTION������ */
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*����̳����̶�������*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /* ����Ǳ���λ����н��� */
    {
        if (ATG_DCI_RC_OK != upflag) /* ���Ǹ��� */
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            
            /* �������ӹ�ϵ:����������hqos���صĲ�λ��ҵ�����λ���˿�֮���voq���ӹ�ϵ */
            ret = spm_hqos_create_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_qos_vc_key_pd(&(pcb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)
                ApiAradHqosPwDelete(UNIT_0,pcb->pw_id,plsp->lsp_id);
#endif
                spm_free_hqos_vc_cb(pcb NBB_CCXT);

                /* ��������ɾ����ȷ */ 
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
        if ((pA1->schedule_cfg_cb) && (0 != pA1->schedule_cfg_cb->mode)) /* ���Ȳ��� */
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

            /* ��pw�Ķ��е���ģʽд�������� */
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
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*��һ����tlv���ô�����ɾ��LSP*/
        {
        
            /* ɾ���Ѿ�������voqȫ���� */
            spm_hqos_del_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
#if defined (SPU) || defined (SRC)
            ApiAradHqosPwDelete(UNIT_0, pcb->pw_id, plsp->lsp_id);
#endif
            spm_free_hqos_vc_cb(pcb NBB_CCXT);
            ret = ATG_DCI_RC_ADD_FAILED;
            goto EXIT_LABEL;
        }

        /*ֻ�Գ����������ü������������ʧ�ܣ����������޷��·�����*/
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

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);
    
    /* �������ָ�������Ч */
    NBB_ASSERT(plsp != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(data != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->flow_queue_qos_policy_index NBB_CCXT);

    /*policy���ò�����*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�ǵ��Ƚڵ�,policy��basic_cfg_cb�ֶ�������Ϊ���Ƚڵ�ģʽ���Ƕ�c-b��ģʽ*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*c-b�����ǿ�*/
    c2b_1 = AVLL_FIRST(pB1->c2b_tree);
    c2b_2 = AVLL_FIRST(pB2->c2b_tree);
    if ((NULL == c2b_1) || (NULL == c2b_2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*���Ƚڵ�c-b����action�ǿ�*/
    pA1 = spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION������*/
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the Action can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*����̳����̶�������*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }

    }

    /*��ȡ����λ�Ĳ�λ��*/
    spm_hardw_getslot(&fapid);

    /*����Ǳ���λ��Ŀ�Ĳ�λ����н���,��ֻ�ڳ������·�hqos������*/
    if (fapid == slot) 
    {
        if (ATG_DCI_RC_OK != upflag) /* �½�,���Ǹ��� */
        {
#if defined (SPU) || defined (SRC)

            /*����PW,�󶨵�ָ��LSP,��LSP�����ȴ���(ֻ�ڳ����̴���)*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            
            /*��������������������������hqos���صĵ���֮������ӹ�ϵ,���������������Լ����Լ�֮������ӹ�ϵ*/
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

                /*��������ɾ����ȷ*/
                goto EXIT_LABEL;
            }
        }

        /*PW����,��vrf����*/
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

        /* vrf�ĵ��Ȳ���,�����vrf����WFQģʽʱ��PW��Ȩ�� */
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

        /*vrf�и��������ε�����*/
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

            /*PW�а˸����еĵ��Ȳ���*/
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
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*��һ����tlv���ô�����ɾ��LSP*/
        {
            /* �ڳ��ڶ�ɾ��������VC��ɾ������ڶ�VOQ�����Ĺ��� */
            spm_hqos_del_voq(slot, port, data->node_index, pcb->pw_id NBB_CCXT);
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosPwDelete(0, pcb->pw_id, plsp->lsp_id);
#endif
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            goto EXIT_LABEL;
        }

        /*ֻ�Գ����������ü������������ʧ�ܣ����������޷��·�����*/
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

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(index != 0);

    /*ָ��Ϊ��*/
    if (0 == index)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR policy inedx is 0.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (NULL == pcb)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR SPM_HQOS_LOG_GROUP_CB pcb is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pB = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(index NBB_CCXT);

    /* policy���ò����� */
    if ((NULL == pB) || (NULL == pB->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if (0 != pB->basic_cfg_cb->mode) /* �ǵ��Ƚڵ� */
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    c2b = AVLL_FIRST(pB->c2b_tree);
    if (NULL == c2b)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR the c2b tree of Policy can't be NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pA = spm_qos_find_action_cb(c2b->c2b_key.qos_behavior_index NBB_CCXT);

    /*ACTION������*/
    if (NULL == pA)
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*����Ǳ���λ����н���*/
    {
        if (ATG_DCI_RC_OK != upflag) /*�״δ���*/
        {
#if defined (SPU) || defined (SRC)
            ret = ApiAradHqosLspCreate(0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != ret) /*ʧ��*/
        {
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1, "s d s s s s d d d d", 
						   "HQOS crete lsp",ret,"slot","port","lsp id","upflag",
						   slot,port + PTN_690_PORT_OFFSET,pcb->lsp_id,upflag));
            spm_hqos_arad_create_lsp_error_log(port + PTN_690_PORT_OFFSET, 
                        pcb->lsp_id,__FUNCTION__,__LINE__,ret NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (pA->shaping_cfg_cb) /*��������*/
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
        if ((pA->schedule_cfg_cb) && (0 != pA->schedule_cfg_cb->mode)) /*���Ȳ���*/
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
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*��һ����tlv���ô�����ɾ��LSP*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }

        /* ֻ�Գ����������ü���,�������ʧ��,���������޷��·����� */
        (pB->cnt)++;
        (pA->cnt)++;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(plsp != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(data != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == data) || (NULL == pcb) || (NULL == plsp))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR data/pcb/plsp is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pB1 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->user_qos_policy_index NBB_CCXT);
    pB2 = (SPM_QOS_POLICY_CB *)spm_qos_find_policy_cb(data->prio_queue_qos_policy_index NBB_CCXT);

    /*policy���ò�����*/
    if ((NULL == pB1) || (NULL == pB1->basic_cfg_cb)
        || (NULL == pB2) || (NULL == pB2->basic_cfg_cb))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Policy or basic_cfg_cb of Policy is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�ǵ��Ƚڵ�*/
    if ((0 != pB1->basic_cfg_cb->mode) || (0 != pB2->basic_cfg_cb->mode))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Mode of Policy can be only behavior-only,can't be multi-c2b.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pA1 = (SPM_QOS_ACTION_CB *)spm_qos_find_action_cb(c2b_1->c2b_key.qos_behavior_index NBB_CCXT);
    pA2 = (SPM_QOS_ACTION_CB *)spm_qos_find_action_cb(c2b_2->c2b_key.qos_behavior_index NBB_CCXT);

    /* ACTION������ */
    if ((NULL == pA1) || (NULL == pA2))
    {
        OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR Action is NULL.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*����̳����̶�������*/
    for (i = 0; i < pA2->que_congest_num; i++)
    {
        ret = spm_add_hqos_wred(slot, &(pA2->que_congst_cfg_cb[i]), 
              pcb->flow_id, data->node_index NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }

    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
    if (fapid == slot) /*����Ǳ���λ����н���*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }
#endif
        if (ATG_DCI_RC_OK != upflag) /*���Ǹ���*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }

            /*�������ӹ�ϵ*/
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

                /*��������ɾ����ȷ*/
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
        if ((pA1->schedule_cfg_cb) && (0 != pA1->schedule_cfg_cb->mode)) /*���Ȳ���*/
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
        if ((ATG_DCI_RC_OK != flag) && (ATG_DCI_RC_OK != upflag)) /*��һ����tlv���ô�����ɾ��LSP*/
        {
        
            /* ɾ���Ѿ�������voqȫ���� */
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

            /*�쳣����*/
            goto EXIT_LABEL;

        }

        /*ֻ�Գ����������ü������������ʧ�ܣ����������޷��·�����*/
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

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    if ((NULL == pcb) || (0 == slot))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /*����ɾ��B�ڵ㺯��*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosLspDelete(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*��������ɾ��B�ڵ����*/
    {
        spm_qos_txlsp_key_pd(&(pcb->lsp_tx_key),HQOS_LSP_ERROR,rv NBB_CCXT);
        spm_hqos_arad_del_lsp_error_log(port + PTN_690_PORT_OFFSET,
            pcb->lsp_id,__FUNCTION__,__LINE__,rv NBB_CCXT);
        ret = rv;
    }
    AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pcb->spm_hqos_lsp_tx_node);
    rv = spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*�ͷ�B�ڵ��ڴ����*/
    {
        ret = rv;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(pB != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /* ɾ���Ѿ�������voqȫ���� */
    spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

    /*����ɾ��A�ڵ㺯��*/
    if (fapid == slot)
    { 
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosPwDelete(UNIT_0, pcb->pw_id, pB->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*��������ɾ��B�ڵ����*/
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
    if (ATG_DCI_RC_OK != rv) /*�ͷ�A�ڵ��ڴ����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(pB != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /*��ʱ�Ѿ�������ɾ������δ���������������ж��Ƿ�Ϊ0*/
    if(0 == spm_hqos_vrftree_counter(pB,&(pcb->vrf_key) NBB_CCXT))
    {
        /* ֻ�ڳ�����ִ��,ɾ�������̺������֮������ӹ�ϵ */
        spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

        /* ɾ��A�ڵ�:ɾ��ָ����PW ID */
        if (fapid == slot)
        {
#if defined (SPU) || defined (SRC)
            rv = ApiAradHqosPwDelete(0, pcb->pw_id, pB->lsp_id);
#endif
        }
        if (ATG_DCI_RC_OK != rv) /*��������ɾ��B�ڵ����*/
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
    if (ATG_DCI_RC_OK != rv) /*�ͷ�A�ڵ��ڴ����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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
        printf("**QOS ERROR**%s,%d,ָ��Ϊ��\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"QOS ָ��Ϊ��",ATG_DCI_RC_UNSUCCESSFUL,
					"","","","",
					0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
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

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
	ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
	if (ATG_DCI_RC_OK != ret)
	{
	    ret = ATG_DCI_RC_OK;

        /*�쳣����*/
	    goto EXIT_LABEL;
	}

    for(j = 0; j < MAX_SLOT_NUM;j++)
    {
        
        /* �ж�slot�Ƿ���hqos�Ŀ��� */
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

                            /* ɾ���Ѿ�������voqȫ���� */
                            spm_hqos_del_voq(j + 1,i,vrf_cb->vrf_data.node_index, vrf_cb->pw_id NBB_CCXT);

                            voq = slot_index* NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
                            baseVCId = fapid_index * NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;

#if defined (SPU) || defined (SRC)

                            /* ɾ������̺ͳ�����֮���voqȫ���� */
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

    if(ATG_DCI_RC_OK == flag)/*���û���ҵ���������������*/
    {
        ret = ATG_DCI_RC_OK;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    if ((0 >= slot) || (MAX_SLOT_NUM < slot))
    {
        spm_qos_param_error_log(__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    
    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /*����ɾ��B�ڵ㺯��*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosLspDelete(UNIT_0, port + PTN_690_PORT_OFFSET, pcb->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*��������ɾ��B�ڵ����*/
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
    if (ATG_DCI_RC_OK != rv) /*�ͷ�B�ڵ��ڴ����*/
    {
        ret = rv;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(pcb != NULL);

    /* �������ָ�������Ч */
    NBB_ASSERT(pB != NULL);

    /*ָ��Ϊ��*/
    if ((NULL == pcb) || (NULL == pB))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /* ɾ���Ѿ�������voqȫ���� */
    spm_hqos_del_voq(slot,port,node_index,pcb->pw_id NBB_CCXT);

    /*����ɾ��A�ڵ㺯��*/
    if (fapid == slot)
    {
#if defined (SPU) || defined (SRC)
        rv = ApiAradHqosPwDelete(0, pcb->pw_id, pB->lsp_id);
#endif
    }
    if (ATG_DCI_RC_OK != rv) /*��������ɾ��B�ڵ����*/
    {
        printf("**QOS ERROR**%s,%d ret = %ld ApiAradHqosPwDelete\n",
                    __FUNCTION__,__LINE__,rv);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS ApiAradHqosPwDelete",rv,
						"slot","port","lsp_id","pw id",
						slot,port + PTN_690_PORT_OFFSET,pB->lsp_id,pcb->pw_id));
    }
    rv = spm_free_hqos_log_usr_cb(pcb NBB_CCXT);
    if (ATG_DCI_RC_OK != rv) /*�ͷ�A�ڵ��ڴ����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB->pw_cnt--;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


#if 1

/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* �������ָ�������Ч */
    NBB_ASSERT(logic_key != 0);

    if ((0 == group_key) || (0 == logic_key) || (NULL == sub_port))
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(sub_port->slot, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(fapid, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�����Ƿ����B�ڵ�*/
    pB = AVLL_FIND(SHARED.qos_port_cb[sub_port->slot - 1][sub_port->port].group_tree, &(sub_port->port));

    /* �����Ŀ�����������Ĭ��B�ڵ��LSP ID*/
    if (pB == NULL)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;

    }
    pcb = AVLL_FIND(pB->usr_tree, &logic_key);

    /* �����Ŀ�����������A�ڵ��ڴ��pw id*/
    if (pcb == NULL)
    {
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    for(unit = 0; unit < SHARED.c3_num;unit++)
    {
#if defined (SPU) || defined (PTN690_CES)
        ret += ApiC3SetL3UniHqos(unit, posid,NULL, 1, pcb->flow_id);
        if (ATG_DCI_RC_OK != ret)
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#endif  
    } 

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(stPortInfo.slot_id, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*�쳣����*/
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

        /*�����»�l3uni��hqos:��hqosʹ�ܿ��غ�flowid���ݸ�c3����*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#endif 
    }
    
    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}

/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
*****************************************************************************/
NBB_LONG spm_hqos_add_usr_node(SUB_PORT *sub_port,
    NBB_ULONG logic_key,
    NBB_ULONG group_key,
    ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *data NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    SPM_HQOS_LOG_USR_CB *pcb = NULL;
    SPM_HQOS_LOG_GROUP_CB *pB = NULL;
    NBB_LONG ret = ATG_DCI_RC_OK;
    NBB_LONG rv = ATG_DCI_RC_OK;
    NBB_ULONG old_index = 0;
    NBB_ULONG index = 0;
    NBB_ULONG flag = 0;
    NBB_BYTE fapid = 0;
    NBB_LONG voq = 0;
    TX_PORT_T l3uni = {0};
#if defined (SPU) || defined (PTN690_CES)
    NBB_LONG unit = 0;
#endif
    NBB_LONG slot_index = 0;
    NBB_LONG fapid_index = 0;
    NBB_ULONG offset = MAX_HQOS_SLOT_NUM * NUM_COS;
    NBB_LONG baseVCId = 0;
    NBB_CHAR ucMessage[QOS_MSG_INFO_LEN];

    NBB_TRC_ENTRY(__FUNCTION__);

    if ((0 == group_key) || (0 == logic_key) || (NULL == data) || (NULL == sub_port))
    {
        printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(0 == sub_port->slot)
    {
        printf("**QOS ERROR**%s,%d,param=NULL\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"QOS param ERROR",ATG_DCI_RC_UNSUCCESSFUL,
						"","","","",
						0,0,0,0));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(sub_port->slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(ATG_DCI_RC_OK != qos_log_cfg_print)
    {
        printf("%s %s,%d port=%d,ovlan=%d,logic_key=%ld,node_index=%ld,\n"
            "group index=%ld,user_qos_policy_index=%ld,prio_queue_qos_policy_index=%ld\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            sub_port->port,sub_port->ovlan,logic_key,data->node_index,group_key,
            data->user_qos_policy_index,data->prio_queue_qos_policy_index);
        OS_SPRINTF(ucMessage,"%s %s,%d port=%d,ovlan=%d,logic_key=%ld,node_index=%ld,\n"
            "group index=%ld,user_qos_policy_index=%ld,prio_queue_qos_policy_index=%ld\n",
            QOS_CFG_STRING,__FUNCTION__,__LINE__,
            sub_port->port,sub_port->ovlan,logic_key,data->node_index,group_key,
            data->user_qos_policy_index,data->prio_queue_qos_policy_index);
       BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
    if(sub_port->port > 0x80)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    l3uni.slot = sub_port->slot;
    l3uni.port = sub_port->port;
    l3uni.vid = sub_port->ovlan;
    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * data->node_index + HQOS_OFFSET;

    /*�����Ƿ����B�ڵ�*/
    pB = AVLL_FIND(SHARED.qos_port_cb[sub_port->slot - 1][sub_port->port].group_tree, &group_key);

    /* �����Ŀ�����������Ĭ��B�ڵ��LSP ID*/
    if (pB == NULL)
    {
        printf("**QOS ERROR**%s,%d can't find group node\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS vrf pw config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
						"slot","port","posid","ovlan",
				        sub_port->slot,sub_port->port,
						sub_port->posid,sub_port->ovlan));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;

    }
    if((sub_port->slot != pB->slot) || (sub_port->port != pB->port))
    {
        printf("**QOS ERROR**%s,%d slot port cfg miss macth,old=%d,new=%ld\n",
            __FUNCTION__,__LINE__,sub_port->port,pB->port);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS node_index cfg miss macth",ATG_DCI_RC_UNSUCCESSFUL,
						"slot","port","old node","new node",
				        sub_port->slot,sub_port->port,
						pB->slot,pB->port));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(pB->usr_tree, &logic_key);

    /* �����Ŀ�����������A�ڵ��ڴ��pw id*/
    if (pcb == NULL)
    {
        pcb = spm_alloc_hqos_log_usr_cb(logic_key NBB_CCXT);

        /*�����ڴ�ʧ��*/
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    if ((0 != pcb->usr_data.node_index) && (pcb->usr_data.node_index != data->node_index))
    {
        printf("**QOS ERROR**%s,%d node_index cfg miss macth,old=%ld,new=%ld\n",
            __FUNCTION__,__LINE__,pcb->usr_data.node_index,data->node_index);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS node_index cfg miss macth",ATG_DCI_RC_UNSUCCESSFUL,
						"slot","port","old node","new node",
				        sub_port->slot,sub_port->port,
						pcb->usr_data.node_index,data->node_index));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    old_index = (pcb->usr_data.user_qos_policy_index) | (pcb->usr_data.prio_queue_qos_policy_index);
    index = (data->user_qos_policy_index) | (data->prio_queue_qos_policy_index);
    flag = ((pcb->usr_data.user_qos_policy_index != data->user_qos_policy_index)
           || (pcb->usr_data.prio_queue_qos_policy_index != data->prio_queue_qos_policy_index));
    if ((0 == old_index) && (0 != index)) /*����*/
    {
        spm_dbg_record_qos_logic_subport_head(sub_port,logic_key,SPM_OPER_ADD NBB_CCXT);
#if defined (SPU) || defined (SRC)

        /* ��������̺ͳ�����֮���ȫ���� */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, sub_port->slot, sub_port->port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_subport_pd(sub_port,logic_key,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,sub_port->slot,sub_port->port + PTN_690_PORT_OFFSET,
                baseVCId, __FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_log_usr_cb(pcb NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_log_usr(sub_port->slot, sub_port->port, pcb, pB,
            data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, sub_port->slot, sub_port->port + PTN_690_PORT_OFFSET, baseVCId);
#endif

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {
            if(0 != sub_port->posid)
            {

                ret += ApiC3SetVpHqos(unit,sub_port->posid,0, 1, voq);
            }
            else
            {
                /*�����»�l3uni��hqos:��hqosʹ�ܿ��غ�flowid���ݸ�c3����*/
                ret += ApiC3SetL3UniHqos(unit, 0,&l3uni, 1, voq);
            }
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_subport_pd(sub_port,logic_key,HQOS_USR_ERROR,ret NBB_CCXT);
            if(0 == sub_port->posid)
            {
                spm_hqos_l3uni_error_log(voq,1,&l3uni,__FUNCTION__,__LINE__,ret NBB_CCXT);
            }
            else
            {
                spm_hqos_vp_error_log(voq,1,sub_port->posid,0,__FUNCTION__,__LINE__,ret NBB_CCXT);
            }
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, sub_port->slot, sub_port->port, baseVCId);
#endif
            spm_hqos_del_log_usr(sub_port->slot, sub_port->port, pB,
                pcb,data->node_index NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#endif 
#ifdef PTN690
        spm_hqos_add_intf_pmline(logic_key,voq NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_addline_hqos(data->node_index,sub_port->slot,voq);
        if (ATG_DCI_RC_OK != rv)
        {
            spm_hqos_almpm_addline_error_log(voq,data->node_index,sub_port->slot,
                __FUNCTION__,__LINE__,rv NBB_CCXT);
        }
#endif 
#endif
        OS_MEMCPY(&(pcb->usr_data), data, sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS));
        pcb->flow_id = voq;
        pB->slot = sub_port->slot;
        pB->port = sub_port->port;
        rv = AVLL_INSERT(pB->usr_tree, pcb->spm_hqos_usr_node);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 != flag)) /*����A�ڵ�policy index*/
    {
        spm_dbg_record_qos_logic_subport_head(sub_port,logic_key,SPM_OPER_UPD NBB_CCXT);
        ret = spm_hqos_add_log_usr(sub_port->slot, sub_port->port, pcb,
            pB, data, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {
        
            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (sub_port->slot == fapid) /*���ü���ֻ�ڳ�����*/
        {
            ret = spm_disconnect_policy_cnt(pcb->usr_data.user_qos_policy_index,
                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {
            
                /*�쳣����*/
                goto EXIT_LABEL;
            }
            ret = spm_disconnect_policy_cnt(pcb->usr_data.prio_queue_qos_policy_index,
                ATG_DCI_RC_OK NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }
        OS_MEMCPY(&(pcb->usr_data), data, sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS));
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 == flag)) /*ͬ��������*/
    {
        printf("**QOS ERROR**%s,%d the same index=%ld\n",
                __FUNCTION__,__LINE__,index);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS the same index",ret,
					"policy index","","","",
					index,0,0,0));

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 == index)) /*ɾ��A�ڵ�*/
    {
        spm_dbg_record_qos_logic_subport_head(sub_port,logic_key,SPM_OPER_DEL NBB_CCXT);
        if (sub_port->slot == fapid) /*���ü���ֻ�ڳ�����*/
        {
            spm_disconnect_policy_cnt(pcb->usr_data.user_qos_policy_index,
                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            spm_disconnect_policy_cnt(pcb->usr_data.prio_queue_qos_policy_index,
                ATG_DCI_RC_OK NBB_CCXT);

        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {
            if(0 != sub_port->posid)
            {
                rv += ApiC3SetVpHqos(unit,sub_port->posid,0, 0, voq);
            }
            else
            {
                rv += ApiC3SetL3UniHqos(unit, 0,&l3uni, 0, 0);
            }
        }
        if (ATG_DCI_RC_OK != rv)
        {
            if(0 == sub_port->posid)
            {
                spm_hqos_l3uni_error_log(voq,0,&l3uni,__FUNCTION__,__LINE__,ret NBB_CCXT);
            }
            else
            {
                spm_hqos_vp_error_log(voq,0,sub_port->posid,0,__FUNCTION__,__LINE__,ret NBB_CCXT);
            }
            ret = rv;
        }
#endif

#if defined (SPU) || defined (SRC)

        /* ɾ������̺ͳ�����֮���voqȫ���� */
        rv = ApiAradHqosVoqDelete(UNIT_0, voq, sub_port->slot, sub_port->port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != rv)
        {
            printf("**QOS ERROR**%s,%d ApiAradHqosVoqDelete err: %ld (baseQueue=%ld,slot=%d,port=%d,vc=%ld)\n",
                __FUNCTION__,__LINE__,rv, voq, 
                sub_port->slot, sub_port->port, baseVCId);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS del voq",rv,
						"baseQueue","slot","port","baseVCId",
						voq,sub_port->slot,sub_port->port,baseVCId));
            ret = rv;
        }
#ifdef PTN690
		spm_hqos_del_intf_pmline(logic_key NBB_CCXT);
#else
#if defined (SPU) || defined (PTN690_CES)
        rv = almpm_delline_hqos(data->node_index,sub_port->slot);
        if (ATG_DCI_RC_OK != rv)
        {
            printf("**QOS ERROR**%s,%d almpm_delline_hqos,node_index=%ld",
                __FUNCTION__,__LINE__,data->node_index);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS almpm_delline_hqos",rv,
						"baseQueue","slot","port","baseVCId",
						voq,sub_port->slot,sub_port->port,baseVCId));
            ret = rv;
        }
#endif
#endif
        AVLL_DELETE(pB->usr_tree, pcb->spm_hqos_usr_node);
        rv = spm_hqos_del_log_usr(sub_port->slot, sub_port->port, 
            pB, pcb,data->node_index NBB_CCXT);
        if (ATG_DCI_RC_OK != rv) /*���ش���*/
        {
            ret = rv;
        }

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 == old_index) && (0 == index))
    {
        printf("**QOS ERROR**%s,%d usr hqos config err\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS usr hqos config missmatch",ret,
					"node_index","user_qos_policy_index",
					"prio_queue_qos_policy_index","group_key",
					data->node_index,data->user_qos_policy_index,
					data->prio_queue_qos_policy_index,group_key));
        spm_free_hqos_log_usr_cb(pcb NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else /*�Ƿ�����*/
    {
        printf("**QOS ERROR**%s,%d usr hqos config missmatch\n",
                __FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS usr hqos config missmatch",ret,
					"node_index","user_qos_policy_index",
					"prio_queue_qos_policy_index","group_key",
					data->node_index,data->user_qos_policy_index,
					data->prio_queue_qos_policy_index,group_key));
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(slot, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(fapid, NULL NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK; 

        /*�쳣����*/
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

        /*�쳣����*/
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
        /* ����û���ڵ�������Ƿ�Ϊ�� */
        p_group = spm_check_group_down_cb(index,slot, port, &policy_index NBB_CCXT);
        if ((NULL == p_group) || (0 == policy_index))
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* group_tree:���˿ڳ���λ�����󶨵��»��û���QoS���õ���; */
        /*            ����,������keyֵΪ�û���ڵ������ֵindex.   */
        pcb = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].group_tree, &index);
    }

    /******************************����B(LSP)�ڵ�************************************/
    if ((pcb == NULL) && (0 != index))
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_ADD NBB_CCXT);
        pcb = spm_alloc_hqos_log_group_cb(index NBB_CCXT);
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_log_group(slot, port, pcb, ATG_DCI_RC_UNSUCCESSFUL, policy_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
            spm_free_hqos_log_group_cb(pcb NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (NULL == p_group)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            spm_free_hqos_log_group_cb(pcb NBB_CCXT);

            /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 != index) && (pcb->group_index != index)) /*ǿ�Ƹ���*/
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_UPD NBB_CCXT);
#if 0
        ret = spm_hqos_add_log_group(slot, port, pcb, ATG_DCI_RC_OK, index, nhi);

        if (ATG_DCI_RC_OK != ret) /*����ʧ��*/
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

        if (slot == fapid) /*���ü���ֻ�ڳ�����*/
        {
            ret = spm_disconnect_policy_cnt(pcb->policy_index);

            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 != index) && (pcb->group_index == index)) /*һ��������*/
    {
        printf("**QOS ERROR**%s,LINE=%d the same group cfg old=%ld,new=%ld\n",
                __FUNCTION__,__LINE__,pcb->group_index,index);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
					"HQOS the same group cfg",ret,
					"old group","new group","slot","port",
					pcb->group_index,index,slot,port));

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((NULL != pcb) && (0 != pcb->group_index) && (0 == index)) /*ɾ��B�ڵ�����*/
    {
        spm_dbg_record_qos_logic_head(log_key,SPM_OPER_DEL NBB_CCXT);
        
        /*���A�ڵ㻹��������������ɾ��B�ڵ�*/
        if (NULL != AVLL_FIRST(pcb->usr_tree))
        {
            printf("**QOS ERROR**%s,LINE=%d usr tree is in used can't del group\n",
                __FUNCTION__,__LINE__);
            NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
    					"HQOS usr tree is in used can't del group",ret,
    					"old group","new group","slot","port",
    					pcb->group_index,index,slot,port));
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*���ü���ֻ�ڳ�����*/
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

        /*����ɾ��B�ڵ㺯��*/
        ret = spm_hqos_del_log_group(slot, port, pcb NBB_CCXT); 

        /*�쳣����*/
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

        /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
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
            (0 == pkey->type)?"FTN":"CRLSP",(1 == pkey->flag)?"����":"����",
            (pstUpTnnlQos->mode)?"TE":"ģ��",
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
            (0 == pkey->type)?"FTN":"CRLSP",(1 == pkey->flag)?"����":"����",
            (pstUpTnnlQos->mode)?"TE":"ģ��",
            pstUpTnnlQos->node_index,pstUpTnnlQos->qos_policy_index,
            pstUpTnnlQos->cir,pstUpTnnlQos->pir);
    BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
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

    /* �����Ƿ����,�ؼ���pkeyΪ��lsp��Type��Flag��Tx_Lsp��keyֵ(ingress��egress��tunnelid��lspid) */
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ģʽ���л���֧�������õ�������л� */
    if ((NULL != pcb) && (0 != pcb->node_index) && (pcb->mode != mode)) 
    {
        spm_check_hqos_lsp_mode_cfg_log(pkey,pcb->mode,mode,
            __FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ���lsp��vpn����Ҫ����hqosʱ,����������lsp��hqos������vpn��hqos;����������vpn��   */
    /* hqos������lsp��hqosʱ,��Ҫ��ȥ��vpn��hqos,Ȼ������lsp��hqos���������vpn��hqos. */
    if((NULL != pcb) && (pcb->lsp_id <= MAX_PHYSIC_PORT_NUM))
    {
        spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_ADD NBB_CCXT);
        spm_check_hqos_lsp_lspid_err_log(pcb->lsp_id,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    voq = slot_index* NUM_COS + offset * node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * node_index + HQOS_OFFSET;

    /* TEģʽ */
    if ((0 != mode) && (NULL != shaping))
    {
        /* ���� */
        if ((pcb == NULL) && (0 != shaping->pir))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_ADD NBB_CCXT);
            pcb = spm_alloc_hqos_lsp_tx_cb(pkey,ATG_DCI_RC_OK NBB_CCXT);
            if (NULL == pcb)
            {
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*�쳣����*/
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

                    /*�쳣����*/
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

                    /*�쳣����*/
                    goto EXIT_LABEL;
                }
            }
            pcb->mode = mode;
            pcb->node_index = node_index;
            OS_MEMCPY(&(pcb->shaping), shaping, sizeof(SPM_TE_SHAPING));

            rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pcb->spm_hqos_lsp_tx_node); 

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* ���� */
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

                    /*�쳣����*/
                    goto EXIT_LABEL;
                }
            }
            pcb->mode = mode;
            pcb->node_index = node_index;
            OS_MEMCPY(&(pcb->shaping), shaping, sizeof(SPM_TE_SHAPING));

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* ɾ�� */
        else if ((NULL != pcb) && (0 == shaping->pir))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_DEL NBB_CCXT);
            if (NULL != AVLL_FIRST(pcb->vc_tree) || NULL != AVLL_FIRST(pcb->vrf_tree))
            {
                spm_check_hqos_lsp_del_cfg_log(pkey,__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*�쳣����*/
                goto EXIT_LABEL;
            }

            /* ����ɾ��B�ڵ㺯�� */
            ret = spm_hqos_del_lsp(slot, port, pcb, node_index, nhi NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* ��������� */
        else
        {
            spm_check_hqos_lsp_cfg_err_log(pkey,nflag,__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
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

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            ret = spm_hqos_add_lsp(slot, port, pcb, ATG_DCI_RC_UNSUCCESSFUL,
                  node_index, index, nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {
                spm_free_hqos_lsp_tx_cb(pcb NBB_CCXT);

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            pcb->policy_index = index;
            pcb->mode = mode;
            pcb->node_index = node_index;
            
            rv = AVLL_INSERT(SHARED.qos_port_cb[slot - 1][port].lsp_tree, pcb->spm_hqos_lsp_tx_node);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 != index) && (pcb->policy_index != index)) /*ǿ�Ƹ���*/
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_UPD NBB_CCXT);
            ret = spm_hqos_add_lsp(slot,port,pcb,ATG_DCI_RC_OK,node_index,index,nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {

                /*�쳣����*/
                goto EXIT_LABEL;
            }

            /* ���ü���ֻ�ڳ����� */
            if (slot == fapid)
            {
                spm_disconnect_policy_cnt(pcb->policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            }
            pcb->policy_index = index;
            pcb->node_index = node_index;

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* ��txlsp������hqos���� */
        else if ((NULL != pcb) && (0 == pcb->policy_index) && (0 != index))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_UPD NBB_CCXT);
            ret = spm_hqos_add_lsp(slot,port,pcb,ATG_DCI_RC_OK,node_index,index,nhi NBB_CCXT);
            if (ATG_DCI_RC_OK != ret)
            {

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            pcb->policy_index = index;
            pcb->node_index = node_index;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 != index) && (pcb->policy_index == index)) /*һ��������*/
        {
            spm_check_hqos_lsp_cfg_same_log(pkey,index,__FUNCTION__,__LINE__ NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }

        /* ɾ��B�ڵ����� */
        else if ((NULL != pcb) && (0 != pcb->policy_index) && (0 == index))
        {
            spm_dbg_record_qos_lsp_head(pkey,SPM_OPER_DEL NBB_CCXT);
            
            /* ���A�ڵ㻹��������������ɾ��B�ڵ� */
            if (NULL != AVLL_FIRST(pcb->vc_tree) || NULL != AVLL_FIRST(pcb->vrf_tree))
            {
                spm_check_hqos_lsp_del_cfg_log(pkey,__FUNCTION__,__LINE__ NBB_CCXT);
                ret = ATG_DCI_RC_UNSUCCESSFUL;

                /*�쳣����*/
                goto EXIT_LABEL;
            }

            /* ���ü���ֻ�ڳ����� */
            if (slot == fapid)
            {
               spm_disconnect_policy_cnt(pcb->policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            }

            /* ����ɾ��B�ڵ㺯�� */
            ret = spm_hqos_del_lsp(slot, port, pcb, node_index, nhi NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        else
        {
            spm_check_hqos_lsp_cfg_err_log(pkey,nflag,__FUNCTION__,__LINE__ NBB_CCXT);
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

        /*�쳣����*/
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
            (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"����":"����",
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
            (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"����":"����",
            data->node_index,data->qos_policy_index,
            data->flow_queue_qos_policy_index);
       BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);
        
    /* �ж�slot�Ƿ���hqos�Ŀ��� */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(port > 0x80)
    {
    OS_SPRINTF(ucMessage,"%s,%d slot=%d,port=%ld,vc_id=%ld,peer_ip=0x%lx : "
               "LAG NONSUPPORT HQOS, JUST PRINT THE CONFIGURATION.\n\n",
               __FUNCTION__,__LINE__,slot,port,pkey->vc_id,pkey->peer_ip);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if((0 != skey->type) && (0 == skey->tx_lsp.lspid)) /*LSP����ΪCRLSP_TX*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    
    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * data->node_index + HQOS_OFFSET;

    /*�����Ƿ����B�ڵ�*/
    pB = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].lsp_tree, skey);

    /* �����Ŀ�����������Ĭ��B�ڵ��LSP ID*/
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_ADD NBB_CCXT);
        pcb = spm_alloc_hqos_vc_cb(pkey NBB_CCXT);

        /*�����ڴ�ʧ��*/
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB = spm_alloc_hqos_lsp_tx_cb(skey,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);

        /*�����ڴ�ʧ��*/
        if (NULL == pB)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB->lsp_id = port + 1;
#if defined (SPU) || defined (SRC)

        /* ��������̺ͳ�����֮���ȫ���� */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                baseVCId, __FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vc_cb(pcb NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
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

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vc(slot, port, pB, pcb,data->node_index NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /************************�Ƚ�LSP�ڵ�*****************************/   
    pcb = AVLL_FIND(pB->vc_tree, pkey);

    /* �����Ŀ�����������A�ڵ��ڴ��pw id*/
    if (pcb == NULL)
    {
        pcb = spm_alloc_hqos_vc_cb(pkey NBB_CCXT);

        /*�����ڴ�ʧ��*/
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }
    if ((0 != pcb->vc_data.node_index) && (pcb->vc_data.node_index != data->node_index))
    {
        spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
        spm_check_hqos_vc_node_index_cfg_log(pkey,pcb->vc_data.node_index,
            data->node_index,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    old_index = (pcb->vc_data.qos_policy_index) | (pcb->vc_data.flow_queue_qos_policy_index);
    index = (data->qos_policy_index) | (data->flow_queue_qos_policy_index);
    flag = ((pcb->vc_data.qos_policy_index != data->qos_policy_index) || 
          (pcb->vc_data.flow_queue_qos_policy_index != data->flow_queue_qos_policy_index));
    if ((0 == old_index) && (0 != index)) /*����*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_ADD NBB_CCXT);
#if defined (SPU) || defined (SRC)

        /* ��������̺ͳ�����֮���ȫ���� */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vc_key_pd(pkey,HQOS_VC_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                baseVCId, __FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vc_cb(pcb NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_vc_cb(pcb NBB_CCXT);

            /*�쳣����*/
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

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vc(slot, port, pB, pcb,data->node_index NBB_CCXT);

            /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 != flag)) /*����A�ڵ�policy index*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_UPD NBB_CCXT);
        ret = spm_hqos_add_vc(slot, port, pcb, pB, data, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*���ü���ֻ�ڳ�����*/
        {
            ret = spm_disconnect_policy_cnt(pcb->vc_data.qos_policy_index,
                ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            ret = spm_disconnect_policy_cnt(pcb->vc_data.flow_queue_qos_policy_index,
                ATG_DCI_RC_OK NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }
        OS_MEMCPY(&(pcb->vc_data), data, sizeof(ATG_DCI_VC_UP_VPN_QOS_POLICY));

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 == flag)) /*ͬ��������*/
    {
    /*
        spm_qos_vc_key_pd(pkey,HQOS_BMU_ERROR,ret NBB_CCXT);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS pw the same index",ATG_DCI_RC_OK,
						"index","old_index","flag","",
						index,old_index,flag,0));
    */  
    
        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 == index)) /*ɾ��A�ڵ�*/
    {
        spm_dbg_record_qos_vc_head(pkey,SPM_OPER_DEL NBB_CCXT);
        if (slot == fapid) /*���ü���ֻ�ڳ�����*/
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

        /* ɾ������̺ͳ�����֮���voqȫ���� */
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
        if (ATG_DCI_RC_OK != rv) /*���ش���*/
        {
            ret = rv;
        }
        if (NULL == AVLL_FIRST(pB->vc_tree) && NULL == AVLL_FIRST(pB->vrf_tree)
            && (0 == pB->node_index))
        {
           AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pB->spm_hqos_lsp_tx_node);
           spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);
        }

        /*�쳣����*/
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
    else /*�Ƿ�����*/
    {
        /*printf("**QOS ERROR**%s,%d vc hqos config missmatch\n",__FUNCTION__,__LINE__);
        NBB_EXCEPTION((PCT_SPM| QOS_PD, 1,  "s d s s s s d d d d", 
						"HQOS vc pw config missmatch",ATG_DCI_RC_UNSUCCESSFUL,
						"qos_policy_index","flow_queue_qos_policy_indexdata",
						"node_index","posid",
						data->qos_policy_index,data->flow_queue_qos_policy_index,
						data->node_index,posid));*/
        ret = ATG_DCI_RC_UNSUCCESSFUL;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*�쳣����*/
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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

    /* vrf�񵴹� */
    if(0 == posid)
    {
        ret = spm_hqos_clear_vrf_mem(pkey,data NBB_CCXT);

        /*�쳣����*/
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

        /*�쳣����*/
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
                  (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"����":"����",
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
                   (0 == skey->type)?"FTN":"CRLSP",(1 == skey->flag)?"����":"����",
                   data->node_index,data->qos_policy_index,
                   data->flow_queue_qos_policy_index);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
    }

    /* �ж�Ŀ�Ĳ�λ�Ƿ��ѿ���hqos���� */
    ret = spm_hqos_checkslot(slot, &slot_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;
        OS_SPRINTF(ucMessage,"%s %s,%d : Objective slot=%d don't open HQOS switch.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__,slot);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* ��ȡ����λ�Ĳ�λ�� */
    spm_hardw_getslot(&fapid);

    /* �жϱ���λ�Ƿ��ѿ���hqos���� */
    ret = spm_hqos_checkslot(fapid, &fapid_index NBB_CCXT);
    if (ATG_DCI_RC_OK != ret)
    {
        ret = ATG_DCI_RC_OK;
        OS_SPRINTF(ucMessage,"%s %s,%d : This slot=%d don't open HQOS switch.\n\n",
                   QOS_CFG_STRING,__FUNCTION__,__LINE__,fapid);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    if(port > 0x80)
    {
        OS_SPRINTF(ucMessage,"%s,%d slot=%d,port=%ld,label=%ld,vrf_id=%d,peer_ip=0x%lx : "
        "LAG NONSUPPORT HQOS, JUST PRINT THE CONFIGURATION.\n\n",
                   __FUNCTION__,__LINE__,slot,port,pkey->label,pkey->vrf_id,pkey->peer_ip);
        BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
        ret = ATG_DCI_RC_OK;

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /*
      slot_index  : Ŀ�Ĳ�λ����ֵ
      fapid_index : ����λ����ֵ
    */
    voq = slot_index* NUM_COS + offset * data->node_index + HQOS_OFFSET;
    baseVCId = fapid_index * NUM_COS + offset * data->node_index + HQOS_OFFSET;

    /* ����Ŀ�Ĳ�λ��Ŀ�Ķ˿��Ƿ����B�ڵ�,��lsp�ڵ� */
    pB = AVLL_FIND(SHARED.qos_port_cb[slot - 1][port].lsp_tree, skey);

    /* �����Ŀ�����������Ĭ��B�ڵ��LSP ID */
    if (pB == NULL)
    {
        /* ������Ϊ0 */
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_ADD NBB_CCXT);

        /* ����B�ڵ� */
        pB = spm_alloc_hqos_lsp_tx_cb(skey,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (NULL == pB)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_alloc_hqos_lsp_tx_cb failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        pB->lsp_id = port + 1;

        /* ��vrf��hqos�����ð���ͬһ���ṹ����,������B�ڵ��,���뵽B�ڵ��vrf���� */
        pcb = spm_alloc_hqos_vrf_cb(pkey,ATG_DCI_RC_OK  NBB_CCXT);
        if (NULL == pcb)
        {
            ret = ATG_DCI_RC_UNSUCCESSFUL;
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_alloc_hqos_vrf_cb failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (SRC)

        /* ��������̺ͳ�����֮���ȫ���� */
        ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
        if (ATG_DCI_RC_OK != ret)       
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                        baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }    

        /* ��vrf��hqos����д������ */
        ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)
        {
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* ��txpw������д��flowid,����ihmtͷ�м���flowid�ֶ� */
            ret += ApiC3SetL3TxPwHqos(unit, posid, 1, voq);
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_l3txpw_error_log(voq,1,posid,__FUNCTION__,__LINE__,ret NBB_CCXT);
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
            ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            spm_hqos_del_vrf(slot, port, pB, pcb,data->node_index NBB_CCXT);
            spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);

            /*�쳣����*/
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    pcb = AVLL_FIND(pB->vrf_tree, pkey);

    /* �����Ŀ�����������A�ڵ��ڴ��pw id */
    if (pcb == NULL)
    {
    	/* ��ֹ�ظ�ע��,��Ϊһ��vrfʵ���п��ܴ��ڶ����ǩ,��ʱֻ��Ҫע��һ�� */
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

            /*�쳣����*/
            goto EXIT_LABEL;
        }
        ret = spm_hqos_find_vrf_pwid(pB,pcb NBB_CCXT);
        if(ATG_DCI_RC_OK != ret)
        {
            spm_free_hqos_vrf_cb(pcb NBB_CCXT);

            /*�쳣����*/
            goto EXIT_LABEL;
        }
    }

    /* ��ͬvrfid����hqos������ʱ,node_index����һ�·���ֱ�ӱ��� */
    if ((0 != pcb->vrf_data.node_index) && (pcb->vrf_data.node_index != data->node_index))
    {
        spm_check_hqos_vrf_node_index_cfg_log(pkey,pcb->vrf_data.node_index,
                           data->node_index,__FUNCTION__,__LINE__ NBB_CCXT);
        ret = ATG_DCI_RC_UNSUCCESSFUL;
        
        /*�������ܹҵ�*/

        //goto EXIT_LABEL;
    }

    /* flagΪ0��ʾhqos������δ����,����������ͬ */
    old_index = (pcb->vrf_data.qos_policy_index) | (pcb->vrf_data.flow_queue_qos_policy_index);
    index = (data->qos_policy_index) | (data->flow_queue_qos_policy_index);
    flag = ((pcb->vrf_data.qos_policy_index != data->qos_policy_index) || 
           (pcb->vrf_data.flow_queue_qos_policy_index != data->flow_queue_qos_policy_index));
    if ((0 == old_index) && (0 != index)) /*����*/
    {
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_ADD NBB_CCXT);

        /* ��ֹ�ظ�ע��,��Ϊһ��vrfʵ���п��ܴ��ڶ����ǩ,��ʱֻ��Ҫע��һ�� */
        if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
#if defined (SPU) || defined (SRC)

            /* ��������̺ͳ�����֮���ȫ���� */
            ret = ApiAradHqosVoqSet(UNIT_0, voq, NUM_COS, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
            if (ATG_DCI_RC_OK != ret)       
            {
                spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
                spm_hqos_arad_voq_error_log(voq,slot,port + PTN_690_PORT_OFFSET,
                                 baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                spm_free_hqos_vrf_cb(pcb NBB_CCXT);

                /*�쳣����*/
                goto EXIT_LABEL;
            }
            ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
            {
#if defined (SPU) || defined (SRC)

                /* ɾ������̺ͳ�����֮���voqȫ���� */
                ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif

                /*�쳣����*/
                goto EXIT_LABEL;
            }
        }

#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* ��txpw������д��flowid,����ihmtͷ�м���flowid�ֶ� */
            ret += ApiC3SetL3TxPwHqos(unit, posid, 1, voq);
        }
        if (ATG_DCI_RC_OK != ret)
        {
            spm_qos_vrf_key_pd(pkey,HQOS_VRF_ERROR,ret NBB_CCXT);
            spm_hqos_l3txpw_error_log(voq,1,posid,__FUNCTION__,__LINE__,ret NBB_CCXT);

            if(0 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
            {
#if defined (SPU) || defined (SRC)

                /* ɾ������̺ͳ�����֮���voqȫ���� */
                ApiAradHqosVoqDelete(UNIT_0, voq, slot, port + PTN_690_PORT_OFFSET, baseVCId);
#endif
                spm_hqos_del_vrf(slot, port, pB, pcb,data->node_index NBB_CCXT);
            }

            /*�쳣����*/
            goto EXIT_LABEL;
        }
#endif

        /* ��ֹ�ظ�ע��,��Ϊһ��vrfʵ���п��ܴ��ڶ����ǩ,��ʱֻ��Ҫע��һ�� */
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

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* flagΪ0��ʾvrf��hqos��ԭ��������������ͬ */
    else if ((0 != old_index) && (0 != index) && (0 != flag)) /*����A�ڵ�policy index*/
    {

        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_UPD NBB_CCXT);
#if 0
        ret = spm_hqos_add_vrf(slot, port, pcb, pB, data, ATG_DCI_RC_OK NBB_CCXT);
        if (ATG_DCI_RC_OK != ret) /*�����������ش���*/
        {
            OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_hqos_add_vrf failed.\n\n",
                       QOS_CFG_STRING,__FUNCTION__,__LINE__);
            BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
            goto EXIT_LABEL;
        }
        if (slot == fapid) /*���ü���ֻ�ڳ�����*/
        {
            ret = spm_disconnect_policy_cnt(pcb->vrf_data.qos_policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_disconnect_policy_cnt failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
            ret = spm_disconnect_policy_cnt(pcb->vrf_data.flow_queue_qos_policy_index,ATG_DCI_RC_OK NBB_CCXT);
            if (ATG_DCI_RC_OK != ret) /*���ü�����1ʧ��*/
            {
                OS_SPRINTF(ucMessage,"%s %s,%d : QOS ERROR spm_disconnect_policy_cnt failed.\n\n",
                           QOS_CFG_STRING,__FUNCTION__,__LINE__);
                BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);
                goto EXIT_LABEL;
            }
        }
        OS_MEMCPY(&(pcb->vrf_data), data, sizeof(ATG_DCI_VRF_INSTANCE_UP_VPN_QOS));
#endif

        /*�쳣����*/
        goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 != index) && (0 == flag)) /*ͬ��������*/
    {
	     OS_SPRINTF(ucMessage,"##### VRF:SAME HQOS CONFIGURATION (vrf_id=%d,peer_ip=0x%lx,"
	        		"label=%ld,index=%ld,old_index=%ld) #####\n",
	                pkey->vrf_id,pkey->peer_ip,pkey->label,index,old_index);
	     BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage); 
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* ��txpw������д��flowid,����ihmtͷ�м���flowid�ֶ� */
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

         /*�쳣����*/
         goto EXIT_LABEL;
    }
    else if ((0 != old_index) && (0 == index)) /* ɾ��A�ڵ� */
    {
        spm_dbg_record_qos_vrf_head(pkey,SPM_OPER_DEL NBB_CCXT);
        if ((1 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT)) && (slot == fapid)) /* ɾ��vrf�ڵ�ʱ,���ü���ֻ�ڳ����� */
        {
            spm_disconnect_policy_cnt(pcb->vrf_data.qos_policy_index,ATG_DCI_RC_UNSUCCESSFUL NBB_CCXT);
            spm_disconnect_policy_cnt(pcb->vrf_data.flow_queue_qos_policy_index,ATG_DCI_RC_OK NBB_CCXT);
        }
#if defined (SPU) || defined (PTN690_CES)
        for(unit = 0; unit < SHARED.c3_num;unit++)
        {

            /* ��txpw������д��flowid,����ihmtͷ�м���flowid�ֶ� */
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

		/* ��ֹ�ظ�ע��,ɾ��ʱֻ�����һ�β�ע��;����ʱֻ�ڵ�һ�β�ע�� */
        if(1 == spm_hqos_vrftree_counter(pB,pkey NBB_CCXT))
        {
#if defined (SPU) || defined (SRC)

            /* ɾ������̺ͳ�����֮���voqȫ���� */
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

        /* ����ʱlsp�ڵ���δ����,��vc��vrf������Ϊ��,��ɾ��Ŀ�Ĳ�λ�˿��ϵ�lsp���ڵ� */
        if (NULL == AVLL_FIRST(pB->vc_tree) && NULL == AVLL_FIRST(pB->vrf_tree)
            && (0 == pB->node_index))
        {
           AVLL_DELETE((SHARED.qos_port_cb[slot - 1][port]).lsp_tree, pB->spm_hqos_lsp_tx_node);
           spm_free_hqos_lsp_tx_cb(pB NBB_CCXT);
        }

        /*�쳣����*/
        goto EXIT_LABEL;
    }

    /* �Ƿ����� */
    else
    {
	     OS_SPRINTF(ucMessage,"##### VRF:ERROR HQOS CONFIGURATION (vrf_id=%d,peer_ip=0x%lx,label=%ld) #####\n"
	  			    "node_index=%ld,qos_policy_index=%ld,flow_queue_qos_policy_index=%ld,posid=%ld\n\n",
                    pkey->vrf_id,pkey->peer_ip,pkey->label,data->node_index,data->qos_policy_index,
				    data->flow_queue_qos_policy_index,posid);
	     BMU_SLOG(BMU_INFO, SPM_QOS_LOG_DIR, ucMessage);  
         ret = ATG_DCI_RC_OK;

         /*�쳣����*/
         goto EXIT_LABEL;
    }

    /* �쳣���� */
    EXIT_LABEL: NBB_TRC_EXIT();
    return ret;
}


/*****************************************************************************
   �� �� ��  : spm_disconnect_usr_group_cnt
   ��������  : �ͷ�HQOS���Ľڵ㲢�ͷ��ڴ�
   �������  : upflag=0��������
   �������  :
   �� �� ֵ  :
   ���ú���  :
   ��������  :
   �޸���ʷ  :
   ��    ��  : 2013��1��15�� ���ڶ�
   ��    ��  : zenglu
   �޸�����  : �����ɺ���
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
        /* �ж�slot�Ƿ���hqos�Ŀ��� */
        ret = spm_hqos_checkslot(j + 1, &slot_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)/*��֧��HQOS�Ĳ�λ����*/
        {
            continue;
        }
        
        /* �ж�slot�Ƿ���hqos�Ŀ��� */
        ret = spm_hqos_checkslot(v_spm_shared->local_slot_id, &fapid_index NBB_CCXT);
        if (ATG_DCI_RC_OK != ret)/*��֧��HQOS�Ĳ�λ����*/
        {
            continue;
        }

        /* ���Ŀ�Ĳ�λĿ�Ķ˿���lsp����hqos��ص�������Ϣ */
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
                    /* ��ͬһ��vrf���ж���·��ʱ,ֻ�����һ��·��ʱ��ɾ������vrf��hqos������Ϣ */
                    if(1 == spm_hqos_vrftree_counter(lsp_cb,&(vrf_cb->vrf_key) NBB_CCXT))
                    {
                        voq = slot_index* NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
                        baseVCId = fapid_index * NUM_COS + offset * vrf_cb->vrf_data.node_index + HQOS_OFFSET;
#if defined (SPU) || defined (SRC)

                        /* ɾ������̺ͳ�����֮���voqȫ���� */
                        ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                        if (ATG_DCI_RC_OK != ret)
                        {
                            spm_qos_vrf_key_pd(&(vrf_cb->vrf_key),HQOS_VRF_ERROR,ret NBB_CCXT);
                            spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                                  baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                        }

                        /* ɾ���Ѿ�������voqȫ���� */
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

                    /* ɾ������̺ͳ�����֮���voqȫ���� */
                    ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                    if (ATG_DCI_RC_OK != ret)
                    {
                        spm_qos_vc_key_pd(&(vc_cb->vc_key),HQOS_VC_ERROR,ret NBB_CCXT);
                        spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                              baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                    }

                    /* ɾ���Ѿ�������voqȫ���� */
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

                /* �ڳ�������ɾ��hqos���ù����д�����lspid */
                if(((j + 1) == v_spm_shared->local_slot_id)
                    && (lsp_cb->lsp_id > MAX_PHYSIC_PORT_NUM))
                {
#if defined (SPU) || defined (SRC)
                    ret = ApiAradHqosLspDelete(UNIT_0,i,lsp_cb->lsp_id);
#endif
                    if (ATG_DCI_RC_OK != ret) /*ʧ��*/
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

                    /* ɾ������̺ͳ�����֮���voqȫ���� */
                    ret = ApiAradHqosVoqDelete(UNIT_0, voq, j + 1, i + PTN_690_PORT_OFFSET, baseVCId);
#endif
                    if (ATG_DCI_RC_OK != ret)
                    {
                        spm_qos_log_key_pd(usr_cb->index,HQOS_USR_ERROR,ret NBB_CCXT);
                        spm_hqos_arad_voq_del_error_log(voq,slot,i + PTN_690_PORT_OFFSET,
                                              baseVCId,__FUNCTION__,__LINE__,ret NBB_CCXT);
                    }

                    /* ɾ���Ѿ�������voqȫ���� */
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
                    if (ATG_DCI_RC_OK != ret) /*ʧ��*/
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