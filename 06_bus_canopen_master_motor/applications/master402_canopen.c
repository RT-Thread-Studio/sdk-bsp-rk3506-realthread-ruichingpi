/**
 * @file master402_canopen.c
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @copyright Copyright (c) 2022
 * @attention 
 * @par 修改日志:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 */
#include <rtthread.h>
/* Includes ------------------------------------------------------------------*/
#include "master402_canopen.h"
/* Private includes ----------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <finsh.h>

#include "canfestival.h"
#include "canopen_callback.h"
#include "timers_driver.h"

#include "motor_control.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/*DS301心跳协议写入节点心跳格式
  NodeID  : 节点
  Interval: 心跳间隔*/
#define HEARTBEAT_FORMAT(NodeID,Interval) (((NodeID) << 16) | (Interval))
/*DS301协议 31位置1关闭PDO（RPDO/TPDO）传输
  NodeID  : 节点
  Default :  默认值*/
#define PDO_DISANBLE(NodeID,Default) ((1<<31) | (Default) | (NodeID))
/*DS301协议 31位置1使能PDO（RPDO/TPDO）传输
  NodeID  : 节点
  Default :  默认值*/
#define PDO_ENANBLE(Default,NodeID) ((Default) | (NodeID))
/*DS301协议 31位置1使能SYNC同步消息
  NodeID  : 节点*/
#define SYNC_ENANBLE(NodeID) ((1 << 30) | (NodeID))
/* Private variables ---------------------------------------------------------*/
CO_Data *OD_Data = &master402_Data;
static node_list can_node[2] = 
{
    {MASTER_NODEID, "master", {0, 0}, NULL},  // 主节点，无电机配置
    {SERVO_NODEID_1, "walk", {200, 1000}, NULL}  // 电子齿轮比 200:1000 = 1:5
};

static s_BOARD agv_board  = {CANFESTIVAL_CAN_DEVICE_NAME};
static node_config_state slave_conf;//配置状态
/* Private function prototypes -----------------------------------------------*/
static void config_node_param(uint8_t nodeId, node_config_state *conf);
/**
  * @brief  判断整数的位数
  * @param  long n:数字
  * @retval 位数.
  * @note   https://blog.csdn.net/qq_40757240/article/details/85727578
*/
int Bit_Int_2(long n)
{
	if(n==0)
		return 1;
	else
		return log10(abs(n))+1;//求整数的位数
	//例如  1234  lgx+1;
}
/***********************初始化操作状态函数**************************************************/
/**
  * @brief  初始节点
  * @param  None
  * @retval None
  * @note   
*/
static void InitNodes(CO_Data* d, UNS32 id)
{
	setNodeId(OD_Data,MASTER_NODEID);
	setState(OD_Data, Initialisation);
}
/**
  * @brief  初始化canopen
  * @param  None
  * @retval None
  * @note   
*/
void canopen_task(void *pram)
{
  CAN_PORT port;
  OD_Data->heartbeatError = master402_heartbeatError;
	OD_Data->initialisation = master402_initialisation;
	OD_Data->preOperational = master402_preOperational;
	OD_Data->operational = master402_operational;
	OD_Data->stopped   = master402_stopped;
	OD_Data->post_sync = master402_post_sync;
	OD_Data->post_TPDO = master402_post_TPDO;
	OD_Data->storeODSubIndex = (storeODSubIndex_t)master402_storeODSubIndex;
	OD_Data->post_emcy = (post_emcy_t)master402_post_emcy;

  PDODisable(OD_Data,1);
  PDODisable(OD_Data,3);

	port = canOpen(&agv_board, OD_Data);
  if (port != RT_NULL)
  {
      OD_Data->canHandle = port;
  }
  else
  {
      return;
  }
	initTimer();
	StartTimerLoop(&InitNodes);
  //挂钩相关指针
  slave_conf.list = &can_node[1];
  can_node[1].nmt_state = &OD_Data->NMTable[2];
  can_node[0].nmt_state = &OD_Data->nodeState;
}

void canopen_start(void)
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("canopen_task", canopen_task, RT_NULL, 10240, 7, 10);

    if(tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("create thread err\n");
    }
}
MSH_CMD_EXPORT(canopen_start, canopen start);
/**
  * @brief  None
  * @param  None
  * @retval None
  * @note   
*/
static __attribute__((unused)) void Exit(CO_Data* d, UNS32 id)
{

}
/************************外部调用函数********************************************************/
/**
  * @brief  获取节点名称
  * @param  des:目标地址
  * @param  nodeID:节点ID
  * @retval des:目标地址
  * @note   若输入节点ID不正确，将返回空指针
*/
char *nodeID_get_name(char* des,uint8_t nodeID)
{
  if(des == NULL)
    return RT_NULL;

  if(nodeID == can_node[nodeID - 1].nodeID)
  {
    rt_memcpy(des,can_node[nodeID - 1].name,RT_NAME_MAX);
    return des;
  }
  else
    return RT_NULL;
}
/**
  * @brief  获取节点NMT状态
  * @param  nodeID:节点ID
  * @retval 返回NMT状态
  * @note   若输入节点ID不正确，将返回0XFF
*/
e_nodeState nodeID_get_nmt(uint8_t nodeID)
{
  if(nodeID > 2)
    return 0XFF;
  if(nodeID == can_node[nodeID - 1].nodeID)
    return *can_node[nodeID - 1].nmt_state;
  else
    return 0XFF;
}
/**
  * @brief  设置节点错误代码.
  * @param  nodeID:节点ID
  * @param  errcode:错误代码
  * @retval 成功返回RT_EOK，失败返回-RT_ERROR.
  * @note   None.
*/
rt_err_t nodeID_set_errcode(uint8_t nodeID,uint16_t errcode)
{
  if(nodeID > 2)
  {
    return -RT_ERROR;
  }
  else
  {
    slave_conf.err_code = errcode;
    return RT_EOK;
  }
}
/**
  * @brief  查看节点错误代码.
  * @param  nodeID:节点ID
  * @retval 成功返回节点错误代码.失败返回0
  * @note   None.
*/
rt_err_t nodeID_get_errcode(uint8_t nodeID)
{
  if(nodeID > 2)
    return 0;
  if(nodeID == MASTER_NODEID)
    return 0;
  else
    return slave_conf.err_code;
}
/**
  * @brief  设置节点具体错误
  * @param  nodeID:节点ID
  * @param  errSpec:具体错误
  * @retval 成功返回RT_EOK,失败返回-RT_ERROR
  * @note   None.
*/
rt_err_t nodeID_set_errSpec(uint8_t nodeID,const uint8_t errSpec[5])
{
  if(nodeID > 2)
  {
    return -RT_ERROR;
  }
  else
  {
    rt_memcpy(slave_conf.errSpec,errSpec,5);
    return RT_EOK;
  }
}
/**
  * @brief  获取节点具体错误
  * @param  des:目标地址
  * @param  nodeID:节点ID
  * @retval des:目标地址,失败返回RT_NULL
  * @note   None.
*/
char* nodeID_get_errSpec(char* des,uint8_t nodeID)
{
  if(des == NULL)
    return RT_NULL;

  if(nodeID == MASTER_NODEID || nodeID > 2 || nodeID == 0)
  {
    return RT_NULL;
  }
  else
  {
    if(slave_conf.errSpec[0] + slave_conf.errSpec[1] + \
       slave_conf.errSpec[2] + slave_conf.errSpec[3] + 
       slave_conf.errSpec[4])
    {
      rt_memcpy(des,slave_conf.errSpec,5);
      return des;
    }
    else
    {
      return RT_NULL;
    }
  }
}
/**
 * @brief  
 * @note   
 * @param  num: 
 * @retval 
 */
