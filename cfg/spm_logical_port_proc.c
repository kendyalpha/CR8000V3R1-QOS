/******************************************************************************

                  版权所有 (C), 1999-2013, 烽火通信科技股份有限公司

******************************************************************************
   文 件 名   : spm_logical_port_proc.c
   版 本 号   : 初稿
   作    者   : xiaoxiang
   生成日期   : 2012年9月18日
   最近修改   :
   功能描述   : 端口逻辑配置处理
   函数列表   :
   修改历史   :
   1.日    期   : 2012年9月18日
    作    者   : xiaoxiang
    修改内容   : 创建文件

******************************************************************************/
#define SHARED_DATA_TYPE SPM_SHARED_LOCAL

#include <nbase.h>
#include <spmincl.h>

#ifdef LOGICAL_PORT_CFG

extern unsigned char logical_port_cfg_print_setting;

/*****************************************************************************
   函 数 名  : spm_check_if_vip_vmac_exist
   功能描述  : 检查一个逻辑端口中虚拟MAC和IP地址配置是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_VIP_VMAC_DATA *pstVipVmac
   输出参数  : 无
   返 回 值  : 0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_if_vipv6_vmac_exist(NBB_ULONG ulPortIndex, 
    ATG_DCI_LOG_VRRP_IPV6_MAC_L3 *pst_vipv6_vmac NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR uc_message[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;/*返回值*/
    SPM_LOGICAL_PORT_CB *pst_logical_port = NULL;

    NBB_TRC_ENTRY("spm_check_if_vipv6_vmac_exist");

    if (pst_vipv6_vmac == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_vipv6_vmac_exist(pstVipVmac==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_vipv6_vmac_exist(pstVipVmac==NULL)\n");

        OS_SPRINTF(uc_message, "***ERROR***:spm_check_if_vipv6_vmac_exist(pstVipVmac==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, uc_message);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", uc_message));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pst_logical_port = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pst_logical_port == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_vipv6_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_vipv6_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(uc_message,
            "***ERROR***:spm_check_if_vipv6_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在 \n",
            ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, uc_message);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", uc_message));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        if (pst_logical_port->vipv6_vmac_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果虚拟MAC和IP地址配置相等，返回存储位置
            if ((spm_ipv6_mac_key_compare(pst_vipv6_vmac, pst_logical_port->vipv6_vmac_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_rcv_dci_set_logical_port
   功能描述  : 端口逻辑配置处理
   输入参数  : ATG_DCI_SET_LOGICAL_PORT *pstSetLogicalPort
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月12日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_rcv_dci_set_logical_port(ATG_DCI_SET_LOGICAL_PORT *pstSetLogicalPort NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];
    NBB_BYTE ucMessageLen = 0;

    NBB_ULONG i = 0;
    NBB_ULONG j = 0;
    NBB_BYTE ucC3Unit = 0;
    NBB_INT ret = SUCCESS;
    NBB_INT iCfgPos = 0;
    NBB_INT v6cfgpos = 0;/* vrrp 的存储位置*/
    NBB_BYTE ucIfExist = ATG_DCI_EXIST;
    NBB_BYTE ucLocalSlot = 0;
    NBB_USHORT usFlowId = 0;
    NBB_ULONG ulVlanPosId = 0;
    NBB_ULONG ul_vlanposid2 = 0;  /* 条目的键值 */
    NBB_BYTE ulAssociateIfKey = 0;

    INTF_T stIntfL3;
    INTF_T stIntfFlow; /* vrrp的结构体*/
    VRRP_ACL_T stvrrp; /* vrrp的结构体*/
    VRRP_ACL_T stvrrp_v6; /* vrrp ipv6的结构体*/
    NBB_BYTE ve_mac[6] = {0x00, 0x01, 0x002, 0x03, 0x04, 0x05};
    SPM_PORT_INFO_CB stPortInfo;
    SPM_VPN_PORT_INFO_CB stVpPortInfo;

    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;
    SPM_TERMINAL_IF_CB *tempterminalifcb = NULL;/* VLAN接口首地址*/
    SPM_TERMINAL_IF_CB *tempterminalifcb_v6 = NULL;/* VLAN接口首地址*/
    NBB_ULONG terminalifnum = 0;/* if num*/
    NBB_ULONG terminalifnum_v6 = 0;/* if num*/
    ATG_DCI_LOG_PORT_LOGIC_L2_DATA stLogicL2Data;
    ATG_DCI_VPWS_UNI_DATA stVpwsUni;
    ATG_DCI_VPLS_UNI_DATA stVplsUni;
    ATG_DCI_LOG_PORT_FLOW_DIFF_SERV stFlowDiffServDel;
    ATG_DCI_LOG_PORT_DIFF_SERV_DATA stDiffServDel;
    ATG_DCI_LOG_PORT_INCLASSIFY_QOS stInclassifyQosDel;
    ATG_DCI_LOG_UP_USER_QOS_POLICY stUpUserQosDel;
    ATG_DCI_LOG_FLOW_UP_USER_QOS stFlowUpUserQosDel;
    SPM_ASSOCIATE_IF_CB *pstAssociateIfCb = NULL;
    SPM_DIFF_SERV_CB *pstdiffservcb = NULL;    
    SPM_FLOW_DIFF_SERV_CB *pstflowdiffservcb = NULL;  
    SPM_INCLASSIFY_QOS_CB *pstinclassifyqoscb = NULL;     
    SPM_TRAFFIC_FILTER_CB *psttrafficfiltercb = NULL; 
    
    /* 获取的子配置 */
    ATG_DCI_LOG_PORT_BASIC_DATA *pstBasicData = NULL;
    ATG_DCI_LOG_PORT_PHY_DATA *pstPhyData = NULL;
    ATG_DCI_LOG_PORT_LOGIC_L3_DATA *pstLogicL3Data = NULL;
    ATG_DCI_LOG_PORT_LOGIC_L2_DATA *pstLogicL2Data = NULL;
    ATG_DCI_LOG_PORT_FLOW_DATA *pstFlowData = NULL;
    ATG_DCI_LOG_PORT_CES_DATA *pstCesData = NULL;
    ATG_DCI_LOG_PORT_VE_MAC_DATA *pstVeMacData = NULL;
    ATG_DCI_LOG_PORT_DIFF_SERV_DATA *pstDiffServData = NULL;
    ATG_DCI_LOG_PORT_INCLASSIFY_QOS *pstInclassifyQosData = NULL;
    ATG_DCI_LOG_UP_USER_QOS_POLICY *pstUpUserQosData = NULL;
    ATG_DCI_LOG_UP_USER_GROUP_QOS *pstUpGroupQosData = NULL;
    ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *pstDownUserQosData = NULL;
    ATG_DCI_LOG_DOWN_USER_GROUP_QOS *pstDownGroupQosData = NULL;
    ATG_DCI_LOG_FLOW_UP_USER_QOS *pstFlowUpUserQosData = NULL;
    ATG_DCI_LOG_PORT_VLAN *pstTerminalIf = NULL;
    ATG_DCI_LOG_PORT_FLOW_DIFF_SERV *pstFlowDiffServData = NULL;
    ATG_DCI_LOG_PORT_TRAFFIC_FILTER *pstTrafficFilter = NULL;
    ATG_DCI_LOG_PORT_DS_L2_DATA *pstDsL2Data = NULL;

    //ATG_DCI_LOG_PORT_FLOW_DATA *pstFlowData[ATG_DCI_LOG_PORT_FLOW_NUM];
    //ATG_DCI_LOG_PORT_FLOW_DIFF_SERV *pstFlowDiffServData[ATG_DCI_LOG_FLOW_DIFF_SERV_NUM];    
    ATG_DCI_LOG_PORT_IPV4_DATA *pstIpv4Data[ATG_DCI_LOG_PORT_IPV4_NUM];
    ATG_DCI_LOG_PORT_IPV6_DATA *pstIpv6Data[ATG_DCI_LOG_PORT_IPV6_NUM];
    ATG_DCI_LOG_PORT_MC_IPV4_DATA *pstMcIpv4Data[ATG_DCI_LOG_PORT_MC_IPV4_NUM];
    ATG_DCI_LOG_PORT_VIP_VMAC_DATA *pstVipVmacData[ATG_DCI_LOG_PORT_VIP_VMAC_NUM];

    /* 获取的子配置 */
    ATG_DCI_LOG_VRRP_IPV6_MAC_L3 *pst_vipv6_vmac_data[ATG_DCI_LOG_PORT_VIP_VMAC_NUM];
    ATG_DCI_LOG_PORT_MC_IPV6_DATA * pstMcIpv6Data[ATG_DCI_LOG_PORT_MC_IPV6_NUM];
    //ATG_DCI_LOG_PORT_TRAFFIC_FILTER *pstTrafficFilter[ATG_DCI_LOG_TRAFFIC_FILTER_NUM];
    //ATG_DCI_LOG_PORT_TERMINAL_IF *pstTerminalIf[ATG_DCI_LOG_TERMINAL_IF_NUM];

    /* IPS消息偏移的首地址 */
    NBB_BYTE *pucBasicDataStart = NULL;
    NBB_BYTE *pucPhyDataStart = NULL;
    NBB_BYTE *pucLogicL3DataStart = NULL;
    NBB_BYTE *pucLogicL2DataStart = NULL;
    NBB_BYTE *pucFlowDataStart = NULL;
    NBB_BYTE *pucCesDataStart = NULL;
    NBB_BYTE *pucIpv4DataStart = NULL;
    NBB_BYTE *pucIpv6DataStart = NULL;
    NBB_BYTE *pucMcMacDataStart = NULL;
    NBB_BYTE *pucVipVmacDataStart = NULL;
    NBB_BYTE *puc_vipv6_vmac_data_start = NULL;/* IPS消息偏移的首地址 */
    NBB_BYTE *pucVeMacDataStart = NULL;
    NBB_BYTE *pucDiffServDataStart = NULL;
    NBB_BYTE *pucFlowDiffServDataStart = NULL;
    NBB_BYTE *pucInclassifyQosDataStart = NULL;
    NBB_BYTE *pucUpUserQosDataStart = NULL;
    NBB_BYTE *pucUpGroupQosDataStart = NULL;
    NBB_BYTE *pucDownUserQosDataStart = NULL;
    NBB_BYTE *pucDownGroupQosDataStart = NULL;
    NBB_BYTE *pucFlowUpUserQosDataStart = NULL;
    NBB_BYTE *pucTrafficFilterDataStart = NULL;
    NBB_BYTE *pucTerminalIfDataStart = NULL;
    NBB_BYTE *pucIpv6McMacDataStart = NULL;
    NBB_BYTE *pucDsL2DataStart = NULL;

    /* 子配置的操作模式 */
    NBB_ULONG ulOperBasic = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperPhy = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperLogicL3 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperLogicL2 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperFlow = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperCes = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperIpv4 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperIpv6 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperMcIpv4 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperVipVmac = ATG_DCI_OPER_NULL;
    NBB_ULONG ul_oper_vipv6_vmac = ATG_DCI_OPER_NULL;/* 子配置的操作模式 */
    NBB_ULONG ulOperVeMac = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperDiffServ = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperFlowDiffServ = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperInclassifyQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperUpUserQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperUpGroupQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperDownUserQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperDownGroupQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperFlowUpUserQos = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperTrafficFilter = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperTerminalIf = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperMcIpv6 = ATG_DCI_OPER_NULL;
    NBB_ULONG ulOperDsL2 = ATG_DCI_OPER_NULL; 

    //QoS结构体
    SPM_QOS_LOGIC_FLOW_KEY stQosFlowKey;
    SPM_QOS_LOGIC_INTF_KEY stQosIntfKey;
    SPM_QOS_LOGIC_UNIVP stUniVp;
    TX_UNI_PORT stTxUniPort;
    SUB_PORT stSubPort;

    NBB_BYTE trigger_if_flag = 0;
    NBB_BYTE act_if_flag = 0;
    NBB_BYTE ucExistPos_trigger = 0;  
    NBB_BYTE ucExistPos_act = 0; 
    NBB_USHORT usPortId = 0;
    NBB_USHORT usMcVePortId = 0;
    NBB_BYTE usSlotId = 0;
    PORT_GROUP_INFO IfGoupInfo;

    NBB_ULONG ulPortIndexKey = 0;
    NBB_ULONG intf_pos_id_temp = 0; /* 条目的键值 */
    NBB_ULONG vrrp4_pos_id_temp = 0; /* 条目的键值 */
    NBB_ULONG vrrp6_pos_id_temp = 0; /* 条目的键值 */
    
    /* 条目的键值 */
    NBB_USHORT usVplsIdKey = 0;
    NBB_USHORT usVsiMcId = 0;
    SPM_VPLS_CB *pstVpls = NULL;
    ATG_DCI_LOG_PORT_VLAN flowdiffservcfgkey ;
    ATG_DCI_LOG_PORT_VLAN inclassifyqoscfgkey ;
    ATG_DCI_LOG_PORT_VLAN trafficfiltercfgkey ;
    ATG_DCI_LOG_PORT_VLAN diffservcfgkey ;/* 条目的键值 */

    NBB_TRC_ENTRY("spm_rcv_dci_set_logical_port");

    /* 输入参数指针必须有效 */
    NBB_ASSERT(pstSetLogicalPort != NULL);

	if (pstSetLogicalPort == NULL)
	{
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:spm_rcv_dci_set_logical_port(pstSetLogicalPort==NULL)"));

        OS_PRINTF("***ERROR***:spm_rcv_dci_set_logical_port(pstSetLogicalPort==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_rcv_dci_set_logical_port(pstSetLogicalPort==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 0, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));

        goto EXIT_LABEL;
    }

	OS_MEMSET(&stIntfL3, 0, sizeof(INTF_T));
	OS_MEMSET(&stIntfFlow, 0, sizeof(INTF_T));
    OS_MEMSET(&stvrrp, 0, sizeof(VRRP_ACL_T));
    OS_MEMSET(&stvrrp_v6, 0, sizeof(VRRP_ACL_T));
    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
    OS_MEMSET(&stLogicL2Data, 0, sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));
    OS_MEMSET(&stVpwsUni, 0, sizeof(ATG_DCI_VPWS_UNI_DATA));
    OS_MEMSET(&stVplsUni, 0, sizeof(ATG_DCI_VPLS_UNI_DATA));
    OS_MEMSET(&stVpPortInfo, 0, sizeof(SPM_VPN_PORT_INFO_CB));
    OS_MEMSET(&stSubPort, 0, sizeof(SUB_PORT));
    OS_MEMSET(&stTxUniPort, 0, sizeof(TX_UNI_PORT));
    OS_MEMSET(&stFlowDiffServDel, 0, sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
    OS_MEMSET(&stDiffServDel, 0, sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
    OS_MEMSET(&stInclassifyQosDel, 0, sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
    OS_MEMSET(&stUpUserQosDel, 0, sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY));
    OS_MEMSET(&stFlowUpUserQosDel, 0, sizeof(ATG_DCI_LOG_FLOW_UP_USER_QOS));
    OS_MEMSET(&stUniVp, 0, sizeof(SPM_QOS_LOGIC_UNIVP));
    OS_MEMSET(&flowdiffservcfgkey, 0, sizeof(ATG_DCI_LOG_PORT_VLAN));
    OS_MEMSET(&inclassifyqoscfgkey, 0, sizeof(ATG_DCI_LOG_PORT_VLAN));
    OS_MEMSET(&trafficfiltercfgkey, 0, sizeof(ATG_DCI_LOG_PORT_VLAN));
    OS_MEMSET(&diffservcfgkey, 0, sizeof(ATG_DCI_LOG_PORT_VLAN));

    //首先将IPS消息的返回值设置为OK，如果有一个子配置失败，则置为FAIL
    pstSetLogicalPort->return_code = ATG_DCI_RC_OK;

    //printf("***ERROR***:PORT_INDEX=%d, ApiC3AddIntf() ret=%d 增加index=%d的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //不挂
    //OS_PRINTF("***ERROR***:PORT_INDEX=%ld, ApiC3AddIntf() ret=%d 增加index=%ld的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //挂
    //OS_PRINTF("***ERROR***:PORT_INDEX=%d, ApiC3AddIntf() ret=%d 增加index=%d的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //不挂
    //OS_PRINTF("***ERROR***:PORT_INDEX=%d, ApiC3AddIntf() ret=%d 增加index=%ld的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //挂
    //OS_PRINTF("***ERROR***:PORT_INDEX=%ld, ApiC3AddIntf() ret=%d 增加index=%d的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //不挂
    //OS_PRINTF("***ERROR***:PORT_INDEX=%d, ApiC3AddIntf() ret=%d 增加index=%d 的INTF失败!\n", ulPortIndexKey, ret,
    // ulPortIndexKey);

    //初始化流配置
    //for (i = 0; i < ATG_DCI_LOG_PORT_FLOW_NUM; i++)
    //{
    //    pstFlowData = NULL;
    //}

    //初始化流相关Diff-Serv配置L2
    //for (i = 0; i < ATG_DCI_LOG_FLOW_DIFF_SERV_NUM; i++)
    //{
    //    pstFlowDiffServData[i] = NULL;
    //}

    //初始化Ipv4地址配置
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV4_NUM; i++)
    {
        pstIpv4Data[i] = NULL;
    }

    //初始化Ipv6地址配置
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV6_NUM; i++)
    {
        pstIpv6Data[i] = NULL;
    }

    //初始化组播组地址
    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV4_NUM; i++)
    {
        pstMcIpv4Data[i] = NULL;
    }

    //初始化VRRP虚拟MAC和IP地址配置
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        pstVipVmacData[i] = NULL;
    }
    
    //初始化VRRP ipv6虚拟MAC和IP地址配置
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        pst_vipv6_vmac_data[i] = NULL;
    }
    

    //初始化ipv6组播地址配置
    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV6_NUM; i++)
    {
        pstMcIpv6Data[i] = NULL;
    }

    //初始化包过滤器traffic_filter配置
    //for (i = 0; i < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; i++)
    //{
    //    pstTrafficFilter[i] = NULL;
    //}

    
#ifdef SPU

    /* 初始化intf */
    ApiC3InitIntfStruct(&stIntfL3);
    ApiC3InitIntfStruct(&stIntfFlow);
#endif

    ulPortIndexKey = pstSetLogicalPort->key;

    NBB_TRC_DETAIL((NBB_FORMAT "ulPortIndexKey = %ld", ulPortIndexKey));

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndexKey);

    if (pstLogicalPort == NULL) /* 如果条目不存在*/
    {
        ucIfExist = ATG_DCI_UNEXIST;
    }

    /* 获取子配置操作 */
    ulOperBasic = pstSetLogicalPort->oper_basic;
    ulOperPhy = pstSetLogicalPort->oper_physical;
    ulOperLogicL3 = pstSetLogicalPort->oper_logical_l3;
    ulOperLogicL2 = pstSetLogicalPort->oper_logical_l2;
    ulOperFlow = pstSetLogicalPort->oper_flow;
    ulOperCes = pstSetLogicalPort->oper_ces;
    ulOperIpv4 = pstSetLogicalPort->oper_ipv4;
    ulOperIpv6 = pstSetLogicalPort->oper_ipv6;
    ulOperMcIpv4 = pstSetLogicalPort->oper_mc_ipv4;
    ulOperVipVmac = pstSetLogicalPort->oper_vip_vmac;
    ul_oper_vipv6_vmac = pstSetLogicalPort->oper_vrrp_ipv6_mac_l3;
    ulOperVeMac = pstSetLogicalPort->oper_ve_mac;
    ulOperDiffServ = pstSetLogicalPort->oper_diff_serv;
    ulOperFlowDiffServ = pstSetLogicalPort->oper_flow_diff_serv;
    ulOperInclassifyQos = pstSetLogicalPort->oper_inclassify_qos;
    ulOperUpUserQos = pstSetLogicalPort->oper_up_user_qos;
    ulOperUpGroupQos = pstSetLogicalPort->oper_upuser_group_qos;
    ulOperDownUserQos = pstSetLogicalPort->oper_down_user_queue_qos;
    ulOperDownGroupQos = pstSetLogicalPort->oper_down_user_group_qos;
    ulOperFlowUpUserQos = pstSetLogicalPort->oper_flow_upuser_qos;
    ulOperTrafficFilter = pstSetLogicalPort->oper_traffic_filter;
    ulOperTerminalIf = pstSetLogicalPort->oper_vlan;
    ulOperMcIpv6 = pstSetLogicalPort->oper_mc_ipv6;
    ulOperDsL2 = pstSetLogicalPort->oper_ds_l2;

    /* 删除整个条目 */
    if (pstSetLogicalPort->delete_struct == TRUE)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  删除PORT_INDEX=%ld 的端口逻辑配置", ulPortIndexKey));

        /* 如果条目不存在，不删除 */
        if (ucIfExist == ATG_DCI_UNEXIST)
        {
            NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:要删除的配置并不存在"));

            OS_PRINTF("***ERROR***:要删除的PORT_INDEX=%ld 的端口逻辑配置并不存在!\n", ulPortIndexKey);

            OS_SPRINTF(ucMessage, "***ERROR***:要删除的PORT_INDEX=%ld 的端口逻辑配置并不存在!\n", ulPortIndexKey);
            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, DEL_UNEXIST_CONFIG_ERROR,
                    "DEL_UNEXIST_CONFIG_ERROR", ucMessage));
                    
            pstSetLogicalPort->return_code = ATG_DCI_RC_OK;

            goto EXIT_LABEL;
        }

        /* 存在删除 */
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT "  查询到此配置，从tree中删除"));
            spm_dbg_print_logical_port_head(ulPortIndexKey, SPM_OPER_DEL);
            spm_dbg_record_logical_port_head(ulPortIndexKey, SPM_OPER_DEL);

            //删除端口逻辑配置驱动配置
            spm_del_logical_port_cfg(pstLogicalPort NBB_CCXT);

			if (pstLogicalPort != NULL)
			{
	            AVLL_DELETE(SHARED.logical_port_tree, pstLogicalPort->spm_logical_port_node);

	            //释放端口逻辑配置节点的内存空间
	            spm_free_logical_port_cb(pstLogicalPort NBB_CCXT);
			}
        }
    }

    /* 增加或更新条目 */
    else
    {

        /* 如果条目不存在，树中要增加条目 */
        if (ucIfExist == ATG_DCI_UNEXIST)
        {
            NBB_TRC_DETAIL((NBB_FORMAT "  增加PORT_INDEX=%ld的 端口逻辑配置", ulPortIndexKey));
            spm_dbg_print_logical_port_head(ulPortIndexKey, SPM_OPER_ADD);
            spm_dbg_record_logical_port_head(ulPortIndexKey, SPM_OPER_ADD);

            pstLogicalPort = spm_alloc_logical_port_cb(NBB_CXT);

			if (pstLogicalPort != NULL)
			{
	            pstLogicalPort->port_index_key = ulPortIndexKey;

	            //coverity[no_effect_test]
	            AVLL_INSERT(SHARED.logical_port_tree, pstLogicalPort->spm_logical_port_node);
			}
        }
        else
        {
            NBB_TRC_DETAIL((NBB_FORMAT "  更新PORT_INDEX=%ld的 端口逻辑配置", ulPortIndexKey));
            spm_dbg_print_logical_port_head(ulPortIndexKey, SPM_OPER_UPD);
            spm_dbg_record_logical_port_head(ulPortIndexKey, SPM_OPER_UPD);
        }

		//增加此判断，取消PC-Lint告警
        if (pstLogicalPort == NULL)
        {
            NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:要增加的PORT_INDEX=%ld 的端口逻辑配置未能成功申请内存!", ulPortIndexKey));

            OS_PRINTF("***ERROR***:要增加的PORT_INDEX=%ld 的端口逻辑配置未能成功申请内存!\n", ulPortIndexKey);

            OS_SPRINTF(ucMessage, "***ERROR***:要增加的PORT_INDEX=%ld 的端口逻辑配置未能成功申请内存!\n", ulPortIndexKey);
            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ALLOC_MEMORY_ERROR,
                    "ALLOC_MEMORY_ERROR", ucMessage));
                    
            pstSetLogicalPort->return_code = ATG_DCI_RC_UNSUCCESSFUL;

            goto EXIT_LABEL;
        }

        /***************************************************************************/
        /* 获取配置                                                                */
        /***************************************************************************/
        /******************************** 基本配置 *********************************/
        if (ulOperBasic == ATG_DCI_OPER_ADD)
        {

            /* 计算第一个entry的地址。*/
            pucBasicDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                &pstSetLogicalPort->basic_data);

            /* 首地址为NULL，异常 */
            if (pucBasicDataStart == NULL)
            {
                NBB_TRC_FLOW((NBB_FORMAT "  LOGICAL PORT pucBasicDataStart is NULL."));
                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
            }
            else
            {
                pstBasicData = (ATG_DCI_LOG_PORT_BASIC_DATA *)pucBasicDataStart;

                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                {
                    printf("  1）基本配置\n");
                    spm_dbg_print_logical_port_basic_cfg(pstBasicData);
                }

                OS_SPRINTF(ucMessage, "  1）基本配置\n");
                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                
                //spm_dbg_record_logical_port_basic_cfg(pstBasicData);

                NBB_TRC_DETAIL((NBB_FORMAT "  端口标识 = %d", pstBasicData->port_flag));
                NBB_TRC_DETAIL((NBB_FORMAT "  端口子类型 = %d", pstBasicData->port_sub_type));
                NBB_TRC_DETAIL((NBB_FORMAT "  接口路由能力 = %d", pstBasicData->port_route_type));
                NBB_TRC_DETAIL((NBB_FORMAT "  子接口类型 = %d", pstBasicData->sub_if_type));

                //获取本槽位号
                //spm_hardw_getslot(&ucLocalSlot);

                /* 保存数据 */
                if (pstLogicalPort->basic_cfg_cb == NULL)
                {
                    pstLogicalPort->basic_cfg_cb =
                        (ATG_DCI_LOG_PORT_BASIC_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_BASIC_DATA),
                        NBB_NORETRY_ACT,
                        MEM_SPM_LOG_PORT_BASIC_CB);
                }

                OS_MEMCPY(pstLogicalPort->basic_cfg_cb, pstBasicData, sizeof(ATG_DCI_LOG_PORT_BASIC_DATA));

                if (pstBasicData->port_flag == ATG_DCI_LOOPBACK)
                {
                    goto EXIT_LABEL;
                }
            }

        }
        else if (ulOperBasic == ATG_DCI_OPER_DEL)
        {
            NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
        }
        else if (ulOperBasic == ATG_DCI_OPER_UPDATE)
        {
            NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
        }

        /******************************** 物理配置 ********************************/
        if (ulOperPhy == ATG_DCI_OPER_ADD)
        {

            /* 计算第一个entry的地址。*/
            pucPhyDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                &pstSetLogicalPort->physical_data);

            /* 首地址为NULL，异常 */
            if (pucPhyDataStart == NULL)
            {
                NBB_TRC_FLOW((NBB_FORMAT "  LOGICAL PORT pucPhyDataStart is NULL."));
                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
            }
            else
            {
                pstPhyData = (ATG_DCI_LOG_PORT_PHY_DATA *)pucPhyDataStart;

                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                {
                    printf("  2）物理配置\n");
                    spm_dbg_print_logical_port_phy_cfg(pstPhyData);
                }

                OS_SPRINTF(ucMessage, "  2）物理配置\n");
                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                
                //spm_dbg_record_logical_port_phy_cfg(pstPhyData);

                NBB_TRC_DETAIL((NBB_FORMAT "  物理接口index  = %ld", pstPhyData->phy_port_index));
                NBB_TRC_DETAIL((NBB_FORMAT "  链路聚合LAG_ID = %ld", pstPhyData->lag_id));
                NBB_TRC_DETAIL((NBB_FORMAT "  虚拟以太网组ID = %ld", pstPhyData->ve_group_id));

				
                //如果条目为更新，且此次下发没有更新基本配置，且之前的条目配置了基本配置
                if (pstLogicalPort->basic_cfg_cb != NULL)
                {
                	//如果端口不为tunnel口或loopback口，需要获取实际portid
                	if ((pstLogicalPort->basic_cfg_cb->port_flag != ATG_DCI_TUNNEL)
				     && (pstLogicalPort->basic_cfg_cb->port_flag != ATG_DCI_LOOPBACK))
					{
	                    if ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_SUB_INTFACE)
	                        || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE))
	                    {
	                    	//coverity[check_return]
	                        spm_get_portid_from_physical_port_index(pstPhyData->phy_port_index,
	                            &stPortInfo NBB_CCXT);
	                    }
	                    else if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)
	                    {
	                        //unit?
	                        stPortInfo.port_id = pstPhyData->ve_group_id;
                            stPortInfo.slot_id = pstPhyData->ve_slot;
#ifdef SPU                  

	                        //ApiC3SetPortType(UNIT_0, stPortInfo.port_id, VE);
#endif

	                        //NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%d, 端口%d为VE, ApiC3SetPortType() ret=%d",
	                        // ulPortIndexKey, stPortInfo.port_id, ret));
	                    }
	                    else if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG)
	                    {
	                        stPortInfo.port_id = pstPhyData->lag_id + ATG_DCI_LAG_OFFSET;
	                    }

	                    pstLogicalPort->port_id = stPortInfo.port_id;
	                    pstLogicalPort->slot_id = stPortInfo.slot_id;
	                    pstLogicalPort->unit_id = stPortInfo.unit_id;

		                //如果获取端口正确才保存
		                if (pstLogicalPort->port_id != 0)
		                {
		                    /* 保存数据 */
		                    if (pstLogicalPort->phy_cfg_cb == NULL)
		                    {
		                        pstLogicalPort->phy_cfg_cb =
		                            (ATG_DCI_LOG_PORT_PHY_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_PHY_DATA),
		                            NBB_NORETRY_ACT,
		                            MEM_SPM_LOG_PORT_PHY_CB);
		                    }

		                    OS_MEMCPY(pstLogicalPort->phy_cfg_cb, pstPhyData, sizeof(ATG_DCI_LOG_PORT_PHY_DATA)); 
                        }
		                else
		                {
		                    pstSetLogicalPort->phy_return_code = ATG_DCI_RC_UNSUCCESSFUL;
		                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:逻辑端口 PORT_INDEX=%ld, 获取物理端口失败",
		                            ulPortIndexKey));

		                    OS_PRINTF("***ERROR***:逻辑端口 PORT_INDEX=%ld, 获取物理端口失败\n", ulPortIndexKey);

		                    OS_SPRINTF(ucMessage, "***ERROR***:逻辑端口 PORT_INDEX=%ld, 获取物理端口失败\n", ulPortIndexKey);
		                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

		                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, PHYSICAL_PORT_NOT_EXIST,
		                            "PHYSICAL_PORT_NOT_EXIST", ucMessage));
		                }
					}	
					else	//如果端口为tunnel口或loopback口，主控会下全0数据给单盘，只保存即可
					{
	                    /* 保存数据 */
	                    if (pstLogicalPort->phy_cfg_cb == NULL)
	                    {
	                        pstLogicalPort->phy_cfg_cb =
	                            (ATG_DCI_LOG_PORT_PHY_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_PHY_DATA),
	                            NBB_NORETRY_ACT,
	                            MEM_SPM_LOG_PORT_PHY_CB);
	                    }

	                    OS_MEMCPY(pstLogicalPort->phy_cfg_cb, pstPhyData, sizeof(ATG_DCI_LOG_PORT_PHY_DATA));
					}

                }
                else
                {
                    pstSetLogicalPort->phy_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    OS_PRINTF("PORT_INDEX=%ld, 物理配置失配，基本配置无法获取!\n", ulPortIndexKey);

                    OS_SPRINTF(ucMessage, "PORT_INDEX=%ld, 物理配置失配，基本配置无法获取!\n", ulPortIndexKey);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, BASIC_CONFIG_NOT_EXIST,
                            "BASIC_CONFIG_NOT_EXIST", ucMessage));
                }

            }
        }
        else if (ulOperPhy == ATG_DCI_OPER_DEL)
        {
            NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
        }
        else if (ulOperPhy == ATG_DCI_OPER_UPDATE)
        {
            NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
        }


        /*****************************************************************************/
        /* 如果下发了新的基本配置。                                                  */
        /* 或条目有更新。                                                            */
        /*                                                                           */
        /* L2端口                                                                    */
        /*****************************************************************************/
        if ((pstLogicalPort->basic_cfg_cb != NULL) && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L2))
        {

            /****************************** 逻辑配置L2 ******************************/
            if (ulOperLogicL2 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucLogicL2DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->logical_l2_data);

                /* 首地址为NULL，异常 */
                if (pucLogicL2DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  LOGICAL PORT pucLogicL2DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstLogicL2Data = (ATG_DCI_LOG_PORT_LOGIC_L2_DATA *)pucLogicL2DataStart;

                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  4）逻辑配置L2\n");
                        spm_dbg_print_logical_port_logic_l2_cfg(pstLogicL2Data);
                    }

                    OS_SPRINTF(ucMessage, "  4）逻辑配置L2\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    //spm_dbg_record_logical_port_logic_l2_cfg(pstLogicL2Data);

                    NBB_TRC_DETAIL((NBB_FORMAT "  接收TAG行为 = %s", pstLogicL2Data->recv_action ? "TAGGED_VLAN" : "RAW_ETH"));
                    NBB_TRC_DETAIL((NBB_FORMAT "  发送TAG行为 = %d", pstLogicL2Data->send_tag_action));
                    NBB_TRC_DETAIL((NBB_FORMAT "  发送增加/替换VLAN ID = %d", pstLogicL2Data->send_vlan_id));
                    NBB_TRC_DETAIL((NBB_FORMAT "  发送增加/替换VLAN PRI = %d", pstLogicL2Data->send_vlan_pri));
                    NBB_TRC_DETAIL((NBB_FORMAT "  IGMP  SNOOPING使能 = %s", pstLogicL2Data->igmp_snooping ? "使能" : "不使能"));
                    NBB_TRC_DETAIL((NBB_FORMAT "  TagType = %x", pstLogicL2Data->tagtype));
                    NBB_TRC_DETAIL((NBB_FORMAT "  MAC学习开关 = %s", pstLogicL2Data->mac_learning_enable ? "开" : "关"));
                    NBB_TRC_DETAIL((NBB_FORMAT "  水平分割开关 = %s", pstLogicL2Data->split_horizon_enable ? "开" : "关"));
                    //NBB_TRC_DETAIL((NBB_FORMAT "  流类型 = %d", pstLogicL2Data->flow_type));

                    //如果posid==0，说明vp并没有加入到VPWS/VPLS中，只需要保存
                    if (pstLogicalPort->logic_port_info_cb.vp_idx == 0)
                    {
                        /* 保存数据 */
                        if (pstLogicalPort->logic_l2_cfg_cb == NULL)/*lint !e613 */
                        {
                            pstLogicalPort->logic_l2_cfg_cb =
                                (ATG_DCI_LOG_PORT_LOGIC_L2_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA),
                                NBB_NORETRY_ACT,
                                MEM_SPM_LOG_PORT_L2_CB);
                            
                            if (pstLogicalPort->logic_l2_cfg_cb == NULL)/*lint !e613 */
                            {
                                OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
            	
                    			OS_SPRINTF(ucMessage, "	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                    			BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                goto EXIT_LABEL;
                    			
                            }
                        }

                        OS_MEMCPY(pstLogicalPort->logic_l2_cfg_cb, pstLogicL2Data,
                            sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));/*lint !e613 */
                    }
                    else    //如果posid!=0，说明vp已经加入到VPWS/VPLS中，需要更新
                    {
                        //临时保存一下，如果更新不成功，还原配置
                        OS_MEMCPY(&stLogicL2Data, pstLogicalPort->logic_l2_cfg_cb,
                            sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));
                        OS_MEMCPY(pstLogicalPort->logic_l2_cfg_cb, pstLogicL2Data,
                            sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));
                        
