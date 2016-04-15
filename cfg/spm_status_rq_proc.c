/******************************************************************************

                  ��Ȩ���� (C), 1999-2013, ���ͨ�ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : spm_status_rq_proc.c
  �� �� ��   : ����
  ��    ��   : lh09
  ��������   : 2012��6��8��
  ����޸�   :
  ��������   : ������ѯ����״̬����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2012��6��8��
    ��    ��   : lh09
    �޸�����   : �����ļ�

******************************************************************************/

#define SHARED_DATA_TYPE SPM_SHARED_LOCAL

#include <nbase.h>
#include <spmincl.h>
#include "bmuInclude.h"

static unsigned int g_dbg_rq_print_en = 0; //rq��ӡʹ��
static unsigned short g_dbg_rq_focus_type = 0; //�ض�������


NBB_VOID spm_rcv_dci_status_rq(const NBB_BYTE *rq_buffer, NBB_ULONG len);

extern NBB_VOID spm_hardw_sendboardstate(NBB_VOID);
extern NBB_VOID spm_ptp_report_port_status(NBB_ULONG ulIndex);
extern NBB_VOID spm_ptp_report_global_status(NBB_BYTE ucCmd);
/* 1.ʱ�ӹ���״̬  */
extern NBB_VOID spm_clk_state_report(NBB_VOID);

/*2.ʱ��Դ״̬*/
extern NBB_INT spm_clk_source_report(NBB_ULONG spec); 

/* vpls mac����Ϣ */
extern NBB_INT ApiMacGetMacInfo(const void *buff);
/* vpls mac��������Ϣ */
extern NBB_INT ApiMacGetMacExistStatOrNum(const void *buff);
/* �ϱ����̶˿���Ϣ  */
extern NBB_INT spm_state_port(NBB_VOID);
/**PROC+**********************************************************************/
/* Name:      spm_proc_unspport_inst_function                                */
/*                                                                           */
/* Purpose:   ͨ�õĲ�֧�ֵ�Э������������� .                               */
/*                                                                           */
/* Returns:   ״̬��ѯ�������.                                              */
/*                                                                           */
/* Params:    rq_buf - ״̬��ѯbuffer.                                       */
/*            len - ״̬��ѯbuffer ����.                                     */
/*            buf -  ״̬��ѯ���.                                           */
/*            ucCharacter -  �������ֶ�.                                     */
/*                                                                           */
/* Operation: ͨ��״̬��������.                                              */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_LONG spm_proc_unspport_inst_function(NBB_CONST NBB_BYTE *rq_buf, NBB_CONST NBB_ULONG len, NBB_BYTE *buf, NBB_BYTE *ucCharacter)
{
        NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();
    NBB_TRC_ENTRY("spm_proc_unspport_inst_function");    

    /* �������ָ�������Ч */
    NBB_ASSERT(rq_buf != NULL);

    /* ����֧�ֵ���������ԭ������. */
    NBB_MEMCPY(buf, rq_buf, len);
    /* �������ֶ���Ϊ ��֧�ֵ�Э��  */
    *ucCharacter = *ucCharacter | SPM_PROT_RET_UNSUPPORT;

    NBB_TRC_EXIT();

    return len;
}

/**PROC+**********************************************************************/
/* Name:      spm_proc_state_function                                        */
/*                                                                           */
/* Purpose:   ͨ��״̬�������� .                                             */
/*                                                                           */
/* Returns:   ״̬��ѯ�������.                                              */
/*                                                                           */
/* Params:    rq_buf - ״̬��ѯbuffer.                                       */
/*            len - ״̬��ѯbuffer ����.                                     */
/*            buf -  ״̬��ѯ���.                                           */
/*            ucCharacter -  �������ֶ�.                                     */
/*                                                                           */
/* Operation: ͨ��״̬��������.                                              */
/*                                                                           */
/**PROC-**********************************************************************/