node_config_state *slave_conf_get(NODEID_NUM num)
{
  return &slave_conf;
}
#ifdef RT_USING_MSH
/**
  * @brief  打印节点状态
  * @param  state：节点状态
  * @retval None.
  * @note   None.
*/
static void printf_state(e_nodeState state)
{
  switch(state)
  {
  case Initialisation:
    rt_kprintf("Initialisation");
    break;
  case Stopped:
    rt_kprintf("Stopped");
    break;
  case Operational:
    rt_kprintf("Operational");
    break;
  case Pre_operational:
    rt_kprintf("Pre_operational");
    break;
  case Disconnected:
    rt_kprintf("Disconnected");
    break;   
  case Unknown_state:
    rt_kprintf("Unknown_state");
    break;
  default:
    rt_kprintf("%d",state);
    break;
  }
}
/**
  * @brief  查询或控制nmt状态
  * @param  None
  * @retval None
  * @note   None
*/
static void cmd_canopen_nmt(uint8_t argc, char **argv) 
{
#define NMT_CMD_LIST                    0
#define NMT_CMD_GET                     1
#define NMT_CMD_SLAVE_SET               2
#define NMT_CMD_PRE                     NMT_CMD_SLAVE_SET+2

  const char* help_info[] =
    {
        [NMT_CMD_LIST]           = "canopen_nmt list                        - Print all node NMT.",
        [NMT_CMD_GET]            = "canopen_nmt get [nodeID]                - Get nodeID state.",
        [NMT_CMD_SLAVE_SET]      = "canopen_nmt s   [operational] <nodeID>  - Slvae  NMT set start/stop/pre/rn/rc.",
        [NMT_CMD_SLAVE_SET+1]    = "                                        - rn:Reset_Node rc:Reset_Comunication.",
        [NMT_CMD_PRE]            = "canopen_nmt m   [operational] <nodeID>  - Master NMT set start/stop/pre.",
    };

    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        for (size_t i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
        return;
    }
    else
    {
        const char *operator = argv[1];
        if (!strcmp(operator, "list"))//打印所有节点NMT状态
        {
          const char *item_title = "NMT";
          int maxlen = RT_NAME_MAX;    

          rt_kprintf("%-*.*s nodeID  status\n",maxlen,maxlen,item_title);
          rt_kprintf("-------- ------  -------\n");
          for(UNS8 i = 0; i < 2; i++)
          {
              rt_kprintf("%-*.*s 0X%02X    ",maxlen,maxlen,can_node[i].name,can_node[i].nodeID);
              printf_state(*can_node[i].nmt_state);
              rt_kprintf("\n");
          }
        }
        else if (!strcmp(operator, "get"))//查询节点NMT
        {
          if(argc <= 2) 
          {
              rt_kprintf("Usage:canoepn get [nodeID]  - Get nodeID state\n");
              return;
          }
          uint8_t nodeid = atoi(argv[2]);
          rt_kprintf("%s 0X%02X    ",can_node[nodeid - 1].name,can_node[nodeid - 1].nodeID);
          rt_kprintf("nodeID state is ");
          printf_state(*can_node[nodeid - 1].nmt_state);
          rt_kprintf("\n");       
        }
        else if (!strcmp(operator, "s"))//从机设置NMT
        {
          uint8_t nodeID = 2;
          UNS8    cs = 0X00; 
          if(argc <= 2) 
          {
              rt_kprintf("Usage:canopen_nmt s   [operational] <nodeID>  - Slvae  NMT set start/stop/pre/rn/rc.\n");
              rt_kprintf("                                              - rn:Reset_Node rc:Reset_Comunication.\n");
              return;
          }
          if(argc > 2)
          {
            const char *operator = argv[2];
            if (!strcmp(operator, "start"))
            {
              cs = NMT_Start_Node;
            }
            else if(!strcmp(operator, "stop"))
            {
              cs = NMT_Stop_Node;
            } 
            else if(!strcmp(operator, "pre"))
            {
              cs = NMT_Enter_PreOperational;
            } 
            else if(!strcmp(operator, "rn"))
            {
              cs = NMT_Reset_Node;
            } 
            else if(!strcmp(operator, "rc"))
            {
              cs = NMT_Reset_Comunication;
            }
            else
            {
              rt_kprintf("Usage:canopen_nmt s   [operational] <nodeID>  - Slvae  NMT set start/stop/pre.\n");
              return;
            }
          }
          if(argc > 3)
          {
             nodeID = atoi(argv[4]);
          }
          masterSendNMTstateChange(OD_Data,nodeID,cs);
        }
        else if (!strcmp(operator, "m"))//主机设置NMT
        {
          const char *operator = argv[2];
          if(argc <= 2) 
          {
              rt_kprintf("Usage:canopen_nmt m   [operational] <nodeID>  - Master NMT set start/stop/pre.\n");
              return;
          }
          if(argc > 2)
          {
            e_nodeState newState;
            if (!strcmp(operator, "start"))
            {
              newState = Operational;
            }
            else if (!strcmp(operator, "stop"))
            {
              newState = Stopped;
            }
            else if (!strcmp(operator, "pre"))
            {
              newState = Pre_operational;
            }
            else
            {
                rt_kprintf("Usage:canopen_nmt m   [operational] <nodeID>  - Master NMT set start/stop/pre/rn/rc.\n");
                rt_kprintf("                                              - rn:Reset_Node rc:Reset_Comunication.\n");
                return;
            }
            setState(OD_Data, newState);
          }
        }
        else
        {
          rt_kprintf("Usage:\n");
          for (size_t i = 0; i < sizeof(help_info) / sizeof(char*); i++)
          {
              rt_kprintf("%s\n", help_info[i]);
          }
          rt_kprintf("\n");
          return;
        }
    }
}
MSH_CMD_EXPORT_ALIAS(cmd_canopen_nmt,canopen_nmt,canoepn nmt cmd.);
#endif /*RT_USING_MSH*/
/***********************预操作状态函数**************************************************/
/**
  * @brief  配置参数完成回调
  * @param  None
  * @retval None
  * @note   None
*/
static void config_node_param_cb(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;
	UNS8 res;
	node_config_state *conf;

	conf = &slave_conf;
	res = getWriteResultNetworkDict(OD_Data, nodeId, &abortCode);
	closeSDOtransfer(OD_Data, nodeId, SDO_CLIENT);
	if(res != SDO_FINISHED)
	{
		conf->try_cnt++;
		if(conf->try_cnt < 3)
		{
      rt_kprintf("write SDO failed!  nodeId = %d, abortCode = 0x%08X,step = %d\n", nodeId, abortCode,conf->state + 1);
			config_node_param(nodeId, conf);
		}
		else
		{
			rt_sem_release(&(conf->finish_sem));
			conf->state = 0;
			conf->try_cnt = 0;
      conf->err_code = NODEID_CONFIG_NO_RESPOND;
			rt_kprintf("SDO config try count > 3, config failed!,error = %d\n",conf->err_code);
		}
	}
	else
	{
    conf->err_code = NODEID_CONFIG_SUCCESS;
		conf->state++;
		conf->try_cnt = 0;
		config_node_param(nodeId, conf);
	}
}
/**
  * @brief  配置参数
  * @param  None
  * @retval None
  * @note   互斥量等待配置完成
*/
void config_node(uint8_t nodeId)
{
  if(Write_SLAVE_control_word(nodeId,0x80) == 0xFF)//初始化进行错误重置
  {
      rt_kprintf("nodeId:%d,Failed to clear error.The current node is not in operation\n",nodeId);
      rt_sem_init(&(slave_conf.finish_sem), "servocnf1", 0, RT_IPC_FLAG_FIFO);
      if(rt_sem_take(&(slave_conf.finish_sem), SDO_REPLY_TIMEOUT) != RT_EOK)
      {
        slave_conf.err_code = NODEID_CONFIG_NO_RESPOND;
        //掉线情况执行，重新上电情况无需处理
        rt_kprintf("Waiting for the repair to complete, CAN communication is currently unavailable\n");
        master402_fix_config_err(OD_Data,nodeId);
      }
      rt_sem_detach(&(slave_conf.finish_sem));
  }
  else
  {
      slave_conf.state = 0;
      slave_conf.try_cnt = 0;
      rt_sem_init(&(slave_conf.finish_sem), "servocnf1", 0, RT_IPC_FLAG_FIFO);
      EnterMutex();
      config_node_param(nodeId, &slave_conf);
      LeaveMutex();
      rt_sem_take(&(slave_conf.finish_sem), RT_WAITING_FOREVER);
      rt_sem_detach(&(slave_conf.finish_sem));

      if(slave_conf.err_code != 0X00)//因配置错误导致的退出
      {
        rt_kprintf("Failed to configure the dictionary for node %d\n",nodeId);
        if(slave_conf.err_code == NODEID_CONFIG_NO_SEND)
        {
          rt_kprintf("The configuration was not sent because the local dictionary failed\n");
        }
        else if(slave_conf.err_code == NODEID_CONFIG_NO_RESPOND)
        {
          rt_kprintf("The configuration reply did not respond, and the node dictionary failed\n");
        }
        rt_kprintf("Waiting for the repair to complete, CAN communication is currently unavailable\n");
        master402_fix_config_err(OD_Data,nodeId);
        return; //退出线程
      }
      else
      {
        UNS32 errorCode,map_val, size = SDO_MAX_LENGTH_TRANSFER;
        UNS8 data_type;
        errorCode = readLocalDict(OD_Data, 0x1016, nodeId - 1, &map_val, &size, &data_type, 0);
        if(errorCode == OD_SUCCESSFUL)
        {
          /**写入主机消费者/接收端判断心跳超时时间  DS301定义**/
          /**有格式定义，字典工具没有支持，需要自己写入**/
          UNS32 consumer_heartbeat_time = HEARTBEAT_FORMAT(nodeId,CONSUMER_HEARTBEAT_TIME);//写入节点的心跳时间
          errorCode = writeLocalDict(OD_Data, 0x1016, nodeId - 1, &consumer_heartbeat_time, &size, 0);
          if(errorCode != OD_SUCCESSFUL)
            rt_kprintf("index:0X%04X,subIndex:0X%X,write Local Dict false,abort code is 0X%08X\n",0x1016,nodeId - 1,errorCode);
        }
        else
        {
          rt_kprintf("index:0X%04X,subIndex:0X%X,write Local Dict false,abort code is 0X%08X\n",0x1016,nodeId - 1,errorCode);
          rt_kprintf("Node %d is not configured with a consumer heartbeat\n",nodeId);
        }
        //节点进入操作状态
        masterSendNMTstateChange(OD_Data, nodeId, NMT_Start_Node);
        rt_kprintf("Node %d configuration Complete\n",nodeId);
        rt_thread_mdelay(200);//确保NMT命令下发成功
      }
  }
}
/**
  * @brief  配置当前节点
  * @param  None
  * @retval None
  * @note   None
*/
static void config_single_node(void *parameter)
{
	uint32_t nodeId;
	nodeId = (uint32_t)parameter;
	config_node(nodeId);
}
/**
  * @brief  从机上线处理函数
  * @param  None
  * @retval None
  * @note   创建config_single_servo线程
*/
static void slaveBootupHdl(CO_Data* d, UNS8 nodeId)
{
  master_resume_start(d,nodeId);//是否需要恢复操作模式
	rt_thread_t tid;
  rt_kprintf("Node %d has gone online\n",nodeId);
  //判断信号量是否初始化
  if(!rt_list_isempty(&slave_conf.finish_sem.parent.suspend_thread))
  {
    rt_kprintf("Node %d is powered on before the MCU\n",nodeId);
    rt_sem_release(&(slave_conf.finish_sem));
  }
  else
  {
    rt_kprintf("After the MCU is powered on, node %d is powered on\n",nodeId);
  }
	tid = rt_thread_create("co_cfg", config_single_node, (void *)(int)nodeId, 10240, 12 + nodeId, 2);
	if(tid == RT_NULL)
	{
		rt_kprintf("canopen config thread start failed!\n");
	}
	else
	{
		rt_thread_startup(tid);
	}
}
/**
  * @brief  写入本地字典参数，并切换nmt启动
  * @param  None
  * @retval None
  * @note   nmt状态切换为operation状态处理线程，运行完删除线程
*/
void canopen_start_thread_entry(void *parameter)
{
  // assert(parameter);
  if (parameter == NULL)
  {
    return;
  }
	UNS32 size,errorCode;
	UNS8 data_type;
  CO_Data *d = (CO_Data *)parameter;
  /*写入节点字典*/
  // UNS8 i = 0;//调试取消注释这行，注释下行。用来单节点初始化
  // for (UNS8 i = 0; i < MAX_NODE_COUNT - 2; i++)
  // {
    config_node(slave_conf.list->nodeID);//初始化时使用此配置，减少sem初始化与删除操作
  // }
  /*写入本地字典*/
	d->post_SlaveBootup = slaveBootupHdl;
  /**写入主机生成者/发送端心跳发送时间  DS301定义**/
  UNS16 producer_heartbeat_time = PRODUCER_HEARTBEAT_TIME;
	size = 2;
  errorCode = writeLocalDict(d, 0x1017, 0, &producer_heartbeat_time, &size, 0);
	if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,write Local Dict false,abort code is 0X%08X\n",0x1017,0,errorCode);
    setState(d, Stopped);//主站不发送生产者心跳，就进行操作。可能发送主站掉线，节点不知道还在运行
    return ;
  }
  //主站进入操作模式
  setState(d, Operational);
  /**有格式定义，字典工具没有支持，需要自己写入**/
  UNS32 sync_id;
	data_type = uint32;
	size = 4;
  errorCode = readLocalDict(d, 0x1005, 0, &sync_id, &size, &data_type, 0);
	if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",0x1005,0,errorCode);
    setState(d, Stopped);//主站不发送同步消息不可能操作节点
    return ;
  }
  else
  {
    sync_id = SYNC_ENANBLE(sync_id);//DS301 30位置1 ，启用CANopen设备生成同步消息
    errorCode = writeLocalDict(d, 0x1005, 0, &sync_id, &size, 0);
    if(errorCode != OD_SUCCESSFUL)
    {
      rt_kprintf("index:0X%04X,subIndex:0X%X,write Local Dict false,abort code is 0X%08X\n",0x1005,0,errorCode);
      setState(d, Stopped);//主站不发送同步消息不可能操作节点
      return ;
    }
  }
}
/******************************写入节点字典操作函数**********************************/
/**
  * @brief  发送完成回调
  * @param  d:字典结构体
  * @param  nodeId：节点ID
  * @retval FLASE:错误,TRUE:成功.
  * @note   仅限初始化结束后使用
*/
static void writeNetworkDictSyncCb(CO_Data* d, UNS8 nodeId) 
{
    node_config_state *conf = &slave_conf;
    rt_sem_release(&conf->finish_sem);
}
/**
  * @brief  异步写入节点字典
  * @param  d:字典结构体
  * @param  nodeId：节点ID
  * @retval FLASE:错误,TRUE:成功.
  * @note   仅限初始化结束后使用
*/
static bool writeNetworkDictSync (CO_Data* d, UNS8 nodeId, UNS16 index,
        UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, UNS8 useBlockMode) 
{
    if(nodeId <= 1 || nodeId > 2) 
    {
      rt_kprintf("invalid nodeId:%d, should be 2\n",nodeId);
      return false;
    }

    int try_cnt = 3;
    node_config_state* conf = &slave_conf;
    rt_sem_init(&conf->finish_sem, "servocnf", 0, RT_IPC_FLAG_FIFO);
    
    while(try_cnt--) 
    {
        if(writeNetworkDictCallBack(d, nodeId, index, subIndex,
                count, dataType, data, writeNetworkDictSyncCb, useBlockMode) != 0)
        {
            rt_kprintf("write SDO failed!  nodeId = %d, index: 0X%04X, subIndex: 0X%X\n", nodeId, index, subIndex);
            closeSDOtransfer(d, nodeId, SDO_CLIENT);
            continue;
        }

        if(rt_sem_take(&conf->finish_sem, SDO_REPLY_TIMEOUT) != RT_EOK) 
        {
            rt_kprintf("write SDO timeout!  nodeId = %d, index: 0X%04X, subIndex: 0X%X\n", nodeId, index, subIndex);
            closeSDOtransfer(d, nodeId, SDO_CLIENT);
            continue;
        }

        UNS32 abortCode = 0;
        UNS8 res = getWriteResultNetworkDict(d, nodeId, &abortCode);
        closeSDOtransfer(d, nodeId, SDO_CLIENT);

        if(res != SDO_FINISHED)
        {
            rt_kprintf("get SDO write result failed!  nodeId = %d, index: 0X%04X, subIndex: 0X%X, abortCode = 0X%08X res:%d\n", nodeId, index, subIndex, abortCode,res);
            continue;
        }
        rt_sem_detach(&conf->finish_sem);
        return true;
    }
    rt_sem_detach(&conf->finish_sem);
    return false;
}
/**
  * @brief  初始化结束后读取本地字典参数并写入节点字典
  * @param  d:字典结构体
  * @param  nodeId:节点ID
  * @param  local_index:本地索引
  * @param  local_subIndex:本地子索引
  * @param  slave_Index:从机索引
  * @param  slave_subIndex:从机子索引
  * @retval 0XFF:错误，0X00:成功.
  * @note   仅限初始化结束后使用
            初始化过程使用，会导致信号量重复初始化，导致系统卡死
            读取本地字典变量值发送至节点该变量值。
*/
static UNS8 read_local_send_node_init_end(CO_Data* d,UNS8 nodeId,UNS16 local_index,UNS8 local_subIndex,UNS16 slave_index,UNS8 slave_subIndex)
{
  UNS32  size = SDO_MAX_LENGTH_TRANSFER;
  UNS32 pdo_map_val = 0,errorCode = 0; 
  UNS8  dataType = 0;

  errorCode = readLocalDict(d,local_index,local_subIndex,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",local_index,local_subIndex,errorCode);
    return 0xFF;
  }
  errorCode = writeNetworkDictSync(d,nodeId,slave_index,slave_subIndex,size,dataType,&pdo_map_val,0);
  if(errorCode != true)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,write slave Dict false,abort code is 0X%08X\n",slave_index,slave_subIndex,errorCode);
    return 0xFF;
  }
  return 0x00;
}
/**
  * @brief  读取本地字典值并发送至从机节点 仅适用于主站TPDO至RPDO与RPDO至TPDO
  * @param  index:本地索引
  * @param  subIndex:本地子索引
  * @param  nodeId:节点ID
  * @retval 0XFF:错误，0X00:成功..
  * @note   仅适用于主站TPDO至RPDO与RPDO至TPDO
            读取对应地图映射变量索引相关参数并发送至从机
*/
static UNS8 read_local_send_node_pdo_T_R(UNS16 index,UNS8 subIndex,uint8_t nodeId)
{
  UNS32 pdo_map_val;
  UNS32  size = SDO_MAX_LENGTH_TRANSFER;
  UNS8  dataType;
  UNS32 errorCode;
  UNS16  slave_Index;
  
  if(0X1600 <= index && index <= 0X17FF)//从机 Receive PDO Parameters
  {
    //在主站Transmit PDO Parameters寻找对应参数
    slave_Index = index + 0X400;
  }
  else if(0X1A00 <= index && index <= 0X1BFF)//Transmit PDO Parameters
  {
    //在主站Receive PDO Parameters寻找对应参数
    slave_Index = index - 0X400;
  }
  else
  {
    rt_kprintf("The index: 0X%04X is not a valid PDO parameter\n",index);
    return 0XFF;
  }
  errorCode = readLocalDict(OD_Data,slave_Index,subIndex,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",index,subIndex,errorCode);
    return 0XFF;
  }
  else
  {
    return writeNetworkDictCallBack(OD_Data,nodeId,index,subIndex,size,dataType,&pdo_map_val,config_node_param_cb,0);
  }
}
/**
  * @brief  读取本地字典配置并发送至从机节点
  * @param  local_index:本地索引
  * @param  local_subIndex:本地子索引
  * @param  slave_Index:从机索引
  * @param  slave_subIndex:从机子索引
  * @param  nodeId:节点ID
  * @retval 0XFF:错误，0X00:成功.
  * @note   仅限PDO配置发送[不包括PDO CNT设置]。
            读取本地字典索引与大小计算传输数据
*/
static __attribute__((unused)) UNS8 read_local_send_node_pdo(UNS16 local_index,UNS8 local_subIndex,UNS16 slave_Index,UNS8 slave_subIndex,uint8_t nodeId)
{
  UNS32 pdo_map_val;
  UNS32 size = SDO_MAX_LENGTH_TRANSFER;
  UNS8  dataType;
  UNS32 errorCode;
  UNS32 SDO_data;

  errorCode = readLocalDict(OD_Data,local_index,local_subIndex,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",local_index,local_subIndex,errorCode);
    return 0XFF;
  }
  else
  {
    /*
          位             功能 
    Bit 0 ~ Bit 7     对象数据长度 
    Bit 8 ~ Bit 15    对象子索引 
    Bit 16 ~ Bit 31   对象索引 
    */
    SDO_data = (local_index << 16) + (local_subIndex << 8) + (size * 8);
    pdo_map_val = 0;size = SDO_MAX_LENGTH_TRANSFER;dataType = 0;//清除遗留数据
    errorCode = readLocalDict(OD_Data,slave_Index,slave_subIndex,(void *)&pdo_map_val,&size,&dataType,0);
    if(errorCode != OD_SUCCESSFUL)
    {
      rt_kprintf("index:0X%X,subIndex:0X%X,read Local Dict false,abort code is 0X%X\n",slave_Index,slave_subIndex,errorCode);
      return 0XFF;
    }
    else
    {
      return writeNetworkDictCallBack(OD_Data,nodeId,slave_Index,slave_subIndex,size,dataType,&SDO_data,config_node_param_cb,0);
    }
  }
}