/*                       
                        ret = SUCCESS;

                        for (ucC3Unit = 0; ucC3Unit < SHARED.c3_num; ucC3Unit++)
                        {
                            ret += ApiC3DelMplsPortFromVpn(ucC3Unit,
                                pstLogicalPort->logic_port_info_cb.vpn_type,
                                pstLogicalPort->logic_port_info_cb.vpn_id,
                                pstLogicalPort->logic_port_info_cb.vp_idx);
                        }

#endif

                        //更新VP
                        if (ret == SUCCESS)
                        {
                            stVpwsUni.port_index = ulPortIndexKey;
                            stVpwsUni.uni_no = pstLogicalPort->logic_port_info_cb.vp_idx;

                            OS_MEMSET(&stVpPortInfo, 0, sizeof(SPM_VPN_PORT_INFO_CB));

                            //stVpPortInfo.vp_idx = pstLogicalPort->logic_port_info_cb.vp_idx;

*/
                        ret = SUCCESS;

                        //更新intf的入口识别动�
                        for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
                	         pstTerminalIfCb != NULL;
                	         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
                	                       pstTerminalIfCb->spm_terminal_if_node))
                        {
                            if(0 != pstTerminalIfCb->intf_pos_id)
                            {
                                #ifdef SPU
                                ret = fhdrv_psn_l3_set_intf_ptag(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,
                                    pstLogicalPort->logic_l2_cfg_cb->recv_action);                              

                                if((2 == SHARED.c3_num) && 
                                    ((stPortInfo.port_type == ATG_DCI_LAG) 
                                    || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                {
                                    ret += fhdrv_psn_l3_set_intf_ptag(1, pstTerminalIfCb->intf2_pos_id,
                                        pstLogicalPort->logic_l2_cfg_cb->recv_action);
                                } 

                                if(SUCCESS != ret)
                                {
                                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3SetIntfPtag() ret=%d, "
                                            "PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L2刷新失败",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id));

			                        OS_PRINTF("***ERROR***:ApiC3SetIntfPtag() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d "
                                            "CVLAN_ID=%d 的 逻辑配置L2刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            	"***ERROR***:ApiC3SetIntfPtag() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d "
			                            	"CVLAN_ID=%d 的 逻辑配置L2刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
                                }
                                
                                #endif
                            }
                        }   

                        //更新vp
                        if (ret == SUCCESS)
                        {
                            stVpwsUni.port_index = ulPortIndexKey;
                            stVpwsUni.uni_no = pstLogicalPort->logic_port_info_cb.vp_idx;/*lint !e613 */

                            OS_MEMSET(&stVpPortInfo, 0, sizeof(SPM_VPN_PORT_INFO_CB));

                            stVpPortInfo.vp_idx = pstLogicalPort->logic_port_info_cb.vp_idx;/*lint !e613 */

                            usVsiMcId = 0;
                            
                            if (L2VPN_VPLS == pstLogicalPort->logic_port_info_cb.vpn_type)    //条目不存在
                            {
                                usVplsIdKey = pstLogicalPort->logic_port_info_cb.vpn_id;    
                                pstVpls = AVLL_FIND(SHARED.vpls_tree, &usVplsIdKey);
                                
                                if ((NULL != pstVpls) && (NULL != pstVpls->basic_cfg_cb))   //条目不存在
                                {
                                    usVsiMcId = pstVpls->basic_cfg_cb->mc_id;
                                }    
                            }                           
   
                            ret = spm_vpn_add_vp_uni(pstLogicalPort->logic_port_info_cb.vpn_type,
                                pstLogicalPort->logic_port_info_cb.vpn_id,
                                usVsiMcId,
                                &stVpwsUni,
                                &stVpPortInfo NBB_CCXT);/*lint !e613 */

                            if (ret != SUCCESS)
                            {
                                OS_MEMCPY(pstLogicalPort->logic_l2_cfg_cb, &stLogicL2Data,
                                    sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));

                                NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:spm_vpn_add_vp_uni() ret=%d 更新 VP_UNI失败 vpnid=%d vpntype=%d",
                                        ret,
                                        pstLogicalPort->logic_port_info_cb.vpn_id,
                                        pstLogicalPort->logic_port_info_cb.vpn_type));

                                OS_PRINTF("***ERROR***:spm_vpn_add_vp_uni() ret=%d 更新 VP_UNI失败 vpnid=%d vpntype=%d\n",
                                    ret,
                                    pstLogicalPort->logic_port_info_cb.vpn_id,
                                    pstLogicalPort->logic_port_info_cb.vpn_type);

                                OS_SPRINTF(ucMessage,
                                    "***ERROR***:spm_vpn_add_vp_uni() ret=%d 更新 VP_UNI失败 vpnid=%d vpntype=%d\n",
                                    ret,
                                    pstLogicalPort->logic_port_info_cb.vpn_id,
                                    pstLogicalPort->logic_port_info_cb.vpn_type);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));
                            }
                        }
                        else
                        {
                            OS_MEMCPY(pstLogicalPort->logic_l2_cfg_cb, &stLogicL2Data,
                                sizeof(ATG_DCI_LOG_PORT_LOGIC_L2_DATA));

                            NBB_TRC_DETAIL((NBB_FORMAT
                                    "  ***ERROR***:PORT_INDEX=%ld, ApiC3DelMplsPortFromVpn() ret=%d! 删除 VP_UNI失败",
                                    ulPortIndexKey, ret));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld, ApiC3DelMplsPortFromVpn() ret=%d! 删除 VP_UNI失败",
                                ulPortIndexKey,
                                ret);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, ApiC3DelMplsPortFromVpn() ret=%d! 删除 VP_UNI失败",
                                ulPortIndexKey,
                                ret);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_C3_FUNC_ERROR", ucMessage));
                        }
                    }
                }
            }
            else if (ulOperLogicL2 == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperLogicL2 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 逻辑配置CES/CEP ******************************/
            if (ulOperCes == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucCesDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ces_data);

                /* 首地址为NULL，异常 */
                if (pucCesDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucCesDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstCesData = (ATG_DCI_LOG_PORT_CES_DATA *)pucCesDataStart;

                    //NBB_TRC_DETAIL((NBB_FORMAT "CES链路ID   = %d", pstCesData->ces_id));
                    //NBB_TRC_DETAIL((NBB_FORMAT "QOS策略索引 = %d", pstCesData->qos_id));
                    //NBB_TRC_DETAIL((NBB_FORMAT "ACL策略索引 = %d", pstCesData->acl_id));

                    /* 配置处理，todo */

                    /* 保存数据 */
                    if (pstLogicalPort->ces_cfg_cb == NULL)
                    {
                        pstLogicalPort->ces_cfg_cb =
                            (ATG_DCI_LOG_PORT_CES_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_CES_DATA),
                            NBB_NORETRY_ACT,
                            MEM_SPM_LOG_PORT_CES_CB);
                    }

                    OS_MEMCPY(pstLogicalPort->ces_cfg_cb, pstCesData, sizeof(ATG_DCI_LOG_PORT_CES_DATA));
                }
            }
            else if (ulOperCes == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperCes == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

	        /****************************** VLAN属性 L2处理******************************/
	        if (ulOperTerminalIf == ATG_DCI_OPER_ADD)
			{

	            /* 计算第一个entry的地址。*/
	            pucTerminalIfDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
	                &pstSetLogicalPort->vlan_data);

	            /* 首地址为NULL，异常 */
	            if (pucTerminalIfDataStart == NULL)
	            {
	                NBB_TRC_FLOW((NBB_FORMAT "  pucTerminalIfDataStart is NULL."));
	                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
	            }
	            else
	            {
	                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                {
	                    printf("  21）VLAN属性 L2 (num = %d) ADD\n", pstSetLogicalPort->vlan_num);
	                }

	                OS_SPRINTF(ucMessage, "  21）VLAN属性 L2 (num = %d) ADD\n", 
                           pstSetLogicalPort->vlan_num);
	                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                            
                    //如果此逻辑端口不存在，只保存flow vlan配置，建VPWS/VPLS时建流。
                    if (ucIfExist == ATG_DCI_UNEXIST)
                    {

	                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
		                {
		                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
		                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

		                    /* 配置处理，todo */
		                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
		                    {
		                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
		                    }

		                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

		                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
		                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

			                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

							//如果不存在，申请树节点，插入树中，计数加1
			                if (NULL == pstTerminalIfCb)
			                {
			                	pstTerminalIfCb = spm_alloc_terminal_if_cb(NBB_CXT);
			                	
				                //保存数据并插入树中
				                //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
				                NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
				                AVLL_INSERT(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

				                pstLogicalPort->terminal_if_num++;
			                }
							else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
							{
								pstTerminalIfCb->intf_pos_id = stIntfFlow.posId;
								NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
							}
		                }
                    }
                    //如果逻辑端口存在，流有增加或更新操作，需要循环遍历已存在的流
                    else
                    {
                        OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                        spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                        //1、如果端口不属于任何VPWS/VPLS
                        //2、如果已经属于VPWS/VPLS，但端口不属于本槽位，只保存配置
                        if ((pstLogicalPort->logic_port_info_cb.vp_idx == 0)
                            || ((pstLogicalPort->logic_port_info_cb.vp_idx != 0)
                                && (stPortInfo.slot_id != SHARED.local_slot_id)
                                && (stPortInfo.port_type != ATG_DCI_VE_PORT)
                                && (stPortInfo.port_type != ATG_DCI_LAG)))
                        {

		                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
			                {
			                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
			                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

			                    /* 配置处理，todo */
			                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
			                    {
			                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
			                    }

			                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

			                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
			                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

				                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

								//如果不存在，申请树节点，插入树中，计数加1
				                if (NULL == pstTerminalIfCb)
				                {
				                	pstTerminalIfCb = spm_alloc_terminal_if_cb(NBB_CXT);
				                	
					                //保存数据并插入树中
					                //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
					                NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
					                AVLL_INSERT(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

					                pstLogicalPort->terminal_if_num++;
				                }
								else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
								{
									pstTerminalIfCb->intf_pos_id = stIntfFlow.posId;
									NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
								}
			                }
                        }
                        //如果端口已经在VPWS/VPLS中
                        else
                        {

		                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
			                {
			                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
			                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

			                    /* 配置处理，todo */
			                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
			                    {
			                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
			                    }

			                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

			                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
			                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

                                //stIntfFlow.flags |= INTF_COUNTER_ON;

								//入口识别
                                if ((pstLogicalPort->logic_l2_cfg_cb != NULL) 
                                 && (pstLogicalPort->logic_l2_cfg_cb->recv_action == ATG_DCI_TAGGED_VLAN))
                                {
                                	stIntfFlow.flags |= INTF_PTAG_IDENTIFY_ENABLE;
                                }

                                stIntfFlow.mCardPort = pstLogicalPort->port_id;      //这里要用获取端口函数替换;//todo
                                stIntfFlow.eIntfType = INTF_TYPE_L2;
                                stIntfFlow.bindId = pstLogicalPort->logic_port_info_cb.vp_idx;

                                NBB_TRC_DETAIL((NBB_FORMAT "  stIntfFlow.bindId = %d", stIntfFlow.bindId));

			                    //流类型 :基于端口
			                    if ((pstTerminalIf->svlan_id == 0) && (pstTerminalIf->cvlan_id == 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfFlow.mIntfMatchType = INTF_MATCH_L2_VE_PORT;
				                    }
				                    else
				                    {
				                    	stIntfFlow.mIntfMatchType = INTF_MATCH_PORT;
				                    }
			                        
			                        stIntfFlow.mOvid = 0;
			                        stIntfFlow.mIvid = 0;
			                    }

			                    //基于端口+VLAN（Trunk）
			                    else if ((pstTerminalIf->svlan_id != 0) && (pstTerminalIf->cvlan_id == 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfFlow.mIntfMatchType = INTF_MATCH_L2_VE_PORT_OVID;
				                    }
				                    else
				                    {
				                    	stIntfFlow.mIntfMatchType = INTF_MATCH_PORT_OVID;
				                    }
			                        
			                        stIntfFlow.mOvid = pstTerminalIf->svlan_id;
			                        stIntfFlow.mIvid = 0;
			                    }

			                    //基于端口+QINQ
			                    else if ((pstTerminalIf->svlan_id != 0) && (pstTerminalIf->cvlan_id != 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfFlow.mIntfMatchType = INTF_MATCH_L2_VE_PORT_OVID_IVID;
				                    }
				                    else
				                    {
				                    	stIntfFlow.mIntfMatchType = INTF_MATCH_PORT_OVID_IVID;
				                    }
			                        
			                        stIntfFlow.mOvid = pstTerminalIf->svlan_id;
			                        stIntfFlow.mIvid = pstTerminalIf->cvlan_id;
			                    }
			                    else
			                    {
			                        OS_PRINTF("***WARNNING***: PORT_INDEX=%ld 只配置了CVlan，未配置SVlan\n", ulPortIndexKey);
			                    }

	                        	ret = SUCCESS;
#ifdef SPU
	                            ret = ApiC3AddIntf(pstLogicalPort->unit_id, &stIntfFlow);
                                   intf_pos_id_temp = stIntfFlow.posId;
	                            NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%ld, ApiC3AddIntf() ret = %d",
	                                    ulPortIndexKey, ret));
                                   stIntfFlow.posId = 0;
                                
                                   if((2 == SHARED.c3_num) && 
                                        ((stPortInfo.port_type == ATG_DCI_LAG) 
                                        || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                   {
                                        ret += ApiC3AddIntf(1, &stIntfFlow);
                                        NBB_TRC_DETAIL((NBB_FORMAT "  1: PORT_INDEX=%ld, ApiC3AddIntf() ret = %d",
	                                    ulPortIndexKey, ret));
                                   }                             
#endif
			                    if (ret == SUCCESS)
			                    {

					                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

									//如果不存在，申请树节点，插入树中，计数加
					                if (NULL == pstTerminalIfCb)
					                {
					                	pstTerminalIfCb = spm_alloc_terminal_if_cb(NBB_CXT);
					                	
						                //保存数据并插入树中
						                pstTerminalIfCb->intf_pos_id = intf_pos_id_temp;
                                                          pstTerminalIfCb->intf2_pos_id = stIntfFlow.posId;
						                NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
						                AVLL_INSERT(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

						                pstLogicalPort->terminal_if_num++;
					                }
									else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
									{
                                                                    pstTerminalIfCb->intf_pos_id = intf_pos_id_temp;
                                                                    pstTerminalIfCb->intf2_pos_id = stIntfFlow.posId;
										NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
									}
#ifdef SPU
									almpm_addcfg_intf_onePos(ulPortIndexKey, pstTerminalIfCb->intf_pos_id);
#endif					                
			                    }
			                    else
			                    {
			                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

			                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 增加失败",
			                        		ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id));

			                        OS_PRINTF("***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 增加失败\n",
			                        		ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            "***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 增加失败\n",
			                            	ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
			                    }
			                }
                        }
                    }	                
	            }
	        }
	        else if (ulOperTerminalIf == ATG_DCI_OPER_DEL)
	        {

	            /* 计算第一个entry的地址。*/
	            pucTerminalIfDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
	                &pstSetLogicalPort->vlan_data);

	            /* 首地址为NULL，异常 */
	            if (pucTerminalIfDataStart == NULL)
	            {
	                NBB_TRC_FLOW((NBB_FORMAT "  pucTerminalIfDataStart is NULL."));
	                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
	            }
	            else
	            {
	                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                {
	                    printf("  21）VLAN属性 L2 (num = %d) DEL\n", pstSetLogicalPort->vlan_num);
	                }

	                OS_SPRINTF(ucMessage, "  21）VLAN属性 L2 (num = %d) DEL\n", 
                           pstSetLogicalPort->vlan_num);
	                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                
                    //如果逻辑端口存在，流有删除操作，需要循环遍历已存在的流
                    if (ucIfExist == ATG_DCI_EXIST)
                    {
                        OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                        spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);
                        
                        //1、如果端口不属于任何VPWS/VPLS
                        //2、如果已经属于VPWS/VPLS，但端口不属于本槽位，只删除配置
                        if ((pstLogicalPort->logic_port_info_cb.vp_idx == 0)
                            || ((pstLogicalPort->logic_port_info_cb.vp_idx != 0)
                                && (stPortInfo.slot_id != SHARED.local_slot_id)
                                && (stPortInfo.port_type != ATG_DCI_VE_PORT)
                                && (stPortInfo.port_type != ATG_DCI_LAG)))
                        {

                            //获取配置
		                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
			                {
			                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
			                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

			                    /* 配置处理，todo */
			                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
			                    {
			                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
			                    }

			                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

			                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
			                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

				                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

								//如果不存在，无法删除
				                if (NULL == pstTerminalIfCb)
				                {
			                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

			                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id));

			                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            "***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
				                }
								else
								{
						            AVLL_DELETE(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

						            //释放端口逻辑配置节点的内存空间
						            spm_free_terminal_if_cb(pstTerminalIfCb NBB_CCXT);
						            
									pstLogicalPort->terminal_if_num--;									
								}
			                    
                            }
                        }
                        //如果端口已经在VPWS/VPLS中，说明已经建流，删除相应的流
                        else
                        {

                            //获取配置
		                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
			                {
			                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
			                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

			                    /* 配置处理，todo */
			                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
			                    {
			                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
			                    }

			                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

			                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
			                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

								pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

								//如果不存在，无法删除
				                if (NULL == pstTerminalIfCb)
				                {
			                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

			                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id));

			                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            "***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
				                }
				                else	//存在删除
                                            {
                                                ret = SUCCESS;
#ifdef SPU
                                                ret = ApiC3DelIntf(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id);

                                                NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%ld, ApiC3DelIntf() ret = %d",
                                                ulPortIndexKey, ret));

                                                if((2 == SHARED.c3_num) && 
                                                    ((stPortInfo.port_type == ATG_DCI_LAG) 
                                                    || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                                {
                                                    ret += ApiC3DelIntf(1, pstTerminalIfCb->intf2_pos_id);
                                                    NBB_TRC_DETAIL((
                                                    NBB_FORMAT "  1: PORT_INDEX=%ld, ApiC3DelIntf() ret = %d",
                                                    ulPortIndexKey, ret));
                                                }                                     
#endif
									if (ret == SUCCESS)
									{
#ifdef SPU
										almpm_delline_intf(ulPortIndexKey, pstTerminalIfCb->intf_pos_id, 1);
#endif
							            AVLL_DELETE(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

							            //释放端口逻辑配置节点的内存空间
							            spm_free_terminal_if_cb(pstTerminalIfCb NBB_CCXT);
							            
										pstLogicalPort->terminal_if_num--;
									}
									else
									{
				                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

				                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3DelIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 增加失败",
				                        		ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id));

				                        OS_PRINTF("***ERROR***:ApiC3DelIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 删除失败\n",
				                        		ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id);

				                        OS_SPRINTF(ucMessage,
				                            "***ERROR***:ApiC3DelIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L2 删除失败\n",
				                            	ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id);
				                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

				                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
				                                "CALL_FUNC_ERROR", ucMessage));
				                    }
								}		
                            }			                
                        }
                    }	                
	            }
	        }
	        else if (ulOperTerminalIf == ATG_DCI_OPER_UPDATE)
	        {
	            NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
	        }

            /****************************** 流相关Diff-Serv配置L2 ******************************/
            if (ulOperFlowDiffServ == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucFlowDiffServDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->flow_diff_serv_data);

                /* 首地址为NULL，异常 */
                if (pucFlowDiffServDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucFlowDiffServDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                
                    OS_SPRINTF(ucMessage, "  13）流相关Diff-Serv配置L2 ADD\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->flow_diff_serv_num; i++)
                    {
                        pstFlowDiffServData = (ATG_DCI_LOG_PORT_FLOW_DIFF_SERV *)
                            (pucFlowDiffServDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV)))
                            * i);

                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            printf("  13）流相关Diff-Serv配置L2 ADD\n");
                            spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                        }

                        //OS_SPRINTF(ucMessage, "  13）流相关Diff-Serv配置L2 ADD\n");
                        //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
                        //spm_dbg_record_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);

                        NBB_TRC_DETAIL((NBB_FORMAT "  Diff-Serv模板索引   = %d", pstFlowDiffServData->diff_serv_id));


                        //如果逻辑端口存在，且流已加入到VPN，可以建QoS
                        if (ucIfExist == ATG_DCI_EXIST)
                        {
                            OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                            spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);
                            stSubPort.unit = stPortInfo.unit_id;
                            stSubPort.port = stPortInfo.port_id;
                            stSubPort.slot = stPortInfo.slot_id;
                            stSubPort.ivlan = stPortInfo.cvlan;
                            stSubPort.ovlan = stPortInfo.svlan;

                            /* 配置处理，todo */
                            stQosFlowKey.index = ulPortIndexKey;
                            stQosFlowKey.svlan = pstFlowDiffServData->svlan;
                            stQosFlowKey.cvlan = pstFlowDiffServData->cvlan;

							ulVlanPosId = 0;
		                    spm_get_poscnt_from_intf(ulPortIndexKey, 
		                    	pstFlowDiffServData->svlan, 
		                    	pstFlowDiffServData->cvlan, 
		                    	&ulVlanPosId NBB_CCXT);
		                    	
		                    stSubPort.posid = ulVlanPosId;
                            if(0 == ulVlanPosId)
                            {
                                NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败,posId=0!\n",
                                        ulPortIndexKey, pstFlowDiffServData->svlan, pstFlowDiffServData->cvlan));

                                OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败,posId=0!\n",
                                        ulPortIndexKey, pstFlowDiffServData->svlan, pstFlowDiffServData->cvlan);
                                    
                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));

                                OS_SPRINTF(ucMessage,
                                    "  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败,posId=0!\n",
                                        ulPortIndexKey, pstFlowDiffServData->svlan, pstFlowDiffServData->cvlan);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            }

                            else
                            {
                                ret = spm_ds_add_logic_flow_node(&stSubPort,
                                &stQosFlowKey,
                                pstFlowDiffServData
                                NBB_CCXT);
                                
                                if((2 == SHARED.c3_num) && 
                                     ((stPortInfo.port_type == ATG_DCI_LAG) 
                                     || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                {
                                    ul_vlanposid2 = 0;
                                    spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                    pstFlowDiffServData->svlan, 
                                    pstFlowDiffServData->cvlan, 
                                    &ul_vlanposid2 NBB_CCXT);

                                    if(0 != ul_vlanposid2)
                                    {
                                        stSubPort.unit = 1;
                                        stSubPort.posid = ul_vlanposid2;
                                        ret += spm_ds_add_logic_flow_node(&stSubPort,
                                        &stQosFlowKey,
                                        pstFlowDiffServData
                                        NBB_CCXT);                                     
                                    }
                                }                               

                                if (ret != SUCCESS)
                                {
                                    NBB_TRC_DETAIL((NBB_FORMAT
                                            "  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败",
                                            ulPortIndexKey, pstFlowDiffServData->svlan, pstFlowDiffServData->cvlan));

                                    OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败!\n",
                                        ulPortIndexKey,
                                        ret,
                                        pstFlowDiffServData->svlan, 
                                        pstFlowDiffServData->cvlan);
                                        
                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                            "CALL_FUNC_ERROR", ucMessage));

                                    OS_SPRINTF(ucMessage,
                                        "***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 增加 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败!\n",
                                        ulPortIndexKey,
                                        ret,
                                        pstFlowDiffServData->svlan, 
                                        pstFlowDiffServData->cvlan);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                }
                                else
                                {
                                    //flowdiffservcfgkey.svlan_id = pstFlowDiffServData->svlan;
                                    //flowdiffservcfgkey.cvlan_id = pstFlowDiffServData->cvlan;
                                    //pstflowdiffservcb = AVLL_FIND(pstLogicalPort->flow_diff_serv_tree, 
                                        //&flowdiffservcfgkey);
                                    pstflowdiffservcb = AVLL_FIND(pstLogicalPort->flow_diff_serv_tree, 
                                        pstFlowDiffServData);

                                    //如果不存在，申请树节点，插入树中，计数加1
                                    if (NULL == pstflowdiffservcb)
                                    {
                                        pstflowdiffservcb = spm_alloc_flow_diff_serv_cb(NBB_CXT);
                                        
                                        if (NULL == pstflowdiffservcb)/*lint !e613 */
                                        {
                                            OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                        	
                                			OS_SPRINTF(ucMessage, "	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                                			BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                            goto EXIT_LABEL;
                                			
                                        }
                                        
                                        //保存数据并插入树中
                                        //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
                                        //NBB_MEMCPY(&pstflowdiffservcb->key,&flowdiffservcfgkey,
                                            //sizeof(ATG_DCI_LOG_PORT_VLAN));
                                        NBB_MEMCPY(&(pstflowdiffservcb->flow_diff_serv_cfg),
                                            pstFlowDiffServData, sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
                                        AVLL_INSERT(pstLogicalPort->flow_diff_serv_tree, 
                                            pstflowdiffservcb->spm_flow_diff_serv_node);

                                        pstLogicalPort->flow_diff_serv_num++;
                                    }
                                    else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
                                    {
                                        //pstdiffservcb->intf_pos_id = stIntfFlow.posId;
                                        NBB_MEMCPY(&(pstflowdiffservcb->flow_diff_serv_cfg), 
                                            pstFlowDiffServData, sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
                                    }
                                }                                
                            }
#if 0
                            }
		                    
                            
                            if (ret == SUCCESS)
                            {
                                pstLogicalPort->logic_port_info_cb.flow_info_cb.if_qos = ATG_DCI_USED;

                                /* 保存数据 */
                                if (pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1] == NULL)
                                {
                                    pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1] =
                                        (ATG_DCI_LOG_PORT_FLOW_DIFF_SERV *)NBB_MM_ALLOC(sizeof(
                                            ATG_DCI_LOG_PORT_FLOW_DIFF_SERV),
                                        NBB_NORETRY_ACT,
                                        MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);

                                    OS_MEMCPY(pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1],
                                        pstFlowDiffServData,
                                        sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));

                                    pstLogicalPort->flow_diff_serv_num++;
                                }
                                else    //如果不为NULL，说明数据需要更新
                                {
                                    OS_MEMCPY(pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1],
                                        pstFlowDiffServData,
                                        sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
                                }
                            }
                            else
                            {
                                //NBB_TRC_DETAIL((NBB_FORMAT
                                        //"  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 修改 flow_id=%d 的流相关Diff-Serv配置失败",
                                        //ulPortIndexKey, usFlowId));

                                //OS_PRINTF(
                                   // "***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 修改flow_id=%d 的流相关Diff-Serv配置失败!\n",
                                   // ulPortIndexKey,
                                   // ret,
                                   // usFlowId);
                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));

                                //OS_SPRINTF(
                                    //ucMessage,
                                    //"***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 修改flow_id=%d 的流相关Diff-Serv配置失败!\n",
                                    //ulPortIndexKey,
                                    //ret,
                                    //usFlowId);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                pstSetLogicalPort->flow_diff_serv_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                            }
#endif 
                        }
                        else
                        {
                            pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立Flow-Diff-Serv失败",
                                ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立Flow-Diff-Serv失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立Flow-Diff-Serv失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                        }                    
                    }
                }
            }
            else if (ulOperFlowDiffServ == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucFlowDiffServDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->flow_diff_serv_data);

                /* 首地址为NULL，异常 */
                if (pucFlowDiffServDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucFlowDiffServDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                { 
                
                    OS_SPRINTF(ucMessage, "  13）流相关Diff-Serv配置L2 DEL\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->flow_diff_serv_num; i++)
                    {
                        pstFlowDiffServData = (ATG_DCI_LOG_PORT_FLOW_DIFF_SERV *)
                            (pucFlowDiffServDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV)))
                            * i);

                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            printf("  13）流相关Diff-Serv配置L2 DEL\n");
                            spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                        }

                        //OS_SPRINTF(ucMessage, "  13）流相关Diff-Serv配置L2 DEL\n");
                        //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
                        //spm_dbg_record_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);

                        //NBB_TRC_DETAIL((NBB_FORMAT "  流ID                = %d", pstFlowDiffServData[i]->flow_id));
                        NBB_TRC_DETAIL((NBB_FORMAT "  Diff-Serv模板索引   = %d", pstFlowDiffServData->diff_serv_id));

                        //usFlowId = pstFlowDiffServData[i]->flow_id;

                        //如果逻辑端口存在，且流已加入到VPN，可以增加QoS
                        if (ucIfExist == ATG_DCI_EXIST)
                        {
                            OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                            spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);
                            stSubPort.unit = stPortInfo.unit_id;
                            stSubPort.port = stPortInfo.port_id;
                            stSubPort.slot = stPortInfo.slot_id;
                            stSubPort.ivlan = stPortInfo.cvlan;
                            stSubPort.ovlan = stPortInfo.svlan;

                            /* 配置处理，todo */
                            stQosFlowKey.index = ulPortIndexKey;
                            stQosFlowKey.svlan = pstFlowDiffServData->svlan;
                            stQosFlowKey.cvlan = pstFlowDiffServData->cvlan;

							ulVlanPosId = 0;
		                    spm_get_poscnt_from_intf(ulPortIndexKey, 
		                    	pstFlowDiffServData->svlan, 
		                    	pstFlowDiffServData->cvlan, 
		                    	&ulVlanPosId NBB_CCXT);
		                    	
		                    stSubPort.posid = ulVlanPosId;
		                    
                            pstFlowDiffServData->diff_serv_id = 0;

                            ret = spm_ds_add_logic_flow_node(&stSubPort,
                                &stQosFlowKey,
                                pstFlowDiffServData
                                NBB_CCXT);
                            
                            if((2 == SHARED.c3_num) && 
                                 ((stPortInfo.port_type == ATG_DCI_LAG) || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                            {
                                ul_vlanposid2 = 0;
                                spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                pstFlowDiffServData->svlan, 
                                pstFlowDiffServData->cvlan, 
                                &ul_vlanposid2 NBB_CCXT);

                                if(0 != ul_vlanposid2)
                                {
                                    stSubPort.unit = 1;
                                    stSubPort.posid = ul_vlanposid2;
                                    ret += spm_ds_add_logic_flow_node(&stSubPort,
                                    &stQosFlowKey,
                                    pstFlowDiffServData
                                    NBB_CCXT);                                     
                                }
                            } 
                            

                            if (ret != SUCCESS)
                            {
                                NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 删除 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败",
                                        ulPortIndexKey, pstFlowDiffServData->svlan, pstFlowDiffServData->cvlan));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 删除 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstFlowDiffServData->svlan, 
                                    pstFlowDiffServData->cvlan);
                                    
                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));

                                OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 删除 SVlan=%d CVlan=%d 的流相关Diff-Serv配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstFlowDiffServData->svlan, 
                                    pstFlowDiffServData->cvlan);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            }
                            else
                            {
                                    //flowdiffservcfgkey.svlan_id = pstFlowDiffServData->svlan;
                                    //flowdiffservcfgkey.cvlan_id = pstFlowDiffServData->cvlan;
                                    //pstflowdiffservcb = AVLL_FIND(pstLogicalPort->flow_diff_serv_tree, 
                                        //&flowdiffservcfgkey);
                                    pstflowdiffservcb = AVLL_FIND(pstLogicalPort->flow_diff_serv_tree, 
                                        pstFlowDiffServData);

                                //如果不存在，无法删除
                                if (NULL == pstflowdiffservcb)
                                {
                                    pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                    NBB_TRC_DETAIL((NBB_FORMAT 
                                    "  ***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的Flow-Diff-Serv配置不存在，无法删除",
                                    ulPortIndexKey,
                                    pstFlowDiffServData->svlan,
                                    pstFlowDiffServData->cvlan));

                                    OS_PRINTF(
                                    "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的Flow-Diff-Serv配置不存在，无法删除\n",
                                    ulPortIndexKey,
                                    pstFlowDiffServData->svlan,
                                    pstFlowDiffServData->cvlan);

                                    OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的Flow-Diff-Serv配置不存在，无法删除\n",
                                    ulPortIndexKey,
                                    pstFlowDiffServData->svlan,
                                    pstFlowDiffServData->cvlan);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                    ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                                }
                                else
                                {
                                    AVLL_DELETE(pstLogicalPort->flow_diff_serv_tree, 
                                    pstflowdiffservcb->spm_flow_diff_serv_node);

                                    //释放端口逻辑配置节点的内存空间
                                    spm_free_flow_diff_serv_cb(pstflowdiffservcb NBB_CCXT);

                                    pstLogicalPort->flow_diff_serv_num--;									
                                }
                            }    
                            
#if 0
                            if (ret == SUCCESS)
                            {
                                pstLogicalPort->logic_port_info_cb.flow_info_cb.if_qos = ATG_DCI_UNUSED;

                                NBB_MM_FREE(pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1],
                                    MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);
                                pstLogicalPort->flow_diff_serv_cfg_cb[usFlowId - 1] = NULL;

                                pstLogicalPort->flow_diff_serv_num--;
                            }
                            else
                            {
                                //NBB_TRC_DETAIL((NBB_FORMAT
                                //        "  ***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() error! 删除 flow_id=%d 的流相关Diff-Serv配置失败",
                                //        ulPortIndexKey, usFlowId));

                                //OS_PRINTF(
                                //    "***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 删除 flow_id=%d 的流相关Diff-Serv配置失败!\n",
                                //    ulPortIndexKey,
                                //    ret,
                                //    usFlowId);
                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));

                                //OS_SPRINTF(
                                //    ucMessage,
                                //    "***ERROR***:PORT_INDEX=%ld, spm_ds_add_logic_flow_node() ret=%d 删除 flow_id=%d 的流相关Diff-Serv配置失败!\n",
                                //    ulPortIndexKey,
                                //    ret,
                                //    usFlowId);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                pstSetLogicalPort->flow_diff_serv_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                            }
