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
#include <string.h>
#include "open62541.h"
#include <netdev.h>

static void updateCurrentTime(UA_Server* server)
{
    UA_DateTime now = UA_DateTime_now();
    UA_Variant value;
    UA_Variant_setScalar(&value, &now, &UA_TYPES[UA_TYPES_DATETIME]);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time-value-callback");
    UA_Server_writeValue(server, currentNodeId, value);
}

static void addCurrentTimeVariable(UA_Server* server)
{
    UA_DateTime now = 0;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Current time - value callback");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&attr.value, &now, &UA_TYPES[UA_TYPES_DATETIME]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time-value-callback");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-time-value-callback");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr, NULL, NULL);

    updateCurrentTime(server);
}

static void beforeReadTime(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeid, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data)
{
    updateCurrentTime(server);
}

static void afterWriteTime(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "The variable was updated");
}

static void addValueCallbackToCurrentTimeVariable(UA_Server* server)
{
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time-value-callback");
    UA_ValueCallback callback;
    callback.onRead = beforeReadTime;
    callback.onWrite = afterWriteTime;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

static UA_StatusCode readCurrentTime(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    UA_Boolean sourceTimeStamp, const UA_NumericRange* range,
    UA_DataValue* dataValue)
{
    UA_DateTime now = UA_DateTime_now();
    UA_Variant_setScalarCopy(&dataValue->value, &now,
        &UA_TYPES[UA_TYPES_DATETIME]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeCurrentTime(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Changing the system time is not implemented");
    return UA_STATUSCODE_BADINTERNALERROR;
}

static void addCurrentTimeDataSourceVariable(UA_Server* server)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Current time - data source");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time-datasource");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-time-datasource");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource timeDataSource;
    timeDataSource.read = readCurrentTime;
    timeDataSource.write = writeCurrentTime;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr,
        timeDataSource, NULL, NULL);
}

static UA_DataValue* externalValue;

static void addCurrentTimeExternalDataSource(UA_Server* server)
{
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time-external-source");

    UA_ValueBackend valueBackend;
    valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
    valueBackend.backend.external.value = &externalValue;

    UA_Server_setVariableNode_valueBackend(server, currentNodeId, valueBackend);
}

void server_demo(char *ip)
{
    UA_Boolean running = true;

    rt_kprintf("opc ua server ip :%s port:4840\n", ip);

    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    // UA_ServerConfig_setCustomHostname(UA_Server_getConfig(server), UA_STRING(server_ip));
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_String_clear(&config->customHostname);
    config->customHostname = UA_String_fromChars(ip);

    addCurrentTimeVariable(server);
    addValueCallbackToCurrentTimeVariable(server);
    addCurrentTimeDataSourceVariable(server);

    addCurrentTimeExternalDataSource(server);

    UA_Server_run(server, &running);

    UA_Server_delete(server);
}

static void opc_ua_server_entry(void* ip)
{
    UA_Logger *logger = (UA_Logger *)UA_Log_Stdout;
    logger->context = (void*)UA_LOGLEVEL_WARNING;

    server_demo(ip);

    while (1)
    {
        rt_thread_mdelay(1000);
    }
}

int open62541_server(int argc, char** argv)
{
    rt_thread_t tid = RT_NULL;
    static char ip_addr[16] = {0};
    char gw_addr[16] = {0};
    char nm_addr[16] = {0};

    if (argc != 2)
    {
        rt_kprintf("open62541_server [net dev name]\n");
        return -RT_ERROR;
    }

    if (if_get_ip(argv[1], ip_addr, gw_addr, nm_addr) != 0)
    {
        rt_kprintf("Failed to get IP address\n");
        return -1;
    }

    tid = rt_thread_create("open62541", opc_ua_server_entry, ip_addr, 102400, 25, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("rt_thread_create: failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(open62541_server, open62541_server);

int open62541_client(int argc, char **argv)
{
    char ip_data[128] = {0};

    if (argc != 3)
    {
        rt_kprintf("open62541_client [ip] [port]\n");
        return -RT_ERROR;
    }
    rt_sprintf(ip_data,"opc.tcp://%s:%s", argv[1], argv[2]);

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    UA_StatusCode retval = UA_Client_connect(client, ip_data);
    if(retval != UA_STATUSCODE_GOOD)
    {
        UA_Client_delete(client);
        return (int)retval;
    }

    /*read the value attribute of the node.UA_Client_readValueAttribute is a wrapper for the
    raw read service available as UA_Client_Service_read.*/
    UA_Variant value;
    UA_Variant_init(&value);

    UA_NodeId testvar = UA_NODEID_STRING(1,"the.answer");
    retval = UA_Client_readValueAttribute(client, testvar, &value);

    if(retval == UA_STATUSCODE_GOOD)
    {
        UA_Int32 *p = (UA_Int32 *)value.data;
        rt_kprintf("[%s] = [%d] \n", testvar.identifier.byteString.data, *p);
    }
    else
    {
        rt_kprintf("read [%s] failed\n", testvar.identifier.byteString.data);
    }

    /*Clean up */
    UA_Variant_clear(&value);
    UA_Client_delete(client);

    return EXIT_SUCCESS;
}
MSH_CMD_EXPORT(open62541_client, open62541_client);