/**
  * @brief  初始化阶段读取本地字典参数并写入节点字典
  * @param  d:字典结构体
  * @param  nodeId:节点ID
  * @param  index:本地索引
  * @param  subIndex:本地子索引
  * @retval 0XFF:错误，0X00:成功.
  * @note   仅初始化使用
            仅做读取写入，不做任何计算
*/
static __attribute__((unused)) UNS8 read_local_send_node_init_start(UNS16 index,UNS8 subIndex,UNS8 nodeId)
{
  UNS32  size = SDO_MAX_LENGTH_TRANSFER;
  UNS32 pdo_map_val = 0,errorCode = 0; 
  UNS8  dataType = 0;

  errorCode = readLocalDict(OD_Data,index,subIndex,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",index,subIndex,errorCode);
    return 0xFF;
  }
  return writeNetworkDictCallBack(OD_Data,nodeId,index,subIndex,size,dataType,&pdo_map_val,config_node_param_cb,0);
}
/******************************SDO设置参数操作**********************************/
/**
  * @brief  写入从机Modes_of_operation设置
  * @param  nodeId:节点ID
  * @param  mode:Modes_of_operation 
  * @retval 成功返回0X00,失败返回0XFF
  * @note   None.
*/
UNS8 Write_SLAVE_Modes_of_operation(UNS8 nodeId,INTEGER8 mode)
{
  if(0 <= *Modes_of_operation_Node[nodeId - 2].map_val && *Modes_of_operation_Node[nodeId - 2].map_val <= 10)
  {
    //参数写入本地字典
    *Modes_of_operation_Node[nodeId - 2].map_val = mode;
  }
  else
  {
    rt_kprintf("The Modes_of_operation : %d is out of range. Select a value from 0 to 10\n",*Modes_of_operation_Node[nodeId - 2].map_val);
    return 0XFF;
  }

  //参数发送节点字典
  if(read_local_send_node_init_end(OD_Data,nodeId,
                          Modes_of_operation_Node[nodeId - 2].index,0x00,//读取本地变量
                          0x6060,0x00) == 0XFF)//发送至从机变量
    return 0XFF;

  return 0X00;
}