#endif
                        }
                        else
                        {
                            pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除Flow-Diff-Serv失败",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除Flow-Diff-Serv失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除Flow-Diff-Serv失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                    BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                        }                        
                    }
                }
            }
            else if (ulOperFlowDiffServ == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 流相关上话用户QOS策略配置L2 ******************************/
            if (ulOperFlowUpUserQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucFlowUpUserQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->flow_upuser_qos_data);

                /* 首地址为NULL，异常 */
                if (pucFlowUpUserQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucFlowUpUserQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    OS_SPRINTF(ucMessage, "  19）流相关上话用户QOS策略配置L2\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->flow_upuser_qos_num; i++)
	                {
                        pstFlowUpUserQosData = (ATG_DCI_LOG_FLOW_UP_USER_QOS *)
                            (pucFlowUpUserQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_FLOW_UP_USER_QOS)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  19）流相关上话用户QOS策略配置L2\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                    }

	                    //NBB_TRC_DETAIL((NBB_FORMAT "  流ID                = %d", pstFlowUpUserQosData->flow_id));
	                    NBB_TRC_DETAIL((NBB_FORMAT "  QOS策略模板索引     = %d", pstFlowUpUserQosData->qos_policy_index));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;

						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstFlowUpUserQosData->svlan, 
	                    	pstFlowUpUserQosData->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;

                        if(0 == ulVlanPosId)
                        {
                            NBB_TRC_DETAIL((NBB_FORMAT
                                    "  ***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node() error! 增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败，posId=0!\n",
                                    ulPortIndexKey, pstFlowUpUserQosData->svlan, pstFlowUpUserQosData->cvlan));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node() 增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败，posId=0!\n",
                                ulPortIndexKey,
                                pstFlowUpUserQosData->svlan, 
                                pstFlowUpUserQosData->cvlan);
                                
                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node()增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败，posId=0!\n",
                                ulPortIndexKey,
                                pstFlowUpUserQosData->svlan, 
                                pstFlowUpUserQosData->cvlan);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            pstSetLogicalPort->flow_upuser_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        }

                        else
                        {
                             ret = spm_add_flow_up_usr_qos_node(ulPortIndexKey,
	                        							&stSubPort,
	                        							pstFlowUpUserQosData);
                             if (ret != SUCCESS)
                            {
                                NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node() error! 增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败",
                                        ulPortIndexKey, pstFlowUpUserQosData->svlan, pstFlowUpUserQosData->cvlan));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node() ret=%d 增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstFlowUpUserQosData->svlan, 
                                    pstFlowUpUserQosData->cvlan);
                                    
                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));

                                OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld, spm_add_flow_up_usr_qos_node() ret=%d 增加 SVlan=%d CVlan=%d 的流相关上话用户QOS策略配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstFlowUpUserQosData->svlan, 
                                    pstFlowUpUserQosData->cvlan);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                pstSetLogicalPort->flow_upuser_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            }
                        }
	                    
	                   
	                }   
                }
            }
            else if (ulOperFlowUpUserQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperFlowUpUserQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 上话复杂流分类QOS策略配置L2 ******************************/
            if (ulOperInclassifyQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucInclassifyQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->inclassify_qos_data);

                /* 首地址为NULL，异常 */
                if (pucInclassifyQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucInclassifyQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  14)上话复杂流分类QOS策略配置L2 ADD\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);    
                    
                    for (i = 0; i < pstSetLogicalPort->inclassify_qos_num; i++)
	                {
                        pstInclassifyQosData = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)
                            (pucInclassifyQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  14)上话复杂流分类QOS策略配置L2 ADD\n");
                            
                               //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                        spm_dbg_print_logical_port_inclassify_qos_cfg(pstInclassifyQosData);
	                    }
                        
                           //OS_SPRINTF(ucMessage, "  14)上话复杂流分类QOS策略配置L2 ADD\n");
                           //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                            
	                    NBB_TRC_DETAIL((NBB_FORMAT "  复杂流分类QOS策略模板索引 = %d",
	                            pstInclassifyQosData->qos_policy_index));
                        
                            if (ucIfExist == ATG_DCI_EXIST)
                            {
                                OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                                spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                                stSubPort.unit = stPortInfo.unit_id;
                                stSubPort.port = stPortInfo.port_id;
                                stSubPort.slot = stPortInfo.slot_id;
                                stSubPort.ivlan = stPortInfo.cvlan;
                                stSubPort.ovlan = stPortInfo.svlan;

                                ulVlanPosId = 0;
                                spm_get_poscnt_from_intf(ulPortIndexKey, 
                                pstInclassifyQosData->svlan, 
                                pstInclassifyQosData->cvlan, 
                                &ulVlanPosId NBB_CCXT);

                                stSubPort.posid = ulVlanPosId;

                                if(0 == ulVlanPosId)
                                {
                                    pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                    NBB_TRC_DETAIL((NBB_FORMAT
                                    "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

                                    OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

                                    OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                                }
                                else
                                {
                                    ret = spm_add_logic_flow_classify_node(ulPortIndexKey,
                                    pstInclassifyQosData,
                                    &stSubPort
                                    NBB_CCXT);
                                    
                                    if((2 == SHARED.c3_num) && 
                                         ((stPortInfo.port_type == ATG_DCI_LAG) 
                                         || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                    {
                                        ul_vlanposid2 = 0;
                                        spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                        pstInclassifyQosData->svlan, 
                                        pstInclassifyQosData->cvlan, 
                                        &ul_vlanposid2 NBB_CCXT);

                                        if(0 != ul_vlanposid2)
                                        {
                                            stSubPort.unit = 1;
                                            stSubPort.posid = ul_vlanposid2;
                                            ret += spm_add_logic_flow_classify_node(ulPortIndexKey,
                                            pstInclassifyQosData,
                                            &stSubPort
                                            NBB_CCXT);                                    
                                        }
                                    } 
                                
                                    if (ret != SUCCESS)
                                    {
                                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                        NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败!\n",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

                                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败!\n",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

                                        OS_SPRINTF(ucMessage,
                                        "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败!\n",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
                                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));
                                    }
                                    else
                                    {
                                        //inclassifyqoscfgkey.svlan_id = pstInclassifyQosData->svlan;
                                        //inclassifyqoscfgkey.cvlan_id = pstInclassifyQosData->cvlan;
                                        //pstinclassifyqoscb = 
                                            //AVLL_FIND(pstLogicalPort->inclassify_qos_tree, &inclassifyqoscfgkey);
                                        pstinclassifyqoscb = 
                                            AVLL_FIND(pstLogicalPort->inclassify_qos_tree, pstInclassifyQosData);
                                        
                                        //如果不存在，申请树节点，插入树中，计数加1
                                        if (NULL == pstinclassifyqoscb)
                                        {
                                            pstinclassifyqoscb = spm_alloc_inclassify_qos_cb(NBB_CXT);

                                            if (NULL == pstinclassifyqoscb)/*lint !e613 */
                                            {
                                                OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);

                                                OS_SPRINTF(ucMessage, 
                                                    "   ***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                                goto EXIT_LABEL;

                                            }

                                            //保存数据并插入树中
                                            //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
                                            //NBB_MEMCPY(&pstinclassifyqoscb->key,&inclassifyqoscfgkey,
                                                //sizeof(ATG_DCI_LOG_PORT_VLAN));
                                            NBB_MEMCPY(&(pstinclassifyqoscb->inclassify_qos_cfg),
                                            pstInclassifyQosData, sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
                                            AVLL_INSERT(pstLogicalPort->inclassify_qos_tree, 
                                                pstinclassifyqoscb->spm_inclassify_qos_node);

                                            pstLogicalPort->inclassify_qos_num++;
                                        }
                                        else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
                                        {
                                            //pstdiffservcb->intf_pos_id = stIntfFlow.posId;
                                            NBB_MEMCPY(&(pstinclassifyqoscb->inclassify_qos_cfg), 
                                            pstInclassifyQosData, sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
                                        }
                                    }                                
                                }


                            }
                            else
                            {
                                pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立inclassify_qos失败",
                                    ulPortIndexKey));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立inclassify_qos失败\n", ulPortIndexKey);

                                OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立inclassify_qos失败\n",
                                    ulPortIndexKey);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                    BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                            }                               
                    }
                }
            }
            else if (ulOperInclassifyQos == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucInclassifyQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->inclassify_qos_data);

                /* 首地址为NULL，异常 */
                if (pucInclassifyQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucInclassifyQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    OS_SPRINTF(ucMessage, "  14) 上话复杂流分类QOS策略配置L2 DEL\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->inclassify_qos_num; i++)
	                {
                        pstInclassifyQosData = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)
                            (pucInclassifyQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf(" 14) 上话复杂流分类QOS策略配置L2 DEL\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                        spm_dbg_print_logical_port_inclassify_qos_cfg(pstInclassifyQosData);
	                    }
                        
                           //OS_SPRINTF(ucMessage, "  14) 上话复杂流分类QOS策略配置L2 DEL\n");
                           //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                           
	                    NBB_TRC_DETAIL((NBB_FORMAT "  复杂流分类QOS策略模板索引 = %d",
	                            pstInclassifyQosData->qos_policy_index));

                            if (ucIfExist == ATG_DCI_EXIST)
                            {
                                OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                                spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                                stSubPort.unit = stPortInfo.unit_id;
                                stSubPort.port = stPortInfo.port_id;
                                stSubPort.slot = stPortInfo.slot_id;
                                stSubPort.ivlan = stPortInfo.cvlan;
                                stSubPort.ovlan = stPortInfo.svlan;

                                ulVlanPosId = 0;
                                spm_get_poscnt_from_intf(ulPortIndexKey, 
                                pstInclassifyQosData->svlan, 
                                pstInclassifyQosData->cvlan, 
                                &ulVlanPosId NBB_CCXT);

                                stSubPort.posid = ulVlanPosId;
                                if(0 == ulVlanPosId)
                                {
                                    pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                    NBB_TRC_DETAIL((NBB_FORMAT
                                    "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey,pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

                                    OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey,pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

                                    OS_SPRINTF(ucMessage,
                                    "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node()删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败,posId=0!\n",
                                    ulPortIndexKey,pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                                }

                                else
                                {
                                    pstInclassifyQosData->qos_policy_index = 0;

                                    ret = spm_add_logic_flow_classify_node(ulPortIndexKey,
                                    pstInclassifyQosData,
                                    &stSubPort
                                    NBB_CCXT);
                                    
                                    if((2 == SHARED.c3_num) && 
                                         ((stPortInfo.port_type == ATG_DCI_LAG) 
                                         || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                    {
                                        ul_vlanposid2 = 0;
                                        spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                        pstInclassifyQosData->svlan, 
                                        pstInclassifyQosData->cvlan, 
                                        &ul_vlanposid2 NBB_CCXT);

                                        if(0 != ul_vlanposid2)
                                        {
                                            stSubPort.unit = 1;
                                            stSubPort.posid = ul_vlanposid2;
                                            ret += spm_add_logic_flow_classify_node(ulPortIndexKey,
                                            pstInclassifyQosData,
                                            &stSubPort
                                            NBB_CCXT);                                    
                                        }
                                    }                                    

                                    if (ret != SUCCESS)
                                    {
                                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                        NBB_TRC_DETAIL((NBB_FORMAT
                                        "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

                                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

                                        OS_SPRINTF(ucMessage,
                                        "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
                                        ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
                                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                        "CALL_FUNC_ERROR", ucMessage));
                                    }
                                    else
                                    {
                                        //inclassifyqoscfgkey.svlan_id = pstInclassifyQosData->svlan;
                                        //inclassifyqoscfgkey.cvlan_id = pstInclassifyQosData->cvlan;
                                        //pstinclassifyqoscb = 
                                            //AVLL_FIND(pstLogicalPort->inclassify_qos_tree, &inclassifyqoscfgkey);
                                        pstinclassifyqoscb = 
                                            AVLL_FIND(pstLogicalPort->inclassify_qos_tree, pstInclassifyQosData);
                                        
                                        //如果不存在，无法删除
                                        if (NULL == pstinclassifyqoscb)
                                        {
                                            pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                            NBB_TRC_DETAIL((NBB_FORMAT 
                                            "  ***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的inclassify_qos配置不存在，无法删除",
                                            ulPortIndexKey,
                                            pstInclassifyQosData->svlan,
                                            pstInclassifyQosData->cvlan));

                                            OS_PRINTF(
                                            "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的inclassify_qos配置不存在，无法删除\n",
                                            ulPortIndexKey,
                                            pstInclassifyQosData->svlan,
                                            pstInclassifyQosData->cvlan);

                                            OS_SPRINTF(ucMessage,
                                            "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的inclassify_qos配置不存在，无法删除\n",
                                            ulPortIndexKey,
                                            pstInclassifyQosData->svlan,
                                            pstInclassifyQosData->cvlan);
                                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                            ulPortIndexKey, ret,
                                            "CALL_FUNC_ERROR", ucMessage));
                                        }
                                        else
                                        {
                                            AVLL_DELETE(pstLogicalPort->inclassify_qos_tree, 
                                            pstinclassifyqoscb->spm_inclassify_qos_node);

                                            //释放端口逻辑配置节点的内存空间
                                            spm_free_inclassify_qos_cb(pstinclassifyqoscb NBB_CCXT);

                                            pstLogicalPort->inclassify_qos_num--;									
                                        }
                                    }                                      
                                }

                            } 
                            else
                            {
                                pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除inclassify_qos失败",
                                        ulPortIndexKey));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除inclassify_qos失败\n", ulPortIndexKey);

                                OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除inclassify_qos失败\n",
                                    ulPortIndexKey);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                        BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                            }                                  
	                }
                }
            }
            else if (ulOperInclassifyQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 下话用户组QOS配置L2 ******************************/
            if (ulOperDownGroupQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDownGroupQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->down_user_group_qos_data);

                /* 首地址为NULL，异常 */
                if (pucDownGroupQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDownGroupQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstDownGroupQosData = (ATG_DCI_LOG_DOWN_USER_GROUP_QOS *)pucDownGroupQosDataStart;
                    
                    OS_SPRINTF(ucMessage, "  18）下话用户组QOS配置L2\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  18）下话用户组QOS配置L2\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }

                    NBB_TRC_DETAIL((NBB_FORMAT "  用户组索引  = %d", pstDownGroupQosData->user_group_index));

                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                    ret = spm_hqos_add_group_node(stPortInfo.slot_id,
                        stPortInfo.port_id,
                        pstDownGroupQosData->user_group_index,
                        ulPortIndexKey
                        NBB_CCXT);

                    if (ret == SUCCESS)
                    {
                        /* 保存数据 */
                        if (pstLogicalPort->down_user_group_qos_cfg_cb == NULL)
                        {
                            pstLogicalPort->down_user_group_qos_cfg_cb =
                                (ATG_DCI_LOG_DOWN_USER_GROUP_QOS *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_DOWN_USER_GROUP_QOS),
                                NBB_NORETRY_ACT,
                                MEM_SPM_LOG_PORT_DOWN_GROUP_QOS_CB);
                        }

                        OS_MEMCPY(pstLogicalPort->down_user_group_qos_cfg_cb, pstDownGroupQosData,
                            sizeof(ATG_DCI_LOG_DOWN_USER_GROUP_QOS));
                    }
                    else
                    {
                        pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() error",
                                ulPortIndexKey));

                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n", ulPortIndexKey, ret);

                        OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                            ulPortIndexKey,
                            ret);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                "CALL_FUNC_ERROR", ucMessage));
                    }
                }
            }
            else if (ulOperDownGroupQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperDownGroupQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 下话用户队列QOS策略配置L2 ******************************/
            if (ulOperDownUserQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDownUserQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->down_user_queue_qos_data);

                /* 首地址为NULL，异常 */
                if (pucDownUserQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDownUserQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstDownUserQosData = (ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *)pucDownUserQosDataStart;
                    OS_SPRINTF(ucMessage, "  17）下话用户队列QOS策略配置L2\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  17）下话用户队列QOS策略配置L2\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }

                    NBB_TRC_DETAIL((NBB_FORMAT "  NodeIndex  = %d", pstDownUserQosData->node_index));
                    NBB_TRC_DETAIL((NBB_FORMAT "  用户QOS策略模板索引  = %d",
                            pstDownUserQosData->user_qos_policy_index));
                    NBB_TRC_DETAIL((NBB_FORMAT "  优先级队列QOS策略模板索引  = %d",
                            pstDownUserQosData->prio_queue_qos_policy_index));

                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                    stSubPort.unit = stPortInfo.unit_id;
                    stSubPort.port = stPortInfo.port_id;
                    stSubPort.slot = stPortInfo.slot_id;
                    stSubPort.ivlan = stPortInfo.cvlan;
                    stSubPort.ovlan = stPortInfo.svlan;
                    stSubPort.posid = stPortInfo.vp_pos_id;

                    if (pstLogicalPort->down_user_group_qos_cfg_cb != NULL)
                    {
                        ret = spm_hqos_add_usr_node(&stSubPort,
                            ulPortIndexKey,
                            pstLogicalPort->down_user_group_qos_cfg_cb->user_group_index,
                            pstDownUserQosData
                            NBB_CCXT);
                            
                        if (ret == SUCCESS)
                        {
                            /* 保存数据 */
                            if (pstLogicalPort->down_user_qos_cfg_cb == NULL)
                            {
                                pstLogicalPort->down_user_qos_cfg_cb = (ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *)NBB_MM_ALLOC(
                                    sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS),
                                    NBB_NORETRY_ACT,
                                    MEM_SPM_LOG_PORT_DOWN_USER_QOS_CB);
                            }

                            OS_MEMCPY(pstLogicalPort->down_user_qos_cfg_cb, pstDownUserQosData,
                                sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS));
                        }
                        else
                        {
                            pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() error",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                                ulPortIndexKey,
                                ret);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                                ulPortIndexKey,
                                ret);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }
                    }
                    else
                    {
                        pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        NBB_TRC_DETAIL((NBB_FORMAT
                                "  ***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败",
                                ulPortIndexKey));

                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败\n",
                            ulPortIndexKey);

                        OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败\n",
                            ulPortIndexKey);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                    }
                }
            }
            else if (ulOperDownUserQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperDownUserQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 包过滤器traffic_filter L2 ******************************/
            if (ulOperTrafficFilter == ATG_DCI_OPER_ADD)
			{

                /* 计算第一个entry的地址。*/
                pucTrafficFilterDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->traffic_filter_data);

                /* 首地址为NULL，异常 */
                if (pucTrafficFilterDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucTrafficFilterDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    //if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    //{
                        //printf("  20）包过滤器traffic_filter L2 ADD\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    //}
                    OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L2 ADD \n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->traffic_filter_num; i++)
                    {
                        pstTrafficFilter = (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)
                            (pucTrafficFilterDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER)))
                            * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            //spm_dbg_print_logical_port_ipv4_cfg(pstIpv4Data[i]);
                            printf("  20）包过滤器traffic_filter L2 ADD \n");

                            spm_dbg_print_logical_port_traffic_filter_cfg(pstTrafficFilter);                            
                        }
                        
                        //OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L2 ADD\\n");
                        //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
                        NBB_TRC_DETAIL((NBB_FORMAT "  acl_id  = %d", pstTrafficFilter->acl_id));
                        NBB_TRC_DETAIL((NBB_FORMAT "  方向direction  = %d", pstTrafficFilter->direction));

                        if (ucIfExist == ATG_DCI_EXIST)
                        {
                            OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                            spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                            stSubPort.unit = stPortInfo.unit_id;
                            stSubPort.port = stPortInfo.port_id;
                            stSubPort.slot = stPortInfo.slot_id;
                            stSubPort.ivlan = stPortInfo.cvlan;
                            stSubPort.ovlan = stPortInfo.svlan;

                            ulVlanPosId = 0;
                            spm_get_poscnt_from_intf(ulPortIndexKey, 
                            pstTrafficFilter->svlan, 
                            pstTrafficFilter->cvlan, 
                            &ulVlanPosId NBB_CCXT);

                            stSubPort.posid = ulVlanPosId;

                            ret = spm_add_logic_acl_node(ulPortIndexKey,
                            pstTrafficFilter,
                            &stSubPort
                            NBB_CCXT);

                            if((2 == SHARED.c3_num) && 
                                 ((stPortInfo.port_type == ATG_DCI_LAG) || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                            {
                                ul_vlanposid2 = 0;
                                spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan, 
                                &ul_vlanposid2 NBB_CCXT);

                                if(0 != ul_vlanposid2)
                                {
                                    stSubPort.unit = 1;
                                    stSubPort.posid = ul_vlanposid2;
                                    ret += spm_add_logic_acl_node(ulPortIndexKey,
                                    pstTrafficFilter,
                                    &stSubPort
                                    NBB_CCXT);                                    
                                }
                            } 
                            

                            if (ret != SUCCESS)
                            {
                                pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan);

                                OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                "CALL_FUNC_ERROR", ucMessage));
                            }                            
                            else
                            {
                                //psttrafficfiltercb = AVLL_FIND(pstLogicalPort->traffic_filter_tree, pstTrafficFilter);
                                //trafficfiltercfgkey.svlan_id = pstTrafficFilter->svlan;
                                //trafficfiltercfgkey.cvlan_id = pstTrafficFilter->cvlan;
                                //psttrafficfiltercb = 
                                    //AVLL_FIND(pstLogicalPort->traffic_filter_tree, &trafficfiltercfgkey);
                                psttrafficfiltercb = 
                                    AVLL_FIND(pstLogicalPort->traffic_filter_tree, pstTrafficFilter);
                                
                                //如果不存在，申请树节点，插入树中，计数加1
                                if (NULL == psttrafficfiltercb)
                                {
                                    psttrafficfiltercb = spm_alloc_traffic_filter_cb(NBB_CXT);

                                    if (NULL == psttrafficfiltercb)/*lint !e613 */
                                    {
                                        OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);

                                        OS_SPRINTF(ucMessage, "	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                        goto EXIT_LABEL;

                                    }

                                    //保存数据并插入树中
                                    //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
                                    //NBB_MEMCPY(&psttrafficfiltercb->key,
                                        //&trafficfiltercfgkey,sizeof(ATG_DCI_LOG_PORT_VLAN));
                                    NBB_MEMCPY(&(psttrafficfiltercb->traffic_filter_cfg),
                                    pstTrafficFilter, sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                                    AVLL_INSERT(pstLogicalPort->traffic_filter_tree, 
                                        psttrafficfiltercb->spm_traffic_filter_node);

                                    pstLogicalPort->traffic_filter_num++;
                                }
                                else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
                                {
                                    //pstdiffservcb->intf_pos_id = stIntfFlow.posId;
                                    NBB_MEMCPY(&(psttrafficfiltercb->traffic_filter_cfg), 
                                    pstTrafficFilter, sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                                }
                            }                                
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立traffic_filter失败",
                            ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立traffic_filter失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，建立traffic_filter失败\n",
                            ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                            BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                        }  
#if 0
                        if (ret == SUCCESS)
                        {
                            iCfgPos = spm_check_if_acl_exist(ulPortIndexKey, pstTrafficFilter);

                            if (iCfgPos <= 0)
                            {
                                for (j = 0; j < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; j++)
                                {
                                    /* 保存数据 */
                                    if (pstLogicalPort->traffic_filter_cfg_cb[j] == NULL)
                                    {
                                        pstLogicalPort->traffic_filter_cfg_cb[j] =
                                            (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)NBB_MM_ALLOC(sizeof(
                                                ATG_DCI_LOG_PORT_TRAFFIC_FILTER),
                                            NBB_NORETRY_ACT,
                                            MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);

	                                    OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[j], pstTrafficFilter,
	                                        sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));

	                                    pstLogicalPort->traffic_filter_num++;

	                                    break;
                                    }
                                }
                            }
                            else
                            {
                                OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[iCfgPos - 1],
                                    pstTrafficFilter,
                                    sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                            }
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }

#endif
                    }
                }
            }
			else if (ulOperTrafficFilter == ATG_DCI_OPER_DEL)
			{

                /* 计算第一个entry的地址。*/
                pucTrafficFilterDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->traffic_filter_data);

                /* 首地址为NULL，异常 */
                if (pucTrafficFilterDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucTrafficFilterDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    //if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    //{
                        //printf("  20）包过滤器traffic_filter L2 DEL\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    //}
                    
                    OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L2 DEL\\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->traffic_filter_num; i++)
                    {
                        pstTrafficFilter = (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)
                            (pucTrafficFilterDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER)))
                            * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            //spm_dbg_print_logical_port_ipv4_cfg(pstIpv4Data[i]);
                            printf("  20）包过滤器traffic_filter L2 DEL\n");
                            spm_dbg_print_logical_port_traffic_filter_cfg(pstTrafficFilter);
                        }
                        
                        //OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L2 DEL\\n");
                        //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
                        NBB_TRC_DETAIL((NBB_FORMAT "  acl_id  = %d", pstTrafficFilter->acl_id));
                        NBB_TRC_DETAIL((NBB_FORMAT "  方向direction  = %d", pstTrafficFilter->direction));
                        
                        if (ucIfExist == ATG_DCI_EXIST)
                        {
                            OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                            spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                            stSubPort.unit = stPortInfo.unit_id;
                            stSubPort.port = stPortInfo.port_id;
                            stSubPort.slot = stPortInfo.slot_id;
                            stSubPort.ivlan = stPortInfo.cvlan;
                            stSubPort.ovlan = stPortInfo.svlan;

                            ulVlanPosId = 0;
                            spm_get_poscnt_from_intf(ulPortIndexKey, 
                            pstTrafficFilter->svlan, 
                            pstTrafficFilter->cvlan, 
                            &ulVlanPosId NBB_CCXT);

                            stSubPort.posid = ulVlanPosId;

                            pstTrafficFilter->acl_id = 0;

                            ret = spm_add_logic_acl_node(ulPortIndexKey,
                            pstTrafficFilter,
                            &stSubPort
                            NBB_CCXT);
                            
                            if((2 == SHARED.c3_num) && 
                                 ((stPortInfo.port_type == ATG_DCI_LAG) || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                            {
                                ul_vlanposid2 = 0;
                                spm_get_poscnt_from_intf2(ulPortIndexKey, 
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan, 
                                &ul_vlanposid2 NBB_CCXT);

                                if(0 != ul_vlanposid2)
                                {
                                    stSubPort.unit = 1;
                                    stSubPort.posid = ul_vlanposid2;
                                    ret += spm_add_logic_acl_node(ulPortIndexKey,
                                    pstTrafficFilter,
                                    &stSubPort
                                    NBB_CCXT);                                    
                                }
                            }                            

                            if (ret != SUCCESS)
                            {
                                pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                NBB_TRC_DETAIL((NBB_FORMAT 
                                "  ***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d"
                                "删除SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan));

                                OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d"
                                "删除SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan);

                                OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d"
                                "删除SVlan=%d CVlan=%d 的包过滤器traffic_filter L2配置失败!\n",
                                ulPortIndexKey,
                                ret,
                                pstTrafficFilter->svlan, 
                                pstTrafficFilter->cvlan);
                                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                "CALL_FUNC_ERROR", ucMessage));
                            }

                            else
                            {
                                //psttrafficfiltercb = AVLL_FIND(pstLogicalPort->traffic_filter_tree, pstTrafficFilter);
                                //psttrafficfiltercb = AVLL_FIND(pstLogicalPort->traffic_filter_tree, pstTrafficFilter);
                                //trafficfiltercfgkey.svlan_id = pstTrafficFilter->svlan;
                                //trafficfiltercfgkey.cvlan_id = pstTrafficFilter->cvlan;
                                //psttrafficfiltercb = 
                                    //AVLL_FIND(pstLogicalPort->traffic_filter_tree, &trafficfiltercfgkey);
                                psttrafficfiltercb = 
                                    AVLL_FIND(pstLogicalPort->traffic_filter_tree, pstTrafficFilter);

                                //如果不存在，无法删除
                                if (NULL == psttrafficfiltercb)
                                {
                                    pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                    NBB_TRC_DETAIL((NBB_FORMAT 
                                    "  ***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的traffic_filter配置不存在，无法删除",
                                    ulPortIndexKey,
                                    pstTrafficFilter->svlan,
                                    pstTrafficFilter->cvlan));

                                    OS_PRINTF(
                                    "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的traffic_filter配置不存在，无法删除\n",
                                    ulPortIndexKey,
                                    pstTrafficFilter->svlan,
                                    pstTrafficFilter->cvlan);

                                    OS_SPRINTF(ucMessage,
                                    "***ERROR***:PORT_INDEX=%ld SVLAN=%d CVLAN=%d 的traffic_filter配置不存在，无法删除\n",
                                    ulPortIndexKey,
                                    pstTrafficFilter->svlan,
                                    pstTrafficFilter->cvlan);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                    ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                                }
                                else
                                {
                                    AVLL_DELETE(pstLogicalPort->traffic_filter_tree, 
                                    psttrafficfiltercb->spm_traffic_filter_node);

                                    //释放端口逻辑配置节点的内存空间
                                    spm_free_traffic_filter_cb(psttrafficfiltercb NBB_CCXT);

                                    pstLogicalPort->traffic_filter_num--;									
                                }
                            }                        
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除traffic_filter失败",
                            ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除traffic_filter失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld 的逻辑配置未下，删除traffic_filter失败\n",
                            ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                            BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                        }      
                        
#if 0
                        if (ret == SUCCESS)
                        {
                            iCfgPos = spm_check_if_acl_exist(ulPortIndexKey, pstTrafficFilter);

                            if (iCfgPos <= 0)
                            {
                                for (j = 0; j < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; j++)
                                {
                                    /* 保存数据 */
                                    if (pstLogicalPort->traffic_filter_cfg_cb[j] == NULL)
                                    {
                                        pstLogicalPort->traffic_filter_cfg_cb[j] =
                                            (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)NBB_MM_ALLOC(sizeof(
                                                ATG_DCI_LOG_PORT_TRAFFIC_FILTER),
                                            NBB_NORETRY_ACT,
                                            MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);

	                                    OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[j], pstTrafficFilter,
	                                        sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));

	                                    pstLogicalPort->traffic_filter_num++;

	                                    break;
                                    }
                                }
                            }
                            else
                            {
                                OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[iCfgPos - 1],
                                    pstTrafficFilter,
                                    sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                            }
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }

#endif
                    }
                }
            }
            else if (ulOperTrafficFilter == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }
            
            /****************************** Diff-Serv配置L2（VP下话UNI侧） ******************************/
            if (ulOperDsL2 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDsL2DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ds_l2_data);

                /* 首地址为NULL，异常 */
                if (pucDsL2DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDsL2DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstDsL2Data = (ATG_DCI_LOG_PORT_DS_L2_DATA *)pucDsL2DataStart;
                    OS_SPRINTF(ucMessage, "  23）Diff-Serv配置L2（VP下话UNI侧）\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  23）Diff-Serv配置L2（VP下话UNI侧）\n");

                        spm_dbg_print_logical_port_ds_l2_cfg(pstDsL2Data);
                    }

                    NBB_TRC_DETAIL((NBB_FORMAT "  Diff-Serv模板索引 = %d", pstDsL2Data->ds_id));

                    //OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                    //spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

					stUniVp.vpn_id = pstLogicalPort->logic_port_info_cb.vpn_id;
					stUniVp.vp_type = pstLogicalPort->logic_port_info_cb.vpn_type;
					stUniVp.vp_posid = pstLogicalPort->logic_port_info_cb.vp_idx;

                    ret = spm_ds_add_logic_univp_node(ulPortIndexKey,
													&stUniVp,
							                        pstDsL2Data
							                        NBB_CCXT);
                        
                    if (ret == SUCCESS)
                    {
                        /* 保存数据 */
                        if (pstLogicalPort->ds_l2_cfg_cb == NULL)
                        {
                            pstLogicalPort->ds_l2_cfg_cb = (ATG_DCI_LOG_PORT_DS_L2_DATA *)NBB_MM_ALLOC(
                                sizeof(ATG_DCI_LOG_PORT_DS_L2_DATA),
                                NBB_NORETRY_ACT,
                                MEM_SPM_LOG_PORT_DS_L2_CB);
                        }

                        OS_MEMCPY(pstLogicalPort->ds_l2_cfg_cb, pstDsL2Data,
                            sizeof(ATG_DCI_LOG_PORT_DS_L2_DATA));
                    }
                    else
                    {
                        pstSetLogicalPort->ds_l2_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_univp_node() error",
                                ulPortIndexKey));

                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_univp_node() ret=%d\n",
                            ulPortIndexKey,
                            ret);

                        OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_univp_node() ret=%d\n",
                            ulPortIndexKey,
                            ret);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                "CALL_FUNC_ERROR", ucMessage));
                    }
                }
            }
            else if (ulOperDsL2 == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperDsL2 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }
        }

        /****************************************************************************/
        /* L3端口                                                                   */
        /****************************************************************************/
        else if ((pstLogicalPort->basic_cfg_cb != NULL)
            && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3))
        {
            /****************************** 逻辑配置L3 ******************************/
            if (ulOperLogicL3 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucLogicL3DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->logical_l3_data);

                /* 首地址为NULL，异常 */
                if (pucLogicL3DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  LOGICAL PORT pucLogicL3DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstLogicL3Data = (ATG_DCI_LOG_PORT_LOGIC_L3_DATA *)pucLogicL3DataStart;

                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  3）逻辑配置L3\n");
                        spm_dbg_print_logical_port_logic_l3_cfg(pstLogicL3Data);
                    }

                    OS_SPRINTF(ucMessage, "  3）逻辑配置L3\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    //spm_dbg_record_logical_port_logic_l3_cfg(pstLogicL3Data);

                    //NBB_TRC_DETAIL((NBB_FORMAT "  CONTROL VID = %d", pstLogicL3Data->control_vid));
                    NBB_TRC_DETAIL((NBB_FORMAT "  IPV4 URPF使能 = %d", pstLogicL3Data->ipv4_urpf_mode));
                    NBB_TRC_DETAIL((NBB_FORMAT "  IPV4 缺省路由检查 = %s",
                            pstLogicL3Data->ipv4_default_check ? "使能" : "不使能"));
                    NBB_TRC_DETAIL((NBB_FORMAT "  IPV6 URPF使能 = %d", pstLogicL3Data->ipv6_urpf_mode));
                    NBB_TRC_DETAIL((NBB_FORMAT "  IPV6 缺省路由检查 = %s",
                            pstLogicL3Data->ipv6_default_check ? "使能" : "不使能"));
                    NBB_TRC_DETAIL((NBB_FORMAT "  VRF ID = %d", pstLogicL3Data->vrf_id));
                    NBB_TRC_DETAIL((NBB_FORMAT "  INGRESS = %s", spm_set_ulong_to_ipv4(pstLogicL3Data->ingress  NBB_CCXT)));
                    NBB_TRC_DETAIL((NBB_FORMAT "  EGRESS = %s", spm_set_ulong_to_ipv4(pstLogicL3Data->egress NBB_CCXT)));
                    NBB_TRC_DETAIL((NBB_FORMAT "  TUNNEL_ID = %ld", pstLogicL3Data->tunnel_id));
                    NBB_TRC_DETAIL((NBB_FORMAT "  MPLS转发使能 = %s", pstLogicL3Data->mpls_forward_enable ? "使能" : "不使能"));  

                    stIntfL3.mCardPort = pstLogicalPort->port_id;
                    stIntfL3.bindId = pstLogicL3Data->vrf_id;
                    stIntfL3.eIntfType = INTF_TYPE_L3;

                    #if 0
                    //IPV4 URPF使能
                    if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                    {
                        stIntfL3.eIpv4UrpfMode = URPF_OFF;
                    }
                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                    {
                        stIntfL3.eIpv4UrpfMode = URPF_STRICT;
                    }
                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                    {
                        stIntfL3.eIpv4UrpfMode = URPF_LOOSE;
                    }

					//IPV6 URPF使能
                    if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                    {
                        stIntfL3.eIpv6UrpfMode = URPF_OFF;
                    }
                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                    {
                        stIntfL3.eIpv6UrpfMode = URPF_STRICT;
                    }
                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                    {
                        stIntfL3.eIpv6UrpfMode = URPF_LOOSE;
                    }

                    //缺省路由检查使能
                    if (pstLogicL3Data->ipv4_default_check == ENABLE)
                    {
                        stIntfL3.flags |= INTF_V4_ALLOW_DEFAULT_ROUTE;
                    }

                    if (pstLogicL3Data->ipv6_default_check == ENABLE)
                    {
                        stIntfL3.flags |= INTF_V6_ALLOW_DEFAULT_ROUTE;
                    }
                    
                    //MPLS转发不使能
                    if (pstLogicL3Data->mpls_forward_enable == DISABLE)
                    {
                        stIntfL3.flags |= INTF_MPLS_DISABLE;
                    }
                    
                    //stIntfL3.flags |= INTF_COUNTER_ON;
                    #endif

					//如果Vlan属性个数不等于0，说明配置了intf，需要删除，重建
					if (pstLogicalPort->terminal_if_num != 0)
					{
					    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
					         pstTerminalIfCb != NULL;
					         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
					                       pstTerminalIfCb->spm_terminal_if_node))
					    {
					    	if (pstTerminalIfCb->intf_pos_id != 0)
					    	{

								//注释掉，解决更新L3配置时 删除 再 增加 INTF 后QOS配置没有了，改成Mod INTF
					    		//ApiC3DelIntf(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id);

					    		//almpm_delline_intf(ulPortIndexKey, pstTerminalIfCb->intf_pos_id, 1);

                                //由于调用ModIntf接口会冲掉qos配置，改为调用设置各个字段的子接口�
                                
			                    //ApiC3SetIntfVrf(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,pstLogicL3Data->vrf_id); //to do

                                stIntfL3.posId = pstTerminalIfCb->intf_pos_id;

                                #ifdef SPU

                                //刷新vrfid
                                ret = ApiC3SetIntfBindId(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, INTF_TYPE_L3, pstLogicL3Data->vrf_id);
                                if(SUCCESS != ret)
                                {
                                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3SetIntfBindId() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id));

			                        OS_PRINTF("***ERROR***:ApiC3SetIntfBindId() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            	"***ERROR***:ApiC3SetIntfBindId() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
                                }

                                //IPV4 URPF使能
			                    if (pstLogicL3Data->ipv4_default_check == ENABLE)
                                {
                                    if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_OFF,ENABLE);
                                        
                                    }
                                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_STRICT,ENABLE);
                                       
                                    }
                                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_LOOSE,ENABLE);
                                    } 
                                }    

                                else if(pstLogicL3Data->ipv4_default_check == DISABLE)
                                {
                                    if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_OFF,DISABLE);
                                        
                                    }
                                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_STRICT,DISABLE);
                                       
                                    }
                                    else if (pstLogicL3Data->ipv4_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                                    {
                                        ret = ApiC3SetIntfV4UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_LOOSE,DISABLE);
                                    } 
                                }    

                                if(SUCCESS != ret)
                                {
                                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3SetIntfV4UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id));

			                        OS_PRINTF("***ERROR***:ApiC3SetIntfV4UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            	"***ERROR***:ApiC3SetIntfV4UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
											ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIfCb->terminal_if_cfg.svlan_id,
			                        		pstTerminalIfCb->terminal_if_cfg.cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
                                }

                                if (pstLogicL3Data->ipv6_default_check == ENABLE)
                                {
                                    if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_OFF,ENABLE);
                                    }
                                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_STRICT,ENABLE);
                                    }
                                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_LOOSE,ENABLE);
                                    }
                                }

                                else if (pstLogicL3Data->ipv6_default_check == DISABLE)
                                {
                                    if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_OFF,DISABLE);
                                    }
                                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_STRICT,DISABLE);
                                    }
                                    else if (pstLogicL3Data->ipv6_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
                                    {
                                        ret = ApiC3SetIntfV6UrpfMode(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,URPF_LOOSE,DISABLE);
                                    }
                                }

                                if(SUCCESS != ret)
                                {
                                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3SetIntfV6UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败",
                                                ret,
                                                ulPortIndexKey,
                                                pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                                pstTerminalIfCb->terminal_if_cfg.cvlan_id));

                                        OS_PRINTF("***ERROR***:ApiC3SetIntfV6UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
                                                ret,
                                                ulPortIndexKey,
                                                pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                                pstTerminalIfCb->terminal_if_cfg.cvlan_id);

                                        OS_SPRINTF(ucMessage,
                                                "***ERROR***:ApiC3SetIntfV6UrpfMode() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
                                                ret,
                                                ulPortIndexKey,
                                                pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                                pstTerminalIfCb->terminal_if_cfg.cvlan_id);
                                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                                "CALL_FUNC_ERROR", ucMessage));
                                    
                                }
                                
                                //MPLS转发不使能
                                if (pstLogicL3Data->mpls_forward_enable == DISABLE)
                                {
                                    ApiC3SetIntfMplsDisable(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,ENABLE);//使能:0/1-enable /disable
                                }

                                else if(pstLogicL3Data->mpls_forward_enable == ENABLE)
                                {
                                    ApiC3SetIntfMplsDisable(pstLogicalPort->unit_id,pstTerminalIfCb->intf_pos_id,DISABLE);//使能:0/1-enable /disable
                                }

                                if(SUCCESS != ret)
                                {
                                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3SetIntfMplsDisable() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败",
                                            ret,
                                            ulPortIndexKey,
                                            pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                            pstTerminalIfCb->terminal_if_cfg.cvlan_id));

                                    OS_PRINTF("***ERROR***:ApiC3SetIntfMplsDisable() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
                                            ret,
                                            ulPortIndexKey,
                                            pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                            pstTerminalIfCb->terminal_if_cfg.cvlan_id);

                                    OS_SPRINTF(ucMessage,
                                            "***ERROR***:ApiC3SetIntfMplsDisable() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 逻辑配置L3 刷新失败\n",
                                            ret,
                                            ulPortIndexKey,
                                            pstTerminalIfCb->terminal_if_cfg.svlan_id,
                                            pstTerminalIfCb->terminal_if_cfg.cvlan_id);
                                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                            "CALL_FUNC_ERROR", ucMessage));
                                    
                                }

                                #endif
                                
