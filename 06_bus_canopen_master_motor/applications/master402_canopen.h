/**
 * @file master402_canopen.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MASTER402_CANOPEN_H
#define __MASTER402_CANOPEN_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "master402_od.h"
/* Exported types ------------------------------------------------------------*/  
/** 
  * @brief  节点ID
  */  
typedef enum
{
  SERVO_NODEID_1 = 0x02,
  SERVO_NODEID_2,
  MAX_NODE_COUNT,         //最大节点数量
}NODEID_NUM;
/** 
  * @brief  节点错误代码
  */  
typedef enum
{
  NODEID_CONFIG_SUCCESS,      //节点配置成功
  NODEID_BACK_ONLINE,         //节点重新上线
  NODEID_CONFIG_NO_RESPOND,   //配置回复未响应，节点字典出错
  NODEID_CONFIG_NO_SEND,      //配置未发送,本地字典出错
}NODEID_ERRCODE;
/* 
 * 节点链表
*/
typedef struct
{
  uint8_t nodeID;
  char name[RT_NAME_MAX];
  /**
   * @brief 电机配置结构体
   * @note  numerator:  运动一圈所需脉冲数
   *        denominator:电机编码器分辨率
   */
  struct
  {
    uint32_t numerator;   //电子齿轮比分子 
    uint32_t denominator; //电子齿轮比分母
  }config;                //电机配置
  e_nodeState *nmt_state;
}node_list;
/* 
 * 节点配置状态结构体
*/
typedef struct 
{
  node_list *list;
  uint8_t state;
  uint8_t try_cnt;
  uint16_t err_code;
  uint8_t errSpec[5];
  struct rt_semaphore finish_sem;
}node_config_state;
/* Exported constants --------------------------------------------------------*/
#define MASTER_NODEID 	  1//控制器ID
#define PDO_TRANSMISSION_TYPE 1//PDO传输类型

#define SDO_REPLY_TIMEOUT     50//等待超时
#define PRODUCER_HEARTBEAT_TIME 500 //生产者心跳间隔
#define CONSUMER_HEARTBEAT_TIME 1000//消费者心跳间隔
/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern CO_Data *OD_Data;
/* Exported functions prototypes ---------------------------------------------*/
extern void config_node(uint8_t nodeId);
extern void canopen_start_thread_entry(void *parameter);
extern UNS8 Write_SLAVE_control_word(UNS8 nodeId,UNS16 value);
extern UNS8 Write_SLAVE_Modes_of_operation(UNS8 nodeId,INTEGER8 mode);
extern UNS8 Write_SLAVE_profile_position_speed_set(UNS8 nodeId,float speed);
extern UNS8 Write_SLAVE_Interpolation_time_period(UNS8 nodeId);
extern UNS8 Write_SLAVE_Homing_set(UNS8 nodeId,UNS32 offset,UNS8 method,float switch_speed,float zero_speed);
/********************节点信息查询与操作****************************************/
extern char *nodeID_get_name(char* des,uint8_t nodeid);
extern e_nodeState nodeID_get_nmt(uint8_t nodeid);
extern rt_err_t nodeID_set_errcode(uint8_t nodeid,uint16_t errcode);
extern rt_err_t nodeID_get_errcode(uint8_t nodeid);
extern rt_err_t nodeID_set_errSpec(uint8_t nodeID,const uint8_t errSpec[5]);
extern char* nodeID_get_errSpec(char* des,uint8_t nodeID);
extern node_config_state *slave_conf_get(NODEID_NUM num);

#ifdef __cplusplus
}
#endif

#endif /* __MASTER402_CANOPEN_H */