/**
  * @brief  写入从机control_word设置
  * @param  nodeId:节点ID
  * @param  value:Controlword 
  * @retval 成功返回0X00,失败返回0XFF
  * @note   None.
*/
UNS8 Write_SLAVE_control_word(UNS8 nodeId,UNS16 value)
{
  //参数写入本地字典
  *Controlword_Node[nodeId - 2].map_val = value;
  //参数发送节点字典
  if(read_local_send_node_init_end(OD_Data,nodeId,
                          Controlword_Node[nodeId - 2].index,0x00,//读取本地变量
                          0x6040,0x00) == 0XFF)//发送至从机变量
    return 0XFF;

  return 0X00;
}
/**
  * @brief  写入从机位置规划模式的速度设置
  * @param  nodeId:节点ID
  * @param  speed:运动速度 单位:RPM
  * @retval 成功返回0X00,失败返回0XFF
  * @note   Profile_velocity 单位PUU/s 设定范围 UNSIGNED32 默认值 10000
*/
UNS8 Write_SLAVE_profile_position_speed_set(UNS8 nodeId,float speed)
{
  //参数写入本地字典
  if(0 <= speed && speed <= 3000)
    Profile_velocity = speed * slave_conf.list->config.numerator / 60;//RPM单位转换为PUU/s
  else
  {
    rt_kprintf("The Profile_velocity is out of range. Select a value from 0 to 3000\n");
    return 0XFF;
  }

  if(read_local_send_node_init_end(OD_Data,nodeId,
                          0x6081,0x00,//读取本地变量
                          0x6081,0x00) == 0XFF)//发送至从机变量
        return 0XFF;

  return 0X00;
}