#if 0


			                    if ((pstTerminalIfCb->terminal_if_cfg.svlan_id == 0) 
			                     && (pstTerminalIfCb->terminal_if_cfg.cvlan_id == 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT;
				                    }
				                    else
				                    {
				                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT;
				                    }
			                        
			                        stIntfL3.mOvid = 0;
			                        stIntfL3.mIvid = 0;
			                    }

			                    //Dot1q终结子接口应用于L3VPN，SVLAN有效、CVLAN无效
			                    else if ((pstTerminalIfCb->terminal_if_cfg.svlan_id != 0) 
			                          && (pstTerminalIfCb->terminal_if_cfg.cvlan_id == 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT_OVID;
				                    }
				                    else
				                    {
				                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT_OVID;
				                    }
			                        
			                        stIntfL3.mOvid = pstTerminalIfCb->terminal_if_cfg.svlan_id;
			                        stIntfL3.mIvid = 0;
			                    }

			                    //QinQ终结子接口应用于L3VPN，SVLAN和CVLAN同时有效
			                    else if ((pstTerminalIfCb->terminal_if_cfg.svlan_id != 0) 
			                          && (pstTerminalIfCb->terminal_if_cfg.cvlan_id != 0))
			                    {
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
				                    {
				                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT_OVID_IVID;
				                    }
				                    else
				                    {
				                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT_OVID_IVID;
				                    }
			                        
			                        stIntfL3.mOvid = pstTerminalIfCb->terminal_if_cfg.svlan_id;
			                        stIntfL3.mIvid = pstTerminalIfCb->terminal_if_cfg.cvlan_id;
			                    }
			                    else
			                    {
			                        OS_PRINTF("***WARNNING***: PORT_INDEX=%ld 只配置了CVlan，未配置SVlan\n", ulPortIndexKey);
			                    }

			                    stIntfL3.posId = pstTerminalIfCb->intf_pos_id;


	                            ret = ApiC3ModIntf(pstLogicalPort->unit_id, &stIntfL3);
#endif

	                            if (ret == SUCCESS)
	                            {
#ifdef SPU

	                            	pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
	                            	almpm_addcfg_intf_onePos(ulPortIndexKey, stIntfL3.posId);
#endif
/*
									//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
									{
										if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
										{
											ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, stIntfL3.posId, DISABLE);
											OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
				                        		ret,
				                        		stIntfL3.posId);
										}
										else
										{
											ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, stIntfL3.posId, ENABLE);
											OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
				                        		ret,
				                        		stIntfL3.posId);
										}
									}
*/
	                            }
	                            

					    	}
					    }
					}

                    /* 保存数据 */
                    if (pstLogicalPort->logic_l3_cfg_cb == NULL)
                    {
                        pstLogicalPort->logic_l3_cfg_cb =
                            (ATG_DCI_LOG_PORT_LOGIC_L3_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_LOGIC_L3_DATA),
                            NBB_NORETRY_ACT,
                            MEM_SPM_LOG_PORT_L3_CB);
                    }

                    OS_MEMCPY(pstLogicalPort->logic_l3_cfg_cb, pstLogicL3Data,
                        sizeof(ATG_DCI_LOG_PORT_LOGIC_L3_DATA));

                }
            }
            else if (ulOperLogicL3 == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperLogicL3 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** Ipv4地址配置L3 ******************************/
            if (ulOperIpv4 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucIpv4DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ipv4_data);

                /* 首地址为NULL，异常 */
                if (pucIpv4DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv4DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  7) Ipv4地址配置L3 (ipv4_num = %d) ADD\n", pstSetLogicalPort->ipv4_num);
                    }

                    OS_SPRINTF(ucMessage, "  7) Ipv4地址配置L3 (ipv4_num = %d) ADD\n", 
                        pstSetLogicalPort->ipv4_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->ipv4_num; i++)
                    {
                        pstIpv4Data[i] = (ATG_DCI_LOG_PORT_IPV4_DATA *)
                            (pucIpv4DataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_IPV4_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_ipv4_cfg(pstIpv4Data[i]);
                        }

                        //spm_dbg_record_logical_port_ipv4_cfg(pstIpv4Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IP地址前缀 = %s", spm_set_ulong_to_ipv4(pstIpv4Data[i]->ip_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstIpv4Data[i]->mask_len));

                        //检查ipv4是否配置过
                        iCfgPos = spm_check_if_ipv4_exist(ulPortIndexKey, pstIpv4Data[i] NBB_CCXT);

                        //无重复
                        if (iCfgPos <= 0)
                        {
                            for (j = 0; j < ATG_DCI_LOG_PORT_IPV4_NUM; j++)
                            {
                                //如果为NULL，说明这个位置为无数据，可以存储数据
                                if (pstLogicalPort->ipv4_cfg_cb[j] == NULL)
                                {
                                    pstLogicalPort->ipv4_cfg_cb[j] =
                                        (ATG_DCI_LOG_PORT_IPV4_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_IPV4_DATA),
                                        NBB_NORETRY_ACT,
                                        MEM_SPM_LOG_PORT_IPV4_CB);
                                    OS_MEMCPY(pstLogicalPort->ipv4_cfg_cb[j], pstIpv4Data[i],
                                        sizeof(ATG_DCI_LOG_PORT_IPV4_DATA));

                                    pstLogicalPort->ipv4_num++;

                                    break;
                                }
                            }
                        }
                        else    //有重复，更新配置
                        {
                            OS_MEMCPY(pstLogicalPort->ipv4_cfg_cb[iCfgPos - 1], pstIpv4Data[i],
                                sizeof(ATG_DCI_LOG_PORT_IPV4_DATA));
                        }
                    }

#ifdef SPU
/*
				    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
				         pstTerminalIfCb != NULL;
				         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
				                       pstTerminalIfCb->spm_terminal_if_node))
				    {
				    	if (pstTerminalIfCb->intf_pos_id != 0)
				    	{
							//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
							if ((pstLogicalPort->basic_cfg_cb != NULL)
							 && (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
							 && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3))
							{
								if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, DISABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
								else
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, ENABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
							}
				    	}
				    }
*/
#endif
                }
            }
            else if (ulOperIpv4 == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucIpv4DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ipv4_data);

                /* 首地址为NULL，异常 */
                if (pucIpv4DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv4DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  7) Ipv4地址配置L3 (ipv4_num = %d) DEL\n", pstSetLogicalPort->ipv4_num);
                    }

                    OS_SPRINTF(ucMessage, "  7) Ipv4地址配置L3 (ipv4_num = %d) DEL\n", 
                        pstSetLogicalPort->ipv4_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->ipv4_num; i++)
                    {
                        pstIpv4Data[i] = (ATG_DCI_LOG_PORT_IPV4_DATA *)
                            (pucIpv4DataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_IPV4_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_ipv4_cfg(pstIpv4Data[i]);
                        }

                        //spm_dbg_record_logical_port_ipv4_cfg(pstIpv4Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IP地址前缀 = %s", spm_set_ulong_to_ipv4(pstIpv4Data[i]->ip_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstIpv4Data[i]->mask_len));

                        for (j = 0; j < ATG_DCI_LOG_PORT_IPV4_NUM; j++)
                        {
                            //如果不为NULL，说明这个位置有数据，可以删除数据
                            if ((pstLogicalPort->ipv4_cfg_cb[j] != NULL)
                                && (!(spm_ipv4_key_compare(pstIpv4Data[i], pstLogicalPort->ipv4_cfg_cb[j] NBB_CCXT))))
                            {
                                NBB_MM_FREE(pstLogicalPort->ipv4_cfg_cb[j], MEM_SPM_LOG_PORT_IPV4_CB);
                                pstLogicalPort->ipv4_cfg_cb[j] = NULL;

                                pstLogicalPort->ipv4_num--;

                                break;
                            }
                        }
                    }
#ifdef SPU
/*
				    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
				         pstTerminalIfCb != NULL;
				         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
				                       pstTerminalIfCb->spm_terminal_if_node))
				    {
				    	if (pstTerminalIfCb->intf_pos_id != 0)
				    	{
							//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
							if ((pstLogicalPort->basic_cfg_cb != NULL)
							 && (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
							 && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3))
							{
								if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, DISABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
								else
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, ENABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
							}
				    	}
				    }
*/
#endif
                }
            }
            else if (ulOperIpv4 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** IPv6地址配置L3 ******************************/
            if (ulOperIpv6 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucIpv6DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ipv6_data);

                /* 首地址为NULL，异常 */
                if (pucIpv6DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv6DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  8) Ipv6地址配置L3 (ipv6_num = %d) ADD\n", pstSetLogicalPort->ipv6_num);
                    }

                    OS_SPRINTF(ucMessage, "  8) Ipv6地址配置L3 (ipv6_num = %d) ADD\n", 
                        pstSetLogicalPort->ipv6_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->ipv6_num; i++)
                    {
                        pstIpv6Data[i] = (ATG_DCI_LOG_PORT_IPV6_DATA *)
                            (pucIpv6DataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_IPV6_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_ipv6_cfg(pstIpv6Data[i]);
                        }

                        //spm_dbg_record_logical_port_ipv6_cfg(pstIpv6Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV6地址前缀 = %x-%x-%x-%x", pstIpv6Data[i]->ip_addr[0],
                                pstIpv6Data[i]->ip_addr[1],
                                pstIpv6Data[i]->ip_addr[2],
                                pstIpv6Data[i]->ip_addr[3]));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstIpv6Data[i]->mask_len));

                        //检查ipv6是否配置过
                        iCfgPos = spm_check_if_ipv6_exist(ulPortIndexKey, pstIpv6Data[i] NBB_CCXT);

                        //无重复
                        if (iCfgPos <= 0)
                        {
                            for (j = 0; j < ATG_DCI_LOG_PORT_IPV6_NUM; j++)
                            {
                                //如果为NULL，说明这个位置为无数据，可以存储数据
                                if (pstLogicalPort->ipv6_cfg_cb[j] == NULL)
                                {
                                    pstLogicalPort->ipv6_cfg_cb[j] =
                                        (ATG_DCI_LOG_PORT_IPV6_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_IPV6_DATA),
                                        NBB_NORETRY_ACT,
                                        MEM_SPM_LOG_PORT_IPV6_CB);
                                    OS_MEMCPY(pstLogicalPort->ipv6_cfg_cb[j], pstIpv6Data[i],
                                        sizeof(ATG_DCI_LOG_PORT_IPV6_DATA));

                                    pstLogicalPort->ipv6_num++;

                                    break;
                                }
                            }
                        }
                        else
                        {
                            OS_MEMCPY(pstLogicalPort->ipv6_cfg_cb[iCfgPos - 1], pstIpv6Data[i],
                                sizeof(ATG_DCI_LOG_PORT_IPV6_DATA));
                        }
                    }
#ifdef SPU
/*
				    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
				         pstTerminalIfCb != NULL;
				         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
				                       pstTerminalIfCb->spm_terminal_if_node))
				    {
				    	if (pstTerminalIfCb->intf_pos_id != 0)
				    	{
							//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
							if ((pstLogicalPort->basic_cfg_cb != NULL)
							 && (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
							 && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3))
							{
								if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, DISABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
								else
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, ENABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
							}
				    	}
				    }
*/
#endif
                }
            }
            else if (ulOperIpv6 == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucIpv6DataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->ipv6_data);

                /* 首地址为NULL，异常 */
                if (pucIpv6DataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv6DataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  8) Ipv6地址配置L3 (ipv6_num = %d) DEL\n", pstSetLogicalPort->ipv6_num);
                    }

                    OS_SPRINTF(ucMessage, "  8) Ipv6地址配置L3 (ipv6_num = %d) DEL\n", 
                        pstSetLogicalPort->ipv6_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->ipv6_num; i++)
                    {
                        pstIpv6Data[i] = (ATG_DCI_LOG_PORT_IPV6_DATA *)
                            (pucIpv6DataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_IPV6_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_ipv6_cfg(pstIpv6Data[i]);
                        }

                        //spm_dbg_record_logical_port_ipv6_cfg(pstIpv6Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV6地址前缀 = %x-%x-%x-%x", pstIpv6Data[i]->ip_addr[0],
                                pstIpv6Data[i]->ip_addr[1],
                                pstIpv6Data[i]->ip_addr[2],
                                pstIpv6Data[i]->ip_addr[3]));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstIpv6Data[i]->mask_len));

                        for (j = 0; j < ATG_DCI_LOG_PORT_IPV6_NUM; j++)
                        {
                            //如果不为NULL，说明这个位置有数据，可以删除数据
                            if ((pstLogicalPort->ipv6_cfg_cb[j] != NULL)
                                && (!(spm_ipv6_key_compare(pstIpv6Data[i], pstLogicalPort->ipv6_cfg_cb[j] NBB_CCXT))))
                            {
                                NBB_MM_FREE(pstLogicalPort->ipv6_cfg_cb[j], MEM_SPM_LOG_PORT_IPV6_CB);
                                pstLogicalPort->ipv6_cfg_cb[j] = NULL;

                                pstLogicalPort->ipv6_num--;

                                break;
                            }
                        }
                    }
#ifdef SPU
/*
				    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
				         pstTerminalIfCb != NULL;
				         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
				                       pstTerminalIfCb->spm_terminal_if_node))
				    {
				    	if (pstTerminalIfCb->intf_pos_id != 0)
				    	{
							//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
							if ((pstLogicalPort->basic_cfg_cb != NULL)
							 && (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
							 && (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3))
							{
								if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, DISABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
								else
								{
									ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, ENABLE);
									OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
		                        		ret,
		                        		pstTerminalIfCb->intf_pos_id);
								}
							}
				    	}
				    }
*/
#endif
                }
            }
            else if (ulOperIpv6 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 组播组地址L3 ******************************/
            if (ulOperMcIpv4 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucMcMacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->mc_ipv4_data);

                /* 首地址为NULL，异常 */
                if (pucMcMacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucMcMacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  9) 组播组地址L3 (mc_ipv4_num = %d) ADD\n", pstSetLogicalPort->mc_ipv4_num);
                    }

                    OS_SPRINTF(ucMessage, "  9) 组播组地址L3 (mc_ipv4_num = %d) ADD\n", 
                        pstSetLogicalPort->mc_ipv4_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->mc_ipv4_num; i++)
                    {
                        pstMcIpv4Data[i] = (ATG_DCI_LOG_PORT_MC_IPV4_DATA *)
                            (pucMcMacDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_MC_IPV4_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);
                        }

                        //spm_dbg_record_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV4组播地址 = %s",
                                spm_set_ulong_to_ipv4(pstMcIpv4Data[i]->mc_ipv4_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstMcIpv4Data[i]->mask_len));

						//调用dk接口
						if ((pstLogicalPort->logic_l3_cfg_cb != NULL) && (pstLogicalPort->basic_cfg_cb != NULL))
						{
							//如果是VE口，根据驱动提供的算法: x*2 + 1 + 0x800，计算出port_id发给驱动
							if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)
							{
								usMcVePortId = (pstLogicalPort->port_id * 2 + 1) + SPM_MC_VE_PORT_OFFSET;
								spm_l3_addonemcip(usMcVePortId, pstLogicalPort->logic_l3_cfg_cb->vrf_id, 0, &pstMcIpv4Data[i]->mc_ipv4_addr);
							}
							else
							{
								spm_l3_addonemcip(pstLogicalPort->port_id, pstLogicalPort->logic_l3_cfg_cb->vrf_id, 0, &pstMcIpv4Data[i]->mc_ipv4_addr);							
							}
						}
						else
						{
	                        pstSetLogicalPort->mc_ipv4_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，组播组地址L3失败",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，组播组地址L3失败\n", ulPortIndexKey);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，组播组地址L3失败\n",
	                            ulPortIndexKey);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
	                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
						}

                        iCfgPos = spm_check_if_mc_ipv4_exist(ulPortIndexKey, pstMcIpv4Data[i] NBB_CCXT);

                        if (iCfgPos <= 0)
                        {
                            for (j = 0; j < ATG_DCI_LOG_PORT_MC_IPV4_NUM; j++)
                            {
                                //如果为NULL，说明这个位置为无数据，可以存储数据
                                if (pstLogicalPort->mc_ipv4_cfg_cb[j] == NULL)
                                {
                                    pstLogicalPort->mc_ipv4_cfg_cb[j] = (ATG_DCI_LOG_PORT_MC_IPV4_DATA *)NBB_MM_ALLOC(
                                        sizeof(ATG_DCI_LOG_PORT_MC_IPV4_DATA),
                                        NBB_NORETRY_ACT,
                                        MEM_SPM_LOG_PORT_MC_IPV4_CB);
                                    OS_MEMCPY(pstLogicalPort->mc_ipv4_cfg_cb[j], pstMcIpv4Data[i],
                                        sizeof(ATG_DCI_LOG_PORT_MC_IPV4_DATA));

                                    pstLogicalPort->mc_ipv4_num++;

                                    break;
                                }
                            }
                        }
                        else
                        {
                            OS_MEMCPY(pstLogicalPort->mc_ipv4_cfg_cb[iCfgPos - 1], pstMcIpv4Data[i],
                                sizeof(ATG_DCI_LOG_PORT_MC_IPV4_DATA));
                        }
                    }
                }
            }
            else if (ulOperMcIpv4 == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucMcMacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->mc_ipv4_data);

                /* 首地址为NULL，异常 */
                if (pucMcMacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucMcMacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  9) 组播组地址L3 (mc_ipv4_num = %d) DEL\n", pstSetLogicalPort->mc_ipv4_num);
                    }

                    OS_SPRINTF(ucMessage, "  9) 组播组地址L3 (mc_ipv4_num = %d) DEL\n", 
                        pstSetLogicalPort->mc_ipv4_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->mc_ipv4_num; i++)
                    {
                        pstMcIpv4Data[i] = (ATG_DCI_LOG_PORT_MC_IPV4_DATA *)
                            (pucMcMacDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_MC_IPV4_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);
                        }

                        //spm_dbg_record_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV4组播地址 = %s",
                                spm_set_ulong_to_ipv4(pstMcIpv4Data[i]->mc_ipv4_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstMcIpv4Data[i]->mask_len));

                        for (j = 0; j < ATG_DCI_LOG_PORT_MC_IPV4_NUM; j++)
                        {
                            //如果不为NULL，说明这个位置有数据，可以删除数据
                            if ((pstLogicalPort->mc_ipv4_cfg_cb[j] != NULL)
                                && (!(spm_ipv4_key_compare(pstMcIpv4Data[i], pstLogicalPort->mc_ipv4_cfg_cb[j] NBB_CCXT))))
                            {
                                NBB_MM_FREE(pstLogicalPort->mc_ipv4_cfg_cb[j], MEM_SPM_LOG_PORT_MC_IPV4_CB);
                                pstLogicalPort->mc_ipv4_cfg_cb[j] = NULL;

                                pstLogicalPort->mc_ipv4_num--;

                                break;
                            }
                        }
                    }
                }
            }
            else if (ulOperMcIpv4 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

	    	/****************************** IPV6组播组地址L3 ******************************/
            if (ulOperMcIpv6 == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucIpv6McMacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->mc_ipv6_data);

                /* 首地址为NULL，异常 */
                if (pucIpv6McMacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv6McMacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  22) IPV6组播组地址L3 (mc_ipv6_num = %d) ADD\n", pstSetLogicalPort->mc_ipv6_num);
                    }

                    OS_SPRINTF(ucMessage, "  22) IPV6组播组地址L3 (mc_ipv6_num = %d) ADD\n", 
                        pstSetLogicalPort->mc_ipv6_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->mc_ipv6_num; i++)
                    {
                        pstMcIpv6Data[i] = (ATG_DCI_LOG_PORT_MC_IPV6_DATA *)
                            (pucIpv6McMacDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_MC_IPV6_DATA))) * i);

                        /* 配置处理，todo */

						if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_mc_ipv6_cfg(pstMcIpv6Data[i]);
                        }

                        //spm_dbg_record_logical_port_mc_ipv6_cfg(pstMcIpv6Data[i]);
						
                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV6组播组地址 = %x-%x-%x-%x", pstMcIpv6Data[i]->mc_ipv6_addr[0],
                                pstMcIpv6Data[i]->mc_ipv6_addr[1],
                                pstMcIpv6Data[i]->mc_ipv6_addr[2],
                                pstMcIpv6Data[i]->mc_ipv6_addr[3]));
                        NBB_TRC_DETAIL((NBB_FORMAT "  前缀长度 = %d", pstMcIpv6Data[i]->mask_len));

						//调用dk接口
						if ((pstLogicalPort->logic_l3_cfg_cb != NULL) && (pstLogicalPort->basic_cfg_cb != NULL))
						{
							//如果是VE口，根据驱动提供的算法: x*2 + 1 + 0x800，计算出port_id发给驱动
							if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)
							{
								usMcVePortId = (pstLogicalPort->port_id * 2 + 1) + SPM_MC_VE_PORT_OFFSET;
								spm_l3_addonemcip(usMcVePortId, pstLogicalPort->logic_l3_cfg_cb->vrf_id, 1, pstMcIpv6Data[i]->mc_ipv6_addr);
							}
							else
							{
								spm_l3_addonemcip(pstLogicalPort->port_id, pstLogicalPort->logic_l3_cfg_cb->vrf_id, 1, pstMcIpv6Data[i]->mc_ipv6_addr);
							}
						}
						else
						{
	                        pstSetLogicalPort->mc_ipv6_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，IPV6组播组地址L3失败",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，IPV6组播组地址L3失败\n", ulPortIndexKey);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，IPV6组播组地址L3失败\n",
	                            ulPortIndexKey);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
	                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
						}

                        iCfgPos = spm_check_if_mc_ipv6_exist(ulPortIndexKey, pstMcIpv6Data[i] NBB_CCXT);

                        if (iCfgPos <= 0)
                        {
                            for (j = 0; j < ATG_DCI_LOG_PORT_MC_IPV6_NUM; j++)
                            {
                                //如果为NULL，说明这个位置为无数据，可以存储数据
                                if (pstLogicalPort->mc_ipv6_cfg_cb[j] == NULL)
                                {
                                    pstLogicalPort->mc_ipv6_cfg_cb[j] = (ATG_DCI_LOG_PORT_MC_IPV6_DATA *)NBB_MM_ALLOC(
                                        sizeof(ATG_DCI_LOG_PORT_MC_IPV6_DATA),
                                        NBB_NORETRY_ACT,
                                        MEM_SPM_LOG_PORT_MC_IPV6_CB);
                                    OS_MEMCPY(pstLogicalPort->mc_ipv6_cfg_cb[j], pstMcIpv6Data[i],
                                        sizeof(ATG_DCI_LOG_PORT_MC_IPV6_DATA));

                                    pstLogicalPort->mc_ipv6_num++;

                                    break;
                                }
                            }
                        }
                        else
                        {
                            OS_MEMCPY(pstLogicalPort->mc_ipv6_cfg_cb[iCfgPos - 1], pstMcIpv6Data[i],
                                sizeof(ATG_DCI_LOG_PORT_MC_IPV6_DATA));
                        }
                    }
                }
            }
            else if (ulOperMcIpv6 == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucIpv6McMacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->mc_ipv6_data);

                /* 首地址为NULL，异常 */
                if (pucIpv6McMacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucIpv6McMacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  22) IPV6组播组地址L3 (mc_ipv6_num = %d) DEL\n", pstSetLogicalPort->mc_ipv6_num);
                    }

                    OS_SPRINTF(ucMessage, "  22) IPV6组播组地址L3 (mc_ipv6_num = %d) DEL\n", 
                        pstSetLogicalPort->mc_ipv6_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->mc_ipv6_num; i++)
                    {
                        pstMcIpv6Data[i] = (ATG_DCI_LOG_PORT_MC_IPV6_DATA *)
                            (pucIpv6McMacDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_MC_IPV6_DATA))) * i);

                        /* 配置处理，todo */
						#if 0
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);
                        }

                        spm_dbg_record_logical_port_mc_ipv4_cfg(pstMcIpv4Data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  IPV4组播地址 = %s",
                                spm_set_ulong_to_ipv4(pstMcIpv4Data[i]->mc_ipv4_addr)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstMcIpv4Data[i]->mask_len));
						#endif

                        for (j = 0; j < ATG_DCI_LOG_PORT_MC_IPV6_NUM; j++)
                        {
                            //如果不为NULL，说明这个位置有数据，可以删除数据
                            if ((pstLogicalPort->mc_ipv6_cfg_cb[j] != NULL)
                                && (!(spm_ipv6_key_compare(pstMcIpv6Data[i], pstLogicalPort->mc_ipv6_cfg_cb[j] NBB_CCXT))))
                            {
                                NBB_MM_FREE(pstLogicalPort->mc_ipv6_cfg_cb[j], MEM_SPM_LOG_PORT_MC_IPV6_CB);
                                pstLogicalPort->mc_ipv6_cfg_cb[j] = NULL;

                                pstLogicalPort->mc_ipv6_num--;

                                break;
                            }
                        }
                    }
                }
            }
            else if (ulOperMcIpv6 == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** VRRP虚拟MAC和IP地址配置L3 ******************************/
            if (ulOperVipVmac == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucVipVmacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->vip_vmac_data);

                /* 首地址为NULL，异常 */
                if (pucVipVmacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucVipVmacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  10) 虚拟MAC和IP地址配置L3 (vip_vmac_num = %d) ADD\n",
                            pstSetLogicalPort->vip_vmac_num);
                    }

                    OS_SPRINTF(ucMessage,
                        "  10) 虚拟MAC和IP地址配置L3 (vip_vmac_num = %d) ADD\n",
                        pstSetLogicalPort->vip_vmac_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    //找到vlan属性第一个配置
                    pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);

                    //没有vlan属性，无法绑定，打印错误，直接退出
                    if(NULL == pstTerminalIfCb)
                    {
                        //此次下发的所有VRRP配置均返回失败
                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP!\n",
			                        		ulPortIndexKey));

                        OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP!\n",
			                        		ulPortIndexKey);

                        OS_SPRINTF(ucMessage,	"  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP!\n",
			                            	ulPortIndexKey);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, BASIC_CONFIG_NOT_EXIST,
			                                "BASIC_CONFIG_NOT_EXIST", ucMessage));		
                    }
                    else
                    {
                        //记录首个VLAN属性配置的地址
                        tempterminalifcb = pstTerminalIfCb;
						
                        //初始化Vrrp的公共成员
                        stvrrp.l3_port_id = pstLogicalPort->port_id;/*lint !e613 */
			   stvrrp.slot = pstLogicalPort->slot_id;/*lint !e613 */

                        //VE口只需置VE标志，非VE口需要填vlan信息
                        if(ATG_DCI_VE_PORT == pstLogicalPort->basic_cfg_cb->port_sub_type)/*lint !e613 */
                        {
                            stvrrp.ve_flag = 1;
                        }

                        //该逻辑接口下的所有INTF都要使能VRRP
#ifdef SPU                        
                        while (pstTerminalIfCb)
                        {

                            ret = fhdrv_psn_l3_set_intf_vrrp(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, 1);
                            
                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                           {                              
                                ret += fhdrv_psn_l3_set_intf_vrrp(1, pstTerminalIfCb->intf2_pos_id, 1);
                           }

                            if (SUCCESS != ret)
                            {
                                   for (i = 0; i < pstSetLogicalPort->vip_vmac_num; i++)
                                   {
                                   	pstSetLogicalPort->vip_vmac_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
                                   }                          	

                            	NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:fhdrv_psn_l3_set_intf_vrrp()"
                                    " ret=%d PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                        		ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id));

                            	OS_PRINTF("  ***ERROR***:fhdrv_psn_l3_set_intf_vrrp() ret=%d "
                                    "PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                        		ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id);

                            	OS_SPRINTF(ucMessage,
    	                            "  ***ERROR***:fhdrv_psn_l3_set_intf_vrrp() ret=%d "
    	                            "PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                            	ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                    ulPortIndexKey, CALL_C3_FUNC_ERROR,
	                                "CALL_C3_FUNC_ERROR", ucMessage));

                                   break;
                            }
							
                            pstTerminalIfCb = AVLL_NEXT(pstLogicalPort->terminal_if_tree, 
                                pstTerminalIfCb->spm_terminal_if_node);
                        }
#endif

                        for (i = 0; i < pstSetLogicalPort->vip_vmac_num; i++)
                        {
                            pstVipVmacData[i] = (ATG_DCI_LOG_PORT_VIP_VMAC_DATA *)
                                (pucVipVmacDataStart + 
                                (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VIP_VMAC_DATA))) * i);

                            /* 配置处理，todo */
                            if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                            {
                                spm_dbg_print_logical_port_vip_vmac_cfg(pstVipVmacData[i]);
                            }

                            //spm_dbg_record_logical_port_vip_vmac_cfg(pstVipVmacData[i]);

                            NBB_TRC_DETAIL((NBB_FORMAT "  MAC地址 = %0x-%0x-%0x-%0x-%0x-%0x", 
                                pstVipVmacData[i]->mac[0],
                                pstVipVmacData[i]->mac[1],
                                pstVipVmacData[i]->mac[2],
                                pstVipVmacData[i]->mac[3],
                                pstVipVmacData[i]->mac[4],
                                pstVipVmacData[i]->mac[5]));
                            NBB_TRC_DETAIL((NBB_FORMAT "  IPv4地址 = %s", 
                                spm_set_ulong_to_ipv4(pstVipVmacData[i]->ip_addr NBB_CCXT)));
                            NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", 
                                pstVipVmacData[i]->mask_len));

                            iCfgPos = spm_check_if_vip_vmac_exist(ulPortIndexKey, 
                                pstVipVmacData[i] NBB_CCXT);

                            //根据本vrrp属性刷新驱动的vrrp配置
                            stvrrp.dmac[0] = pstVipVmacData[i]->mac[0];
                            stvrrp.dmac[1] = pstVipVmacData[i]->mac[1];
                            stvrrp.dmac[2] = pstVipVmacData[i]->mac[2];
                            stvrrp.dmac[3] = pstVipVmacData[i]->mac[3];
                            stvrrp.dmac[4] = pstVipVmacData[i]->mac[4];
                            stvrrp.dmac[5] = pstVipVmacData[i]->mac[5];


			       //首个VLAN属性配置的地址
	                     pstTerminalIfCb =  tempterminalifcb ;
#ifdef SRC				   
			       terminalifnum = pstLogicalPort->terminal_if_num;
#endif	

#ifdef SPU	      
                            terminalifnum = 0;	

                        //该逻辑接口下的所有INTF都要使能VRRP
                        while (pstTerminalIfCb)
                        {

				stvrrp.ovlan_id = pstTerminalIfCb->terminal_if_cfg.svlan_id;
				stvrrp.ivlan_id = pstTerminalIfCb->terminal_if_cfg.cvlan_id;

				//外层VLAN
				if(0  !=  stvrrp.ovlan_id)/*lint !e613 */
				{
					stvrrp.ovlan_id_mask = 0xfff;
				}
				else
				{
					stvrrp.ovlan_id_mask = 0;
				}

				//内外层VLAN
				if(0  !=  stvrrp.ivlan_id)/*lint !e613 */
				{
					stvrrp.ivlan_id_mask = 0xfff;
				}
				else
				{
				 	stvrrp.ivlan_id_mask = 0;
				}						

		              ret = SUCCESS;
			       ret = fhdrv_psn_acl_set_vrrp(pstLogicalPort->unit_id, &stvrrp);
                            vrrp4_pos_id_temp = stvrrp.posid;
                            stvrrp.posid = 0;
                   
                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                           {
                                
                                ret += fhdrv_psn_acl_set_vrrp(1, &stvrrp);
                           }                   
                   
				if(SUCCESS != ret)
                            {
                            	pstSetLogicalPort->vip_vmac_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            	NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:fhdrv_psn_acl_set_vrrp()"
                                            " ret=%d PORT_INDEX=%ld绑定VRRP失败\n",
    		                        		ret, ulPortIndexKey));

                            	OS_PRINTF("  ***ERROR***:fhdrv_psn_acl_set_vrrp() ret=%d PORT_INDEX=%ld绑定VRRP失败\n",
    		                        		ret, ulPortIndexKey);

                            	OS_SPRINTF(ucMessage,
    		                            "  ***ERROR***:fhdrv_psn_acl_set_vrrp() ret=%d PORT_INDEX=%ld 绑定VRRP失败\n",
    		                            	ret, ulPortIndexKey);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, CALL_C3_FUNC_ERROR,
    		                                "CALL_C3_FUNC_ERROR", ucMessage));

                                break;
                            }
				
				terminalifnum++;
							
                            pstTerminalIfCb = AVLL_NEXT(pstLogicalPort->terminal_if_tree, 
                                pstTerminalIfCb->spm_terminal_if_node);
                        }
 #endif                           


                            if(terminalifnum == pstLogicalPort->terminal_if_num)
                            {
                                if (iCfgPos <= 0)
                                {
                                    for (j = 0; j < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; j++)
                                    {
                                        //如果为NULL，说明这个位置为无数据，可以存储数据
                                        if (pstLogicalPort->vip_vmac_cfg_cb[j] == NULL)/*lint !e613 */
                                        {
                                            pstLogicalPort->vip_vmac_cfg_cb[j] =
                                                (ATG_DCI_LOG_PORT_VIP_VMAC_DATA *)NBB_MM_ALLOC(sizeof(
                                                    ATG_DCI_LOG_PORT_VIP_VMAC_DATA),
                                                NBB_NORETRY_ACT,
                                                MEM_SPM_LOG_PORT_VIP_VMAC_CB);/*lint !e613 */

                                            if(pstLogicalPort->vip_vmac_cfg_cb[j] == NULL)/*lint !e613 */
                                            {
                                                OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                            	
                                    			OS_SPRINTF(ucMessage, "	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                                    			BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                                goto EXIT_LABEL;
                                    			
                                            }
                                            OS_MEMCPY(pstLogicalPort->vip_vmac_cfg_cb[j], pstVipVmacData[i],
                                                sizeof(ATG_DCI_LOG_PORT_VIP_VMAC_DATA));/*lint !e613 */

                                            pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[j] =    /*lint !e613 */
                                                vrrp4_pos_id_temp;
                                            pstLogicalPort->logic_port_info_cb.vrrp_ipv4_posid2[j] =    /*lint !e613 */
                                                stvrrp.posid;
                                            pstLogicalPort->vip_vmac_num++;/*lint !e613 */
        
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    OS_MEMCPY(pstLogicalPort->vip_vmac_cfg_cb[iCfgPos - 1], pstVipVmacData[i],
                                        sizeof(ATG_DCI_LOG_PORT_VIP_VMAC_DATA));/*lint !e613 */
                                }
                            }
                        }
                    }


                }
            }
            else if (ulOperVipVmac == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucVipVmacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->vip_vmac_data);

                /* 首地址为NULL，异常 */
                if (pucVipVmacDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucVipVmacDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  10) 虚拟MAC和IP地址配置L3 (vip_vmac_num = %d) DEL\n",
                            pstSetLogicalPort->vip_vmac_num);
                    }

                    OS_SPRINTF(ucMessage,
                        "  10) 虚拟MAC和IP地址配置L3 (vip_vmac_num = %d) DEL\n",
                        pstSetLogicalPort->vip_vmac_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->vip_vmac_num; i++)
                    {
                        pstVipVmacData[i] = (ATG_DCI_LOG_PORT_VIP_VMAC_DATA *)
                            (pucVipVmacDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VIP_VMAC_DATA))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_vip_vmac_cfg(pstVipVmacData[i]);
                        }

                        //spm_dbg_record_logical_port_vip_vmac_cfg(pstVipVmacData[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  MAC地址 = %0x-%0x-%0x-%0x-%0x-%0x", pstVipVmacData[i]->mac[0],
                                pstVipVmacData[i]->mac[1],
                                pstVipVmacData[i]->mac[2],
                                pstVipVmacData[i]->mac[3],
                                pstVipVmacData[i]->mac[4],
                                pstVipVmacData[i]->mac[5]));
                        NBB_TRC_DETAIL((NBB_FORMAT "  IPv4地址 = %s", 
                            spm_set_ulong_to_ipv4(pstVipVmacData[i]->ip_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", pstVipVmacData[i]->mask_len));

                        iCfgPos = spm_check_if_vip_vmac_exist(ulPortIndexKey, pstVipVmacData[i] NBB_CCXT);
                        if(iCfgPos < 0)//不存在，无法删除
                        {
                            pstSetLogicalPort->vip_vmac_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld 的逻辑端口不存在VRRP-MAC地址 "
                                            "=%2x-%2x-%2x-%2x-%2x-%2x,无法删除! \n",
			                        		ulPortIndexKey, pstVipVmacData[i]->mac[0],
			                        		pstVipVmacData[i]->mac[1],
			                        		pstVipVmacData[i]->mac[2],
			                        		pstVipVmacData[i]->mac[3],
			                        		pstVipVmacData[i]->mac[4],
			                        		pstVipVmacData[i]->mac[5]);

                            OS_SPRINTF(ucMessage,
			                            "  ***ERROR***:PORT_INDEX=%ld 的逻辑端口不存在VRRP-MAC地址 "
    			                            "=%2x-%2x-%2x-%2x-%2x-%2x,无法删除! \n",
			                            	ulPortIndexKey, pstVipVmacData[i]->mac[0],
			                        		pstVipVmacData[i]->mac[1],
			                        		pstVipVmacData[i]->mac[2],
			                        		pstVipVmacData[i]->mac[3],
			                        		pstVipVmacData[i]->mac[4],
			                        		pstVipVmacData[i]->mac[5]);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        }
                        else
                        {
#ifdef SPU                        
                            stvrrp.posid = 
                                pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[iCfgPos - 1];/*lint !e613 */
                            ret = SUCCESS;

                            ret = fhdrv_psn_acl_del_vrrp(pstLogicalPort->unit_id, &stvrrp);
                   
                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                           {
                                stvrrp.posid = 
                                    pstLogicalPort->logic_port_info_cb.vrrp_ipv4_posid2[iCfgPos - 1];
                                if(0 != stvrrp.posid)
                                {
                                    ret += fhdrv_psn_acl_del_vrrp(1, &stvrrp);
                                }                                
                           } 

                            if(SUCCESS != ret)
                            {
                                pstSetLogicalPort->vip_vmac_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            	NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld"
                                            " 的vrrp_pos_id=%d删除失败\n",
			                        		ret, ulPortIndexKey, 
			                        		pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[iCfgPos - 1]));

                            	OS_PRINTF(
						"  ***ERROR***:fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld 的vrrp_pos_id=%d删除失败\n",
			                        		ret, ulPortIndexKey, 
			                        		pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[iCfgPos - 1]);

                            	OS_SPRINTF(ucMessage,
			                            "  ***ERROR***:fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld 的vrrp_pos_id=%d删除失败\n",
			                            	ret, ulPortIndexKey, 
			                            	pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[iCfgPos - 1]);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, CALL_C3_FUNC_ERROR,
			                                "CALL_C3_FUNC_ERROR", ucMessage));
                                continue;
                            }
