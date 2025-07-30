/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "paho_mqtt.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DBG_TAG "example.mqtt"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * MQTT URI farmat:
 * domain mode
 * tcp://broker.emqx.io:1883
 *
 * ipv4 mode
 * tcp://192.168.10.1:1883
 * ssl://192.168.10.1:1884
 *
 * ipv6 mode
 * tcp://[fe80::20c:29ff:fe9a:a07e]:1883
 * ssl://[fe80::20c:29ff:fe9a:a07e]:1884
 * 
 */
#define MQTT_URI      "tcp://broker.emqx.io:1883"
#define MQTT_SUBTOPIC "/mqtt/test/"
#define MQTT_PUBTOPIC "/mqtt/test/"

/* define MQTT client context */
static MQTTClient client;
static void mq_start(void);
static void mq_publish(const char *send_str);

char sup_pub_topic[48] = { 0 };

int mqtt_example(void)
{
    mq_start();
    return 0;
}

static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) =
        '\0';
    LOG_D("Topic: %.*s receive a message: %.*s",
        msg_data->topicName->lenstring.len, msg_data->topicName->lenstring.data,
        msg_data->message->payloadlen, (char *)msg_data->message->payload);

    return;
}

static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) =
        '\0';
    LOG_D("mqtt sub default callback: %.*s %.*s",
        msg_data->topicName->lenstring.len, msg_data->topicName->lenstring.data,
        msg_data->message->payloadlen, (char *)msg_data->message->payload);
    return;
}

static void mqtt_connect_callback(MQTTClient *c)
{
    LOG_I("Start to connect mqtt server");
}

static void mqtt_online_callback(MQTTClient *c)
{
    LOG_D("Connect mqtt server success");
    LOG_D("Publish message: Hello,RT-Thread! to topic: %s", sup_pub_topic);
    mq_publish("Hello,RT-Thread!");
}

static void mqtt_offline_callback(MQTTClient *c)
{
    LOG_I("Disconnect from mqtt server");
}

static void mq_start(void)
{
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;
    static char cid[20] = { 0 };

    static int is_started = 0;
    if (is_started)
    {
        return;
    }

    {
        client.isconnected = 0;
        client.uri = MQTT_URI;

        rt_snprintf(cid, sizeof(cid), "rtthread%d", rt_tick_get());
        rt_snprintf(
            sup_pub_topic, sizeof(sup_pub_topic), "%s%s", MQTT_PUBTOPIC, cid);

        memcpy(&client.condata, &condata, sizeof(condata));
        client.condata.clientID.cstring = cid;
        client.condata.keepAliveInterval = 60;
        client.condata.cleansession = 1;
        client.condata.username.cstring = "";
        client.condata.password.cstring = "";

        client.condata.willFlag = 0;
        client.condata.will.qos = 1;
        client.condata.will.retained = 0;
        client.condata.will.topicName.cstring = sup_pub_topic;

        client.buf_size = client.readbuf_size = 1024;
        client.buf = malloc(client.buf_size);
        client.readbuf = malloc(client.readbuf_size);
        if (!(client.buf && client.readbuf))
        {
            LOG_E("no memory for MQTT client buffer!");
            goto _exit;
        }

        client.connect_callback = mqtt_connect_callback;
        client.online_callback = mqtt_online_callback;
        client.offline_callback = mqtt_offline_callback;

        client.messageHandlers[0].topicFilter = sup_pub_topic;
        client.messageHandlers[0].callback = mqtt_sub_callback;
        client.messageHandlers[0].qos = QOS1;

        client.defaultMessageHandler = mqtt_sub_default_callback;
    }

    LOG_D("Start mqtt client and subscribe topic:%s", sup_pub_topic);
    paho_mqtt_start(&client);
    is_started = 1;

_exit:
    return;
}

static void mq_publish(const char *send_str)
{
    MQTTMessage message;
    const char *msg_str = send_str;
    const char *topic = sup_pub_topic;
    message.qos = QOS1;
    message.retained = 0;
    message.payload = (void *)msg_str;
    message.payloadlen = strlen(message.payload);

    MQTTPublish(&client, topic, &message);

    return;
}

static void msh_mq_publish(int argc, char *argv[])
{
    char send_buff[100] = { '\0' };
    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
        {
            strcat(send_buff, " ");
        }
        strcat(send_buff, argv[i]);
    }
    mq_publish(send_buff);
}

MSH_CMD_EXPORT(msh_mq_publish, publish messege by msh);
MSH_CMD_EXPORT(mqtt_example, mqtt example);