/**
  * @brief  写入从机0X60C2插补周期参数
  * @param  nodeId:节点ID
  * @retval 成功返回0X00,失败返回0XFF
  * @note   设定需与通讯周期 OD 1006h相同
  * 插补周期时间 = 60C2H sub1 × 10^(60C2H sub2)
  * 范例：若需要设定插补周期时间为 10 ms，需将 OD 60C2h sub1 = 1，OD 60C2h sub2 = -4
    插补周期时间 = 1 × 10^-2 = 0.01s = 10ms
*/
UNS8 Write_SLAVE_Interpolation_time_period(UNS8 nodeId)
{
  UNS32  size = SDO_MAX_LENGTH_TRANSFER;
  UNS32 pdo_map_val,errorCode; 
  UNS8  dataType;
  UNS16 index = 0X1006;//通讯周期值
  UNS8 subIndex = 0;//子索引0

  errorCode = readLocalDict(OD_Data,index,subIndex,(void *)&pdo_map_val,&size,&dataType,0);//读取0X1006通讯周期值
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%X,subIndex:0X%X,read Local Dict false,SDO abort code is 0X%X\n",index,subIndex,errorCode);
    return 0xFF;
  }
  extern int Bit_Int_2(long n);
  Interpolation_time_period_Interpolation_time_index = (Bit_Int_2(pdo_map_val) - 1) - 6;//十的次方数 = 10^(n-6)
  Interpolation_time_period_Interpolation_time_units = pdo_map_val / pow(10,(Bit_Int_2(pdo_map_val) - 1));//插补周期时间常数
  
  if(read_local_send_node_init_end(OD_Data,nodeId,0X60C2,0x01,0X60C2,0x01) == 0XFF)
    return 0XFF;
  if(read_local_send_node_init_end(OD_Data,nodeId,0X60C2,0x02,0X60C2,0x02) == 0XFF)
    return 0XFF;

  return 0x00;
}
/**
  * @brief  写入从机原点复归模式的相关设置
  * @param  nodeId:节点ID
  * @param  offest:原点偏移值 单位:PUU [注意:只是把原点偏移值点当为0坐标点，并不会运动到0坐标点处]
  * @param  method:回原方式   范围:0 ~ 35
  * @param  switch_speed:寻找原点开关速度 设定范围 0.1 ~ 200 默认值 10  单位rpm 精度:小数点后一位
  * @param  zero_speed:寻找 Z脉冲速度     设定范围 0.1 ~ 50  默认值 2   单位rpm 精度:小数点后一位
  * @retval 成功返回0X00,失败返回0XFF
  * @note   Home Position为执行原点复归时所找到的原点参考点，如原点 Sensor、Z脉冲等。
            当原点参考点找到后，从该点所偏移的位置即为用户定义的原点。

            共 35种原点复归模式供用户设定。第 1 ~ 16种回原点模式会找寻 Z脉冲； 
            而第 17 ~ 34种则是不找寻 Z脉冲；第 35种则是定义当前位置为原点。

            switch_speed和zero_speed写入单位为0.1rpm

            设定原点复归加/减速时间 OD 609Ah 默认值 100 单位 ms 
            此对象所设定的时间斜率为加速 (0 rpm加速到 3000 rpm) 及减速 (3000 rpm减速到0 rpm) 所需要的时间。此对象仅适用于 Homing Mode (原点复归模式)。 
*/
UNS8 Write_SLAVE_Homing_set(UNS8 nodeId,UNS32 offset,UNS8 method,float switch_speed,float zero_speed)
{
  //参数写入本地字典
  Home_offset = offset;
  if(0 <= method && method <= 35)
    Homing_method = method;
  else
  {
    rt_kprintf("The method is out of range. Select a value from 0 to 35\n");
    return 0XFF;
  }
  if(0.1 <= switch_speed && switch_speed <= 200)
    Homing_speeds_Speed_for_switch_search = (uint32_t)(switch_speed * 10);
  else
  {
    rt_kprintf("The switch_speed is out of range. Select a value from 0.1 to 200\n");
    return 0XFF;
  }
  if(0.1 <= zero_speed && zero_speed <= 50)
    Homing_speeds_Speed_for_zero_search = (uint32_t)(zero_speed * 10);
  else
  {
    rt_kprintf("The zero_speed is out of range. Select a value from 0.1 to 50\n");
    return 0XFF;
  }

  //参数发送节点字典
  if(read_local_send_node_init_end(OD_Data,nodeId,0X607C,0x00,0X607C,0x00) == 0XFF)
    return 0XFF;
  if(read_local_send_node_init_end(OD_Data,nodeId,0X6098,0x00,0X6098,0x00) == 0XFF)
    return 0XFF;
  if(read_local_send_node_init_end(OD_Data,nodeId,0X6099,0x01,0X6099,0x01) == 0XFF)
    return 0XFF;
  if(read_local_send_node_init_end(OD_Data,nodeId,0X6099,0x02,0X6099,0x02) == 0XFF)
    return 0XFF;

  return 0X00;
}
/******************************NODE2 设置参数操作****************************************/
/******************************TPDO1 设置参数操作
                               数据长度被限制为 1~8 字节*********************************/