NBB_LONG spm_proc_state_function(NBB_CONST NBB_BYTE *rq_buf, NBB_CONST NBB_ULONG len, NBB_BYTE *buf, NBB_BYTE *ucCharacter)
{
    NBB_USHORT rq_type, rq_len;
    NBB_LONG ret;
    NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();    
    NBB_TRC_ENTRY("spm_proc_state_function");    

    /* �������ָ�������Ч */
    NBB_ASSERT(rq_buf != NULL);

    NBB_GET_SHORT(rq_type, rq_buf);
    NBB_GET_SHORT(rq_len, rq_buf + SPM_SUB_CONF_TYPE_SIZE);

    /*
    if(rq_type < SPM_STATE_TLV_MAXNUM)
    {
        ret = SHARED.state_func[rq_type](rq_buf, len, buf, ucCharacter);
    }
    */

    /* ��ʱ����ǰ�� ͨ�� linx api ����״̬���� */
    spm_rcv_dci_status_rq(rq_buf, len);

    /* ͨ�� linx api ���ͣ�����Ҫͨ������������״̬Ӧ���� */
    ret = -1;

    NBB_TRC_EXIT();

    return ret;
}

/**PROC+**********************************************************************/
/* Name:      spm_proc_state_function                                        */
/*                                                                           */
/* Purpose:   ͨ��״̬�������� .                                             */
/*                                                                           */
/* Returns:   ״̬��ѯ�������.                                              */
/*                                                                           */
/* Params:    rq_buf - ״̬��ѯbuffer.                                       */
/*            len - ״̬��ѯbuffer ����.                                     */
/*            buf -  ״̬��ѯ���.                                           */
/*            ucCharacter -  �������ֶ�.                                     */
/*                                                                           */
/* Operation: ͨ��״̬��������.                                              */
/*                                                                           */
/**PROC-**********************************************************************/
NBB_LONG spm_proc_ofl_function(NBB_CONST NBB_BYTE *rq_buf, 
        NBB_CONST NBB_ULONG len, 
        NBB_BYTE *buf, 
        NBB_BYTE *ucCharacter)
{
    NBB_USHORT rq_type = 0;
    NBB_USHORT rq_len = 0;
    NBB_LONG ret = 0;
    NBB_BYTE ucAgree = 0;
    NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();    
    NBB_TRC_ENTRY("spm_proc_ofl_function");    

    /* �������ָ�������Ч */
    NBB_ASSERT(rq_buf != NULL);

    /*
    if(rq_type < SPM_STATE_TLV_MAXNUM)
    {
        ret = SHARED.state_func[rq_type](rq_buf, len, buf, ucCharacter);
    }
    */
    /* �ֽ�1��ȥ�������ͣ�0/1=�������/������������
       �ֽ�2��Ӧ����Ϣ��0/1=ͬ��/�ܾ�
       �ֽ�3-4������ */
    printf("rq_buf: %d %d\n",*(rq_buf),*(rq_buf++));
    ucAgree = *(rq_buf++);
    spm_hardw_oflproc(ucAgree);

    /* ͨ�� linx api ���ͣ�����Ҫͨ������������״̬Ӧ���� */
    ret = -1;

    NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
 �� �� ��  : spm_get_proc_data_function
 ��������  : �ַ���ͬ��ѯ����
 �������  : NBB_ULONG instruct  
 �������  : ��
*****************************************************************************/
SPM_PROC_DATA_FUNC spm_get_proc_data_function(NBB_ULONG instruct)
{

    SPM_PROC_DATA_FUNC proc_function = spm_proc_unspport_inst_function;
    switch(instruct)
    {
        /* ״̬��ѯ�������� */
        case 0X123457:
            proc_function = spm_proc_state_function;
            break;
        case 0X001400:
            proc_function = spm_proc_ofl_function;
            break;
        /* �澯��ѯ */
        case 0X71000D:
            proc_function = BmuGetCurAlm;
            break;
        /* ���ܲ�ѯ */
        case 0X71000E:
            proc_function = BmuGetPm;
            break;
        /* ��ѯָ������ */
        case 0X71000F:
            proc_function = BmuGetSpecPm;
            break;
        /* �������ο��� */
        case 0X710010:
            proc_function = BmuProcPmMaskConf;
            break;
        /* �澯���� */
        case 0X710110:
            proc_function = BmuProcAlmMaskConf;
            break;
        /* ������� */
        case 0X00B200:
            proc_function = BmuClearAllPm;
            break;
        /* ���ܲɼ����� */
        case 0X71FF10:
            proc_function = BmuProcPmSwConf;
            break;        /* Ĭ����Ϊ��֧�ֵ�Э�� */
        /* ���ָ����·������ */
        case 0X840000:
            proc_function = BmuClearSpecPm;
            break;        
        /* �澯���� */
        case 0X710200:
            proc_function = BmuProcAlmRestrainConf;
            break;        
        /* �澯���Ʋ�ѯ */
        case 0X710201:
            proc_function = BmuGetAlmRestrainInfo;
            break;        /* Ĭ����Ϊ��֧�ֵ�Э�� */
            
        /* NMU��BMU�·���ȡLOG�Ŀ������� */
        case 0x002001:
            proc_function = SysLogCmdUpload;
            break; 
            
        /* NMU��BMU���õ�����־���� */
        case 0x002003:
            proc_function = SysLogSetLogLevel;
            break; 
            
        /* NMU��BMU��ѯ������־���� */
        case 0x002004:
            proc_function = SysLogGetLogLevel;
            break; 
            
        /* NMU��BMU���������־ */
        case 0x002005:
            proc_function = SysLogCmdDel;
            break;     
        case 0x500e04:
        case 0x500e05:
        case 0x500e06:
            printf("inst_code = %x\n",instruct);
            break;            
        default:
            proc_function = spm_proc_unspport_inst_function;
            break;
    }

    return proc_function;
}

/*****************************************************************************
 �� �� ��  : spm_send_sbi_status_rq
 ��������  : �����ӿ���״̬��ѯͨ��ips rq͸�����ӿ�
 �������  : NBB_BYTE *rq_buffer   
             NBB_ULONG length      
             NBB_BYTE ucSubCardNo : 1/2 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��1��7��
    ��    ��   : ���� ����
    �޸�����   : �����ɺ���

*****************************************************************************/
NBB_INT spm_send_sbi_status_rq(const NBB_BYTE *rq_buffer, NBB_ULONG length, NBB_BYTE ucSubCardNo)

{
    NBB_BYTE ucMessage[80];
    NBB_INT iRv = 0;
    ATG_SBI_STATUS_RQ *pstSbistatusRq = NULL;
    NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();    
    NBB_TRC_ENTRY("spm_send_sbi_status_rq");
    
    if (rq_buffer == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_send_sbi_status_rq(rq_buffer==NULL)"));

        OS_PRINTF("***ERROR***:spm_send_sbi_status_rq(rq_buffer==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_send_sbi_status_rq(rq_buffer==NULL)\n");
        
        iRv = ERROR;
        goto EXIT_LABEL;
    }

    if ((ucSubCardNo != 1) && (ucSubCardNo != 2))
    {
        OS_PRINTF("***ERROR***:spm_send_sbi_status_rq(ucSubCardNo==%d)\n",ucSubCardNo);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_send_sbi_status_rq(ucSubCardNo==%d)\n",ucSubCardNo);
        
        iRv = ERROR;
        goto EXIT_LABEL;
    }
    
    if (SHARED.sub_card_cb[ucSubCardNo-1] != NULL)
    {
        pstSbistatusRq = (ATG_SBI_STATUS_RQ*) NBB_GET_BUFFER(NBB_NULL_HANDLE,
                                           NBB_ALIGN_OFFSET(sizeof(ATG_SBI_STATUS_RQ)),
                                           length, NBB_RETRY_DATA | NBB_BUF_PKT); 

        if (NULL == pstSbistatusRq)
        {
            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_send_board_status_ctl_cfg(pstSbistatusRq==NULL)"));

            OS_PRINTF("***ERROR***:spm_send_board_status_ctl_cfg(pstSbistatusRq==NULL)\n");

            OS_SPRINTF(ucMessage, "***ERROR***:spm_send_board_status_ctl_cfg(pstSbistatusRq==NULL)\n");
            
            iRv = ERROR;
            
            goto EXIT_LABEL;
        }
        /*************************************************************************/
        /* ��ʼ����Ϣ��                                                          */
        /*************************************************************************/
        NBB_ZERO_IPS(pstSbistatusRq); 
        
        pstSbistatusRq->ips_hdr.ips_type = IPS_ATG_SBI_STATUS_RQ;

        /*����״̬��ѯ����*/
        NBB_PKT_COPY_IN(pstSbistatusRq, pstSbistatusRq->pkt_hdr.data_start, rq_buffer,
                        pstSbistatusRq->ips_hdr.data_size);

        spm_snd_sbi_ips(SHARED.sub_card_cb[ucSubCardNo-1], pstSbistatusRq, &(pstSbistatusRq->ips_hdr) NBB_CCXT);
    } 
        
    EXIT_LABEL: NBB_TRC_EXIT();
    
    return iRv;
}

/*****************************************************************************
 �� �� ��  : spm_send_sbi_common_command
 ��������  : �����ӿ��� ͨ����Ϣ����.
 �������  : NBB_BYTE *rq_buffer   
             NBB_ULONG length      
             NBB_BYTE ucSubCardNo : 1/2 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��1��7��
    ��    ��   : ����
    �޸�����   : �����ɺ���

*****************************************************************************/
NBB_INT spm_send_sbi_common_command(NBB_BYTE *rq_buffer, NBB_ULONG length, NBB_BYTE ucSubCardNo)

{
    NBB_BYTE ucMessage[80];
    NBB_INT iRv = 0;
    ATG_SBI_COMMON_COMMAND *pstSbiCommand = NULL;
    NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();    
    NBB_TRC_ENTRY("spm_send_sbi_common_command");
    
    if (rq_buffer == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_send_sbi_common_command(rq_buffer==NULL)"));

        OS_PRINTF("***ERROR***:spm_send_sbi_common_command(rq_buffer==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_send_sbi_common_command(rq_buffer==NULL)\n");
        
        iRv = ERROR;
        goto EXIT_LABEL;
    }

    if ((ucSubCardNo != 1) && (ucSubCardNo != 2))
    {
        OS_PRINTF("***ERROR***:spm_send_sbi_common_command(ucSubCardNo==%d)\n",ucSubCardNo);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_send_sbi_common_command(ucSubCardNo==%d)\n",ucSubCardNo);
        
        iRv = ERROR;
        goto EXIT_LABEL;
    }
    
    if (SHARED.sub_card_cb[ucSubCardNo-1] != NULL)
    {
        pstSbiCommand = (ATG_SBI_COMMON_COMMAND*) NBB_GET_BUFFER(NBB_NULL_HANDLE,
                                           NBB_ALIGN_OFFSET(sizeof(ATG_SBI_COMMON_COMMAND)),
                                           length, NBB_RETRY_DATA | NBB_BUF_PKT); 

        if (NULL == pstSbiCommand)
        {
            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_send_sbi_common_command(pstSbistatusRq==NULL)"));

            OS_PRINTF("***ERROR***:spm_send_sbi_common_command(pstSbistatusRq==NULL)\n");

            OS_SPRINTF(ucMessage, "***ERROR***:spm_send_sbi_common_command(pstSbistatusRq==NULL)\n");
            
            iRv = ERROR;
            
            goto EXIT_LABEL;
        }
        /*************************************************************************/
        /* ��ʼ����Ϣ��                                                          */
        /*************************************************************************/
        NBB_ZERO_IPS(pstSbiCommand); 
        
        pstSbiCommand->ips_hdr.ips_type = IPS_ATG_SBI_COMMON_COMMAND;

        pstSbiCommand->pkt_hdr.data_start = 0;
        pstSbiCommand->pkt_hdr.data_len = pstSbiCommand->ips_hdr.data_size;  

        /*����״̬��ѯ����*/
        NBB_PKT_COPY_IN(pstSbiCommand, pstSbiCommand->pkt_hdr.data_start, rq_buffer,
                        pstSbiCommand->ips_hdr.data_size);

        spm_snd_sbi_ips(SHARED.sub_card_cb[ucSubCardNo-1], pstSbiCommand, &(pstSbiCommand->ips_hdr) NBB_CCXT);
    } 
        
    EXIT_LABEL: NBB_TRC_EXIT();
    
    return iRv;
}
NBB_VOID spm_rcv_dci_status_rq_new(ATG_DCI_STATUS_RQ *pstStatusRq)
{
    NBB_BYTE *rq_buffer;
    NBB_BYTE ucCharacter = 0;
    NBB_ULONG bak_ulong;
    NBB_LONG ret_len = -1;
    NBB_BYTE emustat = 0;
    SPM_PROC_DATA_FUNC proc_data_func;
    NBB_ULONG inst_type, inst_code, inst_extend;
    NBB_ULONG instruct;
    NBB_ULONG data_len;
    NBB_USHORT us_cmd_seq = 0;
    NBB_ULONG ui_cmd_code;
    NBB_NULL_THREAD_CONTEXT

    NBB_GET_THREAD_CONTEXT();
    NBB_TRC_ENTRY("spm_rcv_dci_status_rq_new");    

    /* �������ָ�������Ч */
    NBB_ASSERT(pstStatusRq != NULL);

    emustat = spm_hardw_getemustate();
    /* ��������ò�λ��ַ���޷�ȷ�ϣ���ôֱ�Ӳ������������˳� */
    if(emustat == 0XFF)
    {
        return;
    }

    data_len = pstStatusRq->pkt_hdr.data_len;

    rq_buffer = ((NBB_BYTE *)((NBB_IPS *)pstStatusRq)->data_ptr 
    + (pstStatusRq->pkt_hdr.data_start));

    /* ָ������ */
    inst_type = rq_buffer[5];
    /* ָ���� */
    inst_code = rq_buffer[6];
    /* ָ����չ�� */
    inst_extend = rq_buffer[7];

    /* ����һ�������ĵ���Э��ָ������ */
    instruct = (inst_type << 16) + (inst_code << 8) + inst_extend;

    us_cmd_seq = *(volatile unsigned short *)(&rq_buffer[1]);
    bak_ulong = *(volatile unsigned int *)(&rq_buffer[10]);
    ui_cmd_code = *(volatile unsigned int *)(&rq_buffer[4]);
    ui_cmd_code = ui_cmd_code & 0X00FFFFFF;

    /* �����������Ҫת�����ӿ�����ôת�����ӿ�. */
    if(spm_need_send_command_sbi(instruct) > 0)
    {
        /* ����ӿ�1 ���ڣ���ת�����ӿ�1. */
        if(NULL != SHARED.sub_card_cb[0])
        {
            spm_send_sbi_common_command(rq_buffer, data_len, 1);
        }
        /* ����ӿ�2 ���ڣ���ת�����ӿ�2. */
        if(NULL != SHARED.sub_card_cb[1])
        {
            spm_send_sbi_common_command(rq_buffer, data_len, 2);
        }
    }

    /* ��ȡ���ݳ��� */
    NBB_GET_LONG(data_len, rq_buffer + 14);

    proc_data_func = spm_get_proc_data_function(instruct);

    /* �·���ѯʱ�ı����ֶΣ��������ص�ʱ��ԭ������. */
    NBB_GET_LONG(bak_ulong,rq_buffer + 10);

    if(NULL != proc_data_func)
    {
        /* ������ش�������ִ������ */
        ret_len = proc_data_func(rq_buffer + SPM_TCP_RQ_HEAD_DATA_SIZE, data_len, 
                SHARED.data_buffer + 28, &ucCharacter);
    }
    /*  */
    else
    {
        NBB_EXCEPTION((PCT_SPM | 6, 123, "s", "spm error instruct handle function."));
    }
    
    /* ֻҪ����Ҫ���أ�����д���ݣ����з��� */
    if(ret_len >= 0)
    {
        /* ���� L3VN Э��ͷ */
        SHARED.data_buffer[0] = 'L';
        SHARED.data_buffer[1] = '3';
        SHARED.data_buffer[2] = 'V';
        SHARED.data_buffer[3] = 'N';

        /* �ϱ���Э��ͷ�������·������󱣳�һ�� */
        NBB_MEMCPY(SHARED.data_buffer + 4, rq_buffer, 8);

        /*
        ����ʶ1���ֶ����壺
        D0=1����ʾ��͸��֡��
        D0=0����ʾ��͸��֡��
        D1=1����ʾ�������ݣ���NMU��BMU�����ݣ�
        D1=0����ʾ�������ݣ���BMU��NMU�����ݡ�
        D7=1����ʾ�����ڲ��̼߳�ͨ��Э�飬
        D7=0����ʾ�������뵥�̼�ͨ��Э�飬��BMU��NMU�����ݡ�
        */
        SHARED.data_buffer[4] = 0;        

        /* ������1-4 */
        NBB_PUT_LONG(SHARED.data_buffer + 12, GetBoardCode());
        
        /* �̵�ַ1-2 */
        NBB_PUT_SHORT(SHARED.data_buffer + 16, BmuGetBoardAddr());
        
        /* ������1-2 */
        SHARED.data_buffer[18] = ucCharacter;
        SHARED.data_buffer[19] = 0;
        NBB_PUT_LONG(SHARED.data_buffer + 20, bak_ulong);

        /* �����ֶ� 1-4 �������������·��ı��ñ���һ�� */
        NBB_PUT_LONG(SHARED.data_buffer + 20, bak_ulong);

        /* ���ݳ��� 1-4 */
        NBB_PUT_LONG(SHARED.data_buffer + 24, ret_len);

        /* ����������֮ǰ�������õ�ʱ���Ѿ��������˴�����Ҫ��䣬ֱ�ӷ��ͼ���. */

        /*
        spm_send_mtl_data_msg(TYPE_MTL_TNE, SHARED.local_frame, emustat, 1, 
        SHARED.data_buffer, (SPM_DATA_TO_NMU_HEAD_SIZE + ret_len));
        */

      //  SpmSendData(1,ret_len,SHARED.data_buffer + 4);
        //SendDataDirect(0x1, 1, ret_len + SPM_DATA_TO_NMU_HEAD_SIZE, SHARED.data_buffer); 
       bmu_send_data_ext(1, 1, instruct, ret_len, SHARED.data_buffer + SPM_DATA_TO_NMU_HEAD_SIZE,
             ucCharacter, us_cmd_seq, bak_ulong);

    }

    EXIT_LABEL : NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
 �� �� ��  : dbg_show_rq_info
 ��������  : ��ӡ����
 �������  : 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��1��18��
    ��    ��   : lx
    �޸�����   : �����ɺ���

*****************************************************************************/
void dbg_show_rq_info(unsigned char en,unsigned short type)
{
    g_dbg_rq_print_en = en;
    g_dbg_rq_focus_type = type;
}

/*****************************************************************************
 �� �� ��  : spm_rcv_dci_status_rq
 ��������  : ������ѯ����״̬����
 �������  : ATG_DCI_STATUS_RQ *pstStatusRq   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2013��6��8��
    ��    ��   : lh09
    �޸�����   : �����ɺ���

*****************************************************************************/
NBB_VOID spm_rcv_dci_status_rq(const NBB_BYTE *rq_buffer, NBB_ULONG len)
{
    NBB_USHORT rq_type, rq_len;
    NBB_ULONG  ulDiscrim, ret_len = 0; 
    NBB_BYTE  ucDcrim;
    NBB_BYTE pucCharacter;
    NBB_BYTE ucSlot = 0;
    NBB_NULL_THREAD_CONTEXT
    NBB_GET_THREAD_CONTEXT();
    NBB_TRC_ENTRY("spm_rcv_dci_status_rq");    
    NBB_ULONG i = 0;
    
    /* �������ָ�������Ч */
    NBB_ASSERT(rq_buffer != NULL);

    NBB_GET_SHORT(rq_type, rq_buffer);
    NBB_GET_SHORT(rq_len, rq_buffer + SPM_SUB_CONF_TYPE_SIZE);
    NBB_GET_LONG(ulDiscrim,rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
    ucDcrim = *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);


    if(1 == g_dbg_rq_print_en)
    {
        if((0 == g_dbg_rq_focus_type) || (rq_type == g_dbg_rq_focus_type))
        {
            for(i = 0;i < len;i++)
            {
                printf("%d  ",rq_buffer[i]);
                if((0 == i % 16) && (i != 0))
                {
                    printf("\n");
                }
            }
         }
    }
    
    if (SPM_STATUS_RQ_TYPE_BFD == rq_type)
    {
        /* ������ѯBFD״̬��Ϣ. */
        spm_bfd_report_sessioninfo(ulDiscrim);
    }
    else if (SPM_STATUS_RQ_TYPE_BOARDSTATE == rq_type)
    {
        /* �ϱ����̻���״̬��Ϣ add by ljuan 2013-07-03 */
        spm_hardw_sendboardstate();
    }
    else if (SPM_STATUS_RQ_TYPE_PHY_PORT == rq_type)
    {
        /* �ϱ����̶˿���Ϣ add by ljuan ,���Ӻ�gaos*/
        #ifdef SPU
        spm_state_port();
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_SYSSTATE == rq_type)
    {
        /* ��������102. �忨ϵͳ����״̬��Ϣ  */
        ret_len = BmuGetSysPhyStat(rq_buffer, len, SHARED.data_buffer, &pucCharacter);
        if(ret_len > 0)
        {
            SendData(0x1, 1, 0x123457, ret_len, SHARED.data_buffer);
        }
        
    }
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_GLOABALST == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
            0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    }
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_OVERHEAD == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
          0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    } 
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_LINE_SIMULATION == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
          0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    } 
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_FIFOST == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
          0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    }
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_CESCLOCK == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
          0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    }
    else if ( SPM_STATUS_RQ_TYPE_CS1A8_J2STATUS == rq_type)
    {
        /* �ӿ� */
        if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
          0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
        {
            spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));
        }
    } 
    else if (SPM_STATUS_RQ_TYPE_ETH_OPTICALMODULE == rq_type)
    {
     #ifdef SPU
        /* ��̫����ģ��״̬��Ϣ */
        spm_hardw_getslot(&ucSlot);
        if(ucDcrim == ucSlot)//�뱾��λ�ŷ���
        {
            /* �ӿ� */
            if(0x1 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1)||
              0x2 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
            {
                spm_send_sbi_status_rq(rq_buffer,len,*(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1));        
            }
            /* ĸ��*/
            else if(0x0 == *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1))
            {
                almpm_send_opticalModuleSt( ucDcrim, *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+1),\
                                                    *(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE+2) );                                
            }
        }
    #endif
    }    
    else if (SPM_STATUS_RQ_TYPE_TIME_SYNCHRONOUS_PORTST == rq_type)
    {
        /* ʱ��ͬ���˿�״̬��Ϣ */
        #ifdef SRC
        spm_ptp_report_port_status(ulDiscrim);
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_TIME_SYNCHRONOUS_GLOABALST == rq_type)
    {
        /* ʱ��ͬ��ȫ��״̬��Ϣ */
        #ifdef SRC
        spm_ptp_report_global_status(ucDcrim);
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_CLOCK_STATE == rq_type)
    {
        /* ʱ��״̬��Ϣ */
        #ifdef SRC
        spm_clk_state_report();
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_CLOCKSOURCE_STATE == rq_type)
    {
        /* ʱ��Դ״̬��Ϣ */
        #ifdef SRC
        spm_clk_source_report(ulDiscrim);
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_VPLS_MAC_TABLE == rq_type)
    {
        /* vpls mac����Ϣ */
        //#ifdef SRC
        //ApiMacGetMacInfo(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
        //#endif
        
        /* vpls mac����Ϣ */
        #ifdef SPU
        spm_vpls_all_mac_info_report(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
        #endif
    }
    else if (SPM_STATUS_RQ_TYPE_VPLS_MAC_TABLEGATHER == rq_type)
    {
        /* vpls mac��������Ϣ */
        //#ifdef SRC
        //ApiMacGetMacExistStatOrNum(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
        //#endif
        
        /* vpls mac��������Ϣ */
        #ifdef SPU
        spm_vpls_mac_info_report(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
        #endif 
    }

    else if(SPM_STATUS_RQ_TYPE_PROTECT_STATE_INFO == rq_type)
    {
        spm_get_linear_protect_status(rq_buffer + SPM_SUB_CONF_TYPE_SIZE + SPM_SUB_CONF_LENGTH_SIZE);
    }   
    
    else if (SPM_STATUS_RQ_TYPE_BOARD_SHOW_CMD == rq_type)
    {
        /* rq_board_show��Ϣ */
        BmuGetBoardStat(rq_buffer, len, SHARED.data_buffer, &pucCharacter);
    }
    else if(SPM_STATUS_RQ_TYPE_FANCONTROL == rq_type)
    {
       /* �忨�¶ȼ����ȿ�����Ϣ */ 
       almpm_send_boardtemp();
    }
    EXIT_LABEL : NBB_TRC_EXIT();

    return;
}

/* Ĭ���޷��ص�ȱʡ״̬��������  */
NBB_LONG spm_proc_default_state(NBB_CONST NBB_BYTE *rq_buf, NBB_CONST NBB_ULONG len, NBB_BYTE *buf, NBB_BYTE *ucCharacter)
{
    return -1;
}



/*****************************************************************************
 �� �� ��  : spm_need_send_command_sbi
 ��������  : ����ָ���룬�ж��Ƿ���Ҫ������ת�����ӿ�����.
 �������  : NBB_ULONG instruct ָ����   
 �������  : ��
 �� �� ֵ  : NBB_INT
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��1��16��
    ��    ��   : chenpeng
    �޸�����   : �����ɺ���

*****************************************************************************/
NBB_INT spm_need_send_command_sbi(NBB_ULONG instruct)
{
    NBB_INT ret = 0;

    switch(instruct)
    {
        /* �澯��ѯ */
        case 0X71000D:
            ret = 1;
            break;
        /* ���ܲ�ѯ */
        case 0X71000E:
            ret = 1;
            break;
        /* ��ѯָ������ */
        case 0X71000F:
            ret = 1;
            break;
        /* �������ο��� */
        case 0X710010:
            ret = 1;
            break;
        /* �澯���� */
        case 0X710110:
            ret = 1;
            break;
        /* ������� */
        case 0X00B200:
            ret = 1;
            break;
        /* ���ܲɼ����� */
        case 0X71FF10:
            ret = 0;
            break;        /* Ĭ����Ϊ��ת����Э�� */
        case 0XB100: 

            /*broadcast timing cmd*/
            ret = 1;
            break;        
        default:
            ret = 0;
            break;
    }
    return ret;
}

