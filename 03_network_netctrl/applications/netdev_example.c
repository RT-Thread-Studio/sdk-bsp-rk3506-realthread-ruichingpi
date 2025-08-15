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
#include <string.h>
#include <netdev.h>

#define DEV_NAME "e0"

static int mac_config(void)
{
    uint8_t mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    if (if_set_mac(DEV_NAME, mac) != 0)
    {
        rt_kprintf("Failed to set MAC address\n");
        return -1;
    }
    rt_kprintf("MAC address set successfully\n");

    return 0;
}

static int mac_info(void)
{
    uint8_t mac[6] = {0};
    if (if_get_mac(DEV_NAME, mac) != 0)
    {
        rt_kprintf("Failed to get MAC address\n");
        return -1;
    }
    rt_kprintf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return 0;
}

static int dhcp_enable(void)
{
    if (if_dhcp(DEV_NAME, 1) != 0)
    {
        rt_kprintf("Failed to enable DHCP\n");
        return -1;
    }
    rt_kprintf("DHCP enabled successfully\n");
    return 0;
}

static int dhcp_diable(void)
{
    if (if_dhcp(DEV_NAME, 0) != 0)
    {
        rt_kprintf("Failed to disable DHCP\n");
        return -1;
    }
    rt_kprintf("DHCP disabled successfully\n");

    return 0;
}

static int ip_config(void)
{
    char ip_addr[16] = "192.168.1.100";
    char gw_addr[16] = "192.168.1.1";
    char nm_addr[16] = "255.255.255.0";

    if (if_set_ip(DEV_NAME, ip_addr, gw_addr, nm_addr) != 0)
    {
        rt_kprintf("Failed to set IP address\n");
        return -1;
    }

    rt_kprintf("IP address set successfully: %s\n", ip_addr);
    rt_kprintf("Gateway: %s\n", gw_addr);
    rt_kprintf("Netmask: %s\n", nm_addr);

    return 0;
}

static int ip_info(void)
{
    char ip_addr[16] = {0};
    char gw_addr[16] = {0};
    char nm_addr[16] = {0};

    if (if_get_ip(DEV_NAME, ip_addr, gw_addr, nm_addr) != 0)
    {
        rt_kprintf("Failed to get IP address\n");
        return -1;
    }
    rt_kprintf("IP Address: %s\n", ip_addr);
    rt_kprintf("Gateway: %s\n", gw_addr);
    rt_kprintf("Netmask: %s\n", nm_addr);

    return 0;
}

static int dns_config(void)
{
    char dns_server[16] = "223.5.5.5";
    if (if_set_dns(DEV_NAME, 0, dns_server) != 0)
    {
        rt_kprintf("Failed to set DNS server\n");
        return -1;
    }

    rt_kprintf("DNS server set successfully: %s\n", dns_server);

    return 0;
}

static int dns_info(void)
{
    char dns_server[16] = {0};

    if (if_get_dns(DEV_NAME, 0, dns_server) != 0)
    {
        rt_kprintf("Failed to get DNS server\n");
        return -1;
    }

    rt_kprintf("DNS Server: %s\n", dns_server);

    return 0;
}

static int if_config(char argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage: if <command>\n");
        rt_kprintf("Commands:\n");
        rt_kprintf("  mac_config - Set MAC address\n");
        rt_kprintf("  mac_info - Get MAC address\n");
        rt_kprintf("  dhcp_enable - Enable DHCP\n");
        rt_kprintf("  dhcp_disable - Disable DHCP\n");
        rt_kprintf("  ip_config - Set IP address\n");
        rt_kprintf("  ip_info - Get IP address\n");
        rt_kprintf("  dns_config - Set DNS server\n");
        rt_kprintf("  dns_info - Get DNS server\n");
        return -1;
    }

    if (strcmp(argv[1], "mac_config") == 0)
    {
        return mac_config();
    }
    else if (strcmp(argv[1], "mac_info") == 0)
    {
        return mac_info();
    }
    else if (strcmp(argv[1], "dhcp_enable") == 0)
    {
        return dhcp_enable();
    }
    else if (strcmp(argv[1], "dhcp_disable") == 0)
    {
        return dhcp_diable();
    }
    else if (strcmp(argv[1], "ip_config") == 0)
    {
        return ip_config();
    }
    else if (strcmp(argv[1], "ip_info") == 0)
    {
        return ip_info();
    }
    else if (strcmp(argv[1], "dns_config") == 0)
    {
        return dns_config();
    }
    else if (strcmp(argv[1], "dns_info") == 0)
    {
        return dns_info();
    }

    rt_kprintf("Unknown command: %s\n", argv[1]);

    return -1;
}

MSH_CMD_EXPORT_ALIAS(if_config, if, configure netdev);