/**
  * 分析仪显示PDO1发送 , 帧ID0X183 DLC:8
  * 字典工具：0x1400 Receive PDO 1 Parameter:COB ID used by PDO:0x00000183
  * 字典工具：0x1600 Receive PDO 1 Mapping.填入需要发送数据
  * 为从机发送通道TPDO1,即主机接收通道RPDO1
  * PDO DATA:0x50 06 4E C3 00 00 00 00  小端模式.
  * SUBINDEX2:32位,占用4个字节.即00 00 C3 4E = 49998 Position actual value (Ox6064)
  * SUBINDEX2:32位,占用4个字节.即00 00 C3 4E = 49998 Velocity_actual_value (Ox606C)
*/
static UNS8 NODE2_DIS_SLAVE_TPDO1(uint8_t nodeId)
{
// disable Slave's TPDO，关闭从机TPDO1发送用来写入参数
  UNS32 TPDO_COBId = PDO_DISANBLE(0x00000180,nodeId);//0x80000180 + nodeId;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1800, 1, 
    4, uint32, &TPDO_COBId, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_TPDO1_Type(uint8_t nodeId)
{
  UNS8 trans_type = PDO_TRANSMISSION_TYPE;//写入类型
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1800, 2, 
    1, uint8, &trans_type, config_node_param_cb, 0);
}
static __attribute__((unused)) UNS16 NODE2_Write_SLAVE_TPDO1_InhibitTime(uint8_t nodeId)
{
  UNS16 inhibit_time = 500;//写入时间
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1800, 3, 
    2, uint16, &inhibit_time, config_node_param_cb, 0);
}
static UNS8 NODE2_Clear_SLAVE_TPDO1_Cnt(uint8_t nodeId)
{
  UNS8 pdo_map_cnt = 0;//清除从机遗留索引数
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1A00, 0, 
    1, uint8, &pdo_map_cnt, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_TPDO1_Sub1(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1A00,1,nodeId);//写入从机PDO1需要发送映射，即主站需要PDO1接收映射
}
static UNS8 NODE2_Write_SLAVE_TPDO1_Sub2(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1A00,2,nodeId);//写入从机PDO1需要发送映射，即主站需要PDO1接收映射
}
static UNS8 NODE2_Write_SLAVE_TPDO1_Sub0(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1A00,0,nodeId);//重新写入正确索引数
}
static UNS8 NODE2_EN_SLAVE_TPDO1(uint8_t nodeId)
{
//enable Slave's TPDO1 使能从机发送PDO1
  UNS32 TPDO_COBId = PDO_ENANBLE(0x00000180,nodeId);
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1800, 1, 
    4, uint32, &TPDO_COBId, config_node_param_cb, 0);
}
/******************************TPDO2 设置参数操作
                               数据长度被限制为 1~8 字节*********************************/