#endif

                            for (j = 0; j < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; j++)
                            {
                                //如果不为NULL，说明这个位置有数据，可以删除数据
                                if ((pstLogicalPort->vip_vmac_cfg_cb[j] != NULL)
                                    && (!(spm_ip_mac_key_compare(pstVipVmacData[i], 
                                    pstLogicalPort->vip_vmac_cfg_cb[j] NBB_CCXT))))/*lint !e613 */
                                {
                                    NBB_MM_FREE(pstLogicalPort->vip_vmac_cfg_cb[j], /*lint !e613 */
                                        MEM_SPM_LOG_PORT_VIP_VMAC_CB);
                                    pstLogicalPort->vip_vmac_cfg_cb[j] = NULL;/*lint !e613 */
    
                                    pstLogicalPort->logic_port_info_cb.ulVrrpMacPosId[j] = 0;/*lint !e613 */
                                    
                                    if((2 == SHARED.c3_num) && 
                                        ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                        || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                                   {
                                        pstLogicalPort->logic_port_info_cb.vrrp_ipv4_posid2[j] = 0;/*lint !e613 */
                                   } 
                                    
                                    pstLogicalPort->vip_vmac_num--;/*lint !e613 */
    
                                    break;
                                }
                            }
                        }

                    }
                }
            }
            else if (ulOperVipVmac == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

	        /******************************** VE MAC地址配置 *********************************/
	        if (ulOperVeMac == ATG_DCI_OPER_ADD)
	        {

	            /* 计算第一个entry的地址。*/
	            pucVeMacDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
	                &pstSetLogicalPort->ve_mac_data);

	            /* 首地址为NULL，异常 */
	            if (pucVeMacDataStart == NULL)
	            {
	                NBB_TRC_FLOW((NBB_FORMAT "  LOGICAL PORT pucVeMacDataStart is NULL."));
	                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
	            }
	            else
	            {
	                pstVeMacData = (ATG_DCI_LOG_PORT_VE_MAC_DATA *)pucVeMacDataStart;

	                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                {
	                    printf("  11）VE MAC地址配置\n");
	                    spm_dbg_print_logical_port_ve_mac_cfg(pstVeMacData);
	                }

	                OS_SPRINTF(ucMessage, "  11）VE MAC地址配置\n");
	                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
	                //spm_dbg_record_logical_port_ve_mac_cfg(pstVeMacData);

	                NBB_TRC_DETAIL((NBB_FORMAT "  MAC地址 = %0x-%0x-%0x-%0x-%0x-%0x", pstVeMacData->mac[0],
	                        pstVeMacData->mac[1],
	                        pstVeMacData->mac[2],
	                        pstVeMacData->mac[3],
	                        pstVeMacData->mac[4],
	                        pstVeMacData->mac[5]));
	                        
	                ve_mac[0] = pstVeMacData->mac[0];
	                ve_mac[1] = pstVeMacData->mac[1];
	                ve_mac[2] = pstVeMacData->mac[2];
	                ve_mac[3] = pstVeMacData->mac[3];
	                ve_mac[4] = pstVeMacData->mac[4];
	                ve_mac[5] = pstVeMacData->mac[5];

#ifdef SPU
					if ((pstLogicalPort->basic_cfg_cb->port_flag != ATG_DCI_TUNNEL)
					 && (pstLogicalPort->basic_cfg_cb->port_flag != ATG_DCI_LOOPBACK)
					 && (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT))
					{
		                //如果条目为新增，且之前配置过以太网物理配置
		                if (pstLogicalPort->phy_cfg_cb != NULL)
		                {
		                    for (ucC3Unit = 0; ucC3Unit < SHARED.c3_num; ucC3Unit++)
		                    {
		                        ret = ApiC3SetPortMac(ucC3Unit, ATG_DCI_IS_VE, pstLogicalPort->phy_cfg_cb->ve_group_id, ve_mac);
		                        NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%d, ApiC3SetPortMac() ret=%d", ulPortIndexKey, ret));
		                    }
		                }
		                else
		                {
		                    pstSetLogicalPort->ve_mac_return_code = ATG_DCI_RC_UNSUCCESSFUL;
		                    OS_PRINTF("PORT_INDEX=%ld, VE MAC地址配置失配，group_id无法获取!\n", ulPortIndexKey);
		                }
					}

#endif

	                /* 保存数据 */
	                if (pstLogicalPort->ve_mac_cfg_cb == NULL)
	                {
	                    pstLogicalPort->ve_mac_cfg_cb =
	                        (ATG_DCI_LOG_PORT_VE_MAC_DATA *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_PORT_VE_MAC_DATA),
	                        NBB_NORETRY_ACT,
	                        MEM_SPM_LOG_PORT_VE_MAC_CB);
	                }

	                OS_MEMCPY(pstLogicalPort->ve_mac_cfg_cb, pstVeMacData, sizeof(ATG_DCI_LOG_PORT_VE_MAC_DATA));
	            }

	        }
	        else if (ulOperBasic == ATG_DCI_OPER_DEL)
	        {
	            NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
	        }
	        else if (ulOperBasic == ATG_DCI_OPER_UPDATE)
	        {
	            NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
	        }
        
	        /****************************** VLAN属性 L3处理******************************/
	        if (ulOperTerminalIf == ATG_DCI_OPER_ADD)
			{

	            /* 计算第一个entry的地址。*/
	            pucTerminalIfDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
	                &pstSetLogicalPort->vlan_data);

	            /* 首地址为NULL，异常 */
	            if (pucTerminalIfDataStart == NULL)
	            {
	                NBB_TRC_FLOW((NBB_FORMAT "  pucTerminalIfDataStart is NULL."));
	                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
	            }
	            else
	            {

	                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                {
	                    printf("  21）VLAN属性 L3 (num = %d) ADD\n", pstSetLogicalPort->vlan_num);
	                }

	                OS_SPRINTF(ucMessage, "  21）VLAN属性 L3 (num = %d) ADD\n", pstSetLogicalPort->vlan_num);
	                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                //如果为BROADCAST口才建INTF，为TUNNEL口不建INTF，返回错
	                //&& 如果不为 VLAN子接口才建INTF，不然保存
	                if ((pstLogicalPort->basic_cfg_cb != NULL)
	                 && (pstLogicalPort->logic_l3_cfg_cb != NULL)
	                 && (pstLogicalPort->basic_cfg_cb->port_flag == ATG_DCI_BROADCAST)
	                 && (pstLogicalPort->port_id != 0))
	                {

	                    stIntfL3.mCardPort = pstLogicalPort->port_id;
	                    stIntfL3.bindId = pstLogicalPort->logic_l3_cfg_cb->vrf_id;
	                    stIntfL3.eIntfType = INTF_TYPE_L3;

	                    //IPV4 URPF使能
	                    if (pstLogicalPort->logic_l3_cfg_cb->ipv4_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
	                    {
	                        stIntfL3.eIpv4UrpfMode = URPF_OFF;
	                    }
	                    else if (pstLogicalPort->logic_l3_cfg_cb->ipv4_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
	                    {
	                        stIntfL3.eIpv4UrpfMode = URPF_STRICT;
	                    }
	                    else if (pstLogicalPort->logic_l3_cfg_cb->ipv4_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
	                    {
	                        stIntfL3.eIpv4UrpfMode = URPF_LOOSE;
	                    }

	                    //IPV6 URPF使能
	                    if (pstLogicalPort->logic_l3_cfg_cb->ipv6_urpf_mode == ATG_DCI_INTF_URPF_DEFAULT)     //URPF缺省检查
	                    {
	                        stIntfL3.eIpv6UrpfMode = URPF_OFF;
	                    }
	                    else if (pstLogicalPort->logic_l3_cfg_cb->ipv6_urpf_mode == ATG_DCI_INTF_URPF_STRICT)  //URPF严格检查
	                    {
	                        stIntfL3.eIpv6UrpfMode = URPF_STRICT;
	                    }
	                    else if (pstLogicalPort->logic_l3_cfg_cb->ipv6_urpf_mode == ATG_DCI_INTF_URPF_LOOSE)  //URPF松散检查
	                    {
	                        stIntfL3.eIpv6UrpfMode = URPF_LOOSE;
	                    }
	                    
	                    //缺省路由检查使能
	                    if (pstLogicalPort->logic_l3_cfg_cb->ipv4_default_check == ENABLE)
	                    {
	                        stIntfL3.flags |= INTF_V4_ALLOW_DEFAULT_ROUTE;
	                    }

	                    if (pstLogicalPort->logic_l3_cfg_cb->ipv6_default_check == ENABLE)
	                    {
	                        stIntfL3.flags |= INTF_V6_ALLOW_DEFAULT_ROUTE;
	                    }

	                    //MPLS转发不使能
	                    if (pstLogicalPort->logic_l3_cfg_cb->mpls_forward_enable == DISABLE)
	                    {
	                        stIntfL3.flags |= INTF_MPLS_DISABLE;
	                    }
	                    
	                    //stIntfL3.flags |= INTF_COUNTER_ON;  //todo

	                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
		                {
		                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
		                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN)))
		                        * i);

		                    /* 配置处理，todo */
		                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
		                    {
		                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
		                    }

		                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

		                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
		                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));

		                    //基于端口
		                    if ((pstTerminalIf->svlan_id == 0) && (pstTerminalIf->cvlan_id == 0))
		                    {
								if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
			                    {
			                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT;
			                    }
			                    else
			                    {
			                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT;
			                    }
		                        
		                        stIntfL3.mOvid = 0;
		                        stIntfL3.mIvid = 0;
		                    }

		                    //Dot1q终结子接口应用于L3VPN，SVLAN有效、CVLAN无效
		                    else if ((pstTerminalIf->svlan_id != 0) && (pstTerminalIf->cvlan_id == 0))
		                    {
								if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
			                    {
			                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT_OVID;
			                    }
			                    else
			                    {
			                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT_OVID;
			                    }
		                        
		                        stIntfL3.mOvid = pstTerminalIf->svlan_id;
		                        stIntfL3.mIvid = 0;
		                    }

		                    //QinQ终结子接口应用于L3VPN，SVLAN和CVLAN同时有效
		                    else if ((pstTerminalIf->svlan_id != 0) && (pstTerminalIf->cvlan_id != 0))
		                    {
								if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)    //虚拟以太网口
			                    {
			                        stIntfL3.mIntfMatchType = INTF_MATCH_L3_VE_PORT_OVID_IVID;
			                    }
			                    else
			                    {
			                    	stIntfL3.mIntfMatchType = INTF_MATCH_PORT_OVID_IVID;
			                    }
		                        
		                        stIntfL3.mOvid = pstTerminalIf->svlan_id;
		                        stIntfL3.mIvid = pstTerminalIf->cvlan_id;
		                    }
		                    else
		                    {
		                        OS_PRINTF("***WARNNING***: PORT_INDEX=%ld 只配置了CVlan，未配置SVlan\n", ulPortIndexKey);
		                    }


	                        //如果为子接口，只在本槽位建intf；
	                        //如果为LAG或VE口，必须建intf；
	                        if ((((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_SUB_INTFACE))
	                                && (SHARED.local_slot_id == pstLogicalPort->slot_id))
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG)
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT))
	                        {
	                        	ret = SUCCESS;
#ifdef SPU
	                            ret = ApiC3AddIntf(pstLogicalPort->unit_id, &stIntfL3);
                                   intf_pos_id_temp = stIntfL3.posId;
	                            NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%ld, ApiC3AddIntf() ret = %d",
	                                    ulPortIndexKey, ret));
                                   stIntfL3.posId = 0;
                                   
                                    if((2 == SHARED.c3_num) && 
                                        ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                        || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                                   {
                                        
                                        ret += ApiC3AddIntf(1, &stIntfL3);
                                        NBB_TRC_DETAIL((NBB_FORMAT " L3: 1: PORT_INDEX=%ld, ApiC3AddIntf() ret = %d",
	                                    ulPortIndexKey, ret));
                                   }                                 
#endif
			                    if (ret == SUCCESS)
			                    {

					                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

									//如果不存在，申请树节点，插入树中，计数加1
					                if (NULL == pstTerminalIfCb)
					                {
					                	pstTerminalIfCb = spm_alloc_terminal_if_cb(NBB_CXT);
					                	
						                //保存数据并插入树中
						                pstTerminalIfCb->intf_pos_id = intf_pos_id_temp;
                                                          pstTerminalIfCb->intf2_pos_id = stIntfL3.posId;
						                NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
						                AVLL_INSERT(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

						                pstLogicalPort->terminal_if_num++;
					                }
									else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
									{
                                                                    pstTerminalIfCb->intf_pos_id = intf_pos_id_temp;
                                                                    pstTerminalIfCb->intf2_pos_id = stIntfL3.posId;
										NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
									}
#ifdef SPU
									almpm_addcfg_intf_onePos(ulPortIndexKey, pstTerminalIfCb->intf_pos_id);
/*
									//如果是物理接口(主接口)，先判断是否配置了IP，只有配置了IP后才使能
									if (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
									{
										if ((pstLogicalPort->ipv4_num == 0) && (pstLogicalPort->ipv6_num == 0))
										{
											ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, stIntfL3.posId, DISABLE);
											OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d DISABLE\n",
				                        		ret,
				                        		stIntfL3.posId);
										}
										else
										{
											ret = ApiC3SetIntfStat(pstLogicalPort->unit_id, stIntfL3.posId, ENABLE);
											OS_PRINTF("***INFO***:ApiC3SetIntfStat() ret=%d posId=%d ENABLE\n",
				                        		ret,
				                        		stIntfL3.posId);
										}
									}
*/
#endif					                
			                    }
			                    else
			                    {
			                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

			                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败",
			                        		ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id));

			                        OS_PRINTF("***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败\n",
			                        		ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            "***ERROR***:ApiC3AddIntf() ret=%d PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败\n",
			                            	ret,
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
			                    }
	                        }
	                        else
	                        {
				                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

								//如果不存在，申请树节点，插入树中，计数加1
				                if (NULL == pstTerminalIfCb)
				                {
				                	pstTerminalIfCb = spm_alloc_terminal_if_cb(NBB_CXT);
				                	
					                //保存数据并插入树中
					                pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
					                NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
					                AVLL_INSERT(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

					                pstLogicalPort->terminal_if_num++;
				                }
								else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
								{
									pstTerminalIfCb->intf_pos_id = stIntfL3.posId;

									//coverity[bad_sizeof]
									NBB_MEMCPY(&(pstTerminalIfCb->terminal_if_cfg), pstTerminalIf, sizeof(ATG_DCI_LOG_PORT_VLAN));
								}
	                        }
		                }
	                }
	                else
	                {
	                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
		                {
	                    	pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
	                    }

	                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld <基本配置为空><逻辑配置L3为空><端口标识不为BroadCast><port_id=0>",
	                    		ulPortIndexKey));

	                    OS_PRINTF("***ERROR***:PORT_INDEX=%ld <基本配置为空><逻辑配置L3为空><端口标识不为BroadCast><port_id=0>\n",
	                    		ulPortIndexKey);

	                    OS_SPRINTF(ucMessage,
	                        "***ERROR***:PORT_INDEX=%ld <基本配置为空><逻辑配置L3为空><端口标识不为BroadCast><port_id=0>\n",
	                    		ulPortIndexKey);
	                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                            "CALL_FUNC_ERROR", ucMessage));
	                }
	            }
	        }
	        else if (ulOperTerminalIf == ATG_DCI_OPER_DEL)
	        {

	            /* 计算第一个entry的地址。*/
	            pucTerminalIfDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
	                &pstSetLogicalPort->vlan_data);

	            /* 首地址为NULL，异常 */
	            if (pucTerminalIfDataStart == NULL)
	            {
	                NBB_TRC_FLOW((NBB_FORMAT "  pucTerminalIfDataStart is NULL."));
	                NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
	            }
	            else
	            {

	                if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                {
	                    printf("  21）VLAN属性 L3 (num = %d) DEL\n", pstSetLogicalPort->vlan_num);
	                }

	                OS_SPRINTF(ucMessage, "  21）VLAN属性 L3 (num = %d) DEL\n", pstSetLogicalPort->vlan_num);
	                BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
	                
	                //如果为BROADCAST口才建INTF，为TUNNEL口不建INTF，返回错
	                //&& 如果不为 VLAN子接口才建INTF，不然保存
	                if ((pstLogicalPort->basic_cfg_cb != NULL)
	                 && (pstLogicalPort->logic_l3_cfg_cb != NULL)
	                 && (pstLogicalPort->basic_cfg_cb->port_flag == ATG_DCI_BROADCAST)
	                 && (pstLogicalPort->port_id != 0))
	                {

	                	for (i = 0; i < pstSetLogicalPort->vlan_num; i++)
		                {
		                    pstTerminalIf = (ATG_DCI_LOG_PORT_VLAN*)
		                        (pucTerminalIfDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_VLAN))) * i);

		                    /* 配置处理，todo */
		                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
		                    {
		                        spm_dbg_print_logical_port_terminal_if_cfg(pstTerminalIf);
		                    }

		                    //spm_dbg_record_logical_port_terminal_if_cfg(pstTerminalIf);

		                    NBB_TRC_DETAIL((NBB_FORMAT "  SVLAN_ID  = %d", pstTerminalIf->svlan_id));
		                    NBB_TRC_DETAIL((NBB_FORMAT "  CVLAN_ID  = %d", pstTerminalIf->cvlan_id));


	                        //如果为子接口，只在本槽位建intf；
	                        //如果为LAG或VE口，必须建intf；
	                        if ((((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_INTFACE)
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_SUB_INTFACE))
	                                && (SHARED.local_slot_id == pstLogicalPort->slot_id))
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG)
	                            || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT))
	                        {

				                pstTerminalIfCb = AVLL_FIND(pstLogicalPort->terminal_if_tree, pstTerminalIf);

								//如果不存在，无法删除
				                if (NULL == pstTerminalIfCb)
				                {
			                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

			                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 不存在，无法删除",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id));

			                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);

			                        OS_SPRINTF(ucMessage,
			                            "***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 不存在，无法删除\n",
			                        		ulPortIndexKey,
			                        		pstTerminalIf->svlan_id,
			                        		pstTerminalIf->cvlan_id);
			                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

			                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
			                                "CALL_FUNC_ERROR", ucMessage));
				                }
                                            else	//存在删除
                                            {
                                                ret = SUCCESS;
#ifdef SPU
                                                ret = ApiC3DelIntf(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id);

                                                NBB_TRC_DETAIL((NBB_FORMAT "  PORT_INDEX=%ld, ApiC3DelIntf() ret = %d",
                                                ulPortIndexKey, ret));

                                                if((2 == SHARED.c3_num) && 
                                                    ((stPortInfo.port_type == ATG_DCI_LAG) 
                                                    || (stPortInfo.port_type == ATG_DCI_VE_PORT)))
                                                {
                                                    ret += ApiC3DelIntf(1, pstTerminalIfCb->intf2_pos_id);
                                                    NBB_TRC_DETAIL((NBB_FORMAT 
                                                    " L3 1: PORT_INDEX=%ld, ApiC3DelIntf() ret = %d",
                                                    ulPortIndexKey, ret));
                                                }                                      
#endif
									if (ret == SUCCESS)
									{
#ifdef SPU
										almpm_delline_intf(ulPortIndexKey, pstTerminalIfCb->intf_pos_id, 1);
#endif
							            AVLL_DELETE(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

							            //释放端口逻辑配置节点的内存空间
							            spm_free_terminal_if_cb(pstTerminalIfCb NBB_CCXT);
							            
										pstLogicalPort->terminal_if_num--;
									}
									else
									{
				                        pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

				                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3DelIntf() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败",
				                        		ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id));

				                        OS_PRINTF("***ERROR***:ApiC3DelIntf() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败\n",
				                        		ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id);

				                        OS_SPRINTF(ucMessage,
				                            	"***ERROR***:ApiC3DelIntf() ret=%d, PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的 VLAN属性 L3 增加失败\n",
				                        		ret,
				                        		ulPortIndexKey,
				                        		pstTerminalIf->svlan_id,
				                        		pstTerminalIf->cvlan_id);
				                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

				                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
				                                "CALL_FUNC_ERROR", ucMessage));
				                    }
								}			
	                        }    
	                        else
	                        {
	                        	if (pstTerminalIfCb != NULL)
	                        	{
						            AVLL_DELETE(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);

						            //释放端口逻辑配置节点的内存空间
						            spm_free_terminal_if_cb(pstTerminalIfCb NBB_CCXT);
						            
									pstLogicalPort->terminal_if_num--;
	                        	}
	                        }
		                }
	                }
	            }
	        }
	        else if (ulOperTerminalIf == ATG_DCI_OPER_UPDATE)
	        {
	            NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
	        }
	        
            /****************************** Diff-Serv配置 ******************************/
            if (ulOperDiffServ == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDiffServDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->diff_serv_data);

                /* 首地址为NULL，异常 */
                if (pucDiffServDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDiffServDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                
                    OS_SPRINTF(ucMessage, "  12）Diff-Serv配置 ADD\n");
	             BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                 
                    for (i = 0; i < pstSetLogicalPort->diff_serv_num; i++)
                    {
                        pstDiffServData = (ATG_DCI_LOG_PORT_DIFF_SERV_DATA *)
                            (pucDiffServDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  12）Diff-Serv配置 ADD\n");
	                        spm_dbg_print_logical_port_diff_serv_cfg(pstDiffServData);
	                    }

	                    //OS_SPRINTF(ucMessage, "  12）Diff-Serv配置 ADD\n");
	                    //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
	                    //spm_dbg_record_logical_port_diff_serv_cfg(pstDiffServData);

	                    NBB_TRC_DETAIL((NBB_FORMAT "  Diff-ServID   = %d", pstDiffServData->diff_serv_id));

	                    /* 配置处理，todo */
	                    stQosIntfKey.index = ulPortIndexKey;
	                    stQosIntfKey.svlan = pstDiffServData->svlan;
	                    stQosIntfKey.cvlan = pstDiffServData->cvlan;

	                    //如果逻辑接口存在
	                    if (ucIfExist == ATG_DCI_EXIST)
	                    {
	                        OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));

	                        spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);
	                        stSubPort.unit = stPortInfo.unit_id;
	                        stSubPort.port = stPortInfo.port_id;
	                        stSubPort.slot = stPortInfo.slot_id;
	                        stSubPort.ivlan = stPortInfo.cvlan;
	                        stSubPort.ovlan = stPortInfo.svlan;

							ulVlanPosId = 0;
		                    spm_get_poscnt_from_intf(ulPortIndexKey, 
		                    	pstDiffServData->svlan, 
		                    	pstDiffServData->cvlan, 
		                    	&ulVlanPosId NBB_CCXT);
		                    	
		                    stSubPort.posid = ulVlanPosId;

                            /*
                            if(0 == ulVlanPosId)
                            {
                                pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                            NBB_TRC_DETAIL((NBB_FORMAT
	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan));

	                            OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);

	                            OS_SPRINTF(ucMessage,
	                                "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);
	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                    "CALL_FUNC_ERROR", ucMessage));
                            }*/

                            //else
                            //{
                               ret = spm_ds_add_logic_intf_node(&stSubPort,
	                            &stQosIntfKey,
	                            pstDiffServData
	                            NBB_CCXT);

    	                        if (ret != SUCCESS)
    	                        {
    	                            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
    	                            NBB_TRC_DETAIL((NBB_FORMAT
    	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan));

    	                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败\n", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);

    	                            OS_SPRINTF(ucMessage,
    	                                "***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 增加 SVlan=%d CVlan=%d 的Diff-Serv配置失败\n", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);
    	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

    	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
    	                                    "CALL_FUNC_ERROR", ucMessage));
    	                        } 
                                else
                                {
                                    //pstdiffservcb = AVLL_FIND(pstLogicalPort->diff_serv_tree, pstDiffServData);
                                    //diffservcfgkey.svlan_id = pstDiffServData->svlan;
                                    //diffservcfgkey.cvlan_id = pstDiffServData->cvlan;
                                    //pstdiffservcb = 
                                        //AVLL_FIND(pstLogicalPort->diff_serv_tree, &diffservcfgkey);
                                    pstdiffservcb = 
                                        AVLL_FIND(pstLogicalPort->diff_serv_tree, pstDiffServData);  
                                    
                                    //如果不存在，申请树节点，插入树中，计数加1
                                    if (NULL == pstdiffservcb)
                                    {
                                        pstdiffservcb = spm_alloc_diff_serv_cb(NBB_CXT);

                                        //保存数据并插入树中
                                        //pstTerminalIfCb->intf_pos_id = stIntfL3.posId;
                                        //NBB_MEMCPY(&pstdiffservcb->key,&diffservcfgkey,
                                            //sizeof(ATG_DCI_LOG_PORT_VLAN));                                        
                                        NBB_MEMCPY(&(pstdiffservcb->diff_serv_cfg),
                                            pstDiffServData, sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
                                        AVLL_INSERT(pstLogicalPort->diff_serv_tree, pstdiffservcb->spm_diff_serv_node);

                                        pstLogicalPort->diff_serv_num++;
                                    }
                                    else	//应该不会出现此情况，因为下发下来的，必须是配置不同的
                                    {
                                        //pstdiffservcb->intf_pos_id = stIntfFlow.posId;
                                        NBB_MEMCPY(&(pstdiffservcb->diff_serv_cfg), 
                                            pstDiffServData, sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
                                    }
                                }      
								
                            //}

	                        
#if 0
	                        if (ret == SUCCESS)
	                        {
	                            /* 保存数据 */
	                            if (pstLogicalPort->diff_serv_cfg_cb == NULL)
	                            {
	                                pstLogicalPort->diff_serv_cfg_cb = (ATG_DCI_LOG_PORT_DIFF_SERV_DATA *)NBB_MM_ALLOC(
	                                    sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA),
	                                    NBB_NORETRY_ACT,
	                                    MEM_SPM_LOG_PORT_DIFF_SERV_CB);
	                            }

	                            OS_MEMCPY(pstLogicalPort->diff_serv_cfg_cb, pstDiffServData,
	                                sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
	                        }
	                        else
	                        {
	                            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                            NBB_TRC_DETAIL((NBB_FORMAT
	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() error", ulPortIndexKey));

	                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d\n",
	                                ulPortIndexKey,
	                                ret);

	                            OS_SPRINTF(ucMessage,
	                                "***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d\n",
	                                ulPortIndexKey,
	                                ret);
	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                    "CALL_FUNC_ERROR", ucMessage));
	                        }
#endif
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，建立Diff-Serv失败",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，建立Diff-Serv失败\n", ulPortIndexKey);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，建立Diff-Serv失败\n",
	                            ulPortIndexKey);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
	                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
	                    }
                    }
                }
            }
            else if (ulOperDiffServ == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucDiffServDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->diff_serv_data);

                /* 首地址为NULL，异常 */
                if (pucDiffServDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDiffServDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  12）Diff-Serv配置 DEL\n");
	             BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                 
                    for (i = 0; i < pstSetLogicalPort->diff_serv_num; i++)
                    {
                        pstDiffServData = (ATG_DCI_LOG_PORT_DIFF_SERV_DATA *)
                            (pucDiffServDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  12）Diff-Serv配置 DEL\n");
	                        spm_dbg_print_logical_port_diff_serv_cfg(pstDiffServData);
	                    }

	                    //OS_SPRINTF(ucMessage, "  12）Diff-Serv配置 DEL\n");
	                    //BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        
	                    //spm_dbg_record_logical_port_diff_serv_cfg(pstDiffServData);

	                    NBB_TRC_DETAIL((NBB_FORMAT "  Diff-ServID   = %d", pstDiffServData->diff_serv_id));

	                    /* 配置处理，todo */
	                    stQosIntfKey.index = ulPortIndexKey;
	                    stQosIntfKey.svlan = pstDiffServData->svlan;
	                    stQosIntfKey.cvlan = pstDiffServData->cvlan;

	                    //如果逻辑接口存在
	                    if (ucIfExist == ATG_DCI_EXIST)
	                    {
	                        OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));

	                        spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);
	                        stSubPort.unit = stPortInfo.unit_id;
	                        stSubPort.port = stPortInfo.port_id;
	                        stSubPort.slot = stPortInfo.slot_id;
	                        stSubPort.ivlan = stPortInfo.cvlan;
	                        stSubPort.ovlan = stPortInfo.svlan;

							ulVlanPosId = 0;
		                    spm_get_poscnt_from_intf(ulPortIndexKey, 
		                    	pstDiffServData->svlan, 
		                    	pstDiffServData->cvlan, 
		                    	&ulVlanPosId NBB_CCXT);
		                    	
		                    stSubPort.posid = ulVlanPosId;
                            if(0 == ulVlanPosId)
                            {
                                pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                            NBB_TRC_DETAIL((NBB_FORMAT
	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan));

	                            OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);

	                            OS_SPRINTF(ucMessage,
	                                "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node()删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败,posId=0!\n", 
	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);
	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                    "CALL_FUNC_ERROR", ucMessage));
                            }

                            else
                            {
                                pstDiffServData->diff_serv_id = 0;
		                    
    	                        ret = spm_ds_add_logic_intf_node(&stSubPort,
    	                            &stQosIntfKey,
    	                            pstDiffServData
    	                            NBB_CCXT);

    	                        if (ret != SUCCESS)
    	                        {
    	                            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
    	                            NBB_TRC_DETAIL((NBB_FORMAT
    	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan));

    	                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败\n", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);

    	                            OS_SPRINTF(ucMessage,
    	                                "***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d 删除 SVlan=%d CVlan=%d 的Diff-Serv配置失败\n", 
    	                                    	ulPortIndexKey, ret, pstDiffServData->svlan, pstDiffServData->cvlan);
    	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

    	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
    	                                    "CALL_FUNC_ERROR", ucMessage));
    	                        }
                                 else
                                    {
                                        //pstdiffservcb = AVLL_FIND(pstLogicalPort->diff_serv_tree, pstDiffServData);
                                        //diffservcfgkey.svlan_id = pstDiffServData->svlan;
                                        //diffservcfgkey.cvlan_id = pstDiffServData->cvlan;
                                        //pstdiffservcb = 
                                            //AVLL_FIND(pstLogicalPort->diff_serv_tree, &diffservcfgkey);
                                        pstdiffservcb = 
                                            AVLL_FIND(pstLogicalPort->diff_serv_tree, pstDiffServData);  
                                    
                                        //如果不存在，无法删除
                                        if (NULL == pstdiffservcb)
                                        {
                                            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                                            NBB_TRC_DETAIL((NBB_FORMAT 
                                            "  ***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的Diff-Serv配置不存在，无法删除",
                                            ulPortIndexKey,
                                            pstDiffServData->svlan,
                                            pstDiffServData->cvlan));

                                            OS_PRINTF(
                                            "***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的Diff-Serv配置不存在，无法删除\n",
                                            ulPortIndexKey,
                                            pstDiffServData->svlan,
                                            pstDiffServData->cvlan);

                                            OS_SPRINTF(ucMessage,
                                            "***ERROR***:PORT_INDEX=%ld SVLAN_ID=%d CVLAN_ID=%d 的Diff-Serv配置不存在，无法删除\n",
                                            ulPortIndexKey,
                                            pstDiffServData->svlan,
                                            pstDiffServData->cvlan);
                                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                            ulPortIndexKey, ret,
                                            "CALL_FUNC_ERROR", ucMessage));
                                        }
                                        else
                                        {
                                            AVLL_DELETE(pstLogicalPort->diff_serv_tree, 
                                                pstdiffservcb->spm_diff_serv_node);

                                            //释放端口逻辑配置节点的内存空间
                                            spm_free_diff_serv_cb(pstdiffservcb NBB_CCXT);

                                            pstLogicalPort->diff_serv_num--;									
                                        }
                                    }                               
                            }

		                    
#if 0
	                        if (ret == SUCCESS)
	                        {
	                            /* 保存数据 */
	                            if (pstLogicalPort->diff_serv_cfg_cb == NULL)
	                            {
	                                pstLogicalPort->diff_serv_cfg_cb = (ATG_DCI_LOG_PORT_DIFF_SERV_DATA *)NBB_MM_ALLOC(
	                                    sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA),
	                                    NBB_NORETRY_ACT,
	                                    MEM_SPM_LOG_PORT_DIFF_SERV_CB);
	                            }

	                            OS_MEMCPY(pstLogicalPort->diff_serv_cfg_cb, pstDiffServData,
	                                sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
	                        }
	                        else
	                        {
	                            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                            NBB_TRC_DETAIL((NBB_FORMAT
	                                    "  ***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() error", ulPortIndexKey));

	                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d\n",
	                                ulPortIndexKey,
	                                ret);

	                            OS_SPRINTF(ucMessage,
	                                "***ERROR***:PORT_INDEX=%ld spm_ds_add_logic_intf_node() ret=%d\n",
	                                ulPortIndexKey,
	                                ret);
	                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                    "CALL_FUNC_ERROR", ucMessage));
	                        }