/**
  * 分析仪显示PDO2发送 , 帧ID0X283 DLC:4
  * 字典工具：0x1401 Receive PDO 2 Parameter:COB ID used by PDO:0x00000283
  * 字典工具：0x1601 Receive PDO 2 Mapping.填入需要发送数据
  * 为从机发送通道TPDO2,即主机接收通道RPDO2
  * PDO DATA:0x37 16   小端模式.
  * SUBINDEX1:16位,占用2个字节.即0X1637 Statusword (Ox6041) 
*/
static UNS8 NODE2_DIS_SLAVE_TPDO2(uint8_t nodeId)
{
 //disable Slave's TPDO 关闭从机TPDO2发送用来写入参数
  UNS32 TPDO_COBId = PDO_DISANBLE(0x00000280,nodeId);//0x80000280 + nodeId;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1801, 1, 
    4, uint32, &TPDO_COBId, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_TPDO2_Type(uint8_t nodeId)
{
  UNS8 trans_type = PDO_TRANSMISSION_TYPE;//写入类型
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1801, 2, 
    1, uint8, &trans_type, config_node_param_cb, 0);
}
static UNS8 NODE2_Clear_SLAVE_TPDO2_Cnt(uint8_t nodeId)
{
  UNS8 pdo_map_cnt = 0;//清除从机遗留索引数
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1A01, 0, 
    1, uint8, &pdo_map_cnt, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_TPDO2_Sub1(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1A01,1,nodeId);//写入从机PDO1需要接收映射，即主站需要PDO1发送映射
}
static UNS8 NODE2_Write_SLAVE_TPDO2_Sub0(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1A01,0,nodeId);//重新写入正确索引数
}
static UNS8 NODE2_EN_SLAVE_TPDO2(uint8_t nodeId)
{
//enable Slave's TPDO1 使能从机发送PDO1
  UNS32 TPDO_COBId = PDO_ENANBLE(0x00000280,nodeId);
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1801, 1, 
    4, uint32, &TPDO_COBId, config_node_param_cb, 0);
}
/******************************RPDO1 设置参数操作
                               数据长度被限制为 1~8 字节*********************************/
/**
  * 分析仪显示PDO1接收 , 帧ID0X203 DLC:8
  * 字典工具：0x1800 Transmit PDO 1 Parameter:COB ID used by PDO:0x00000203
  * 字典工具：0x1A00 Transmit PDO 1 Mapping.填入需要发送数据
  * 为从机接收通道RPDO1,即主机发送通道TPDO1
  * PDO DATA:0x00 00 01 00 00 00 00   小端模式.
  * SUBINDEX1:32位,占用4个字节.即00 00 C3 50 = 50000  Target position (0x607A)
  * SUBINDEX2:32位,占用4个字节.即00 01 86 A0 = 100000 Target_velocity (0x60FF)
*/
static UNS8 NODE2_DIS_SLAVE_RPDO1(uint8_t nodeId)
{
// disable Slave's RPDO
  UNS32 RPDO_COBId = PDO_DISANBLE(0x80000200,nodeId);;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1400, 1, 
    4, uint32, &RPDO_COBId, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_RPDO1_Type(uint8_t nodeId)
{
  UNS8 trans_type = PDO_TRANSMISSION_TYPE;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1400, 2, 
    1, uint8, &trans_type, config_node_param_cb, 0);
}
static UNS8 NODE2_Clear_SLAVE_RPDO1_Cnt(uint8_t nodeId)
{
  UNS8 pdo_map_cnt = 0;//清除索引
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1600, 0, 
    1, uint8, &pdo_map_cnt, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_RPDO1_Sub1(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1600,1,nodeId);//写入从机PDO1需要接收映射，即主站需要PDO1发送映射
}
static UNS8 NODE2_Write_SLAVE_RPDO1_Sub2(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1600,2,nodeId);//写入从机PDO1需要接收映射，即主站需要PDO1发送映射
}
static UNS8 NODE2_Write_SLAVE_RPDO1_Sub0(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1600,0,nodeId);//重新写入正确索引数
}
static UNS8 NODE2_EN_SLAVE_RPDO1(uint8_t nodeId)
{
// enable Slave's RPDO
  UNS32 RPDO_COBId = PDO_ENANBLE(0x00000200,nodeId);
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1400, 1, 
    4, uint32, &RPDO_COBId, config_node_param_cb, 0);
}
/******************************RPDO2 设置参数操作
                               数据长度被限制为 1~8 字节*********************************/
/**
  * 分析仪显示PDO2接收 , 帧ID0X303 DLC:4
  * 字典工具：0x1801 Transmit PDO 2 Parameter:COB ID used by PDO:0x00000303
  * 字典工具：0x1A01 Transmit PDO 2 Mapping.填入需要发送数据
  * 为从机接收通道RPDO2,即主机发送通道TPDO2
  * PDO DATA:0x50 C3 00 00  小端模式.
  * SUBINDEX1:32位,占用4个字节.即00 00 C3 50 = 50000  POS_CMD (0x60C1)
*/
static UNS8 NODE2_DIS_SLAVE_RPDO2(uint8_t nodeId)
{
// disable Slave's RPDO
  UNS32 RPDO_COBId = PDO_DISANBLE(0x80000300,nodeId);;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1401, 1, 
    4, uint32, &RPDO_COBId, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_RPDO2_Type(uint8_t nodeId)
{
  UNS8 trans_type = PDO_TRANSMISSION_TYPE;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1401, 2, 
    1, uint8, &trans_type, config_node_param_cb, 0);
}
static UNS8 NODE2_Clear_SLAVE_RPDO2_Cnt(uint8_t nodeId)
{
  UNS8 pdo_map_cnt = 0;//清除索引
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1601, 0, 
    1, uint8, &pdo_map_cnt, config_node_param_cb, 0);
}
static UNS8 NODE2_Write_SLAVE_RPDO2_Sub1(uint8_t nodeId)
{
  UNS32 pdo_map_val;
  UNS32 size = SDO_MAX_LENGTH_TRANSFER;
  UNS8  dataType;
  UNS32 errorCode;
  UNS32 SDO_data;

  errorCode = readLocalDict(OD_Data,0X2F00,0,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",0X2F00,0,errorCode);
    return 0XFF;
  }
  else
  {
    /*
          位             功能 
    Bit 0 ~ Bit 7     对象数据长度 
    Bit 8 ~ Bit 15    对象子索引 
    Bit 16 ~ Bit 31   对象索引 
    */
    SDO_data = (0X60C1 << 16) + (1 << 8) + (size * 8);
    pdo_map_val = 0;size = SDO_MAX_LENGTH_TRANSFER;dataType = 0;//清除遗留数据
    errorCode = readLocalDict(OD_Data,0X1A01,1,(void *)&pdo_map_val,&size,&dataType,0);
    if(errorCode != OD_SUCCESSFUL)
    {
      rt_kprintf("index:0X%X,subIndex:0X%X,read Local Dict false,abort code is 0X%X\n",0X1A01,1,errorCode);
      return 0XFF;
    }
    else
    {
      return writeNetworkDictCallBack(OD_Data,nodeId,0X1601,1,size,dataType,&SDO_data,config_node_param_cb,0);
    }
  }
}
static UNS8 NODE2_Write_SLAVE_RPDO2_Sub2(uint8_t nodeId)
{
  UNS32 pdo_map_val;
  UNS32 size = SDO_MAX_LENGTH_TRANSFER;
  UNS8  dataType;
  UNS32 errorCode;
  UNS32 SDO_data;

  errorCode = readLocalDict(OD_Data,0X2F01,0,(void *)&pdo_map_val,&size,&dataType,0);
  if(errorCode != OD_SUCCESSFUL)
  {
    rt_kprintf("index:0X%04X,subIndex:0X%X,read Local Dict false,abort code is 0X%08X\n",0X2F01,0,errorCode);
    return 0XFF;
  }
  else
  {
    /*
          位             功能 
    Bit 0 ~ Bit 7     对象数据长度 
    Bit 8 ~ Bit 15    对象子索引 
    Bit 16 ~ Bit 31   对象索引 
    */
    SDO_data = (0X60C1 << 16) + (2 << 8) + (size * 8);
    pdo_map_val = 0;size = SDO_MAX_LENGTH_TRANSFER;dataType = 0;//清除遗留数据
    errorCode = readLocalDict(OD_Data,0X1A01,2,(void *)&pdo_map_val,&size,&dataType,0);
    if(errorCode != OD_SUCCESSFUL)
    {
      rt_kprintf("index:0X%X,subIndex:0X%X,read Local Dict false,abort code is 0X%X\n",0X1A01,2,errorCode);
      return 0XFF;
    }
    else
    {
      return writeNetworkDictCallBack(OD_Data,nodeId,0X1601,2,size,dataType,&SDO_data,config_node_param_cb,0);
    }
  }
}
static UNS8 NODE2_Write_SLAVE_RPDO2_Sub0(uint8_t nodeId)
{
  return read_local_send_node_pdo_T_R(0X1601,0,nodeId);//重新写入正确索引数
}
static UNS8 NODE2_EN_SLAVE_RPDO2(uint8_t nodeId)
{
// enable Slave's RPDO
  UNS32 TPDO_COBId = PDO_ENANBLE(0x00000300,nodeId);
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1401, 1, 
    4, uint32, &TPDO_COBId, config_node_param_cb, 0);
}
/******************************心跳时间 设置参数操作*********************************/
static UNS8 NODE2_Write_SLAVE_P_heartbeat(uint8_t nodeId)
{
  UNS16 producer_heartbeat_time = PRODUCER_HEARTBEAT_TIME;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1017, 0, 
    2, uint16, &producer_heartbeat_time, config_node_param_cb, 0);
}
static __attribute__((unused)) UNS8 NODE2_Write_SLAVE_C_heartbeat(uint8_t nodeId)
{
  //需要本地字典写入生产者心跳时间，不然驱动器报错
  UNS32 consumer_heartbeat_time = HEARTBEAT_FORMAT(MASTER_NODEID,CONSUMER_HEARTBEAT_TIME);//写入节点1的心跳时间
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x1016, 1, 
    4, uint16, &consumer_heartbeat_time, config_node_param_cb, 0);
}
/*****************************其他参数操作*********************************/
/**
  * @brief  开启从机S型加减速功能
  * @param  None
  * @retval None
  * @note   None
*/
static __attribute__((unused)) UNS8 NODE2_Write_SLAVE_S_Move(uint8_t nodeId)
{
  extern subindex master402_Index2124;
  S_move = TRUE;
  return writeNetworkDictCallBack(OD_Data, nodeId, 0x2124, 0, 
    master402_Index2124.size, master402_Index2124.bDataType, &S_move, config_node_param_cb, 0);
}
/*****************************结束设置参数操作*********************************/
static UNS8 NODE2_MotorCFG_Done(uint8_t nodeId)
{
	node_config_state *conf;
	conf = &slave_conf;
  rt_sem_release(&(conf->finish_sem));
  return 0;
}
/**
  * @brief  函数指针数组.
  * @param  None.
  * @retval None.
  * @note   
注意函数的顺序和数组越界问题
https://blog.csdn.net/feimeng116/article/details/107515317
*/
static UNS8 (*NODECFG_Operation_2[])(uint8_t nodeId) = 
{ 
  //TPDO1通道操作
  NODE2_DIS_SLAVE_TPDO1,
  NODE2_Write_SLAVE_TPDO1_Type,
  NODE2_Clear_SLAVE_TPDO1_Cnt,
  NODE2_Write_SLAVE_TPDO1_Sub1,
  NODE2_Write_SLAVE_TPDO1_Sub2,
  NODE2_Write_SLAVE_TPDO1_Sub0,
  NODE2_EN_SLAVE_TPDO1,
  //TPDO2通道操作
  NODE2_DIS_SLAVE_TPDO2,
  NODE2_Write_SLAVE_TPDO2_Type,
  NODE2_Clear_SLAVE_TPDO2_Cnt,
  NODE2_Write_SLAVE_TPDO2_Sub1,
  NODE2_Write_SLAVE_TPDO2_Sub0,
  NODE2_EN_SLAVE_TPDO2,
  //RPDO1通道操作
  NODE2_DIS_SLAVE_RPDO1,
  NODE2_Write_SLAVE_RPDO1_Type,
  NODE2_Clear_SLAVE_RPDO1_Cnt,
  NODE2_Write_SLAVE_RPDO1_Sub1,
  NODE2_Write_SLAVE_RPDO1_Sub2,
  NODE2_Write_SLAVE_RPDO1_Sub0,
  NODE2_EN_SLAVE_RPDO1,
  //RPDO2通道操作
  NODE2_DIS_SLAVE_RPDO2,
  NODE2_Write_SLAVE_RPDO2_Type,
  NODE2_Clear_SLAVE_RPDO2_Cnt,
  NODE2_Write_SLAVE_RPDO2_Sub1,
  NODE2_Write_SLAVE_RPDO2_Sub2,
  NODE2_Write_SLAVE_RPDO2_Sub0,
  NODE2_EN_SLAVE_RPDO2,
  //写入心跳
  NODE2_Write_SLAVE_P_heartbeat,
  //结束配置
  NODE2_MotorCFG_Done,
};

/**
  * @brief  配置参数
  * @param  None
  * @retval None
  * @note   None
*/
static void config_node_param(uint8_t nodeId, node_config_state *conf)
{
  if(nodeId == SERVO_NODEID_1)
  {
    //判断数据越界 指针无法判断指向内容大小 https://bbs.csdn.net/topics/80323809
    if(conf->state < sizeof(NODECFG_Operation_2) / sizeof(NODECFG_Operation_2[0]))
    {
      conf->err_code = NODECFG_Operation_2[conf->state](nodeId);
    }
  }
  else
  {
    rt_kprintf("Configuration node %d does not exist\n",nodeId);
  }
  if(conf->err_code != 0)//配置参数错误
  {
    rt_sem_release(&(conf->finish_sem));
    rt_kprintf("Step %d configuration parameters are incorrect\n",conf->state + 1);
    conf->state = 0;
  }
  else
  {
    conf->err_code = NODEID_CONFIG_SUCCESS;
  }
}