#endif
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，删除Diff-Serv失败",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，删除Diff-Serv失败\n", ulPortIndexKey);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld 的L3逻辑配置未下，删除Diff-Serv失败\n",
	                            ulPortIndexKey);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
	                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
	                    }
                    }
                }
            }
            else if (ulOperDiffServ == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 上话复杂流分类QOS策略配置L3 ******************************/
            if (ulOperInclassifyQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucInclassifyQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->inclassify_qos_data);

                /* 首地址为NULL，异常 */
                if (pucInclassifyQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucInclassifyQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  上话复杂流分类QOS策略配置L3 ADD\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->inclassify_qos_num; i++)
	                {
                        pstInclassifyQosData = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)
                            (pucInclassifyQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  上话复杂流分类QOS策略配置L3 ADD\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                    }

	                    NBB_TRC_DETAIL((NBB_FORMAT "  复杂流分类QOS策略模板索引 = %d",
	                            pstInclassifyQosData->qos_policy_index));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;

						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstInclassifyQosData->svlan, 
	                    	pstInclassifyQosData->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;

	                    ret = spm_add_logic_flow_classify_node(ulPortIndexKey,
	                        pstInclassifyQosData,
	                        &stSubPort
	                        NBB_CCXT);

						if (ret != SUCCESS)
						{
	                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#if 0
	                    if (ret == SUCCESS)
	                    {
	                        /* 保存数据 */
	                        if (pstLogicalPort->inclassify_qos_cfg_cb == NULL)
	                        {
	                            pstLogicalPort->inclassify_qos_cfg_cb = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)NBB_MM_ALLOC(
	                                sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS),
	                                NBB_NORETRY_ACT,
	                                MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);
	                        }

	                        OS_MEMCPY(pstLogicalPort->inclassify_qos_cfg_cb, pstInclassifyQosData,
	                            sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() error",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#endif
	                }
                }
            }
            else if (ulOperInclassifyQos == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucInclassifyQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->inclassify_qos_data);

                /* 首地址为NULL，异常 */
                if (pucInclassifyQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucInclassifyQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  上话复杂流分类QOS策略配置L3 DEL\n");
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->inclassify_qos_num; i++)
	                {
                        pstInclassifyQosData = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)
                            (pucInclassifyQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  上话复杂流分类QOS策略配置L3 DEL\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                    }

	                    NBB_TRC_DETAIL((NBB_FORMAT "  复杂流分类QOS策略模板索引 = %d",
	                            pstInclassifyQosData->qos_policy_index));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;

						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstInclassifyQosData->svlan, 
	                    	pstInclassifyQosData->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;

	                    pstInclassifyQosData->qos_policy_index = 0;
	                    
	                    ret = spm_add_logic_flow_classify_node(ulPortIndexKey,
	                        pstInclassifyQosData,
	                        &stSubPort
	                        NBB_CCXT);

						if (ret != SUCCESS)
						{
	                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话复杂流分类QOS策略配置失败\n",
	                                ulPortIndexKey, ret, pstInclassifyQosData->svlan, pstInclassifyQosData->cvlan);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#if 0
	                    if (ret == SUCCESS)
	                    {
	                        /* 保存数据 */
	                        if (pstLogicalPort->inclassify_qos_cfg_cb == NULL)
	                        {
	                            pstLogicalPort->inclassify_qos_cfg_cb = (ATG_DCI_LOG_PORT_INCLASSIFY_QOS *)NBB_MM_ALLOC(
	                                sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS),
	                                NBB_NORETRY_ACT,
	                                MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);
	                        }

	                        OS_MEMCPY(pstLogicalPort->inclassify_qos_cfg_cb, pstInclassifyQosData,
	                            sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() error",
	                                ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_flow_classify_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#endif
	                }
                }
            }
            else if (ulOperInclassifyQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 上话用户QOS策略配置L3 ******************************/
            if (ulOperUpUserQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucUpUserQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->up_user_qos_data);

                /* 首地址为NULL，异常 */
                if (pucUpUserQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucUpUserQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  15）上话用户QOS策略配置L3 ADD\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                
                    for (i = 0; i < pstSetLogicalPort->up_user_qos_num; i++)
	                {
                        pstUpUserQosData = (ATG_DCI_LOG_UP_USER_QOS_POLICY *)
                            (pucUpUserQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  15）上话用户QOS策略配置L3 ADD\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                    }

	                    NBB_TRC_DETAIL((NBB_FORMAT "  用户QOS策略模板索引 = %d", pstUpUserQosData->qos_policy_index));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;

						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstUpUserQosData->svlan, 
	                    	pstUpUserQosData->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;
		                    
	                    ret = spm_add_logic_up_usr_qos_node(ulPortIndexKey,
	                        &stSubPort,
	                        pstUpUserQosData
	                        NBB_CCXT);

	                    if (ret != SUCCESS)
	                    {
	                        pstSetLogicalPort->up_user_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败\n", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 增加 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败\n", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#if 0
	                    if (ret == SUCCESS)
	                    {
	                        /* 保存数据 */
	                        if (pstLogicalPort->up_user_qos_cfg_cb == NULL)
	                        {
	                            pstLogicalPort->up_user_qos_cfg_cb =
	                                (ATG_DCI_LOG_UP_USER_QOS_POLICY *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY),
	                                NBB_NORETRY_ACT,
	                                MEM_SPM_LOG_PORT_UP_USER_QOS_CB);
	                        }

	                        OS_MEMCPY(pstLogicalPort->up_user_qos_cfg_cb, pstUpUserQosData,
	                            sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY));
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->up_user_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() error", ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#endif
	                }

                }
            }
            else if (ulOperUpUserQos == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucUpUserQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->up_user_qos_data);

                /* 首地址为NULL，异常 */
                if (pucUpUserQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucUpUserQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    OS_SPRINTF(ucMessage, "  15）上话用户QOS策略配置L3 DEL\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                
                    for (i = 0; i < pstSetLogicalPort->up_user_qos_num; i++)
	                {
                        pstUpUserQosData = (ATG_DCI_LOG_UP_USER_QOS_POLICY *)
                            (pucUpUserQosDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY)))
                            * i);

	                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
	                    {
	                        printf("  15）上话用户QOS策略配置L3 DEL\n");

	                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
	                    }

	                    NBB_TRC_DETAIL((NBB_FORMAT "  用户QOS策略模板索引 = %d", pstUpUserQosData->qos_policy_index));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;

						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstUpUserQosData->svlan, 
	                    	pstUpUserQosData->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;

	                    pstUpUserQosData->qos_policy_index = 0;
	                    
	                    ret = spm_add_logic_up_usr_qos_node(ulPortIndexKey,
	                        &stSubPort,
	                        pstUpUserQosData
	                        NBB_CCXT);

	                    if (ret != SUCCESS)
	                    {
	                        pstSetLogicalPort->up_user_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败\n", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d 删除 SVlan=%d CVlan=%d 的上话用户QOS策略配置失败\n", 
	                                	ulPortIndexKey, ret, pstUpUserQosData->svlan, pstUpUserQosData->cvlan);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#if 0
	                    if (ret == SUCCESS)
	                    {
	                        /* 保存数据 */
	                        if (pstLogicalPort->up_user_qos_cfg_cb == NULL)
	                        {
	                            pstLogicalPort->up_user_qos_cfg_cb =
	                                (ATG_DCI_LOG_UP_USER_QOS_POLICY *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY),
	                                NBB_NORETRY_ACT,
	                                MEM_SPM_LOG_PORT_UP_USER_QOS_CB);
	                        }

	                        OS_MEMCPY(pstLogicalPort->up_user_qos_cfg_cb, pstUpUserQosData,
	                            sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY));
	                    }
	                    else
	                    {
	                        pstSetLogicalPort->up_user_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
	                        NBB_TRC_DETAIL((NBB_FORMAT
	                                "  ***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() error", ulPortIndexKey));

	                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);

	                        OS_SPRINTF(ucMessage,
	                            "***ERROR***:PORT_INDEX=%ld spm_add_logic_up_usr_qos_node() ret=%d\n",
	                            ulPortIndexKey,
	                            ret);
	                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

	                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
	                                "CALL_FUNC_ERROR", ucMessage));
	                    }
#endif
	                }

                }
            }
            else if (ulOperUpUserQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 上话用户组QOS策略配置(无效) ******************************/
            if (ulOperUpGroupQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucUpGroupQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->upuser_group_qos_data);

                /* 首地址为NULL，异常 */
                if (pucUpGroupQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucUpGroupQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstUpGroupQosData = (ATG_DCI_LOG_UP_USER_GROUP_QOS *)pucUpGroupQosDataStart;

                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  16）上话用户组QOS策略配置(无效)\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }

                    //NBB_TRC_DETAIL((NBB_FORMAT "用户QOS策略模板索引 = %d", pstUpUserQosData->qos_policy_index));

                    /* 保存数据 */
                    if (pstLogicalPort->up_user_group_qos_cfg_cb == NULL)
                    {
                        pstLogicalPort->up_user_group_qos_cfg_cb = (ATG_DCI_LOG_UP_USER_GROUP_QOS *)NBB_MM_ALLOC(
                            sizeof(ATG_DCI_LOG_UP_USER_GROUP_QOS),
                            NBB_NORETRY_ACT,
                            MEM_SPM_LOG_PORT_UP_GROUP_QOS_CB);
                    }

                    OS_MEMCPY(pstLogicalPort->up_user_group_qos_cfg_cb, pstUpGroupQosData,
                        sizeof(ATG_DCI_LOG_UP_USER_GROUP_QOS));
                }
            }
            else if (ulOperUpGroupQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperUpGroupQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 下话用户组QOS配置L3 ******************************/
            if (ulOperDownGroupQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDownGroupQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->down_user_group_qos_data);

                /* 首地址为NULL，异常 */
                if (pucDownGroupQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDownGroupQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstDownGroupQosData = (ATG_DCI_LOG_DOWN_USER_GROUP_QOS *)pucDownGroupQosDataStart;
                    OS_SPRINTF(ucMessage, "  18）下话用户组QOS配置L3\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  18）下话用户组QOS配置L3\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }

                    NBB_TRC_DETAIL((NBB_FORMAT "  用户组索引  = %d", pstDownGroupQosData->user_group_index));

                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                    ret = spm_hqos_add_group_node(stPortInfo.slot_id,
                        stPortInfo.port_id,
                        pstDownGroupQosData->user_group_index,
                        ulPortIndexKey
                        NBB_CCXT);

                    if (ret == SUCCESS)
                    {
                        /* 保存数据 */
                        if (pstLogicalPort->down_user_group_qos_cfg_cb == NULL)
                        {
                            pstLogicalPort->down_user_group_qos_cfg_cb =
                                (ATG_DCI_LOG_DOWN_USER_GROUP_QOS *)NBB_MM_ALLOC(sizeof(ATG_DCI_LOG_DOWN_USER_GROUP_QOS),
                                NBB_NORETRY_ACT,
                                MEM_SPM_LOG_PORT_DOWN_GROUP_QOS_CB);
                        }

                        OS_MEMCPY(pstLogicalPort->down_user_group_qos_cfg_cb, pstDownGroupQosData,
                            sizeof(ATG_DCI_LOG_DOWN_USER_GROUP_QOS));
                    }
                    else
                    {
                        pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() error",
                                ulPortIndexKey));

                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n", ulPortIndexKey, ret);

                        OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                            ulPortIndexKey,
                            ret);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                "CALL_FUNC_ERROR", ucMessage));
                    }
                }
            }
            else if (ulOperDownGroupQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperDownGroupQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 下话用户队列QOS策略配置L3 ******************************/
            if (ulOperDownUserQos == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucDownUserQosDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->down_user_queue_qos_data);

                /* 首地址为NULL，异常 */
                if (pucDownUserQosDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucDownUserQosDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    pstDownUserQosData = (ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *)pucDownUserQosDataStart;
                    OS_SPRINTF(ucMessage, "  17）下话用户队列QOS策略配置L3\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  17）下话用户队列QOS策略配置L3\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }

                    NBB_TRC_DETAIL((NBB_FORMAT "  NodeIndex  = %d", pstDownUserQosData->node_index));
                    NBB_TRC_DETAIL((NBB_FORMAT "  用户QOS策略模板索引  = %d",
                            pstDownUserQosData->user_qos_policy_index));
                    NBB_TRC_DETAIL((NBB_FORMAT "  优先级队列QOS策略模板索引  = %d",
                            pstDownUserQosData->prio_queue_qos_policy_index));

                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

                    stSubPort.unit = stPortInfo.unit_id;
                    stSubPort.port = stPortInfo.port_id;
                    stSubPort.slot = stPortInfo.slot_id;
                    stSubPort.ivlan = stPortInfo.cvlan;
                    stSubPort.ovlan = stPortInfo.svlan;
                    stSubPort.posid = 0;

                    if (pstLogicalPort->down_user_group_qos_cfg_cb != NULL)
                    {
                        ret = spm_hqos_add_usr_node(&stSubPort,
                            ulPortIndexKey,
                            pstLogicalPort->down_user_group_qos_cfg_cb->user_group_index,
                            pstDownUserQosData
                            NBB_CCXT);
                            
                        if (ret == SUCCESS)
                        {
                            /* 保存数据 */
                            if (pstLogicalPort->down_user_qos_cfg_cb == NULL)
                            {
                                pstLogicalPort->down_user_qos_cfg_cb = (ATG_DCI_LOG_DOWN_USER_QUEUE_QOS *)NBB_MM_ALLOC(
                                    sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS),
                                    NBB_NORETRY_ACT,
                                    MEM_SPM_LOG_PORT_DOWN_USER_QOS_CB);
                            }

                            OS_MEMCPY(pstLogicalPort->down_user_qos_cfg_cb, pstDownUserQosData,
                                sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS));
                        }
                        else
                        {
                            pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() error",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                                ulPortIndexKey,
                                ret);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld spm_hqos_add_group_node() ret=%d\n",
                                ulPortIndexKey,
                                ret);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }
                    }
                    else
                    {
                        pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                        NBB_TRC_DETAIL((NBB_FORMAT
                                "  ***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败",
                                ulPortIndexKey));

                        OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败\n",
                            ulPortIndexKey);

                        OS_SPRINTF(ucMessage,
                            "***ERROR***:PORT_INDEX=%ld 的下话用户组QOS配置未下，建立下话用户队列QOS策略失败\n",
                            ulPortIndexKey);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey,
                                BASIC_CONFIG_NOT_EXIST, "BASIC_CONFIG_NOT_EXIST", ucMessage));
                    }
                }
            }
            else if (ulOperDownUserQos == ATG_DCI_OPER_DEL)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 1, "lx", SHARED.spm_index));
            }
            else if (ulOperDownUserQos == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** 包过滤器traffic_filterL3 ******************************/
            if (ulOperTrafficFilter == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                pucTrafficFilterDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->traffic_filter_data);

                /* 首地址为NULL，异常 */
                if (pucTrafficFilterDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucTrafficFilterDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  20）包过滤器traffic_filter L3 ADD\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }
                    
                    OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L3 ADD\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->traffic_filter_num; i++)
                    {
                        pstTrafficFilter = (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)
                            (pucTrafficFilterDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER)))
                            * i);

                        NBB_TRC_DETAIL((NBB_FORMAT "  acl_id  = %d", pstTrafficFilter->acl_id));
                        NBB_TRC_DETAIL((NBB_FORMAT "  方向direction  = %d", pstTrafficFilter->direction));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;
                    
						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstTrafficFilter->svlan, 
	                    	pstTrafficFilter->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;
	                    
                        ret = spm_add_logic_acl_node(ulPortIndexKey,
                            pstTrafficFilter,
                            &stSubPort
                            NBB_CCXT);

                        if (ret != SUCCESS)
                        {
                            pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }
#if 0
                        if (ret == SUCCESS)
                        {
                            iCfgPos = spm_check_if_acl_exist(ulPortIndexKey, pstTrafficFilter);

                            if (iCfgPos <= 0)
                            {
                                for (j = 0; j < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; j++)
                                {
                                    /* 保存数据 */
                                    if (pstLogicalPort->traffic_filter_cfg_cb[j] == NULL)
                                    {
                                        pstLogicalPort->traffic_filter_cfg_cb[j] =
                                            (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)NBB_MM_ALLOC(sizeof(
                                                ATG_DCI_LOG_PORT_TRAFFIC_FILTER),
                                            NBB_NORETRY_ACT,
                                            MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);

	                                    OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[j], pstTrafficFilter,
	                                        sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));

	                                    pstLogicalPort->traffic_filter_num++;

	                                    break;
                                    }
                                }
                            }
                            else
                            {
                                OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[iCfgPos - 1],
                                    pstTrafficFilter,
                                    sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                            }
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }

#endif
                    }
                }
            }
            else if (ulOperTrafficFilter == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                pucTrafficFilterDataStart = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->traffic_filter_data);

                /* 首地址为NULL，异常 */
                if (pucTrafficFilterDataStart == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  pucTrafficFilterDataStart is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {

                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  20）包过滤器traffic_filter L3 DEL\n");

                        //spm_dbg_print_logical_port_flow_diff_serv_cfg(pstFlowDiffServData);
                    }
                    
                    OS_SPRINTF(ucMessage, "  20）包过滤器traffic_filter L3 DEL\n"); 
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                    
                    for (i = 0; i < pstSetLogicalPort->traffic_filter_num; i++)
                    {
                        pstTrafficFilter = (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)
                            (pucTrafficFilterDataStart + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER)))
                            * i);

                        NBB_TRC_DETAIL((NBB_FORMAT "  acl_id  = %d", pstTrafficFilter->acl_id));
                        NBB_TRC_DETAIL((NBB_FORMAT "  方向direction  = %d", pstTrafficFilter->direction));

	                    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
	                    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

	                    stSubPort.unit = stPortInfo.unit_id;
	                    stSubPort.port = stPortInfo.port_id;
	                    stSubPort.slot = stPortInfo.slot_id;
	                    stSubPort.ivlan = stPortInfo.cvlan;
	                    stSubPort.ovlan = stPortInfo.svlan;
                    
						ulVlanPosId = 0;
	                    spm_get_poscnt_from_intf(ulPortIndexKey, 
	                    	pstTrafficFilter->svlan, 
	                    	pstTrafficFilter->cvlan, 
	                    	&ulVlanPosId NBB_CCXT);
	                    	
	                    stSubPort.posid = ulVlanPosId;

	                    pstTrafficFilter->acl_id = 0;
	                    
                        ret = spm_add_logic_acl_node(ulPortIndexKey,
                            pstTrafficFilter,
                            &stSubPort
                            NBB_CCXT);

						if (ret != SUCCESS)
						{
                            pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld, spm_add_logic_acl_node() ret=%d 增加 SVlan=%d CVlan=%d 的包过滤器traffic_filter配置失败!\n",
                                    ulPortIndexKey,
                                    ret,
                                    pstTrafficFilter->svlan, 
                                    pstTrafficFilter->cvlan);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }
#if 0
                        if (ret == SUCCESS)
                        {
                            iCfgPos = spm_check_if_acl_exist(ulPortIndexKey, pstTrafficFilter);

                            if (iCfgPos <= 0)
                            {
                                for (j = 0; j < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; j++)
                                {
                                    /* 保存数据 */
                                    if (pstLogicalPort->traffic_filter_cfg_cb[j] == NULL)
                                    {
                                        pstLogicalPort->traffic_filter_cfg_cb[j] =
                                            (ATG_DCI_LOG_PORT_TRAFFIC_FILTER *)NBB_MM_ALLOC(sizeof(
                                                ATG_DCI_LOG_PORT_TRAFFIC_FILTER),
                                            NBB_NORETRY_ACT,
                                            MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);

	                                    OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[j], pstTrafficFilter,
	                                        sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));

	                                    pstLogicalPort->traffic_filter_num++;

	                                    break;
                                    }
                                }
                            }
                            else
                            {
                                OS_MEMCPY(pstLogicalPort->traffic_filter_cfg_cb[iCfgPos - 1],
                                    pstTrafficFilter,
                                    sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
                            }
                        }
                        else
                        {
                            pstSetLogicalPort->traffic_filter_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;

                            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败",
                                    ulPortIndexKey));

                            OS_PRINTF("***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n", ulPortIndexKey);

                            OS_SPRINTF(ucMessage,
                                "***ERROR***:PORT_INDEX=%ld 的包过滤器traffic_filter增加失败\n",
                                ulPortIndexKey);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, ret,
                                    "CALL_FUNC_ERROR", ucMessage));
                        }

#endif
                    }
                }
            }
            else if (ulOperTrafficFilter == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }

            /****************************** VRRP虚拟MAC和IPv6地址配置L3 ******************************/
            if (ul_oper_vipv6_vmac == ATG_DCI_OPER_ADD)
            {

                /* 计算第一个entry的地址。*/
                puc_vipv6_vmac_data_start = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->vrrp_ipv6_mac_l3_data);

                /* 首地址为NULL，异常 */
                if (puc_vipv6_vmac_data_start == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  puc_vipv6_vmac_data_start is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  25) 虚拟MAC和IPv6地址配置L3 (vrrp_ipv6_mac_l3_num = %d) ADD\n",
                            pstSetLogicalPort->vrrp_ipv6_mac_l3_num);
                    }

                    OS_SPRINTF(ucMessage,
                        "  25) 虚拟MAC和IPv6地址配置L3 (vrrp_ipv6_mac_l3_num = %d) ADD\n",
                        pstSetLogicalPort->vrrp_ipv6_mac_l3_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    //找到vlan属性第一个配置
                    pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);

                    //没有vlan属性，无法绑定，打印错误，直接退出
                    if(NULL == pstTerminalIfCb)
                    {
                        //此次下发的所有VRRP配置均返回失败
                        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP ipv6!\n",
			                        		ulPortIndexKey));

                        OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP ipv6!\n",
			                        		ulPortIndexKey);

                        OS_SPRINTF(ucMessage,	"  ***ERROR***:PORT_INDEX=%ld 的 逻辑端口VLAN属性不存在，无法设置VRRP ipv6!\n",
			                            	ulPortIndexKey);
                        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, BASIC_CONFIG_NOT_EXIST,
			                                "BASIC_CONFIG_NOT_EXIST", ucMessage));		
                    }
                    else
                    {
                        //记录首个VLAN属性配置的地址
                        tempterminalifcb_v6 = pstTerminalIfCb;
						
                        //初始化Vrrp的公共成员
                        stvrrp_v6.l3_port_id = pstLogicalPort->port_id;/*lint !e613 */
			   stvrrp_v6.slot = pstLogicalPort->slot_id;/*lint !e613 */

                        //VE口只需置VE标志，非VE口需要填vlan信息
                        if(ATG_DCI_VE_PORT == pstLogicalPort->basic_cfg_cb->port_sub_type)/*lint !e613 */
                        {
                            stvrrp_v6.ve_flag = 1;
                        }

                        //该逻辑接口下的所有INTF都要使能VRRP
#ifdef SPU                        
                        while (pstTerminalIfCb)
                        {

                            ret = fhdrv_psn_l3_set_intf_vrrp(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id, 1);

                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                           {                              
                                ret += fhdrv_psn_l3_set_intf_vrrp(1, pstTerminalIfCb->intf2_pos_id, 1);
                           }

                            if (SUCCESS != ret)
                            {
                                   for (i = 0; i < pstSetLogicalPort->vrrp_ipv6_mac_l3_num; i++)
                                   {
                                   	pstSetLogicalPort->vrrp_ipv6_mac_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;
                                   }                          	

                            	NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:vrrp fhdrv_psn_l3_set_intf_vrrp()"
                                    " ret=%d PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                        		ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id));

                            	OS_PRINTF("  ***ERROR***:vrrp fhdrv_psn_l3_set_intf_vrrp() ret=%d "
                                    "PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                        		ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id);

                            	OS_SPRINTF(ucMessage,
    	                            "  ***ERROR***:vrrp fhdrv_psn_l3_set_intf_vrrp() ret=%d "
    	                            "PORT_INDEX=%ld 的intf_pos_id=%d使能失败\n",
	                            	ret, ulPortIndexKey, pstTerminalIfCb->intf_pos_id);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", 
                                    ulPortIndexKey, CALL_C3_FUNC_ERROR,
	                                "CALL_C3_FUNC_ERROR", ucMessage));

                                   break;
                            }
							
                            pstTerminalIfCb = AVLL_NEXT(pstLogicalPort->terminal_if_tree, 
                                pstTerminalIfCb->spm_terminal_if_node);
                        }
#endif

                        for (i = 0; i < pstSetLogicalPort->vrrp_ipv6_mac_l3_num; i++)
                        {
                            pst_vipv6_vmac_data[i] = (ATG_DCI_LOG_VRRP_IPV6_MAC_L3 *)
                                (puc_vipv6_vmac_data_start + 
                                (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_VRRP_IPV6_MAC_L3))) * i);

                            /* 配置处理，todo */
                            if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                            {
                                spm_dbg_print_logical_port_vipv6_vmac_cfg(pst_vipv6_vmac_data[i]);
                            }

                            //spm_dbg_record_logical_port_vip_vmac_cfg(pst_vipv6_vmac_data[i]);

                            NBB_TRC_DETAIL((NBB_FORMAT "  MAC地址 = %0x-%0x-%0x-%0x-%0x-%0x", 
                                pst_vipv6_vmac_data[i]->mac_address[0],
                                pst_vipv6_vmac_data[i]->mac_address[1],
                                pst_vipv6_vmac_data[i]->mac_address[2],
                                pst_vipv6_vmac_data[i]->mac_address[3],
                                pst_vipv6_vmac_data[i]->mac_address[4],
                                pst_vipv6_vmac_data[i]->mac_address[5]));
                            
                            //NBB_TRC_DETAIL((NBB_FORMAT "  IPv4地址 = %s", 
                                //spm_set_ulong_to_ipv4(pst_vipv6_vmac_data[i]->ip_addr NBB_CCXT)));
                            NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", 
                                pst_vipv6_vmac_data[i]->sub_net_mask_len));

                            v6cfgpos = spm_check_if_vipv6_vmac_exist(ulPortIndexKey, 
                                pst_vipv6_vmac_data[i] NBB_CCXT);

                            //根据本vrrp属性刷新驱动的vrrp配置
                            stvrrp_v6.dmac[0] = pst_vipv6_vmac_data[i]->mac_address[0];
                            stvrrp_v6.dmac[1] = pst_vipv6_vmac_data[i]->mac_address[1];
                            stvrrp_v6.dmac[2] = pst_vipv6_vmac_data[i]->mac_address[2];
                            stvrrp_v6.dmac[3] = pst_vipv6_vmac_data[i]->mac_address[3];
                            stvrrp_v6.dmac[4] = pst_vipv6_vmac_data[i]->mac_address[4];
                            stvrrp_v6.dmac[5] = pst_vipv6_vmac_data[i]->mac_address[5];


			       //首个VLAN属性配置的地址
	                     pstTerminalIfCb =  tempterminalifcb_v6 ;
#ifdef SRC				   
			       terminalifnum_v6 = pstLogicalPort->terminal_if_num;
#endif	

#ifdef SPU	      
                            terminalifnum_v6 = 0;	

                        //该逻辑接口下的所有INTF都要使能VRRP
                        while (pstTerminalIfCb)
                        {

				stvrrp_v6.ovlan_id = pstTerminalIfCb->terminal_if_cfg.svlan_id;
				stvrrp_v6.ivlan_id = pstTerminalIfCb->terminal_if_cfg.cvlan_id;

				//外层VLAN
				if(0  !=  stvrrp_v6.ovlan_id)/*lint !e613 */
				{
					stvrrp_v6.ovlan_id_mask = 0xfff;
				}
				else
				{
					stvrrp_v6.ovlan_id_mask = 0;
				}

				//内外层VLAN
				if(0  !=  stvrrp_v6.ivlan_id)/*lint !e613 */
				{
					stvrrp_v6.ivlan_id_mask = 0xfff;
				}
				else
				{
				 	stvrrp_v6.ivlan_id_mask = 0;
				}						

		              ret = SUCCESS;
			       ret = fhdrv_psn_acl_set_vrrp(pstLogicalPort->unit_id, &stvrrp_v6);

                            vrrp6_pos_id_temp = stvrrp_v6.posid;
                            stvrrp_v6.posid = 0;
                   
                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                            {
                                
                                ret += fhdrv_psn_acl_set_vrrp(1, &stvrrp_v6);
                            }   
                            
				if(SUCCESS != ret)
                            {
                            	pstSetLogicalPort->vrrp_ipv6_mac_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                            	NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:vrrp fhdrv_psn_acl_set_vrrp()"
                                            " ret=%d PORT_INDEX=%ld绑定VRRP失败\n",
    		                        		ret, ulPortIndexKey));

                            	OS_PRINTF("  ***ERROR***:vrrp fhdrv_psn_acl_set_vrrp() ret=%d PORT_INDEX=%ld绑定VRRP失败\n",
    		                        		ret, ulPortIndexKey);

                            	OS_SPRINTF(ucMessage,
    		                            "  ***ERROR***:vrrp fhdrv_psn_acl_set_vrrp() ret=%d PORT_INDEX=%ld 绑定VRRP失败\n",
    		                            	ret, ulPortIndexKey);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, CALL_C3_FUNC_ERROR,
    		                                "CALL_C3_FUNC_ERROR", ucMessage));

                                break;
                            }
				
				terminalifnum_v6++;
							
                            pstTerminalIfCb = AVLL_NEXT(pstLogicalPort->terminal_if_tree, 
                                pstTerminalIfCb->spm_terminal_if_node);
                        }
 #endif                           


                            if(terminalifnum_v6 == pstLogicalPort->terminal_if_num)
                            {
                                if (v6cfgpos <= 0)
                                {
                                    for (j = 0; j < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; j++)
                                    {
                                        //如果为NULL，说明这个位置为无数据，可以存储数据
                                        if (pstLogicalPort->vipv6_vmac_cfg_cb[j] == NULL)/*lint !e613 */
                                        {
                                            pstLogicalPort->vipv6_vmac_cfg_cb[j] =
                                                (ATG_DCI_LOG_VRRP_IPV6_MAC_L3 *)NBB_MM_ALLOC(sizeof(
                                                    ATG_DCI_LOG_VRRP_IPV6_MAC_L3),
                                                NBB_NORETRY_ACT,
                                                MEM_SPM_LOG_PORT_VIPV6_VMAC_CB);/*lint !e613 */

                                            if(pstLogicalPort->vipv6_vmac_cfg_cb[j] == NULL)/*lint !e613 */
                                            {
                                                OS_PRINTF("	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                            	
                                    			OS_SPRINTF(ucMessage, "	***ERROR***:(%s:%d)malloc failed!\n", __FILE__,__LINE__);
                                    			BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                                                goto EXIT_LABEL;
                                    			
                                            }
                                            OS_MEMCPY(pstLogicalPort->vipv6_vmac_cfg_cb[j], pst_vipv6_vmac_data[i],
                                                sizeof(ATG_DCI_LOG_VRRP_IPV6_MAC_L3));/*lint !e613 */
                                            pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[j] =    /*lint !e613 */
                                                vrrp6_pos_id_temp;
                                            pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid2[j] =    /*lint !e613 */
                                                stvrrp_v6.posid;                                                
                                            pstLogicalPort->vipv6_vmac_num++;/*lint !e613 */
        
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    OS_MEMCPY(pstLogicalPort->vipv6_vmac_cfg_cb[v6cfgpos - 1], pst_vipv6_vmac_data[i],
                                        sizeof(ATG_DCI_LOG_VRRP_IPV6_MAC_L3));/*lint !e613 */
                                }
                            }
                        }
                    }


                }
            }
            else if (ul_oper_vipv6_vmac == ATG_DCI_OPER_DEL)
            {

                /* 计算第一个entry的地址。*/
                puc_vipv6_vmac_data_start = (NBB_BYTE *)NTL_OFFLEN_GET_POINTER(pstSetLogicalPort,
                    &pstSetLogicalPort->vrrp_ipv6_mac_l3_data);

                /* 首地址为NULL，异常 */
                if (puc_vipv6_vmac_data_start == NULL)
                {
                    NBB_TRC_FLOW((NBB_FORMAT "  puc_vipv6_vmac_data_start is NULL."));
                    NBB_EXCEPTION((PCT_SPM | 7, 0, "lx", SHARED.spm_index));
                }
                else
                {
                    if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                    {
                        printf("  25) 虚拟MAC和IPv6地址配置L3 (vrrp_ipv6_mac_l3_num = %d) DEL\n",
                            pstSetLogicalPort->vrrp_ipv6_mac_l3_num);
                    }

                    OS_SPRINTF(ucMessage,
                        "  25) 虚拟MAC和IPv6地址配置L3 (vrrp_ipv6_mac_l3_num = %d) DEL\n",
                        pstSetLogicalPort->vrrp_ipv6_mac_l3_num);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                    for (i = 0; i < pstSetLogicalPort->vrrp_ipv6_mac_l3_num; i++)
                    {
                        pst_vipv6_vmac_data[i] = (ATG_DCI_LOG_VRRP_IPV6_MAC_L3 *)
                            (puc_vipv6_vmac_data_start + (NBB_ALIGN_OFFSET(sizeof(ATG_DCI_LOG_VRRP_IPV6_MAC_L3))) * i);

                        /* 配置处理，todo */
                        if (logical_port_cfg_print_setting == SPM_PRINT_CFG)
                        {
                            spm_dbg_print_logical_port_vipv6_vmac_cfg(pst_vipv6_vmac_data[i]);
                        }

                        //spm_dbg_record_logical_port_vip_vmac_cfg(pst_vipv6_vmac_data[i]);

                        NBB_TRC_DETAIL((NBB_FORMAT "  MAC地址 = %0x-%0x-%0x-%0x-%0x-%0x", 
                            pst_vipv6_vmac_data[i]->mac_address[0],
                                pst_vipv6_vmac_data[i]->mac_address[1],
                                pst_vipv6_vmac_data[i]->mac_address[2],
                                pst_vipv6_vmac_data[i]->mac_address[3],
                                pst_vipv6_vmac_data[i]->mac_address[4],
                                pst_vipv6_vmac_data[i]->mac_address[5]));
                        
                        //NBB_TRC_DETAIL((NBB_FORMAT "  IPv4地址 = %s", 
                            //spm_set_ulong_to_ipv4(pst_vipv6_vmac_data[i]->ip_addr NBB_CCXT)));
                        NBB_TRC_DETAIL((NBB_FORMAT "  子网掩码长度 = %d", 
                            pst_vipv6_vmac_data[i]->sub_net_mask_len));

                        v6cfgpos = spm_check_if_vipv6_vmac_exist(ulPortIndexKey, pst_vipv6_vmac_data[i] NBB_CCXT);
                        if(v6cfgpos < 0)//不存在，无法删除
                        {
                            pstSetLogicalPort->vrrp_ipv6_mac_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                            OS_PRINTF("  ***ERROR***:PORT_INDEX=%ld 的逻辑端口不存在VRRP v6-MAC地址 "
                                            "=%2x-%2x-%2x-%2x-%2x-%2x,无法删除! \n",
			                        		ulPortIndexKey, pst_vipv6_vmac_data[i]->mac_address[0],
			                        		pst_vipv6_vmac_data[i]->mac_address[1],
			                        		pst_vipv6_vmac_data[i]->mac_address[2],
			                        		pst_vipv6_vmac_data[i]->mac_address[3],
			                        		pst_vipv6_vmac_data[i]->mac_address[4],
			                        		pst_vipv6_vmac_data[i]->mac_address[5]);

                            OS_SPRINTF(ucMessage,
			                            "  ***ERROR***:PORT_INDEX=%ld 的逻辑端口不存在VRRP v6-MAC地址 "
    			                            "=%2x-%2x-%2x-%2x-%2x-%2x,无法删除! \n",
			                            	ulPortIndexKey, pst_vipv6_vmac_data[i]->mac_address[0],
			                        		pst_vipv6_vmac_data[i]->mac_address[1],
			                        		pst_vipv6_vmac_data[i]->mac_address[2],
			                        		pst_vipv6_vmac_data[i]->mac_address[3],
			                        		pst_vipv6_vmac_data[i]->mac_address[4],
			                        		pst_vipv6_vmac_data[i]->mac_address[5]);
                            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
                        }
                        else
                        {
#ifdef SPU                        
                            stvrrp_v6.posid = 
                            pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[v6cfgpos - 1];/*lint !e613 */
                            ret = SUCCESS;

                            ret = fhdrv_psn_acl_del_vrrp(pstLogicalPort->unit_id, &stvrrp_v6);

                            if((2 == SHARED.c3_num) && 
                                ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                           {
                                stvrrp_v6.posid = 
                                    pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid2[v6cfgpos - 1];
                                if(0 != stvrrp_v6.posid)
                                {
                                    ret += fhdrv_psn_acl_del_vrrp(1, &stvrrp_v6);
                                }                                
                           } 

                            if(SUCCESS != ret)
                            {
                                pstSetLogicalPort->vrrp_ipv6_mac_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;

                            	NBB_TRC_DETAIL((NBB_FORMAT 
                                    "  ***ERROR***:vrrp fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld"
                                            " 的vrrp_pos_id=%d删除失败\n",
			                        		ret, ulPortIndexKey, 
			                        		pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[v6cfgpos - 1]));

                            	OS_PRINTF(
						"  ***ERROR***:vrrp fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld 的vrrp_pos_id=%d删除失败\n",
			                        		ret, ulPortIndexKey, 
			                        		pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[v6cfgpos - 1]);

                            	OS_SPRINTF(ucMessage,
			                            "  ***ERROR***:vrrp fhdrv_psn_acl_del_vrrp() ret=%d PORT_INDEX=%ld 的vrrp_pos_id=%d删除失败\n",
			                            	ret, ulPortIndexKey, 
			                            	pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[v6cfgpos - 1]);
                            	BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

                            	NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, CALL_C3_FUNC_ERROR,
			                                "CALL_C3_FUNC_ERROR", ucMessage));
                                continue;
                            }
#endif

                            for (j = 0; j < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; j++)
                            {
                                //如果不为NULL，说明这个位置有数据，可以删除数据
                                if ((pstLogicalPort->vipv6_vmac_cfg_cb[j] != NULL)
                                    && (!(spm_ipv6_mac_key_compare(pst_vipv6_vmac_data[i], 
                                    pstLogicalPort->vipv6_vmac_cfg_cb[j] NBB_CCXT))))/*lint !e613 */
                                {
                                    NBB_MM_FREE(pstLogicalPort->vipv6_vmac_cfg_cb[j], /*lint !e613 */
                                        MEM_SPM_LOG_PORT_VIPV6_VMAC_CB);
                                    pstLogicalPort->vipv6_vmac_cfg_cb[j] = NULL;/*lint !e613 */
    
                                    pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid[j] = 0;/*lint !e613 */
                                    
                                    if((2 == SHARED.c3_num) && 
                                        ((pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_LAG) 
                                        || (pstLogicalPort->basic_cfg_cb->port_sub_type == ATG_DCI_VE_PORT)))
                                   {
                                        pstLogicalPort->logic_port_info_cb.vrrp_ipv6_posid2[j] = 0;/*lint !e613 */
                                   } 
                                    
                                    pstLogicalPort->vipv6_vmac_num--;/*lint !e613 */
    
                                    break;
                                }
                            }
                        }

                    }
                }
            }
            else if (ul_oper_vipv6_vmac == ATG_DCI_OPER_UPDATE)
            {
                NBB_EXCEPTION((PCT_SPM | 7, 2, "lx", SHARED.spm_index));
            }
            
        }

        //端口属性既不是L2也不是L3
        else
        {
            NBB_TRC_FLOW((NBB_FORMAT "  ***WARNNING***:端口(%ld)属性既不是L2也不是L3(或基本配置不存在)", ulPortIndexKey));
            OS_PRINTF("***WARNNING***:端口(%ld)属性既不是L2也不是L3(或基本配置不存在)\n", ulPortIndexKey);

            OS_SPRINTF(ucMessage,
                "***WARNNING***:端口(%ld)属性既不是L2也不是L3(或基本配置不存在)\n", 
                ulPortIndexKey);
            BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

            NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, BASIC_CONFIG_NOT_EXIST,
                    "BASIC_CONFIG_NOT_EXIST", ucMessage));
                                    
            pstSetLogicalPort->log_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->log_l2_return_code = ATG_DCI_RC_UNSUCCESSFUL;

            for (i = 0; i < ATG_DCI_LOG_PORT_FLOW_NUM; i++)
            {
                pstSetLogicalPort->flow_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            }

            for (i = 0; i < ATG_DCI_LOG_PORT_IPV4_NUM; i++)
            {
                pstSetLogicalPort->ipv4_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            }

            for (i = 0; i < ATG_DCI_LOG_PORT_IPV6_NUM; i++)
            {
                pstSetLogicalPort->ipv6_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            }

            for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV4_NUM; i++)
            {
                pstSetLogicalPort->mc_ipv4_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            }

            pstSetLogicalPort->diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;

            //for (i = 0; i < ATG_DCI_LOG_FLOW_DIFF_SERV_NUM; i++)
            //{
            //    pstSetLogicalPort->flow_diff_serv_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            //}
            pstSetLogicalPort->flow_diff_serv_return_code = ATG_DCI_RC_UNSUCCESSFUL;

            pstSetLogicalPort->inclassify_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->up_user_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->upuser_group_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->down_user_queue_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->down_user_group_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->log_l3_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            pstSetLogicalPort->flow_upuser_qos_return_code = ATG_DCI_RC_UNSUCCESSFUL;
            

            //for (i = 0; i < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; i++)
            //{
            //    pstSetLogicalPort->traffic_filter_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            //}
            pstSetLogicalPort->traffic_filter_return_code = ATG_DCI_RC_UNSUCCESSFUL;

            for (i = 0; i < ATG_DCI_LOG_VLAN_SINGLE_NUM; i++)
            {
                pstSetLogicalPort->vlan_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            }

			for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV6_NUM; i++)
            {
                pstSetLogicalPort->mc_ipv6_return_code[i] = ATG_DCI_RC_UNSUCCESSFUL;
            }

        }

        #if 1

        /******************************** 关联接口组 *********************************/

        /*应用场景:下关联接口组配置时，逻辑接口未下发，当时只保存，在下逻辑接口配置时反刷*/
         
            for (pstAssociateIfCb = (SPM_ASSOCIATE_IF_CB*) AVLL_FIRST(SHARED.associate_if_tree);
                  pstAssociateIfCb != NULL;
                  pstAssociateIfCb = (SPM_ASSOCIATE_IF_CB*) AVLL_NEXT(SHARED.associate_if_tree,
                  pstAssociateIfCb->spm_associate_if_node))
           {
                for(i = 0;i < ATG_DCI_ASSOCIATE_IF_TRIGGER_NUM;i++)
                {
                   if((pstAssociateIfCb->trigger_if_cfg_cb[i] != NULL)&&(pstAssociateIfCb->trigger_if_cfg_cb[i]->interface_index == ulPortIndexKey) 
                      && (pstAssociateIfCb->trigger_if_exist[i] == UNEXIST))
                   {
                        trigger_if_flag = 1;
                        ucExistPos_trigger = i;
                        ulAssociateIfKey = pstAssociateIfCb->key;
                        break;
                   }
                    
                }
             
                if(trigger_if_flag != 1)
                {
                   for(i = 0;i < ATG_DCI_ASSOCIATE_IF_ACT_NUM;i++)
                    {
                       if((pstAssociateIfCb->act_if_cfg_cb[i] != NULL)&& (pstAssociateIfCb->act_if_cfg_cb[i]->interface_index == ulPortIndexKey) 
                         && (pstAssociateIfCb->act_if_exist[i] == UNEXIST))
                       {
                            act_if_flag = 1;
                            ucExistPos_act = i;
                            ulAssociateIfKey = pstAssociateIfCb->key;
                            break;
                       }
                        
                    } 
                }

                if((trigger_if_flag == 1) || (act_if_flag == 1))
                {
                    break;
                }
                                
                             }
            /*该逻辑口的index已经加入到关联接口组中，之前只保存，现在正式加入*/

             if(trigger_if_flag == 1)
             {
                ret = spm_get_portid_from_logical_port_index(ulPortIndexKey,&stPortInfo NBB_CCXT);
                
                if(ret != SUCCESS)
                {
                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***: 逻辑端口index=%ld获取物理口失败， ret=%d",ulPortIndexKey,ret));
    		
					OS_PRINTF("***ERROR***:逻辑端口index=%ld获取物理口失败， ret=%d",ulPortIndexKey,ret);

					OS_SPRINTF(ucMessage, "***ERROR***:逻辑端口index=%ld获取物理口失败， ret=%d",ulPortIndexKey,ret);
					BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

					NBB_EXCEPTION((PCT_SPM | ASSOCIATE_IF_PD, 0, "ld d s s", ulAssociateIfKey, ret, "spm_get_portid_from_logical_port_index", ucMessage));
    				
                }

                /*lag口在lag配置块里加*/
                
			    if((stPortInfo.port_type == ATG_DCI_SUB_INTFACE)||(stPortInfo.port_type == ATG_DCI_INTFACE))
    			{
    				usPortId = stPortInfo.port_id;
    				usSlotId = stPortInfo.slot_id;
    						
    				OS_MEMSET(&IfGoupInfo,0,sizeof(PORT_GROUP_INFO));

    				IfGoupInfo.usSlotPort = (usSlotId << 8)+(usPortId&0xff);
    				IfGoupInfo.cardId = stPortInfo.card_id;
    				IfGoupInfo.lacpState = 1;
    				IfGoupInfo.actionType = 0;
    						
    				ret = SUCCESS;

    				ret = aps_add_monitor_group_port(ulAssociateIfKey,&IfGoupInfo);

                	if(ret == SUCCESS)
                	{	
                	    if(pstAssociateIfCb != NULL)
                        {
                            pstAssociateIfCb->trigger_if_exist[ucExistPos_trigger] = EXIST;
                            pstAssociateIfCb->trigger_if_num++;  
                        }   
                         
                	}
                    
                    if(ret != SUCCESS)
    				{
    				
               			NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***: aps_add_monitor_group_port() ret=%d", ret));

    					OS_PRINTF("***ERROR***:aps_add_monitor_group_port() ret=%d\n", ret);

    					OS_SPRINTF(ucMessage, "***ERROR***:aps_add_monitor_group_port() ret=%d\n", ret);
    					BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
    	
    					NBB_EXCEPTION((PCT_SPM | ASSOCIATE_IF_PD, 0, "ld d s s", ulAssociateIfKey, ret, "aps_add_monitor_group_port", ucMessage));
    			
    				}
							
            }
        }

        
        else if(act_if_flag == 1)
         {
            ret = spm_get_portid_from_logical_port_index(ulPortIndexKey,&stPortInfo NBB_CCXT);

            /*lag口在lag配置块里加*/
            
            if((stPortInfo.port_type == ATG_DCI_SUB_INTFACE)||(stPortInfo.port_type == ATG_DCI_INTFACE))
            {
                usPortId = stPortInfo.port_id;
                usSlotId = stPortInfo.slot_id;
                        
                OS_MEMSET(&IfGoupInfo,0,sizeof(PORT_GROUP_INFO));

                IfGoupInfo.usSlotPort = (usSlotId << 8)+(usPortId&0xff);
                IfGoupInfo.cardId = stPortInfo.card_id;
                IfGoupInfo.lacpState = 1;
                IfGoupInfo.actionType = 1;
                        
                ret = SUCCESS;

                ret = aps_add_monitor_group_port(ulAssociateIfKey,&IfGoupInfo);

                if(ret == SUCCESS)
                {
                    if(pstAssociateIfCb != NULL)
                    {
                        pstAssociateIfCb->act_if_exist[ucExistPos_act] = EXIST;
                        pstAssociateIfCb->act_if_num++;  
                    }
                     
                            
                }
                if(ret != SUCCESS)
                {
                
                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***: aps_add_monitor_group_port() ret=%d", ret));

                    OS_PRINTF("***ERROR***:aps_add_monitor_group_port() ret=%d\n", ret);

                    OS_SPRINTF(ucMessage, "***ERROR***:aps_add_monitor_group_port() ret=%d\n", ret);
                    BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
    
                    NBB_EXCEPTION((PCT_SPM | ASSOCIATE_IF_PD, 0, "ld d s s", ulAssociateIfKey, ret, "aps_add_monitor_group_port", ucMessage));
            
                }
                        
             }
        }
    #endif

    }    
                
        

EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_del_logical_port_cfg
   功能描述  : 删除端口逻辑配置
   输入参数  : SPM_LOGICAL_PORT_CB* pstLogicalPort
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_del_logical_port_cfg(SPM_LOGICAL_PORT_CB *pstLogicalPort NBB_CCXT_T NBB_CXT)
{
    NBB_INT ret = SUCCESS;
    NBB_INT i = 0;
    NBB_BYTE ucPort = 0;
    NBB_BYTE ucSlot = 0;
    NBB_BYTE ucSubBoardNo = 0;
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];
    NBB_ULONG ulPortIndexKey = 0;
    NBB_USHORT usFlowId = 0;
    SPM_PORT_STRUCT stPort;
    SUB_PORT stSubPort;
    SPM_PORT_INFO_CB stPortInfo;
    SPM_QOS_LOGIC_FLOW_KEY stQosFlowKey;
    SPM_QOS_LOGIC_INTF_KEY stQosIntfKey;
    TX_UNI_PORT stTxUniPort;
    SPM_QOS_LOGIC_UNIVP stUniVp;
    ATG_DCI_LOG_DOWN_USER_QUEUE_QOS stDownUserQueueQos;
    ATG_DCI_LOG_UP_USER_QOS_POLICY stUpUserQosPolicy;
    ATG_DCI_LOG_PORT_FLOW_DIFF_SERV stFlowDiffServ;
    ATG_DCI_LOG_PORT_DIFF_SERV_DATA stDiffServ;
    ATG_DCI_LOG_PORT_DS_L2_DATA stDsL2;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;

    NBB_TRC_ENTRY("spm_del_logical_port_cfg");

    if (pstLogicalPort == NULL)
    {
    	goto EXIT_LABEL;
    }

    OS_MEMSET(&stPort, 0, sizeof(SPM_PORT_STRUCT));
    OS_MEMSET(&stSubPort, 0, sizeof(SUB_PORT));
    OS_MEMSET(&stPortInfo, 0, sizeof(SPM_PORT_INFO_CB));
    OS_MEMSET(&stTxUniPort, 0, sizeof(TX_UNI_PORT));
    OS_MEMSET(&stQosFlowKey, 0, sizeof(SPM_QOS_LOGIC_FLOW_KEY));
    OS_MEMSET(&stQosIntfKey, 0, sizeof(SPM_QOS_LOGIC_INTF_KEY));
    OS_MEMSET(&stDownUserQueueQos, 0, sizeof(ATG_DCI_LOG_DOWN_USER_QUEUE_QOS));
    OS_MEMSET(&stUpUserQosPolicy, 0, sizeof(ATG_DCI_LOG_UP_USER_QOS_POLICY));
    OS_MEMSET(&stFlowDiffServ, 0, sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
    OS_MEMSET(&stDiffServ, 0, sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
    OS_MEMSET(&stDsL2, 0, sizeof(ATG_DCI_LOG_PORT_DS_L2_DATA));
    OS_MEMSET(&stUniVp, 0, sizeof(SPM_QOS_LOGIC_UNIVP));

    ulPortIndexKey = pstLogicalPort->port_index_key;

    //如果基本配置不存在，则不需要处理
    if ((pstLogicalPort->basic_cfg_cb == NULL) || (pstLogicalPort->phy_cfg_cb == NULL))
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:端口逻辑配置的基本配置不存在"));

        OS_PRINTF("***ERROR***:要删除的PORT_INDEX=%ld 的端口逻辑配置的基本配置或物理配置并不存在!\n", ulPortIndexKey);

        OS_SPRINTF(ucMessage,
            "***ERROR***:要删除的PORT_INDEX=%ld 的端口逻辑配置的基本配置或物理配置并不存在!\n",
            ulPortIndexKey);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | PHYSICAL_PORT_PD, 0, "ld d s s", ulPortIndexKey, BASIC_CONFIG_NOT_EXIST,
                "BASIC_CONFIG_NOT_EXIST", ucMessage));
                
        goto EXIT_LABEL;
    }

    spm_get_portid_from_logical_port_index(ulPortIndexKey, &stPortInfo NBB_CCXT);

    stQosIntfKey.index = ulPortIndexKey;
    stQosFlowKey.index = ulPortIndexKey;
    stTxUniPort.port = pstLogicalPort->port_id;
    stTxUniPort.sysPort = pstLogicalPort->slot_id;

    stSubPort.unit = stPortInfo.unit_id;
    stSubPort.port = stPortInfo.port_id;
    stSubPort.slot = stPortInfo.slot_id;
    stSubPort.ivlan = stPortInfo.cvlan;
    stSubPort.ovlan = stPortInfo.svlan;
#if 0
    //删除Diff-Serv配置
    if ((pstLogicalPort->diff_serv_cfg_cb != NULL) && (pstLogicalPort->logic_l3_cfg_cb != NULL))
    {
        spm_ds_add_logic_intf_node(&stSubPort,
            &stQosIntfKey,
            &stDiffServ);
    }

    //删除流相关Diff-Serv配置L2
    for (i = 0; i < ATG_DCI_LOG_FLOW_DIFF_SERV_NUM; i++)
    {
        if (pstLogicalPort->flow_diff_serv_cfg_cb[i] != NULL)
        {
            //usFlowId = pstLogicalPort->flow_diff_serv_cfg_cb[i]->flow_id;

            //stQosFlowKey.flow_id = usFlowId;

            spm_ds_add_logic_flow_node(&stSubPort,
                0,
                &stQosFlowKey,
                pstLogicalPort->logic_port_info_cb.flow_info_cb.pos_id);
        }
    }


    //删除上话复杂流分类QOS策略配置
    if (pstLogicalPort->inclassify_qos_cfg_cb != NULL)
    {
        spm_add_logic_flow_classify_node(ulPortIndexKey, 0, &stSubPort);
    }

    //删除上话用户QOS策略配置
    if (pstLogicalPort->up_user_qos_cfg_cb != NULL)
    {
        spm_add_logic_up_usr_qos_node(ulPortIndexKey,
            &stSubPort,
            &stUpUserQosPolicy);
    }
#endif
    //删除下话用户队列QOS策略配置 和 下话用户组QOS配置
    if ((pstLogicalPort->down_user_qos_cfg_cb != NULL) && (pstLogicalPort->down_user_group_qos_cfg_cb != NULL))
    {
        stDownUserQueueQos.node_index = pstLogicalPort->down_user_qos_cfg_cb->node_index;
        stDownUserQueueQos.user_qos_policy_index = 0;
        stDownUserQueueQos.prio_queue_qos_policy_index = 0;

        spm_hqos_add_usr_node(&stSubPort,
            ulPortIndexKey,
            pstLogicalPort->down_user_group_qos_cfg_cb->user_group_index,
            &stDownUserQueueQos
            NBB_CCXT);

        spm_hqos_add_group_node(stPortInfo.slot_id,
            stPortInfo.port_id,
            0,
            ulPortIndexKey
            NBB_CCXT); //nhi暂时获取不到，填0
    }

    //删除Diff-Serv配置L2（VP下话UNI侧）
    if (pstLogicalPort->ds_l2_cfg_cb != NULL)
    {
		stUniVp.vpn_id = pstLogicalPort->logic_port_info_cb.vpn_id;
		stUniVp.vp_type = pstLogicalPort->logic_port_info_cb.vpn_type;
		stUniVp.vp_posid = pstLogicalPort->logic_port_info_cb.vp_idx;

		stDsL2.ds_id = 0;

		spm_ds_add_logic_univp_node(ulPortIndexKey,	&stUniVp, &stDsL2 NBB_CCXT);
    }

	//包过滤器traffic_filter
	spm_qos_clear_logic_acl(ulPortIndexKey NBB_CCXT);

	//上话复杂流分类QOS策略配置
	spm_qos_clear_logic_classify(ulPortIndexKey NBB_CCXT);

	//Diff-Serv配置
	spm_qos_clear_ds_logic_intf(ulPortIndexKey NBB_CCXT);

	//流相关Diff-Serv配置L2
	spm_qos_clear_logic_ds_flow(ulPortIndexKey NBB_CCXT);

	//删除此逻辑接口上的性能绑定
    //almpm_delline_intf(ulPortIndexKey, 1);
	
    //如果是三层接口，删除intf
    if (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L3)
    {
	    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
	         pstTerminalIfCb != NULL;
	         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
	                       pstTerminalIfCb->spm_terminal_if_node))
	    {
	    
#ifdef SPU
			ret = ApiC3DelIntf(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id);
			almpm_delline_intf(ulPortIndexKey, pstTerminalIfCb->intf_pos_id, 1);
#endif

	    }
    }
    else if (pstLogicalPort->basic_cfg_cb->port_route_type == ATG_DCI_L2)   //如果是二层接口
    {
        //L2中没有VP就不可能存在INTF流，所以要删除L2的INTF，必须相应的VP存在
        if (pstLogicalPort->logic_port_info_cb.vp_idx != 0)
        {
		    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
		         pstTerminalIfCb != NULL;
		         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
		                       pstTerminalIfCb->spm_terminal_if_node))
		    {
                //flow不存在
                if (pstTerminalIfCb->intf_pos_id== 0)
                {
                    continue;
                }

#ifdef SPU
				ret = ApiC3DelIntf(pstLogicalPort->unit_id, pstTerminalIfCb->intf_pos_id);
				almpm_delline_intf(ulPortIndexKey, pstTerminalIfCb->intf_pos_id, 1);
#endif

                //删除失败
                if (ret != SUCCESS)
                {
                	//coverity[dead_error_begin]
                    NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:ApiC3DelIntf() L2 ret=%d PORT_INDEX=%ld 删除INTF失败!\n", ret, ulPortIndexKey));
                    OS_PRINTF("***ERROR***:ApiC3DelIntf() L2 ret=%d PORT_INDEX=%ld 删除INTF失败!\n", ret, ulPortIndexKey);

			        OS_SPRINTF(ucMessage, "***ERROR***:ApiC3DelIntf() L2 ret=%d PORT_INDEX=%ld 删除INTF失败!\n", ret, ulPortIndexKey);
			        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);
        
                    NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", pstLogicalPort->port_index_key, ret,
                            "CALL_C3_FUNC_ERROR", "ApiC3DelIntf() L3"));

                    //pstSetLogicalPort->return_code = ATG_DCI_RC_UNSUCCESSFUL;
                    //goto EXIT_LABEL;
                }
            }

        }
        else
        {
            NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:VP不存在! flow不属于任何VP，INTF flow无法删除!"));

            //OS_PRINTF("***ERROR***:VP不存在! flow不属于任何VP，INTF flow无法删除!\n");

            //goto EXIT_LABEL;
        }

    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_check_flow_vlan_conflict
   功能描述  : 检查一个逻辑端口中10条流的vlan是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_FLOW_DATA *pstFlow
   输出参数  : 无
   返 回 值  :  0 ：无重复
             -1 : 有重复
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月7日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_flow_vlan_conflict(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_FLOW_DATA *pstFlow NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_flow_vlan_conflict");

    if (pstFlow == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_flow_vlan_conflict(pstFlow==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_flow_vlan_conflict(pstFlow==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_flow_vlan_conflict(pstFlow==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_flow_vlan_conflict() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_flow_vlan_conflict() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage,
            "***ERROR***:spm_check_flow_vlan_conflict() PORT_INDEX=%ld 逻辑端口不存在 \n",
            ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_FLOW_NUM; i++)
    {
        //if (pstLogicalPort->flow_cfg_cb == NULL)
        //{
        //    continue;
        //}
        //else
        //{
            //如果flow_id相同，那么ovlan可以相同
            //if ((pstLogicalPort->flow_cfg_cb->otci == pstFlow->otci)
            //    && (pstLogicalPort->flow_cfg_cb->flow_id != pstFlow->flow_id))
            //{
            //    ret = ERROR;

            //    goto EXIT_LABEL;
            //}
        //}
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_check_if_ipv4_exist
   功能描述  : 检查一个逻辑端口中ipv4是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_IPV4_DATA *pstIpv4
   输出参数  : 无
   返 回 值  :  0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_if_ipv4_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_IPV4_DATA *pstIpv4 NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_ipv4_exist");

    if (pstIpv4 == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_ipv4_exist(pstIpv4==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_ipv4_exist(pstIpv4==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_ipv4_exist(pstIpv4==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_IPV4_NUM; i++)
    {
        if (pstLogicalPort->ipv4_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果IPV4相等，返回存储位置
            if ((spm_ipv4_key_compare(pstIpv4, pstLogicalPort->ipv4_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_check_if_ipv6_exist
   功能描述  : 检查一个逻辑端口中ipv6是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_IPV6_DATA *pstIpv6
   输出参数  : 无
   返 回 值  :  0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_if_ipv6_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_IPV6_DATA *pstIpv6 NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_ipv6_exist");

    if (pstIpv6 == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_ipv6_exist(pstIpv6==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_ipv6_exist(pstIpv6==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_ipv6_exist(pstIpv6==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_IPV6_NUM; i++)
    {
        if (pstLogicalPort->ipv6_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果IPV4相等，返回存储位置
            if ((spm_ipv6_key_compare(pstIpv6, pstLogicalPort->ipv6_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_check_if_mc_mac_exist
   功能描述  : 检查一个逻辑端口中组播MAC地址是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_MC_MAC_DATA *pstMcMac
   输出参数  : 无
   返 回 值  : 0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_if_mc_ipv4_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_MC_IPV4_DATA *pstMcIpv4 NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_mc_ipv4_exist");

    if (pstMcIpv4 == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_mc_ipv4_exist(pstMcIpv4==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_mc_ipv4_exist(pstMcIpv4==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_mc_ipv4_exist(pstMcIpv4==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_mc_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_mc_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_mc_ipv4_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV4_NUM; i++)
    {
        if (pstLogicalPort->mc_ipv4_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果IPV4相等，返回存储位置
            if ((spm_ipv4_key_compare(pstMcIpv4, pstLogicalPort->mc_ipv4_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_check_if_mc_ipv6_exist
   功能描述  : 检查一个逻辑端口中IPV6组播地址是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_MC_IPV6_DATA *pstMcIpv6
   输出参数  : 无
   返 回 值  : 0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年10月31日
    作    者   : zhangzhm
    修改内容   : 新生成函数

*****************************************************************************/

NBB_INT spm_check_if_mc_ipv6_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_MC_IPV6_DATA *pstMcIpv6 NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_mc_ipv6_exist");

    if (pstMcIpv6 == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_mc_ipv6_exist(pstMcIpv6==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_mc_ipv6_exist(pstMcIpv6==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_mc_ipv6_exist(pstMcIpv6==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_mc_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_mc_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_mc_ipv6_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV6_NUM; i++)
    {
        if (pstLogicalPort->mc_ipv6_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果IPV6相等，返回存储位置
            if ((spm_ipv6_key_compare(pstMcIpv6, pstLogicalPort->mc_ipv6_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}


/*****************************************************************************
   函 数 名  : spm_check_if_vip_vmac_exist
   功能描述  : 检查一个逻辑端口中虚拟MAC和IP地址配置是否有重复
   输入参数  : NBB_ULONG ulPortIndex
             ATG_DCI_LOG_PORT_VIP_VMAC_DATA *pstVipVmac
   输出参数  : 无
   返 回 值  : 0 ：无重复
             n : 有重复，n=数组位置+1
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月14日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_check_if_vip_vmac_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_VIP_VMAC_DATA *pstVipVmac NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_vip_vmac_exist");

    if (pstVipVmac == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_vip_vmac_exist(pstVipVmac==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_vip_vmac_exist(pstVipVmac==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_vip_vmac_exist(pstVipVmac==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_vip_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在",
                ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_vip_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage,
            "***ERROR***:spm_check_if_vip_vmac_exist() PORT_INDEX=%ld 逻辑端口不存在 \n",
            ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        if (pstLogicalPort->vip_vmac_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果虚拟MAC和IP地址配置相等，返回存储位置
            if ((spm_ip_mac_key_compare(pstVipVmac, pstLogicalPort->vip_vmac_cfg_cb[i] NBB_CCXT)) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }

EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

NBB_INT spm_check_if_acl_exist(NBB_ULONG ulPortIndex, ATG_DCI_LOG_PORT_TRAFFIC_FILTER *pstTrafficFilter NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR ucMessage[SPM_MSG_INFO_LEN];

    NBB_INT i = 0;
    NBB_INT ret = SUCCESS;
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;

    NBB_TRC_ENTRY("spm_check_if_vip_vmac_exist");

    if (pstTrafficFilter == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_acl_exist(pstTrafficFilter==NULL)"));

        OS_PRINTF("***ERROR***:spm_check_if_acl_exist(pstTrafficFilter==NULL)\n");

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_acl_exist(pstTrafficFilter==NULL)\n");
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, FUNC_IN_PARAM_IS_NULL,
                "FUNC_IN_PARAM_IS_NULL", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort == NULL)
    {
        NBB_TRC_DETAIL((NBB_FORMAT "  ***ERROR***:spm_check_if_acl_exist() PORT_INDEX=%ld 逻辑端口不存在", ulPortIndex));

        OS_PRINTF("***ERROR***:spm_check_if_acl_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);

        OS_SPRINTF(ucMessage, "***ERROR***:spm_check_if_acl_exist() PORT_INDEX=%ld 逻辑端口不存在 \n", ulPortIndex);
        BMU_SLOG(BMU_INFO, SPM_L2_LOG_DIR, ucMessage);

        NBB_EXCEPTION((PCT_SPM | LOGICAL_PORT_PD, 0, "ld d s s", ulPortIndex, LOGICAL_PORT_NOT_EXIST,
                "LOGICAL_PORT_NOT_EXIST", ucMessage));
                
        ret = ERROR;

        goto EXIT_LABEL;
    }
#if 0
    for (i = 0; i < ATG_DCI_LOG_VLAN_SINGLE_NUM; i++)
    {
        if (pstLogicalPort->traffic_filter_cfg_cb[i] == NULL)
        {
            continue;
        }
        else
        {
            //如果虚拟MAC和IP地址配置相等，返回存储位置
            if ((spm_acl_key_compare(pstTrafficFilter, pstLogicalPort->traffic_filter_cfg_cb[i])) == 0)
            {
                ret = i + 1;
                goto EXIT_LABEL;
            }
        }
    }
#endif
EXIT_LABEL: NBB_TRC_EXIT();

    return ret;
}

/*****************************************************************************
   函 数 名  : spm_logical_port_drvinfo_clear
   功能描述  : 清空驱动返回的值
   输入参数  : NBB_ULONG ulPortIndex
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年5月3日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_logical_port_drvinfo_clear(NBB_ULONG ulPortIndex NBB_CCXT_T NBB_CXT)
{
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort != NULL)
    {
        OS_MEMSET(&pstLogicalPort->logic_port_info_cb, 0, sizeof(SPM_LOGICAL_PORT_INFO_CB));

	    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
	         pstTerminalIfCb != NULL;
	         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_NEXT(pstLogicalPort->terminal_if_tree,
	                       pstTerminalIfCb->spm_terminal_if_node))
	    {
	    	pstTerminalIfCb->intf_pos_id = 0;
	    }
    }

    return SUCCESS;
}

/*****************************************************************************
   函 数 名  : spm_uni_logical_port_drvinfo_clear
   功能描述  : 清空驱动返回的值
   输入参数  : NBB_ULONG ulPortIndex
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2014年1月17日
    作    者   : zhangzm
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_uni_logical_port_drvinfo_clear(NBB_ULONG ulPortIndex NBB_CCXT_T NBB_CXT)
{
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort != NULL)
    {
        OS_MEMSET(&pstLogicalPort->logic_port_info_cb, 0, sizeof(SPM_LOGICAL_PORT_INFO_CB));
    }

    return SUCCESS;
}

/*****************************************************************************
   函 数 名  : spm_uni_logical_port_drvinfo_nhi_clear
   功能描述  : 清空驱动返回的值，保留nhi
   输入参数  : NBB_ULONG ulPortIndex
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2014年8月21日
    作    者   : zhangsen
    修改内容   : 新生成函数

*****************************************************************************/
NBB_INT spm_uni_logical_port_drvinfo_nhi_clear(NBB_ULONG ulPortIndex NBB_CCXT_T NBB_CXT)
{
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    NBB_UINT iNhiId = 0;

    pstLogicalPort = AVLL_FIND(SHARED.logical_port_tree, &ulPortIndex);

    if (pstLogicalPort != NULL)
    {
    	iNhiId = pstLogicalPort->logic_port_info_cb.next_hop_id;
        OS_MEMSET(&pstLogicalPort->logic_port_info_cb, 0, sizeof(SPM_LOGICAL_PORT_INFO_CB));
        pstLogicalPort->logic_port_info_cb.next_hop_id = iNhiId;
    }

    return SUCCESS;
}

/*****************************************************************************
   函 数 名  : spm_init_logical_port_tree
   功能描述  : 初始化存储端口逻辑配置的树
   输入参数  : 无
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年11月24日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_init_logical_port_tree(NBB_CXT_T NBB_CXT)
{
    NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_init_logical_port_tree");

    avll_key_offset = NBB_OFFSETOF(SPM_LOGICAL_PORT_CB, port_index_key);/*lint !e413 */

    //NBB_TRC_DETAIL((NBB_FORMAT "Key offset set to %ld", avll_key_offset));

    AVLL_INIT_TREE(SHARED.logical_port_tree, compare_ulong,
        (NBB_USHORT)avll_key_offset,
        (NBB_USHORT)NBB_OFFSETOF(SPM_LOGICAL_PORT_CB, spm_logical_port_node));

    NBB_TRC_EXIT();
}

/*****************************************************************************
   函 数 名  : spm_alloc_logical_port_cb
   功能描述  : 端口逻辑配置结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_LOGICAL_PORT_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月12日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_LOGICAL_PORT_CB *spm_alloc_logical_port_cb(NBB_CXT_T NBB_CXT)
{
    SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
    NBB_BUF_SIZE avll_key_offset;
    NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_alloc_physical_port_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstLogicalPort = (SPM_LOGICAL_PORT_CB *)NBB_MM_ALLOC(sizeof(SPM_LOGICAL_PORT_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_LOGICAL_PORT_CB);
    if (pstLogicalPort == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstLogicalPort, 0, sizeof(SPM_LOGICAL_PORT_CB));

    /* 初始化指针 */
    pstLogicalPort->basic_cfg_cb = NULL;
    pstLogicalPort->phy_cfg_cb = NULL;
    pstLogicalPort->logic_l3_cfg_cb = NULL;
    pstLogicalPort->logic_l2_cfg_cb = NULL;
    pstLogicalPort->ces_cfg_cb = NULL;
    pstLogicalPort->ve_mac_cfg_cb = NULL;
//    pstLogicalPort->diff_serv_cfg_cb = NULL;
//    pstLogicalPort->inclassify_qos_cfg_cb = NULL;
//    pstLogicalPort->up_user_qos_cfg_cb = NULL;
    pstLogicalPort->up_user_group_qos_cfg_cb = NULL;
    pstLogicalPort->down_user_qos_cfg_cb = NULL;
    pstLogicalPort->down_user_group_qos_cfg_cb = NULL;
    pstLogicalPort->ds_l2_cfg_cb = NULL;

    /***************************************************************************/
    /* 初始化用于存储SPM_FLOW_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
/*    for (i = 0; i < ATG_DCI_LOG_PORT_FLOW_NUM; i++)
    {
        pstLogicalPort->flow_cfg_cb = NULL;
    }
*/
    /***************************************************************************/
    /* 初始化用于存储SPM_FLOW_NODE_CB的AVLL树 Added by xiaoxiang, 2013/4/26*/
    /***************************************************************************/
    //for (i = 0; i < ATG_DCI_LOG_FLOW_DIFF_SERV_NUM; i++)
    //{
    //    pstLogicalPort->flow_diff_serv_cfg_cb[i] = NULL;
    //}

    /***************************************************************************/
    /* 初始化用于存储SPM_IPV4_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV4_NUM; i++)
    {
        pstLogicalPort->ipv4_cfg_cb[i] = NULL;
    }

    /***************************************************************************/
    /* 初始化用于存储SPM_IPV6_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV6_NUM; i++)
    {
        pstLogicalPort->ipv6_cfg_cb[i] = NULL;
    }

    /***************************************************************************/
    /* 初始化用于存储SPM_MC_MAC_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV4_NUM; i++)
    {
        pstLogicalPort->mc_ipv4_cfg_cb[i] = NULL;
    }

	/***************************************************************************/
    /* 初始化IPV6组播配置 Added by zhangzhm, 2013/11/1*/
    /***************************************************************************/
	for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV6_NUM; i++)
    {
        pstLogicalPort->mc_ipv6_cfg_cb[i] = NULL;
    }

    /***************************************************************************/
    /* 初始化用于存储SPM_VIP_VMAC_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        pstLogicalPort->vip_vmac_cfg_cb[i] = NULL;
    }

    /***************************************************************************/
    /* 初始化用于存储SPM_VIP_VMAC_NODE_CB的AVLL树 Added by xiaoxiang, 2012/10/16*/
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        pstLogicalPort->vipv6_vmac_cfg_cb[i] = NULL;
    }

    /***************************************************************************/
    /* 初始化用于存储SPM_VIP_VMAC_NODE_CB的AVLL树 Added by xiaoxiang, 2013/4/26*/
    /***************************************************************************/
    //for (i = 0; i < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; i++)
    //{
    //    pstLogicalPort->traffic_filter_cfg_cb[i] = NULL;
    //}

    /***************************************************************************/
    /* 初始化用于存储SPM_TERMINAL_IF_NODE_CB的AVLL树 Added by xiaoxiang, 2013/9/3*/
    /***************************************************************************/
    //for (i = 0; i < ATG_DCI_LOG_TERMINAL_IF_NUM; i++)
    //{
    //    pstLogicalPort->terminal_if_cfg_cb[i] = NULL;
    //}

    /***************************************************************************/
    /* 初始化用于存储SPM_TERMINAL_IF_CB的AVLL树.                              */
    /***************************************************************************/
    AVLL_INIT_TREE(pstLogicalPort->terminal_if_tree, spm_terminal_if_key_compare,
                   (NBB_USHORT) NBB_OFFSETOF(SPM_TERMINAL_IF_CB, terminal_if_cfg),/*lint !e413 */
                   (NBB_USHORT) NBB_OFFSETOF(SPM_TERMINAL_IF_CB, spm_terminal_if_node));/*lint !e413 */

    /***************************************************************************/
    /* 初始化用于存储SPM_DIFF_SERV_CB的AVLL树.                              */
    /***************************************************************************/
    AVLL_INIT_TREE(pstLogicalPort->diff_serv_tree, spm_diff_serv_key_compare,
                   (NBB_USHORT) NBB_OFFSETOF(SPM_DIFF_SERV_CB, diff_serv_cfg),/*lint !e413 */
                   (NBB_USHORT) NBB_OFFSETOF(SPM_DIFF_SERV_CB, spm_diff_serv_node));/*lint !e413 */

    /***************************************************************************/
    /* 初始化用于存储SPM_FLOW_DIFF_SERV_CB的AVLL树.                              */
    /***************************************************************************/
    AVLL_INIT_TREE(pstLogicalPort->flow_diff_serv_tree, spm_flow_diff_serv_key_compare,
                   (NBB_USHORT) NBB_OFFSETOF(SPM_FLOW_DIFF_SERV_CB, flow_diff_serv_cfg),/*lint !e413 */
                   (NBB_USHORT) NBB_OFFSETOF(SPM_FLOW_DIFF_SERV_CB, spm_flow_diff_serv_node));/*lint !e413 */

    /***************************************************************************/
    /* 初始化用于存储SPM_INCLASSIFY_QOS_CB的AVLL树.                              */
    /***************************************************************************/
    AVLL_INIT_TREE(pstLogicalPort->inclassify_qos_tree, spm_inclassify_qos_key_compare,
                   (NBB_USHORT) NBB_OFFSETOF(SPM_INCLASSIFY_QOS_CB, inclassify_qos_cfg),/*lint !e413 */
                   (NBB_USHORT) NBB_OFFSETOF(SPM_INCLASSIFY_QOS_CB, spm_inclassify_qos_node));/*lint !e413 */

    /***************************************************************************/
    /* 初始化用于存储SPM_TRAFFIC_FILTER_CB的AVLL树.                              */
    /***************************************************************************/
    AVLL_INIT_TREE(pstLogicalPort->traffic_filter_tree, spm_traffic_filter_key_compare,
                   (NBB_USHORT) NBB_OFFSETOF(SPM_TRAFFIC_FILTER_CB, traffic_filter_cfg),/*lint !e413 */
                   (NBB_USHORT) NBB_OFFSETOF(SPM_TRAFFIC_FILTER_CB, spm_traffic_filter_node));/*lint !e413 */    
    
    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstLogicalPort->spm_logical_port_handle = NBB_CREATE_HANDLE(pstLogicalPort,
        HDL_SPM_LOGICAL_PORT_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_LOGICAL_PORT_CB allocated at %p with handle %#lx",
            pstLogicalPort, pstLogicalPort->spm_logical_port_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstLogicalPort->spm_logical_port_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstLogicalPort);
}

/*****************************************************************************
   函 数 名  : spm_free_logical_port_cb
   功能描述  : 释放端口逻辑配置节点的内存空间
   输入参数  : SPM_LOGICAL_PORT_CB *pstLogicalPort
             NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年11月8日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_logical_port_cb(SPM_LOGICAL_PORT_CB *pstLogicalPort NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_INT i = 0;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;
    SPM_DIFF_SERV_CB *pstdiffservcb = NULL;   
    SPM_FLOW_DIFF_SERV_CB *pstflowdiffservcb = NULL; 
    SPM_INCLASSIFY_QOS_CB *pstinclassifyqoscb = NULL; 
    SPM_TRAFFIC_FILTER_CB *psttrafficfiltercb = NULL;     
    AVLL_NODE node = {0};

    NBB_TRC_ENTRY("spm_free_logical_port_cb");
    
    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstLogicalPort, sizeof(SPM_LOGICAL_PORT_CB), MEM_SPM_LOGICAL_PORT_CB);

    if (pstLogicalPort == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    //NBB_TRC_FLOW((NBB_FORMAT "Free SPM_LOGICAL_PORT_CB at %p", pstLogicalPort));
    NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    if (pstLogicalPort->basic_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->basic_cfg_cb, MEM_SPM_LOG_PORT_BASIC_CB);
        pstLogicalPort->basic_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放物理配置数据块。                                                    */
    /***************************************************************************/
    if (pstLogicalPort->phy_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->phy_cfg_cb, MEM_SPM_LOG_PORT_PHY_CB);
        pstLogicalPort->phy_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放逻辑配置L3数据块。                                                  */
    /***************************************************************************/
    if (pstLogicalPort->logic_l3_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->logic_l3_cfg_cb, MEM_SPM_LOG_PORT_L3_CB);
        pstLogicalPort->logic_l3_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放逻辑配置L2数据块。                                                  */
    /***************************************************************************/
    if (pstLogicalPort->logic_l2_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->logic_l2_cfg_cb, MEM_SPM_LOG_PORT_L2_CB);
        pstLogicalPort->logic_l2_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放流相关配置L2数据块。                                                */
    /***************************************************************************/
/*    for (i = 0; i < ATG_DCI_LOG_PORT_FLOW_NUM; i++)
    {
        if (pstLogicalPort->flow_cfg_cb != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->flow_cfg_cb, MEM_SPM_LOG_PORT_FLOW_CB);
            pstLogicalPort->flow_cfg_cb = NULL;
        }
    }
*/
    /***************************************************************************/
    /* 释放逻辑配置CES/CEP数据块。                                             */
    /***************************************************************************/
    if (pstLogicalPort->ces_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->ces_cfg_cb, MEM_SPM_LOG_PORT_CES_CB);
        pstLogicalPort->ces_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放Ipv4地址配置L3数据块。                                             */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV4_NUM; i++)
    {
        if (pstLogicalPort->ipv4_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->ipv4_cfg_cb[i], MEM_SPM_LOG_PORT_IPV4_CB);
            pstLogicalPort->ipv4_cfg_cb[i] = NULL;
        }
    }

    /***************************************************************************/
    /* 释放Ipv6地址配置L3数据块。                                             */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_IPV6_NUM; i++)
    {
        if (pstLogicalPort->ipv6_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->ipv6_cfg_cb[i], MEM_SPM_LOG_PORT_IPV6_CB);
            pstLogicalPort->ipv6_cfg_cb[i] = NULL;
        }
    }

    /***************************************************************************/
    /* 释放组播组地址L3数据块。                                                */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV4_NUM; i++)
    {
        if (pstLogicalPort->mc_ipv4_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->mc_ipv4_cfg_cb[i], MEM_SPM_LOG_PORT_MC_IPV4_CB);
            pstLogicalPort->mc_ipv4_cfg_cb[i] = NULL;
        }
    }

    /***************************************************************************/
    /* 释放虚拟MAC和IP地址配置L3数据块。                                       */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        if (pstLogicalPort->vip_vmac_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->vip_vmac_cfg_cb[i], MEM_SPM_LOG_PORT_VIP_VMAC_CB);
            pstLogicalPort->vip_vmac_cfg_cb[i] = NULL;
        }
    }

    /***************************************************************************/
    /* 释放虚拟MAC和IP地址配置L3数据块。                                       */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_VIP_VMAC_NUM; i++)
    {
        if (pstLogicalPort->vipv6_vmac_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->vipv6_vmac_cfg_cb[i], MEM_SPM_LOG_PORT_VIPV6_VMAC_CB);
            pstLogicalPort->vipv6_vmac_cfg_cb[i] = NULL;
        }
    }   

    /***************************************************************************/
    /* 释放VE MAC地址数据块。                                             */
    /***************************************************************************/
    if (pstLogicalPort->ve_mac_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->ve_mac_cfg_cb, MEM_SPM_LOG_PORT_VE_MAC_CB);
        pstLogicalPort->ve_mac_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放Diff-Serv配置数据块。                                             */
    /***************************************************************************/
    //if (pstLogicalPort->diff_serv_cfg_cb != NULL)
    //{
    //    NBB_MM_FREE(pstLogicalPort->diff_serv_cfg_cb, MEM_SPM_LOG_PORT_DIFF_SERV_CB);
    //    pstLogicalPort->diff_serv_cfg_cb = NULL;
    //}

    /***************************************************************************/
    /* 释放流相关Diff-Serv配置L2数据块。                                                */
    /***************************************************************************/
    //for (i = 0; i < ATG_DCI_LOG_FLOW_DIFF_SERV_NUM; i++)
    //{
    //    if (pstLogicalPort->flow_diff_serv_cfg_cb[i] != NULL)
    //    {
    //        NBB_MM_FREE(pstLogicalPort->flow_diff_serv_cfg_cb[i], MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);
    //        pstLogicalPort->flow_diff_serv_cfg_cb[i] = NULL;
    //    }
    //}
    
    /***************************************************************************/
    /* 释放流相关Diff-Serv配置L2数据块。                                                */
    /***************************************************************************/
    for (pstflowdiffservcb = (SPM_FLOW_DIFF_SERV_CB*) AVLL_FIRST(pstLogicalPort->flow_diff_serv_tree);
         pstflowdiffservcb != NULL;
         pstflowdiffservcb = (SPM_FLOW_DIFF_SERV_CB*) AVLL_FIRST(pstLogicalPort->flow_diff_serv_tree))
    {
    	//NBB_MEMCPY(&node, &(pstTerminalIfCb->spm_terminal_if_node), sizeof(AVLL_NODE));
    	AVLL_DELETE(pstLogicalPort->flow_diff_serv_tree, pstflowdiffservcb->spm_flow_diff_serv_node);
    	spm_free_flow_diff_serv_cb(pstflowdiffservcb NBB_CCXT);
    }
    
    /***************************************************************************/
    /* 释放上话复杂流分类QOS策略配置L3配置数据块。                                         */
    /***************************************************************************/
    //if (pstLogicalPort->inclassify_qos_cfg_cb != NULL)
    //{
    //    NBB_MM_FREE(pstLogicalPort->inclassify_qos_cfg_cb, MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);
    //    pstLogicalPort->inclassify_qos_cfg_cb = NULL;
    //}
    
    /***************************************************************************/
    /* 释放上话复杂流分类QOS策略配置L3配置数据块。                                         */
    /***************************************************************************/
    for (pstinclassifyqoscb = (SPM_INCLASSIFY_QOS_CB*) AVLL_FIRST(pstLogicalPort->inclassify_qos_tree);
         pstinclassifyqoscb != NULL;
         pstinclassifyqoscb = (SPM_INCLASSIFY_QOS_CB*) AVLL_FIRST(pstLogicalPort->inclassify_qos_tree))
    {
    	//NBB_MEMCPY(&node, &(pstTerminalIfCb->spm_terminal_if_node), sizeof(AVLL_NODE));
    	AVLL_DELETE(pstLogicalPort->inclassify_qos_tree, pstinclassifyqoscb->spm_inclassify_qos_node);
    	spm_free_inclassify_qos_cb(pstinclassifyqoscb NBB_CCXT);
    }
    
    /***************************************************************************/
    /* 释放上话用户QOS策略配置L3配置数据块。                                         */
    /***************************************************************************/
    //if (pstLogicalPort->up_user_qos_cfg_cb != NULL)
    //{
    //    NBB_MM_FREE(pstLogicalPort->up_user_qos_cfg_cb, MEM_SPM_LOG_PORT_UP_USER_QOS_CB);
    //    pstLogicalPort->up_user_qos_cfg_cb = NULL;
    //}

    /***************************************************************************/
    /* 释放上话用户组QOS策略配置(无效)配置数据块。                                         */
    /***************************************************************************/
    if (pstLogicalPort->up_user_group_qos_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->up_user_group_qos_cfg_cb, MEM_SPM_LOG_PORT_UP_GROUP_QOS_CB);
        pstLogicalPort->up_user_group_qos_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放下话用户队列QOS策略配置L3配置数据块。                                         */
    /***************************************************************************/
    if (pstLogicalPort->down_user_qos_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->down_user_qos_cfg_cb, MEM_SPM_LOG_PORT_DOWN_USER_QOS_CB);
        pstLogicalPort->down_user_qos_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放下话用户组QOS配置L3配置数据块。                                         */
    /***************************************************************************/
    if (pstLogicalPort->down_user_group_qos_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->down_user_group_qos_cfg_cb, MEM_SPM_LOG_PORT_DOWN_GROUP_QOS_CB);
        pstLogicalPort->down_user_group_qos_cfg_cb = NULL;
    }

    /***************************************************************************/
    /* 释放流相关上话用户QOS策略配置L2配置数据块。                                         */
    /***************************************************************************/
    //if (pstLogicalPort->flow_up_user_qos_cfg_cb != NULL)
    //{
    //    NBB_MM_FREE(pstLogicalPort->flow_up_user_qos_cfg_cb, MEM_SPM_LOG_PORT_FLOW_UP_USER_QOS_CB);
    //    pstLogicalPort->flow_up_user_qos_cfg_cb = NULL;
    //}

    /***************************************************************************/
    /* 释放流包过滤器traffic_filterL3配置数据块。                                         */
    /***************************************************************************/
    for (psttrafficfiltercb = (SPM_TRAFFIC_FILTER_CB*) AVLL_FIRST(pstLogicalPort->traffic_filter_tree);
         psttrafficfiltercb != NULL;
         psttrafficfiltercb = (SPM_TRAFFIC_FILTER_CB*) AVLL_FIRST(pstLogicalPort->traffic_filter_tree))
    {
    	//NBB_MEMCPY(&node, &(pstTerminalIfCb->spm_terminal_if_node), sizeof(AVLL_NODE));
    	AVLL_DELETE(pstLogicalPort->traffic_filter_tree, psttrafficfiltercb->spm_traffic_filter_node);
    	spm_free_traffic_filter_cb(psttrafficfiltercb NBB_CCXT);
    }
    
    /***************************************************************************/
    /* 释放流包过滤器traffic_filterL3配置数据块。                                         */
    /***************************************************************************/
    //for (i = 0; i < ATG_DCI_LOG_TRAFFIC_FILTER_NUM; i++)
    //{
    //    if (pstLogicalPort->traffic_filter_cfg_cb[i] != NULL)
    //    {
    //        NBB_MM_FREE(pstLogicalPort->traffic_filter_cfg_cb[i], MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);
    //        pstLogicalPort->traffic_filter_cfg_cb[i] = NULL;
    //    }
    //}

    /***************************************************************************/
    /* 释放终结子接口配置数据块。                                         */
    /***************************************************************************/
    for (pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree);
         pstTerminalIfCb != NULL;
         pstTerminalIfCb = (SPM_TERMINAL_IF_CB*) AVLL_FIRST(pstLogicalPort->terminal_if_tree))
    {
    	//NBB_MEMCPY(&node, &(pstTerminalIfCb->spm_terminal_if_node), sizeof(AVLL_NODE));
    	AVLL_DELETE(pstLogicalPort->terminal_if_tree, pstTerminalIfCb->spm_terminal_if_node);
    	spm_free_terminal_if_cb(pstTerminalIfCb NBB_CCXT);
    }

    /***************************************************************************/
    /* 释放diff_serv配置数据块。                                         */
    /***************************************************************************/
    for (pstdiffservcb = (SPM_DIFF_SERV_CB*) AVLL_FIRST(pstLogicalPort->diff_serv_tree);
         pstdiffservcb != NULL;
         pstdiffservcb = (SPM_DIFF_SERV_CB*) AVLL_FIRST(pstLogicalPort->diff_serv_tree))
    {
    	//NBB_MEMCPY(&node, &(pstTerminalIfCb->spm_terminal_if_node), sizeof(AVLL_NODE));
    	AVLL_DELETE(pstLogicalPort->diff_serv_tree, pstdiffservcb->spm_diff_serv_node);
    	spm_free_diff_serv_cb(pstdiffservcb NBB_CCXT);
    }

    /***************************************************************************/
    /* 释放IPV6组播组地址L3数据块。                                                */
    /***************************************************************************/
    for (i = 0; i < ATG_DCI_LOG_PORT_MC_IPV6_NUM; i++)
    {
        if (pstLogicalPort->mc_ipv6_cfg_cb[i] != NULL)
        {
            NBB_MM_FREE(pstLogicalPort->mc_ipv6_cfg_cb[i], MEM_SPM_LOG_PORT_MC_IPV6_CB);
            pstLogicalPort->mc_ipv6_cfg_cb[i] = NULL;
        }
    }

    /***************************************************************************/
    /* 释放Diff-Serv配置L2（VP下话UNI侧）配置数据块。                                         */
    /***************************************************************************/
    if (pstLogicalPort->ds_l2_cfg_cb != NULL)
    {
        NBB_MM_FREE(pstLogicalPort->ds_l2_cfg_cb, MEM_SPM_LOG_PORT_DS_L2_CB);
        pstLogicalPort->ds_l2_cfg_cb = NULL;
    }
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    NBB_DESTROY_HANDLE(pstLogicalPort->spm_logical_port_handle, HDL_SPM_LOGICAL_PORT_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(pstLogicalPort, MEM_SPM_LOGICAL_PORT_CB);
    pstLogicalPort = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_alloc_terminal_if_cb
   功能描述  : 端口逻辑配置终结子接口结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_TERMINAL_IF_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年9月5日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_TERMINAL_IF_CB *spm_alloc_terminal_if_cb(NBB_CXT_T NBB_CXT)
{
    NBB_INT i = 0;
    SPM_TERMINAL_IF_CB *pstTerminalIfCb = NULL;

    NBB_TRC_ENTRY("spm_alloc_terminal_if_cb");

    /* 分配一个新的VPWS配置条目。*/
    pstTerminalIfCb = (SPM_TERMINAL_IF_CB *)NBB_MM_ALLOC(sizeof(SPM_TERMINAL_IF_CB), NBB_NORETRY_ACT, MEM_SPM_LOG_PORT_TERMINAL_IF_CB);
    if (pstTerminalIfCb == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:申请pstTerminalIf条目空间为NULL!"));
        goto EXIT_LABEL;
    }

    /* 初始化VPWS配置条目 */
    OS_MEMSET(pstTerminalIfCb, 0, sizeof(SPM_TERMINAL_IF_CB));

    /* 建立用于该VC表配置条目的句柄，作为异步消息交换的相关器。*/
    pstTerminalIfCb->spm_terminal_if_handle = NBB_CREATE_HANDLE(pstTerminalIfCb, HDL_SPM_TERMINAL_IF_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_TERMINAL_IF_CB allocated at %p with handle %#lx",
            pstTerminalIfCb, pstTerminalIfCb->spm_terminal_if_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstTerminalIfCb->spm_terminal_if_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstTerminalIfCb);
}


/*****************************************************************************
   函 数 名  : spm_free_terminal_if_cb
   功能描述  : 释放端口逻辑配置终结子接口节点的内存空间
   输入参数  : (SPM_TERMINAL_IF_CB *pstTerminalIf
             NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年9月5日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_terminal_if_cb(SPM_TERMINAL_IF_CB *pstTerminalIfCb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_free_terminal_if_cb");

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstTerminalIfCb, sizeof(SPM_TERMINAL_IF_CB), MEM_SPM_LOG_PORT_TERMINAL_IF_CB);

    if (pstTerminalIfCb == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    NBB_TRC_FLOW((NBB_FORMAT "Free SPM_TERMINAL_IF_CB at %p", pstTerminalIfCb));
    //NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    OS_MEMSET(&pstTerminalIfCb->terminal_if_cfg, 0, sizeof(ATG_DCI_LOG_PORT_VLAN));
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    NBB_DESTROY_HANDLE(pstTerminalIfCb->spm_terminal_if_handle, HDL_SPM_TERMINAL_IF_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(pstTerminalIfCb, MEM_SPM_LOG_PORT_TERMINAL_IF_CB);
    pstTerminalIfCb = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_alloc_diff_serv_cb
   功能描述  : 端口逻辑配置diff_serv结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年1月22日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
SPM_DIFF_SERV_CB *spm_alloc_diff_serv_cb(NBB_CXT_T NBB_CXT)
{
    //NBB_INT i = 0;
    SPM_DIFF_SERV_CB *pstdiffservcb = NULL;

    NBB_TRC_ENTRY("spm_alloc_diff_serv_cb");

    /* 分配一个新的VPWS配置条目。*/
    pstdiffservcb = (SPM_DIFF_SERV_CB *)NBB_MM_ALLOC(sizeof(SPM_DIFF_SERV_CB), 
        NBB_NORETRY_ACT, MEM_SPM_LOG_PORT_DIFF_SERV_CB);
    
    if (pstdiffservcb == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:申请pstdiffservcb条目空间为NULL!"));
        goto EXIT_LABEL;
    }

    /* 初始化VPWS配置条目 */
    OS_MEMSET(pstdiffservcb, 0, sizeof(SPM_DIFF_SERV_CB));

    /* 建立用于该VC表配置条目的句柄，作为异步消息交换的相关器。*/
    //pstdiffservcb->spm_diff_serv_handle = NBB_CREATE_HANDLE(pstdiffservcb, HDL_SPM_TERMINAL_IF_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    //NBB_TRC_DETAIL((NBB_FORMAT "SPM_DIFF_SERV_CB allocated at %p with handle %#lx",
            //pstdiffservcb, pstdiffservcb->spm_diff_serv_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstdiffservcb->spm_diff_serv_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstdiffservcb);
}

/*****************************************************************************
   函 数 名  : spm_free_diff_serv_cb
  功能描述  : 释放端口逻辑配置diff_serv节点的内存空间
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年1月22日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_diff_serv_cb(SPM_DIFF_SERV_CB *pstdiffservcb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_free_diff_serv_cb");

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstdiffservcb, sizeof(SPM_DIFF_SERV_CB), MEM_SPM_LOG_PORT_DIFF_SERV_CB);

    if (pstdiffservcb == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    NBB_TRC_FLOW((NBB_FORMAT "Free SPM_DIFF_SERV_CB at %p", pstdiffservcb));
    
    //NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    OS_MEMSET(&pstdiffservcb->diff_serv_cfg, 0, sizeof(ATG_DCI_LOG_PORT_DIFF_SERV_DATA));
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    //NBB_DESTROY_HANDLE(pstdiffservcb->spm_diff_serv_handle, HDL_SPM_TERMINAL_IF_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(pstdiffservcb, MEM_SPM_LOG_PORT_DIFF_SERV_CB);
    pstdiffservcb = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_alloc_flow_diff_serv_cb
   功能描述  : 端口逻辑配置diff_serv结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
SPM_FLOW_DIFF_SERV_CB *spm_alloc_flow_diff_serv_cb(NBB_CXT_T NBB_CXT)
{
    //NBB_INT i = 0;
    SPM_FLOW_DIFF_SERV_CB *pstflowdiffservcb = NULL;

    NBB_TRC_ENTRY("spm_alloc_flow_diff_serv_cb");

    /* 分配一个新的VPWS配置条目。*/
    pstflowdiffservcb = (SPM_FLOW_DIFF_SERV_CB *)NBB_MM_ALLOC(sizeof(SPM_FLOW_DIFF_SERV_CB), 
        NBB_NORETRY_ACT, MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);
    
    if (pstflowdiffservcb == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:申请pstflowdiffservcb条目空间为NULL!"));
        goto EXIT_LABEL;
    }

    /* 初始化VPWS配置条目 */
    OS_MEMSET(pstflowdiffservcb, 0, sizeof(SPM_FLOW_DIFF_SERV_CB));

    /* 建立用于该VC表配置条目的句柄，作为异步消息交换的相关器。*/
    //pstdiffservcb->spm_diff_serv_handle = NBB_CREATE_HANDLE(pstdiffservcb, HDL_SPM_TERMINAL_IF_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    //NBB_TRC_DETAIL((NBB_FORMAT "SPM_DIFF_SERV_CB allocated at %p with handle %#lx",
            //pstdiffservcb, pstdiffservcb->spm_diff_serv_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstflowdiffservcb->spm_flow_diff_serv_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstflowdiffservcb);
}

/*****************************************************************************
   函 数 名  : spm_free_flow_diff_serv_cb
  功能描述  : 释放端口逻辑配置diff_serv节点的内存空间
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_flow_diff_serv_cb(SPM_FLOW_DIFF_SERV_CB *pstflowdiffservcb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_free_flow_diff_serv_cb");

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstflowdiffservcb, sizeof(SPM_FLOW_DIFF_SERV_CB), MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);

    if (pstflowdiffservcb == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    NBB_TRC_FLOW((NBB_FORMAT "Free SPM_FLOW_DIFF_SERV_CB at %p", pstflowdiffservcb));
    
    //NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    OS_MEMSET(&pstflowdiffservcb->flow_diff_serv_cfg, 0, sizeof(ATG_DCI_LOG_PORT_FLOW_DIFF_SERV));
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    //NBB_DESTROY_HANDLE(pstdiffservcb->spm_diff_serv_handle, HDL_SPM_TERMINAL_IF_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(pstflowdiffservcb, MEM_SPM_LOG_PORT_FLOW_DIFF_SERV_CB);
    pstflowdiffservcb = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_alloc_inclassify_qos_cb
   功能描述  : 端口逻辑配置diff_serv结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数(SPM_INCLASSIFY_QOS_CB*) AVLL_NEXT(pstLogicalPort->inclassify_qos_tree,

*****************************************************************************/
SPM_INCLASSIFY_QOS_CB *spm_alloc_inclassify_qos_cb(NBB_CXT_T NBB_CXT)
{
    //NBB_INT i = 0;
    SPM_INCLASSIFY_QOS_CB *pstinclassifyqoscb = NULL;

    NBB_TRC_ENTRY("spm_alloc_inclassify_qos_cb");

    /* 分配一个新的VPWS配置条目。*/
    pstinclassifyqoscb = (SPM_INCLASSIFY_QOS_CB *)NBB_MM_ALLOC(sizeof(SPM_INCLASSIFY_QOS_CB), 
        NBB_NORETRY_ACT, MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);
    
    if (pstinclassifyqoscb == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:申请pstinclassifyqoscb条目空间为NULL!"));
        goto EXIT_LABEL;
    }

    /* 初始化VPWS配置条目 */
    OS_MEMSET(pstinclassifyqoscb, 0, sizeof(SPM_INCLASSIFY_QOS_CB));

    /* 建立用于该VC表配置条目的句柄，作为异步消息交换的相关器。*/
    //pstdiffservcb->spm_diff_serv_handle = NBB_CREATE_HANDLE(pstdiffservcb, HDL_SPM_TERMINAL_IF_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    //NBB_TRC_DETAIL((NBB_FORMAT "SPM_DIFF_SERV_CB allocated at %p with handle %#lx",
            //pstdiffservcb, pstdiffservcb->spm_diff_serv_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstinclassifyqoscb->spm_inclassify_qos_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstinclassifyqoscb);
}

/*****************************************************************************
   函 数 名  : spm_free_inclassify_qos_cb
  功能描述  : 释放端口逻辑配置diff_serv节点的内存空间
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_inclassify_qos_cb(SPM_INCLASSIFY_QOS_CB *pstinclassifyqoscb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_free_inclassify_qos_cb");

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(pstinclassifyqoscb, sizeof(SPM_INCLASSIFY_QOS_CB), MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);

    if (pstinclassifyqoscb == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    NBB_TRC_FLOW((NBB_FORMAT "Free SPM_INCLASSIFY_QOS_CB at %p", pstinclassifyqoscb));
    
    //NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    OS_MEMSET(&pstinclassifyqoscb->inclassify_qos_cfg, 0, sizeof(ATG_DCI_LOG_PORT_INCLASSIFY_QOS));
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    //NBB_DESTROY_HANDLE(pstdiffservcb->spm_diff_serv_handle, HDL_SPM_TERMINAL_IF_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(pstinclassifyqoscb, MEM_SPM_LOG_PORT_INCLASSIFY_QOS_CB);
    pstinclassifyqoscb = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_alloc_traffic_filter_cb
   功能描述  : 端口逻辑配置diff_serv结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :(SPM_TRAFFIC_FILTER_CB*) AVLL_NEXT(pstLogicalPort->traffic_filter_tree,

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
SPM_TRAFFIC_FILTER_CB *spm_alloc_traffic_filter_cb(NBB_CXT_T NBB_CXT)
{
    //NBB_INT i = 0;
    SPM_TRAFFIC_FILTER_CB *psttrafficfiltercb = NULL;

    NBB_TRC_ENTRY("spm_alloc_traffic_filter_cb");

    /* 分配一个新的VPWS配置条目。*/
    psttrafficfiltercb = (SPM_TRAFFIC_FILTER_CB *)NBB_MM_ALLOC(sizeof(SPM_TRAFFIC_FILTER_CB), 
        NBB_NORETRY_ACT, MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);
    
    if (psttrafficfiltercb == NULL)
    {
        NBB_TRC_FLOW((NBB_FORMAT "  ***ERROR***:申请psttrafficfiltercb条目空间为NULL!"));
        goto EXIT_LABEL;
    }

    /* 初始化VPWS配置条目 */
    OS_MEMSET(psttrafficfiltercb, 0, sizeof(SPM_TRAFFIC_FILTER_CB));

    /* 建立用于该VC表配置条目的句柄，作为异步消息交换的相关器。*/
    //pstdiffservcb->spm_diff_serv_handle = NBB_CREATE_HANDLE(pstdiffservcb, HDL_SPM_TERMINAL_IF_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    //NBB_TRC_DETAIL((NBB_FORMAT "SPM_DIFF_SERV_CB allocated at %p with handle %#lx",
            //pstdiffservcb, pstdiffservcb->spm_diff_serv_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(psttrafficfiltercb->spm_traffic_filter_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(psttrafficfiltercb);
}

/*****************************************************************************
   函 数 名  : spm_free_traffic_filter_cb
  功能描述  : 释放端口逻辑配置diff_serv节点的内存空间
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_DIFF_SERV_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2015年3月9日
    作    者   : fansongbo
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_traffic_filter_cb(SPM_TRAFFIC_FILTER_CB *psttrafficfiltercb NBB_CCXT_T NBB_CXT)
{
    /***************************************************************************/
    /* Local Variables                                                         */
    /***************************************************************************/
    //NBB_INT i = 0;

    NBB_TRC_ENTRY("spm_free_traffic_filter_cb");

    /***************************************************************************/
    /* 检查控制块的正确性。                                                    */
    /***************************************************************************/
    NBB_ASSERT_MEMORY(psttrafficfiltercb, sizeof(SPM_TRAFFIC_FILTER_CB), MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);

    if (psttrafficfiltercb == NULL)
    {
    	goto EXIT_LABEL;
    }
    
    /***************************************************************************/
    /* 删除单盘信息控制块。                                                    */
    /***************************************************************************/

    NBB_TRC_FLOW((NBB_FORMAT "Free SPM_TRAFFIC_FILTER_CB at %p", psttrafficfiltercb));
    
    //NBB_TRC_FLOW((NBB_FORMAT "Free PORT_INDEX %d", pstLogicalPort->port_index_key));

    /***************************************************************************/
    /* 释放基本配置数据块。                                                    */
    /***************************************************************************/
    OS_MEMSET(&psttrafficfiltercb->traffic_filter_cfg, 0, sizeof(ATG_DCI_LOG_PORT_TRAFFIC_FILTER));
    
    /***************************************************************************/
    /* 删除控制块的句柄。                                                      */
    /***************************************************************************/
    //NBB_DESTROY_HANDLE(pstdiffservcb->spm_diff_serv_handle, HDL_SPM_TERMINAL_IF_CB);

    /***************************************************************************/
    /* 现在释放端口逻辑配置控制块。                                            */
    /***************************************************************************/
    NBB_MM_FREE(psttrafficfiltercb, MEM_SPM_LOG_PORT_TRAFFIC_FILTER_CB);
    psttrafficfiltercb = NULL;

    EXIT_LABEL: NBB_TRC_EXIT();

    return;
}

/*****************************************************************************
   函 数 名  : spm_free_all_logical_port
   功能描述  : 释放端口逻辑配置所有的内存空间
   输入参数  : 无
   输出参数  : 无
   返 回 值  :
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2013年11月6日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
NBB_VOID spm_free_all_logical_port(NBB_CXT_T NBB_CXT)
{
	SPM_LOGICAL_PORT_CB *pstLogicalPort = NULL;
	
    for (pstLogicalPort = (SPM_LOGICAL_PORT_CB*) AVLL_FIRST(SHARED.logical_port_tree);
         pstLogicalPort != NULL;
         pstLogicalPort = (SPM_LOGICAL_PORT_CB*) AVLL_FIRST(SHARED.logical_port_tree))
    {
    	AVLL_DELETE(SHARED.logical_port_tree, pstLogicalPort->spm_logical_port_node);
    	spm_free_logical_port_cb(pstLogicalPort NBB_CCXT);
    }
}


#if 0

/*****************************************************************************
   函 数 名  : spm_alloc_flow_node_cb
   功能描述  : 流节点结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_FLOW_NODE_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月16日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_FLOW_NODE_CB *spm_alloc_flow_node_cb()
{
    SPM_FLOW_NODE_CB *pstFlowNode = NULL;

    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_alloc_flow_node_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstFlowNode = (SPM_FLOW_NODE_CB *)NBB_MM_ALLOC(sizeof(SPM_FLOW_NODE_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_FLOW_NODE_CB);
    if (pstFlowNode == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstFlowNode, 0, sizeof(SPM_FLOW_NODE_CB));

    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstFlowNode->flow_tree_handle = NBB_CREATE_HANDLE(pstFlowNode, HDL_SPM_FLOW_NODE_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_FLOW_NODE_CB allocated at %p with handle %#lx",
            pstFlowNode, pstFlowNode->flow_tree_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstFlowNode->flow_tree_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstFlowNode);
}

/*****************************************************************************
   函 数 名  : spm_alloc_ipv4_node_cb
   功能描述  : ipv4结点结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_IPV4_NODE_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月16日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_IPV4_NODE_CB *spm_alloc_ipv4_node_cb()
{
    SPM_IPV4_NODE_CB *pstIpv4Node = NULL;

    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_alloc_ipv4_node_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstIpv4Node = (SPM_IPV4_NODE_CB *)NBB_MM_ALLOC(sizeof(SPM_IPV4_NODE_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_IPV4_NODE_CB);
    if (pstIpv4Node == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstIpv4Node, 0, sizeof(SPM_IPV4_NODE_CB));

    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstIpv4Node->ipv4_tree_handle = NBB_CREATE_HANDLE(pstIpv4Node, HDL_SPM_IPV4_NODE_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_IPV4_NODE_CB allocated at %p with handle %#lx",
            pstIpv4Node, pstIpv4Node->ipv4_tree_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstIpv4Node->ipv4_tree_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstIpv4Node);
}

/*****************************************************************************
   函 数 名  : spm_alloc_ipv6_node_cb
   功能描述  : ipv6节点结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_IPV6_NODE_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月16日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_IPV6_NODE_CB *spm_alloc_ipv6_node_cb()
{
    SPM_IPV6_NODE_CB *pstIpv6Node = NULL;

    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_alloc_ipv6_node_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstIpv6Node = (SPM_IPV6_NODE_CB *)NBB_MM_ALLOC(sizeof(SPM_IPV6_NODE_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_IPV6_NODE_CB);
    if (pstIpv6Node == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstIpv6Node, 0, sizeof(SPM_IPV6_NODE_CB));

    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstIpv6Node->ipv6_tree_handle = NBB_CREATE_HANDLE(pstIpv6Node, HDL_SPM_IPV6_NODE_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_IPV6_NODE_CB allocated at %p with handle %#lx",
            pstIpv6Node, pstIpv6Node->ipv6_tree_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstIpv6Node->ipv6_tree_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstIpv6Node);
}

/*****************************************************************************
   函 数 名  : spm_alloc_mc_mac_node_cb
   功能描述  : 组播地址节点结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_MC_MAC_NODE_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月16日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_MC_MAC_NODE_CB *spm_alloc_mc_mac_node_cb()
{
    SPM_MC_MAC_NODE_CB *pstMcMacNode = NULL;

    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_alloc_mc_mac_node_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstMcMacNode = (SPM_MC_MAC_NODE_CB *)NBB_MM_ALLOC(sizeof(SPM_MC_MAC_NODE_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_MC_MAC_NODE_CB);
    if (pstMcMacNode == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstMcMacNode, 0, sizeof(SPM_MC_MAC_NODE_CB));

    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstMcMacNode->mc_mac_tree_handle = NBB_CREATE_HANDLE(pstMcMacNode, HDL_SPM_MC_IPV4_NODE_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_MC_MAC_NODE_CB allocated at %p with handle %#lx",
            pstMcMacNode, pstMcMacNode->mc_mac_tree_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstMcMacNode->mc_mac_tree_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstMcMacNode);
}

/*****************************************************************************
   函 数 名  : spm_alloc_vip_vmac_node_cb
   功能描述  : 虚拟MAC和IP地址节点结构内存空间分配
   输入参数  : NBB_CCXT_T NBB_CXT
   输出参数  : 无
   返 回 值  : SPM_VIP_VMAC_NODE_CB
   调用函数  :
   被调函数  :

   修改历史      :
   1.日    期   : 2012年10月16日
    作    者   : xiaoxiang
    修改内容   : 新生成函数

*****************************************************************************/
SPM_VIP_VMAC_NODE_CB *spm_alloc_vip_vmac_node_cb()
{
    SPM_VIP_VMAC_NODE_CB *pstVipVmacNode = NULL;

    //NBB_BUF_SIZE avll_key_offset;

    NBB_TRC_ENTRY("spm_alloc_vip_vmac_node_cb");

    /* 分配一个新的接口物理配置条目。*/
    pstVipVmacNode = (SPM_VIP_VMAC_NODE_CB *)NBB_MM_ALLOC(sizeof(SPM_VIP_VMAC_NODE_CB),
        NBB_NORETRY_ACT,
        MEM_SPM_VIP_VMAC_NODE_CB);
    if (pstVipVmacNode == NULL)
    {
        goto EXIT_LABEL;
    }

    /* 初始化接口物理配置条目 */
    OS_MEMSET(pstVipVmacNode, 0, sizeof(SPM_VIP_VMAC_NODE_CB));

    /* 建立用于该接口物理配置条目的句柄，作为异步消息交换的相关器。*/
    pstVipVmacNode->vip_vmac_tree_handle = NBB_CREATE_HANDLE(pstVipVmacNode, HDL_SPM_VIP_VMAC_NODE_CB);

    /* 成功分配一个新的接口物理配置条目。*/
    NBB_TRC_DETAIL((NBB_FORMAT "SPM_VIP_VMAC_NODE_CB allocated at %p with handle %#lx",
            pstVipVmacNode, pstVipVmacNode->vip_vmac_tree_handle));

    /* Initialize the AVLL node. */
    AVLL_INIT_NODE(pstVipVmacNode->vip_vmac_tree_node);

EXIT_LABEL: NBB_TRC_EXIT();

    return(pstVipVmacNode);
}

#endif

#endif

