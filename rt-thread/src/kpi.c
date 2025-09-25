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
#include <dirent.h>
#include <kpi.h>
#include <newlib_compiler.h>
#include <posix/stdlib.h>
#include <service.h>
#include <sys/select.h>

#ifdef RT_USING_NETDEV
#include <netdev.h>
#endif /* RT_USING_NETDEV */

#ifdef RT_USING_SAL
#include <netdb.h>
#include <sys/socket.h>
#endif /* RT_USING_SAL */

#ifdef RT_USING_POSIX_STDIO
#include <posix/stdio.h>
#endif /* RT_USING_POSIX_STDIO */

#ifdef RT_USING_POSIX_TERMIOS
#include <termios.h>
#endif /* RT_USING_POSIX_TERMIOS */

extern rt_uint8_t __kpi_start[];

/* kpi defined */
KPI_DEFINED(rt_thread_init);
KPI_DEFINED(rt_thread_detach);
KPI_DEFINED(rt_thread_create);
KPI_DEFINED(rt_thread_delete);
KPI_DEFINED(rt_thread_close);
KPI_DEFINED(rt_thread_self);
KPI_DEFINED(rt_thread_find);
KPI_DEFINED(rt_thread_startup);
KPI_DEFINED(rt_thread_yield);
KPI_DEFINED(rt_thread_delay);
KPI_DEFINED(rt_thread_delay_until);
KPI_DEFINED(rt_thread_mdelay);
KPI_DEFINED(rt_thread_control);
KPI_DEFINED(rt_thread_suspend);
KPI_DEFINED(rt_thread_suspend_with_flag);
KPI_DEFINED(rt_thread_resume);
KPI_DEFINED(rt_thread_get_name);
KPI_DEFINED(rt_thread_idle_sethook);
KPI_DEFINED(rt_thread_idle_delhook);
KPI_DEFINED(rt_thread_idle_gethandler);
KPI_DEFINED(rt_enter_critical);
KPI_DEFINED(rt_exit_critical);
KPI_DEFINED(rt_exit_critical_safe);
KPI_DEFINED(rt_critical_level);
KPI_DEFINED(rt_schedule);

KPI_DEFINED(rt_sem_init);
KPI_DEFINED(rt_sem_detach);
KPI_DEFINED(rt_sem_create);
KPI_DEFINED(rt_sem_delete);
KPI_DEFINED(rt_sem_take);
KPI_DEFINED(rt_sem_take_interruptible);
KPI_DEFINED(rt_sem_take_killable);
KPI_DEFINED(rt_sem_trytake);
KPI_DEFINED(rt_sem_release);
KPI_DEFINED(rt_sem_control);

KPI_DEFINED(rt_mutex_init);
KPI_DEFINED(rt_mutex_detach);
KPI_DEFINED(rt_mutex_create);
KPI_DEFINED(rt_mutex_delete);
KPI_DEFINED(rt_mutex_drop_thread);
KPI_DEFINED(rt_mutex_setprioceiling);
KPI_DEFINED(rt_mutex_getprioceiling);
KPI_DEFINED(rt_mutex_take);
KPI_DEFINED(rt_mutex_trytake);
KPI_DEFINED(rt_mutex_take_interruptible);
KPI_DEFINED(rt_mutex_take_killable);
KPI_DEFINED(rt_mutex_release);
KPI_DEFINED(rt_mutex_control);

KPI_DEFINED(rt_event_init);
KPI_DEFINED(rt_event_detach);
KPI_DEFINED(rt_event_create);
KPI_DEFINED(rt_event_delete);
KPI_DEFINED(rt_event_send);
KPI_DEFINED(rt_event_recv);
KPI_DEFINED(rt_event_recv_interruptible);
KPI_DEFINED(rt_event_recv_killable);
KPI_DEFINED(rt_event_control);

KPI_DEFINED(rt_mb_init);
KPI_DEFINED(rt_mb_detach);
KPI_DEFINED(rt_mb_create);
KPI_DEFINED(rt_mb_delete);
KPI_DEFINED(rt_mb_send);
KPI_DEFINED(rt_mb_send_interruptible);
KPI_DEFINED(rt_mb_send_killable);
KPI_DEFINED(rt_mb_send_wait);
KPI_DEFINED(rt_mb_send_wait_interruptible);
KPI_DEFINED(rt_mb_send_wait_killable);
KPI_DEFINED(rt_mb_urgent);
KPI_DEFINED(rt_mb_recv);
KPI_DEFINED(rt_mb_recv_interruptible);
KPI_DEFINED(rt_mb_recv_killable);
KPI_DEFINED(rt_mb_control);

KPI_DEFINED(rt_mq_init);
KPI_DEFINED(rt_mq_detach);
KPI_DEFINED(rt_mq_create);
KPI_DEFINED(rt_mq_delete);
KPI_DEFINED(rt_mq_send);
KPI_DEFINED(rt_mq_send_interruptible);
KPI_DEFINED(rt_mq_send_killable);
KPI_DEFINED(rt_mq_send_wait);
KPI_DEFINED(rt_mq_send_wait_interruptible);
KPI_DEFINED(rt_mq_send_wait_killable);
KPI_DEFINED(rt_mq_urgent);
KPI_DEFINED(rt_mq_recv);
KPI_DEFINED(rt_mq_recv_interruptible);
KPI_DEFINED(rt_mq_recv_killable);
KPI_DEFINED(rt_mq_control);
KPI_DEFINED(rt_mq_send_wait_prio);
KPI_DEFINED(rt_mq_recv_prio);

KPI_DEFINED(rt_tick_get);
KPI_DEFINED(rt_tick_set);
KPI_DEFINED(rt_tick_from_millisecond);
KPI_DEFINED(rt_tick_get_millisecond_r);
KPI_DEFINED(rt_timer_init);
KPI_DEFINED(rt_timer_detach);
KPI_DEFINED(rt_timer_create);
KPI_DEFINED(rt_timer_delete);
KPI_DEFINED(rt_timer_start);
KPI_DEFINED(rt_timer_stop);
KPI_DEFINED(rt_timer_control);

KPI_DEFINED(rt_kprintf);
KPI_DEFINED(rt_kputs);
KPI_DEFINED(rt_malloc);
KPI_DEFINED(rt_free);
KPI_DEFINED(rt_realloc);
KPI_DEFINED(rt_calloc);
KPI_DEFINED(rt_malloc_align);
KPI_DEFINED(rt_free_align);
KPI_DEFINED(rt_get_errno);
KPI_DEFINED(rt_set_errno);
KPI_DEFINED(_rt_errno);
KPI_DEFINED(rt_strerror);
KPI_DEFINED(rt_vsprintf);
KPI_DEFINED(rt_vsnprintf);
KPI_DEFINED(rt_sprintf);
KPI_DEFINED(rt_snprintf);
KPI_DEFINED(rt_vsscanf);
KPI_DEFINED(rt_sscanf);
KPI_DEFINED(rt_memset);
KPI_DEFINED(rt_memcpy);
KPI_DEFINED(rt_memmove);
KPI_DEFINED(rt_memcmp);
KPI_DEFINED(rt_strdup);
KPI_DEFINED(rt_strnlen);
KPI_DEFINED(rt_strstr);
KPI_DEFINED(rt_strcasecmp);
KPI_DEFINED(rt_strcpy);
KPI_DEFINED(rt_strncpy);
KPI_DEFINED(rt_strncmp);
KPI_DEFINED(rt_strcmp);
KPI_DEFINED(rt_strlen);
KPI_DEFINED(rt_assert_handler);

KPI_DEFINED(rt_atomic_load);
KPI_DEFINED(rt_atomic_store);
KPI_DEFINED(rt_atomic_add);
KPI_DEFINED(rt_atomic_sub);
KPI_DEFINED(rt_atomic_and);
KPI_DEFINED(rt_atomic_or);
KPI_DEFINED(rt_atomic_xor);
KPI_DEFINED(rt_atomic_exchange);
KPI_DEFINED(rt_atomic_flag_clear);
KPI_DEFINED(rt_atomic_flag_test_and_set);
KPI_DEFINED(rt_atomic_compare_exchange_strong);

KPI_DEFINED(rt_device_find);
KPI_DEFINED(rt_device_register);
KPI_DEFINED(rt_device_unregister);
KPI_DEFINED(rt_device_create);
KPI_DEFINED(rt_device_destroy);
KPI_DEFINED(rt_device_set_rx_indicate);
KPI_DEFINED(rt_device_set_tx_complete);
KPI_DEFINED(rt_device_init);
KPI_DEFINED(rt_device_open);
KPI_DEFINED(rt_device_close);
KPI_DEFINED(rt_device_read);
KPI_DEFINED(rt_device_write);
KPI_DEFINED(rt_device_control);

KPI_DEFINED(rt_hw_cpu_icache_enable);
KPI_DEFINED(rt_hw_cpu_icache_disable);
KPI_DEFINED(rt_hw_cpu_icache_ops);
KPI_DEFINED(rt_hw_cpu_icache_status);
KPI_DEFINED(rt_hw_cpu_dcache_enable);
KPI_DEFINED(rt_hw_cpu_dcache_disable);
KPI_DEFINED(rt_hw_cpu_dcache_status);
KPI_DEFINED(rt_hw_cpu_dcache_ops);
KPI_DEFINED(rt_hw_isb);
KPI_DEFINED(rt_hw_dmb);
KPI_DEFINED(rt_hw_dsb);
KPI_DEFINED(rt_hw_cpu_relax);
KPI_DEFINED(rt_hw_rmb);

KPI_DEFINED(rt_hw_cpu_reset);
KPI_DEFINED(rt_hw_cpu_shutdown);
KPI_DEFINED(rt_hw_cpu_arch);
KPI_DEFINED(rt_hw_cpu_id);

KPI_DEFINED(rt_hw_local_irq_disable);
KPI_DEFINED(rt_hw_local_irq_enable);
KPI_DEFINED(rt_hw_interrupt_disable);
KPI_DEFINED(rt_hw_interrupt_enable);
KPI_DEFINED(rt_hw_interrupt_is_disabled);
KPI_DEFINED(rt_interrupt_enter);
KPI_DEFINED(rt_interrupt_leave);
KPI_DEFINED(rt_interrupt_get_nest);
KPI_DEFINED(rt_hw_ipi_send);
KPI_DEFINED(rt_hw_ipi_handler_install);
KPI_DEFINED(rt_hw_interrupt_mask);
KPI_DEFINED(rt_hw_interrupt_umask);
KPI_DEFINED(rt_hw_interrupt_install);
KPI_DEFINED(rt_hw_interrupt_uninstall);
KPI_DEFINED(rt_hw_exception_install);

KPI_DEFINED(rt_spin_lock_init);
KPI_DEFINED(rt_spin_lock);
KPI_DEFINED(rt_spin_unlock);
KPI_DEFINED(rt_spin_lock_irqsave);
KPI_DEFINED(rt_spin_unlock_irqrestore);

KPI_DEFINED(rt_hw_us_delay);
KPI_DEFINED(rt_hw_global_timer_get);

KPI_DEFINED(__rt_libc_exit);
#ifdef RT_USING_POSIX_STDIO
KPI_DEFINED(rt_posix_stdio_get_console);
KPI_DEFINED(rt_posix_stdio_set_console);
#endif /* RT_USING_POSIX_STDIO */

KPI_DEFINED(rt_creat);
KPI_DEFINED(rt_open);
KPI_DEFINED(rt_read);
KPI_DEFINED(rt_write);
KPI_DEFINED(rt_close);
KPI_DEFINED(ioctl);
KPI_DEFINED(rt_lseek);
KPI_DEFINED(rt_fstat);
KPI_DEFINED(rt_unlink);
KPI_DEFINED(rt_rename);
KPI_DEFINED(rt_mkdir);
KPI_DEFINED(rt_stat);

#ifdef RT_USING_POSIX_TERMIOS
KPI_DEFINED(cfgetospeed);
KPI_DEFINED(cfgetispeed);
KPI_DEFINED(cfsetospeed);
KPI_DEFINED(cfsetispeed);
KPI_DEFINED(tcgetattr);
KPI_DEFINED(tcsetattr);
KPI_DEFINED(tcsendbreak);
KPI_DEFINED(tcdrain);
KPI_DEFINED(tcflush);
KPI_DEFINED(tcflow);
KPI_DEFINED(tcgetsid);
KPI_DEFINED(cfmakeraw);
KPI_DEFINED(cfsetspeed);
#endif /* RT_USING_POSIX_TERMIOS */

#ifdef RT_USING_SAL
KPI_DEFINED(select);
KPI_DEFINED(accept);
KPI_DEFINED(bind);
KPI_DEFINED(shutdown);
KPI_DEFINED(getpeername);
KPI_DEFINED(getsockname);
KPI_DEFINED(getsockopt);
KPI_DEFINED(setsockopt);
KPI_DEFINED(connect);
KPI_DEFINED(listen);
KPI_DEFINED(recv);
KPI_DEFINED(recvfrom);
KPI_DEFINED(recvmsg);
KPI_DEFINED(sendmsg);
KPI_DEFINED(send);
KPI_DEFINED(sendto);
KPI_DEFINED(socket);
KPI_DEFINED(closesocket);
KPI_DEFINED(ioctlsocket);
KPI_DEFINED(socketpair);
KPI_DEFINED(gethostbyname);
KPI_DEFINED(gethostbyname_r);
KPI_DEFINED(freeaddrinfo);
KPI_DEFINED(getaddrinfo);
#endif /* RT_USING_SAL */

KPI_DEFINED(rt_opendir);
KPI_DEFINED(rt_readdir);
KPI_DEFINED(rt_closedir);
KPI_DEFINED(fsync);
KPI_DEFINED(rt_fcntl);
KPI_DEFINED(ftruncate);
KPI_DEFINED(rt_statfs);
KPI_DEFINED(rt_fstatfs);
KPI_DEFINED(rmdir);
KPI_DEFINED(rt_telldir);
KPI_DEFINED(rt_seekdir);
KPI_DEFINED(rt_rewinddir);
KPI_DEFINED(access);
KPI_DEFINED(setcwd);
KPI_DEFINED(getcwd);

#ifdef RT_USING_NETDEV
KPI_DEFINED(if_set_mac);
KPI_DEFINED(if_get_mac);
KPI_DEFINED(if_set_dns);
KPI_DEFINED(if_get_dns);
KPI_DEFINED(if_dhcp);
KPI_DEFINED(if_set_ip);
KPI_DEFINED(if_get_ip);
KPI_DEFINED(if_up);
KPI_DEFINED(if_down);
#endif /* RT_USING_NETDEV */

KPI_DEFINED(service_find);
KPI_DEFINED(service_register);
KPI_DEFINED(service_unregister);
KPI_DEFINED(service_create);
KPI_DEFINED(service_destroy);
KPI_DEFINED(service_open);
KPI_DEFINED(service_close);
KPI_DEFINED(service_read);
KPI_DEFINED(service_write);
KPI_DEFINED(service_control);
KPI_DEFINED(service_set_rx_indicate);
KPI_DEFINED(service_set_tx_complete);
KPI_DEFINED(service_lock);
KPI_DEFINED(service_unlock);

KPI_DEFINED(rt_bus_for_each_dev);
KPI_DEFINED(rt_bus_for_each_drv);
KPI_DEFINED(rt_bus_add_driver);
KPI_DEFINED(rt_bus_add_device);
KPI_DEFINED(rt_bus_remove_driver);
KPI_DEFINED(rt_bus_remove_device);
KPI_DEFINED(rt_bus_shutdown);
KPI_DEFINED(rt_bus_find_by_name);
KPI_DEFINED(rt_bus_reload_driver_device);
KPI_DEFINED(rt_bus_register);

KPI_DEFINED(rt_dm_ida_alloc);
KPI_DEFINED(rt_dm_ida_take);
KPI_DEFINED(rt_dm_ida_free);
KPI_DEFINED(rt_dm_device_find);
KPI_DEFINED(rt_dm_dev_set_name_auto);
KPI_DEFINED(rt_dm_dev_get_name_id);
KPI_DEFINED(rt_dm_dev_set_name);
KPI_DEFINED(rt_dm_dev_get_name);
KPI_DEFINED(rt_dm_dev_get_address_count);
KPI_DEFINED(rt_dm_dev_get_address);
KPI_DEFINED(rt_dm_dev_get_address_by_name);
KPI_DEFINED(rt_dm_dev_get_address_array);
KPI_DEFINED(rt_dm_dev_iomap);
KPI_DEFINED(rt_dm_dev_iomap_by_name);
KPI_DEFINED(rt_dm_dev_get_irq_count);
KPI_DEFINED(rt_dm_dev_get_irq);
KPI_DEFINED(rt_dm_dev_get_irq_by_name);
KPI_DEFINED(rt_dm_dev_bind_fwdata);
KPI_DEFINED(rt_dm_dev_unbind_fwdata);
KPI_DEFINED(rt_dm_dev_prop_read_u8_array_index);
KPI_DEFINED(rt_dm_dev_prop_read_u16_array_index);
KPI_DEFINED(rt_dm_dev_prop_read_u32_array_index);
KPI_DEFINED(rt_dm_dev_prop_read_u64_array_index);
KPI_DEFINED(rt_dm_dev_prop_read_string_array_index);
KPI_DEFINED(rt_dm_dev_prop_count_of_size);
KPI_DEFINED(rt_dm_dev_prop_index_of_string);
KPI_DEFINED(rt_dm_dev_prop_read_bool);

KPI_DEFINED(rt_platform_device_alloc);
KPI_DEFINED(rt_platform_driver_register);
KPI_DEFINED(rt_platform_device_register);
KPI_DEFINED(rt_platform_ofw_device_probe_child);
KPI_DEFINED(rt_platform_ofw_request);
KPI_DEFINED(rt_platform_ofw_free);

KPI_DEFINED(rt_ofw_bus_addr_cells);
KPI_DEFINED(rt_ofw_bus_size_cells);
KPI_DEFINED(rt_ofw_io_addr_cells);
KPI_DEFINED(rt_ofw_io_size_cells);
KPI_DEFINED(rt_ofw_get_address_count);
KPI_DEFINED(rt_ofw_get_address);
KPI_DEFINED(rt_ofw_get_address_by_name);
KPI_DEFINED(rt_ofw_get_address_array);
KPI_DEFINED(rt_ofw_translate_address);
KPI_DEFINED(rt_ofw_reverse_address);
KPI_DEFINED(rt_ofw_iomap);
KPI_DEFINED(rt_ofw_iomap_by_name);

KPI_DEFINED(rt_ofw_irq_cells);
KPI_DEFINED(rt_ofw_parse_irq_map);
KPI_DEFINED(rt_ofw_parse_irq_cells);
KPI_DEFINED(rt_ofw_find_irq_parent);
KPI_DEFINED(rt_ofw_map_irq);
KPI_DEFINED(rt_ofw_get_irq_count);
KPI_DEFINED(rt_ofw_get_irq);
KPI_DEFINED(rt_ofw_get_irq_by_name);

KPI_DEFINED(rt_ofw_node_destroy);
KPI_DEFINED(rt_ofw_node_get);
KPI_DEFINED(rt_ofw_node_put);
KPI_DEFINED(rt_ofw_node_tag_equ);
KPI_DEFINED(rt_ofw_node_tag_prefix);
KPI_DEFINED(rt_ofw_machine_is_compatible);
KPI_DEFINED(rt_ofw_node_is_available);
KPI_DEFINED(rt_ofw_node_is_compatible);
KPI_DEFINED(rt_ofw_prop_match);
KPI_DEFINED(rt_ofw_node_match);
KPI_DEFINED(rt_ofw_find_node_by_tag);
KPI_DEFINED(rt_ofw_find_node_by_prop_r);
KPI_DEFINED(rt_ofw_find_node_by_name);
KPI_DEFINED(rt_ofw_find_node_by_type);
KPI_DEFINED(rt_ofw_find_node_by_compatible);
KPI_DEFINED(rt_ofw_find_node_by_ids_r);
KPI_DEFINED(rt_ofw_find_node_by_path);
KPI_DEFINED(rt_ofw_find_node_by_phandle);
KPI_DEFINED(rt_ofw_get_parent);
KPI_DEFINED(rt_ofw_get_child_by_tag);
KPI_DEFINED(rt_ofw_get_child_by_compatible);
KPI_DEFINED(rt_ofw_get_child_count);
KPI_DEFINED(rt_ofw_get_available_child_count);
KPI_DEFINED(rt_ofw_get_next_node);
KPI_DEFINED(rt_ofw_get_next_parent);
KPI_DEFINED(rt_ofw_get_next_child);
KPI_DEFINED(rt_ofw_get_next_available_child);
KPI_DEFINED(rt_ofw_get_cpu_node);
KPI_DEFINED(rt_ofw_get_next_cpu_node);
KPI_DEFINED(rt_ofw_get_cpu_state_node);
KPI_DEFINED(rt_ofw_get_cpu_id);
KPI_DEFINED(rt_ofw_get_cpu_hwid);
KPI_DEFINED(rt_ofw_get_alias_node);
KPI_DEFINED(rt_ofw_get_alias_id);
KPI_DEFINED(rt_ofw_get_alias_last_id);
KPI_DEFINED(rt_ofw_map_id);
KPI_DEFINED(rt_ofw_append_child);
KPI_DEFINED(rt_ofw_append_prop);
KPI_DEFINED(rt_ofw_parse_phandle);
KPI_DEFINED(rt_ofw_parse_phandle_cells);
KPI_DEFINED(rt_ofw_count_phandle_cells);
KPI_DEFINED(rt_ofw_get_prop_fuzzy_name);
KPI_DEFINED(rt_ofw_get_prop);
KPI_DEFINED(rt_ofw_prop_read_u8_array_index);
KPI_DEFINED(rt_ofw_prop_read_u16_array_index);
KPI_DEFINED(rt_ofw_prop_read_u32_array_index);
KPI_DEFINED(rt_ofw_prop_read_u64_array_index);
KPI_DEFINED(rt_ofw_prop_read_string_array_index);
KPI_DEFINED(rt_ofw_prop_count_of_size);
KPI_DEFINED(rt_ofw_prop_index_of_string);
KPI_DEFINED(rt_ofw_prop_next_u32);
KPI_DEFINED(rt_ofw_prop_next_string);
KPI_DEFINED(rt_ofw_parse_object);
KPI_DEFINED(rt_ofw_console_setup);
KPI_DEFINED(rt_ofw_bootargs_select);

KPI_DEFINED(rt_pic_attach_irq);
KPI_DEFINED(rt_pic_detach_irq);
KPI_DEFINED(rt_pic_irq_enable);
KPI_DEFINED(rt_pic_irq_disable);
KPI_DEFINED(rt_pic_irq_ack);
KPI_DEFINED(rt_pic_irq_mask);
KPI_DEFINED(rt_pic_irq_unmask);
KPI_DEFINED(rt_pic_irq_eoi);
KPI_DEFINED(rt_pic_irq_set_priority);
KPI_DEFINED(rt_pic_irq_get_priority);
KPI_DEFINED(rt_pic_irq_set_affinity);
KPI_DEFINED(rt_pic_irq_get_affinity);
KPI_DEFINED(rt_pic_irq_set_triger_mode);
KPI_DEFINED(rt_pic_irq_get_triger_mode);
KPI_DEFINED(rt_pic_irq_send_ipi);

KPI_DEFINED(rt_clk_find);
KPI_DEFINED(rt_clk_find_by_id);
KPI_DEFINED(rt_clk_find_by_index);
KPI_DEFINED(rt_clk_find_array);
KPI_DEFINED(rt_clk_create);
KPI_DEFINED(rt_clk_delete);
KPI_DEFINED(rt_clk_register);
KPI_DEFINED(rt_clk_unregister);
KPI_DEFINED(rt_clk_get_name);
KPI_DEFINED(rt_clk_set_parent);
KPI_DEFINED(rt_clk_get_parent);
KPI_DEFINED(rt_clk_enable);
KPI_DEFINED(rt_clk_disable);
KPI_DEFINED(rt_clk_get_status);
KPI_DEFINED(rt_clk_set_rate);
KPI_DEFINED(rt_clk_get_rate);
KPI_DEFINED(rt_clk_round_rate);
KPI_DEFINED(rt_clk_prepare);
KPI_DEFINED(rt_clk_unprepare);
KPI_DEFINED(rt_clk_prepare_enable);
KPI_DEFINED(rt_clk_disable_unprepare);
KPI_DEFINED(rt_clk_array_prepare);
KPI_DEFINED(rt_clk_array_unprepare);
KPI_DEFINED(rt_clk_array_enable);
KPI_DEFINED(rt_clk_array_disable);
KPI_DEFINED(rt_clk_array_prepare_enable);
KPI_DEFINED(rt_clk_array_disable_unprepare);
KPI_DEFINED(rt_clk_put);
KPI_DEFINED(rt_ofw_get_clk_by_name);
KPI_DEFINED(rt_clk_fixed_rate_register);
KPI_DEFINED(rt_clk_fixed_rate_unregister);
KPI_DEFINED(rt_clk_fixed_factor_register);
KPI_DEFINED(rt_clk_fixed_factor_unregister);
KPI_DEFINED(of_clk_add_provider);
KPI_DEFINED(rt_clk_get_by_name);

KPI_DEFINED(rt_completion_init);
KPI_DEFINED(rt_completion_wait);
KPI_DEFINED(rt_completion_wait_noisr);
KPI_DEFINED(rt_completion_wait_flags);
KPI_DEFINED(rt_completion_wait_flags_noisr);
KPI_DEFINED(rt_completion_done);
KPI_DEFINED(rt_completion_wakeup);
KPI_DEFINED(rt_completion_wakeup_by_errno);

KPI_DEFINED(rt_condvar_init);
KPI_DEFINED(rt_condvar_timedwait);
KPI_DEFINED(rt_condvar_signal);
KPI_DEFINED(rt_condvar_broadcast);

KPI_DEFINED(rt_data_queue_init);
KPI_DEFINED(rt_data_queue_push);
KPI_DEFINED(rt_data_queue_pop);
KPI_DEFINED(rt_data_queue_peek);
KPI_DEFINED(rt_data_queue_reset);
KPI_DEFINED(rt_data_queue_deinit);
KPI_DEFINED(rt_data_queue_len);

KPI_DEFINED(rt_pipe_create);
KPI_DEFINED(rt_pipe_open);
KPI_DEFINED(rt_pipe_read);
KPI_DEFINED(rt_pipe_write);
KPI_DEFINED(rt_pipe_control);
KPI_DEFINED(rt_pipe_close);
KPI_DEFINED(rt_pipe_delete);

KPI_DEFINED(rt_rbb_init);
KPI_DEFINED(rt_rbb_get_buf_size);
KPI_DEFINED(rt_rbb_create);
KPI_DEFINED(rt_rbb_destroy);
KPI_DEFINED(rt_rbb_blk_alloc);
KPI_DEFINED(rt_rbb_blk_put);
KPI_DEFINED(rt_rbb_blk_get);
KPI_DEFINED(rt_rbb_blk_size);
KPI_DEFINED(rt_rbb_blk_buf);
KPI_DEFINED(rt_rbb_blk_free);
KPI_DEFINED(rt_rbb_blk_queue_get);
KPI_DEFINED(rt_rbb_blk_queue_len);
KPI_DEFINED(rt_rbb_blk_queue_buf);
KPI_DEFINED(rt_rbb_blk_queue_free);
KPI_DEFINED(rt_rbb_next_blk_queue_len);

KPI_DEFINED(rt_ringbuffer_init);
KPI_DEFINED(rt_ringbuffer_reset);
KPI_DEFINED(rt_ringbuffer_put);
KPI_DEFINED(rt_ringbuffer_put_force);
KPI_DEFINED(rt_ringbuffer_putchar);
KPI_DEFINED(rt_ringbuffer_putchar_force);
KPI_DEFINED(rt_ringbuffer_get);
KPI_DEFINED(rt_ringbuffer_peek);
KPI_DEFINED(rt_ringbuffer_getchar);
KPI_DEFINED(rt_ringbuffer_data_len);
KPI_DEFINED(rt_ringbuffer_create);
KPI_DEFINED(rt_ringbuffer_destroy);

KPI_DEFINED(__wqueue_default_wake);
KPI_DEFINED(rt_wqueue_add);
KPI_DEFINED(rt_wqueue_remove);
KPI_DEFINED(rt_wqueue_wait);
KPI_DEFINED(rt_wqueue_wait_killable);
KPI_DEFINED(rt_wqueue_wait_interruptible);
KPI_DEFINED(rt_wqueue_wakeup);
KPI_DEFINED(rt_wqueue_wakeup_all);

KPI_DEFINED(rt_work_init);
KPI_DEFINED(rt_workqueue_create);
KPI_DEFINED(rt_workqueue_destroy);
KPI_DEFINED(rt_workqueue_dowork);
KPI_DEFINED(rt_workqueue_submit_work);
KPI_DEFINED(rt_workqueue_cancel_work);
KPI_DEFINED(rt_workqueue_cancel_work_sync);
KPI_DEFINED(rt_workqueue_cancel_all_work);
KPI_DEFINED(rt_workqueue_urgent_work);

KPI_DEFINED(rt_work_submit);
KPI_DEFINED(rt_work_urgent);
KPI_DEFINED(rt_work_cancel);

KPI_DEFINED(rt_i2c_driver_register);
KPI_DEFINED(rt_i2c_device_register);
KPI_DEFINED(rt_i2c_bus_device_device_init);
KPI_DEFINED(rt_i2c_bus_device_register);
KPI_DEFINED(rt_i2c_bus_device_find);
KPI_DEFINED(rt_i2c_transfer);
KPI_DEFINED(rt_i2c_control);
KPI_DEFINED(rt_i2c_master_send);
KPI_DEFINED(rt_i2c_master_recv);

KPI_DEFINED(rt_spi_driver_register);
KPI_DEFINED(rt_spi_device_register);
KPI_DEFINED(rt_spi_bus_register);
KPI_DEFINED(rt_spi_bus_attach_device);
KPI_DEFINED(rt_spi_bus_attach_device_cspin);
KPI_DEFINED(rt_spi_bus_configure);
KPI_DEFINED(rt_spi_take_bus);
KPI_DEFINED(rt_spi_release_bus);
KPI_DEFINED(rt_spi_take);
KPI_DEFINED(rt_spi_release);
KPI_DEFINED(rt_spi_configure);
KPI_DEFINED(rt_spi_send_then_recv);
KPI_DEFINED(rt_spi_send_then_send);
KPI_DEFINED(rt_spi_transfer);
KPI_DEFINED(rt_spi_sendrecv8);
KPI_DEFINED(rt_spi_sendrecv16);
KPI_DEFINED(rt_spi_transfer_message);

KPI_DEFINED(rt_hw_dac_register);
KPI_DEFINED(rt_dac_write);
KPI_DEFINED(rt_dac_enable);
KPI_DEFINED(rt_dac_disable);

KPI_DEFINED(rt_hw_watchdog_register);

KPI_DEFINED(rt_syscon_find_by_ofw_phandle);

/* kpi addr init */
void kpi_init(void)
{
    rt_thread_init = KPI_IMPORT(rt_thread_init, 1);
    rt_thread_detach = KPI_IMPORT(rt_thread_detach, 2);
    rt_thread_create = KPI_IMPORT(rt_thread_create, 3);
    rt_thread_delete = KPI_IMPORT(rt_thread_delete, 4);
    rt_thread_close = KPI_IMPORT(rt_thread_close, 5);
    rt_thread_self = KPI_IMPORT(rt_thread_self, 6);
    rt_thread_find = KPI_IMPORT(rt_thread_find, 7);
    rt_thread_startup = KPI_IMPORT(rt_thread_startup, 8);
    rt_thread_yield = KPI_IMPORT(rt_thread_yield, 9);
    rt_thread_delay = KPI_IMPORT(rt_thread_delay, 10);
    rt_thread_delay_until = KPI_IMPORT(rt_thread_delay_until, 11);
    rt_thread_mdelay = KPI_IMPORT(rt_thread_mdelay, 12);
    rt_thread_control = KPI_IMPORT(rt_thread_control, 13);
    rt_thread_suspend = KPI_IMPORT(rt_thread_suspend, 14);
    rt_thread_suspend_with_flag = KPI_IMPORT(rt_thread_suspend_with_flag, 15);
    rt_thread_resume = KPI_IMPORT(rt_thread_resume, 16);
    rt_thread_get_name = KPI_IMPORT(rt_thread_get_name, 17);
    rt_thread_idle_sethook = KPI_IMPORT(rt_thread_idle_sethook, 18);
    rt_thread_idle_delhook = KPI_IMPORT(rt_thread_idle_delhook, 19);
    rt_thread_idle_gethandler = KPI_IMPORT(rt_thread_idle_gethandler, 20);
    rt_enter_critical = KPI_IMPORT(rt_enter_critical, 21);
    rt_exit_critical = KPI_IMPORT(rt_exit_critical, 22);
    rt_exit_critical_safe = KPI_IMPORT(rt_exit_critical_safe, 23);
    rt_critical_level = KPI_IMPORT(rt_critical_level, 24);
    rt_schedule = KPI_IMPORT(rt_schedule, 25);

    rt_sem_init = KPI_IMPORT(rt_sem_init, 36);
    rt_sem_detach = KPI_IMPORT(rt_sem_detach, 37);
    rt_sem_create = KPI_IMPORT(rt_sem_create, 38);
    rt_sem_delete = KPI_IMPORT(rt_sem_delete, 39);
    rt_sem_take = KPI_IMPORT(rt_sem_take, 40);
    rt_sem_take_interruptible = KPI_IMPORT(rt_sem_take_interruptible, 41);
    rt_sem_take_killable = KPI_IMPORT(rt_sem_take_killable, 42);
    rt_sem_trytake = KPI_IMPORT(rt_sem_trytake, 43);
    rt_sem_release = KPI_IMPORT(rt_sem_release, 44);
    rt_sem_control = KPI_IMPORT(rt_sem_control, 45);

    rt_mutex_init = KPI_IMPORT(rt_mutex_init, 57);
    rt_mutex_detach = KPI_IMPORT(rt_mutex_detach, 58);
    rt_mutex_create = KPI_IMPORT(rt_mutex_create, 59);
    rt_mutex_delete = KPI_IMPORT(rt_mutex_delete, 60);
    rt_mutex_drop_thread = KPI_IMPORT(rt_mutex_drop_thread, 61);
    rt_mutex_setprioceiling = KPI_IMPORT(rt_mutex_setprioceiling, 62);
    rt_mutex_getprioceiling = KPI_IMPORT(rt_mutex_getprioceiling, 63);
    rt_mutex_take = KPI_IMPORT(rt_mutex_take, 64);
    rt_mutex_trytake = KPI_IMPORT(rt_mutex_trytake, 65);
    rt_mutex_take_interruptible = KPI_IMPORT(rt_mutex_take_interruptible, 66);
    rt_mutex_take_killable = KPI_IMPORT(rt_mutex_take_killable, 67);
    rt_mutex_release = KPI_IMPORT(rt_mutex_release, 68);
    rt_mutex_control = KPI_IMPORT(rt_mutex_control, 69);

    rt_event_init = KPI_IMPORT(rt_event_init, 81);
    rt_event_detach = KPI_IMPORT(rt_event_detach, 82);
    rt_event_create = KPI_IMPORT(rt_event_create, 83);
    rt_event_delete = KPI_IMPORT(rt_event_delete, 84);
    rt_event_send = KPI_IMPORT(rt_event_send, 85);
    rt_event_recv = KPI_IMPORT(rt_event_recv, 86);
    rt_event_recv_interruptible = KPI_IMPORT(rt_event_recv_interruptible, 87);
    rt_event_recv_killable = KPI_IMPORT(rt_event_recv_killable, 88);
    rt_event_control = KPI_IMPORT(rt_event_control, 89);

    rt_mb_init = KPI_IMPORT(rt_mb_init, 101);
    rt_mb_detach = KPI_IMPORT(rt_mb_detach, 102);
    rt_mb_create = KPI_IMPORT(rt_mb_create, 103);
    rt_mb_delete = KPI_IMPORT(rt_mb_delete, 104);
    rt_mb_send = KPI_IMPORT(rt_mb_send, 105);
    rt_mb_send_interruptible = KPI_IMPORT(rt_mb_send_interruptible, 106);
    rt_mb_send_killable = KPI_IMPORT(rt_mb_send_killable, 107);
    rt_mb_send_wait = KPI_IMPORT(rt_mb_send_wait, 108);
    rt_mb_send_wait_interruptible =
        KPI_IMPORT(rt_mb_send_wait_interruptible, 109);
    rt_mb_send_wait_killable = KPI_IMPORT(rt_mb_send_wait_killable, 110);
    rt_mb_urgent = KPI_IMPORT(rt_mb_urgent, 111);
    rt_mb_recv = KPI_IMPORT(rt_mb_recv, 112);
    rt_mb_recv_interruptible = KPI_IMPORT(rt_mb_recv_interruptible, 113);
    rt_mb_recv_killable = KPI_IMPORT(rt_mb_recv_killable, 114);
    rt_mb_control = KPI_IMPORT(rt_mb_control, 127);

    rt_mq_init = KPI_IMPORT(rt_mq_init, 127);
    rt_mq_detach = KPI_IMPORT(rt_mq_detach, 128);
    rt_mq_create = KPI_IMPORT(rt_mq_create, 129);
    rt_mq_delete = KPI_IMPORT(rt_mq_delete, 130);
    rt_mq_send = KPI_IMPORT(rt_mq_send, 131);
    rt_mq_send_interruptible = KPI_IMPORT(rt_mq_send_interruptible, 132);
    rt_mq_send_killable = KPI_IMPORT(rt_mq_send_killable, 133);
    rt_mq_send_wait = KPI_IMPORT(rt_mq_send_wait, 134);
    rt_mq_send_wait_interruptible =
        KPI_IMPORT(rt_mq_send_wait_interruptible, 135);
    rt_mq_send_wait_killable = KPI_IMPORT(rt_mq_send_wait_killable, 136);
    rt_mq_urgent = KPI_IMPORT(rt_mq_urgent, 137);
    rt_mq_recv = KPI_IMPORT(rt_mq_recv, 138);
    rt_mq_recv_interruptible = KPI_IMPORT(rt_mq_recv_interruptible, 139);
    rt_mq_recv_killable = KPI_IMPORT(rt_mq_recv_killable, 140);
    rt_mq_control = KPI_IMPORT(rt_mq_control, 141);
    rt_mq_send_wait_prio = KPI_IMPORT(rt_mq_send_wait_prio, 142);
    rt_mq_recv_prio = KPI_IMPORT(rt_mq_recv_prio, 143);

    rt_tick_get = KPI_IMPORT(rt_tick_get, 155);
    rt_tick_set = KPI_IMPORT(rt_tick_set, 156);
    rt_tick_from_millisecond = KPI_IMPORT(rt_tick_from_millisecond, 157);
    rt_tick_get_millisecond_r = KPI_IMPORT(rt_tick_get_millisecond_r, 158);
    rt_timer_init = KPI_IMPORT(rt_timer_init, 159);
    rt_timer_detach = KPI_IMPORT(rt_timer_detach, 160);
    rt_timer_create = KPI_IMPORT(rt_timer_create, 161);
    rt_timer_delete = KPI_IMPORT(rt_timer_delete, 162);
    rt_timer_start = KPI_IMPORT(rt_timer_start, 163);
    rt_timer_stop = KPI_IMPORT(rt_timer_stop, 164);
    rt_timer_control = KPI_IMPORT(rt_timer_control, 165);

    rt_kprintf = KPI_IMPORT(rt_kprintf, 177);
    rt_kputs = KPI_IMPORT(rt_kputs, 178);
    rt_malloc = KPI_IMPORT(rt_malloc, 179);
    rt_free = KPI_IMPORT(rt_free, 180);
    rt_realloc = KPI_IMPORT(rt_realloc, 181);
    rt_calloc = KPI_IMPORT(rt_calloc, 182);
    rt_malloc_align = KPI_IMPORT(rt_malloc_align, 183);
    rt_free_align = KPI_IMPORT(rt_free_align, 184);
    rt_get_errno = KPI_IMPORT(rt_get_errno, 185);
    rt_set_errno = KPI_IMPORT(rt_set_errno, 186);
    _rt_errno = KPI_IMPORT(_rt_errno, 187);
    rt_strerror = KPI_IMPORT(rt_strerror, 188);
    rt_vsprintf = KPI_IMPORT(rt_vsprintf, 189);
    rt_vsnprintf = KPI_IMPORT(rt_vsnprintf, 190);
    rt_sprintf = KPI_IMPORT(rt_sprintf, 191);
    rt_snprintf = KPI_IMPORT(rt_snprintf, 192);
    rt_vsscanf = KPI_IMPORT(rt_vsscanf, 193);
    rt_sscanf = KPI_IMPORT(rt_sscanf, 194);
    rt_memset = KPI_IMPORT(rt_memset, 195);
    rt_memcpy = KPI_IMPORT(rt_memcpy, 196);
    rt_memmove = KPI_IMPORT(rt_memmove, 197);
    rt_memcmp = KPI_IMPORT(rt_memcmp, 198);
    rt_strdup = KPI_IMPORT(rt_strdup, 199);
    rt_strnlen = KPI_IMPORT(rt_strnlen, 200);
    rt_strstr = KPI_IMPORT(rt_strstr, 201);
    rt_strcasecmp = KPI_IMPORT(rt_strcasecmp, 202);
    rt_strcpy = KPI_IMPORT(rt_strcpy, 203);
    rt_strncpy = KPI_IMPORT(rt_strncpy, 204);
    rt_strncmp = KPI_IMPORT(rt_strncmp, 205);
    rt_strcmp = KPI_IMPORT(rt_strcmp, 206);
    rt_strlen = KPI_IMPORT(rt_strlen, 207);
    rt_assert_handler = KPI_IMPORT(rt_assert_handler, 208);

    rt_atomic_load = KPI_IMPORT(rt_atomic_load, 259);
    rt_atomic_store = KPI_IMPORT(rt_atomic_store, 260);
    rt_atomic_add = KPI_IMPORT(rt_atomic_add, 261);
    rt_atomic_sub = KPI_IMPORT(rt_atomic_sub, 262);
    rt_atomic_and = KPI_IMPORT(rt_atomic_and, 263);
    rt_atomic_or = KPI_IMPORT(rt_atomic_or, 264);
    rt_atomic_xor = KPI_IMPORT(rt_atomic_xor, 265);
    rt_atomic_exchange = KPI_IMPORT(rt_atomic_exchange, 266);
    rt_atomic_flag_clear = KPI_IMPORT(rt_atomic_flag_clear, 267);
    rt_atomic_flag_test_and_set = KPI_IMPORT(rt_atomic_flag_test_and_set, 268);
    rt_atomic_compare_exchange_strong =
        KPI_IMPORT(rt_atomic_compare_exchange_strong, 269);

    rt_device_find = KPI_IMPORT(rt_device_find, 281);
    rt_device_register = KPI_IMPORT(rt_device_register, 282);
    rt_device_unregister = KPI_IMPORT(rt_device_unregister, 283);
    rt_device_create = KPI_IMPORT(rt_device_create, 284);
    rt_device_destroy = KPI_IMPORT(rt_device_destroy, 285);
    rt_device_set_rx_indicate = KPI_IMPORT(rt_device_set_rx_indicate, 286);
    rt_device_set_tx_complete = KPI_IMPORT(rt_device_set_tx_complete, 287);
    rt_device_init = KPI_IMPORT(rt_device_init, 288);
    rt_device_open = KPI_IMPORT(rt_device_open, 289);
    rt_device_close = KPI_IMPORT(rt_device_close, 290);
    rt_device_read = KPI_IMPORT(rt_device_read, 291);
    rt_device_write = KPI_IMPORT(rt_device_write, 292);
    rt_device_control = KPI_IMPORT(rt_device_control, 293);

    rt_hw_cpu_icache_enable = KPI_IMPORT(rt_hw_cpu_icache_enable, 305);
    rt_hw_cpu_icache_disable = KPI_IMPORT(rt_hw_cpu_icache_disable, 306);
    rt_hw_cpu_icache_ops = KPI_IMPORT(rt_hw_cpu_icache_ops, 307);
    rt_hw_cpu_icache_status = KPI_IMPORT(rt_hw_cpu_icache_status, 308);
    rt_hw_cpu_dcache_enable = KPI_IMPORT(rt_hw_cpu_dcache_enable, 309);
    rt_hw_cpu_dcache_disable = KPI_IMPORT(rt_hw_cpu_dcache_disable, 310);
    rt_hw_cpu_dcache_status = KPI_IMPORT(rt_hw_cpu_dcache_status, 311);
    rt_hw_cpu_dcache_ops = KPI_IMPORT(rt_hw_cpu_dcache_ops, 312);
    rt_hw_isb = KPI_IMPORT(rt_hw_isb, 313);
    rt_hw_dmb = KPI_IMPORT(rt_hw_dmb, 314);
    rt_hw_dsb = KPI_IMPORT(rt_hw_dsb, 315);
    rt_hw_cpu_relax = KPI_IMPORT(rt_hw_cpu_relax, 316);
    rt_hw_rmb = KPI_IMPORT(rt_hw_rmb, 317);

    rt_hw_cpu_reset = KPI_IMPORT(rt_hw_cpu_reset, 329);
    rt_hw_cpu_shutdown = KPI_IMPORT(rt_hw_cpu_shutdown, 330);
    rt_hw_cpu_arch = KPI_IMPORT(rt_hw_cpu_arch, 331);
    rt_hw_cpu_id = KPI_IMPORT(rt_hw_cpu_id, 332);

    rt_hw_local_irq_disable = KPI_IMPORT(rt_hw_local_irq_disable, 344);
    rt_hw_local_irq_enable = KPI_IMPORT(rt_hw_local_irq_enable, 345);
    rt_hw_interrupt_disable = KPI_IMPORT(rt_hw_interrupt_disable, 346);
    rt_hw_interrupt_enable = KPI_IMPORT(rt_hw_interrupt_enable, 347);
    rt_hw_interrupt_is_disabled = KPI_IMPORT(rt_hw_interrupt_is_disabled, 348);
    rt_interrupt_enter = KPI_IMPORT(rt_interrupt_enter, 349);
    rt_interrupt_leave = KPI_IMPORT(rt_interrupt_leave, 350);
    rt_interrupt_get_nest = KPI_IMPORT(rt_interrupt_get_nest, 351);
    rt_hw_ipi_send = KPI_IMPORT(rt_hw_ipi_send, 352);
    rt_hw_ipi_handler_install = KPI_IMPORT(rt_hw_ipi_handler_install, 353);
    rt_hw_interrupt_mask = KPI_IMPORT(rt_hw_interrupt_mask, 354);
    rt_hw_interrupt_umask = KPI_IMPORT(rt_hw_interrupt_umask, 355);
    rt_hw_interrupt_install = KPI_IMPORT(rt_hw_interrupt_install, 356);
    rt_hw_interrupt_uninstall = KPI_IMPORT(rt_hw_interrupt_uninstall, 357);
    rt_hw_exception_install = KPI_IMPORT(rt_hw_exception_install, 358);

    rt_spin_lock_init = KPI_IMPORT(rt_spin_lock_init, 380);
    rt_spin_lock = KPI_IMPORT(rt_spin_lock, 381);
    rt_spin_unlock = KPI_IMPORT(rt_spin_unlock, 382);
    rt_spin_lock_irqsave = KPI_IMPORT(rt_spin_lock_irqsave, 383);
    rt_spin_unlock_irqrestore = KPI_IMPORT(rt_spin_unlock_irqrestore, 384);

    rt_hw_us_delay = KPI_IMPORT(rt_hw_us_delay, 396);
    rt_hw_global_timer_get = KPI_IMPORT(rt_hw_global_timer_get, 397);

    __rt_libc_exit = KPI_IMPORT(__rt_libc_exit, 428);

#ifdef RT_USING_POSIX_STDIO
    rt_posix_stdio_get_console = KPI_IMPORT(rt_posix_stdio_get_console, 429);
    rt_posix_stdio_set_console = KPI_IMPORT(rt_posix_stdio_set_console, 430);
#endif /* RT_USING_POSIX_STDIO */

    rt_creat = KPI_IMPORT(rt_creat, 630);
    rt_open = KPI_IMPORT(rt_open, 631);
    rt_read = KPI_IMPORT(rt_read, 632);
    rt_write = KPI_IMPORT(rt_write, 633);
    rt_close = KPI_IMPORT(rt_close, 634);
    ioctl = KPI_IMPORT(ioctl, 635);
    rt_lseek = KPI_IMPORT(rt_lseek, 636);
    rt_fstat = KPI_IMPORT(rt_fstat, 637);
    rt_unlink = KPI_IMPORT(rt_unlink, 638);
    rt_rename = KPI_IMPORT(rt_rename, 639);
    rt_mkdir = KPI_IMPORT(rt_mkdir, 640);
    rt_stat = KPI_IMPORT(rt_stat, 641);

#ifdef RT_USING_POSIX_TERMIOS
    cfgetospeed = KPI_IMPORT(cfgetospeed, 642);
    cfgetispeed = KPI_IMPORT(cfgetispeed, 643);
    cfsetospeed = KPI_IMPORT(cfsetospeed, 644);
    cfsetispeed = KPI_IMPORT(cfsetispeed, 645);
    tcgetattr = KPI_IMPORT(tcgetattr, 646);
    tcsetattr = KPI_IMPORT(tcsetattr, 647);
    tcsendbreak = KPI_IMPORT(tcsendbreak, 648);
    tcdrain = KPI_IMPORT(tcdrain, 649);
    tcflush = KPI_IMPORT(tcflush, 650);
    tcflow = KPI_IMPORT(tcflow, 651);
    tcgetsid = KPI_IMPORT(tcgetsid, 652);
    cfmakeraw = KPI_IMPORT(cfmakeraw, 653);
    cfsetspeed = KPI_IMPORT(cfsetspeed, 654);
#endif /* RT_USING_POSIX_TERMIOS */

#ifdef RT_USING_SAL
    select = KPI_IMPORT(select, 655);
    accept = KPI_IMPORT(accept, 656);
    bind = KPI_IMPORT(bind, 657);
    shutdown = KPI_IMPORT(shutdown, 658);
    getpeername = KPI_IMPORT(getpeername, 659);
    getsockname = KPI_IMPORT(getsockname, 660);
    getsockopt = KPI_IMPORT(getsockopt, 661);
    setsockopt = KPI_IMPORT(setsockopt, 662);
    connect = KPI_IMPORT(connect, 663);
    listen = KPI_IMPORT(listen, 664);
    recv = KPI_IMPORT(recv, 665);
    recvfrom = KPI_IMPORT(recvfrom, 666);
    recvmsg = KPI_IMPORT(recvmsg, 667);
    sendmsg = KPI_IMPORT(sendmsg, 668);
    send = KPI_IMPORT(send, 669);
    sendto = KPI_IMPORT(sendto, 670);
    socket = KPI_IMPORT(socket, 671);
    closesocket = KPI_IMPORT(closesocket, 672);
    ioctlsocket = KPI_IMPORT(ioctlsocket, 673);
    socketpair = KPI_IMPORT(socketpair, 674);
    gethostbyname = KPI_IMPORT(gethostbyname, 675);
    gethostbyname_r = KPI_IMPORT(gethostbyname_r, 676);
    freeaddrinfo = KPI_IMPORT(freeaddrinfo, 677);
    getaddrinfo = KPI_IMPORT(getaddrinfo, 678);
#endif /* RT_USING_SAL */

    rt_opendir = KPI_IMPORT(rt_opendir, 679);
    rt_readdir = KPI_IMPORT(rt_readdir, 680);
    rt_closedir = KPI_IMPORT(rt_closedir, 681);
    fsync = KPI_IMPORT(fsync, 682);
    rt_fcntl = KPI_IMPORT(rt_fcntl, 683);
    ftruncate = KPI_IMPORT(ftruncate, 684);
    rt_statfs = KPI_IMPORT(rt_statfs, 685);
    rt_fstatfs = KPI_IMPORT(rt_fstatfs, 686);
    rmdir = KPI_IMPORT(rmdir, 687);
    rt_telldir = KPI_IMPORT(rt_telldir, 688);
    rt_seekdir = KPI_IMPORT(rt_seekdir, 689);
    rt_rewinddir = KPI_IMPORT(rt_rewinddir, 690);
    access = KPI_IMPORT(access, 691);
    setcwd = KPI_IMPORT(setcwd, 692);
    getcwd = KPI_IMPORT(getcwd, 693);

#ifdef RT_USING_NETDEV
    if_set_mac = KPI_IMPORT(if_set_mac, 700);
    if_get_mac = KPI_IMPORT(if_get_mac, 701);
    if_set_dns = KPI_IMPORT(if_set_dns, 702);
    if_get_dns = KPI_IMPORT(if_get_dns, 703);
    if_dhcp = KPI_IMPORT(if_dhcp, 704);
    if_set_ip = KPI_IMPORT(if_set_ip, 705);
    if_get_ip = KPI_IMPORT(if_get_ip, 706);
    if_up = KPI_IMPORT(if_up, 707);
    if_down = KPI_IMPORT(if_down, 708);
#endif /* RT_USING_NETDEV */

    service_find = KPI_IMPORT(service_find, 1180);
    service_register = KPI_IMPORT(service_register, 1181);
    service_unregister = KPI_IMPORT(service_unregister, 1182);
    service_create = KPI_IMPORT(service_create, 1183);
    service_destroy = KPI_IMPORT(service_destroy, 1184);
    service_open = KPI_IMPORT(service_open, 1185);
    service_close = KPI_IMPORT(service_close, 1186);
    service_read = KPI_IMPORT(service_read, 1187);
    service_write = KPI_IMPORT(service_write, 1188);
    service_control = KPI_IMPORT(service_control, 1189);
    service_set_rx_indicate = KPI_IMPORT(service_set_rx_indicate, 1190);
    service_set_tx_complete = KPI_IMPORT(service_set_tx_complete, 1191);
    service_lock = KPI_IMPORT(service_lock, 1192);
    service_unlock = KPI_IMPORT(service_unlock, 1193);

    rt_bus_for_each_dev = KPI_IMPORT(rt_bus_for_each_dev, 1306);
    rt_bus_for_each_drv = KPI_IMPORT(rt_bus_for_each_drv, 1307);
    rt_bus_add_driver = KPI_IMPORT(rt_bus_add_driver, 1308);
    rt_bus_add_device = KPI_IMPORT(rt_bus_add_device, 1309);
    rt_bus_remove_driver = KPI_IMPORT(rt_bus_remove_driver, 1310);
    rt_bus_remove_device = KPI_IMPORT(rt_bus_remove_device, 1311);
    rt_bus_shutdown = KPI_IMPORT(rt_bus_shutdown, 1312);
    rt_bus_find_by_name = KPI_IMPORT(rt_bus_find_by_name, 1313);
    rt_bus_reload_driver_device = KPI_IMPORT(rt_bus_reload_driver_device, 1314);
    rt_bus_register = KPI_IMPORT(rt_bus_register, 1315);

    rt_dm_ida_alloc = KPI_IMPORT(rt_dm_ida_alloc, 1327);
    rt_dm_ida_take = KPI_IMPORT(rt_dm_ida_take, 1328);
    rt_dm_ida_free = KPI_IMPORT(rt_dm_ida_free, 1329);
    rt_dm_device_find = KPI_IMPORT(rt_dm_device_find, 1330);
    rt_dm_dev_set_name_auto = KPI_IMPORT(rt_dm_dev_set_name_auto, 1331);
    rt_dm_dev_get_name_id = KPI_IMPORT(rt_dm_dev_get_name_id, 1332);
    rt_dm_dev_set_name = KPI_IMPORT(rt_dm_dev_set_name, 1333);
    rt_dm_dev_get_name = KPI_IMPORT(rt_dm_dev_get_name, 1334);
    rt_dm_dev_get_address_count = KPI_IMPORT(rt_dm_dev_get_address_count, 1335);
    rt_dm_dev_get_address = KPI_IMPORT(rt_dm_dev_get_address, 1336);
    rt_dm_dev_get_address_by_name =
        KPI_IMPORT(rt_dm_dev_get_address_by_name, 1337);
    rt_dm_dev_get_address_array = KPI_IMPORT(rt_dm_dev_get_address_array, 1338);
    rt_dm_dev_iomap = KPI_IMPORT(rt_dm_dev_iomap, 1339);
    rt_dm_dev_iomap_by_name = KPI_IMPORT(rt_dm_dev_iomap_by_name, 1340);
    rt_dm_dev_get_irq_count = KPI_IMPORT(rt_dm_dev_get_irq_count, 1341);
    rt_dm_dev_get_irq = KPI_IMPORT(rt_dm_dev_get_irq, 1342);
    rt_dm_dev_get_irq_by_name = KPI_IMPORT(rt_dm_dev_get_irq_by_name, 1343);
    rt_dm_dev_bind_fwdata = KPI_IMPORT(rt_dm_dev_bind_fwdata, 1344);
    rt_dm_dev_unbind_fwdata = KPI_IMPORT(rt_dm_dev_unbind_fwdata, 1345);
    rt_dm_dev_prop_read_u8_array_index =
        KPI_IMPORT(rt_dm_dev_prop_read_u8_array_index, 1346);
    rt_dm_dev_prop_read_u16_array_index =
        KPI_IMPORT(rt_dm_dev_prop_read_u16_array_index, 1347);
    rt_dm_dev_prop_read_u32_array_index =
        KPI_IMPORT(rt_dm_dev_prop_read_u32_array_index, 1348);
    rt_dm_dev_prop_read_u64_array_index =
        KPI_IMPORT(rt_dm_dev_prop_read_u64_array_index, 1349);
    rt_dm_dev_prop_read_string_array_index =
        KPI_IMPORT(rt_dm_dev_prop_read_string_array_index, 1350);
    rt_dm_dev_prop_count_of_size =
        KPI_IMPORT(rt_dm_dev_prop_count_of_size, 1351);
    rt_dm_dev_prop_index_of_string =
        KPI_IMPORT(rt_dm_dev_prop_index_of_string, 1352);
    rt_dm_dev_prop_read_bool = KPI_IMPORT(rt_dm_dev_prop_read_bool, 1353);

    rt_platform_device_alloc = KPI_IMPORT(rt_platform_device_alloc, 1365);
    rt_platform_driver_register = KPI_IMPORT(rt_platform_driver_register, 1366);
    rt_platform_device_register = KPI_IMPORT(rt_platform_device_register, 1367);
    rt_platform_ofw_device_probe_child =
        KPI_IMPORT(rt_platform_ofw_device_probe_child, 1368);
    rt_platform_ofw_request = KPI_IMPORT(rt_platform_ofw_request, 1369);
    rt_platform_ofw_free = KPI_IMPORT(rt_platform_ofw_free, 1370);

    rt_ofw_bus_addr_cells = KPI_IMPORT(rt_ofw_bus_addr_cells, 1382);
    rt_ofw_bus_size_cells = KPI_IMPORT(rt_ofw_bus_size_cells, 1383);
    rt_ofw_io_addr_cells = KPI_IMPORT(rt_ofw_io_addr_cells, 1384);
    rt_ofw_io_size_cells = KPI_IMPORT(rt_ofw_io_size_cells, 1385);
    rt_ofw_get_address_count = KPI_IMPORT(rt_ofw_get_address_count, 1386);
    rt_ofw_get_address = KPI_IMPORT(rt_ofw_get_address, 1387);
    rt_ofw_get_address_by_name = KPI_IMPORT(rt_ofw_get_address_by_name, 1388);
    rt_ofw_get_address_array = KPI_IMPORT(rt_ofw_get_address_array, 1389);
    rt_ofw_translate_address = KPI_IMPORT(rt_ofw_translate_address, 1390);
    rt_ofw_reverse_address = KPI_IMPORT(rt_ofw_reverse_address, 1391);
    rt_ofw_iomap = KPI_IMPORT(rt_ofw_iomap, 1392);
    rt_ofw_iomap_by_name = KPI_IMPORT(rt_ofw_iomap_by_name, 1393);

    rt_ofw_irq_cells = KPI_IMPORT(rt_ofw_irq_cells, 1394);
    rt_ofw_parse_irq_map = KPI_IMPORT(rt_ofw_parse_irq_map, 1395);
    rt_ofw_parse_irq_cells = KPI_IMPORT(rt_ofw_parse_irq_cells, 1396);
    rt_ofw_find_irq_parent = KPI_IMPORT(rt_ofw_find_irq_parent, 1397);
    rt_ofw_map_irq = KPI_IMPORT(rt_ofw_map_irq, 1398);
    rt_ofw_get_irq_count = KPI_IMPORT(rt_ofw_get_irq_count, 1399);
    rt_ofw_get_irq = KPI_IMPORT(rt_ofw_get_irq, 1400);
    rt_ofw_get_irq_by_name = KPI_IMPORT(rt_ofw_get_irq_by_name, 1401);

    rt_ofw_node_destroy = KPI_IMPORT(rt_ofw_node_destroy, 1402);
    rt_ofw_node_get = KPI_IMPORT(rt_ofw_node_get, 1403);
    rt_ofw_node_put = KPI_IMPORT(rt_ofw_node_put, 1404);
    rt_ofw_node_tag_equ = KPI_IMPORT(rt_ofw_node_tag_equ, 1405);
    rt_ofw_node_tag_prefix = KPI_IMPORT(rt_ofw_node_tag_prefix, 1406);
    rt_ofw_machine_is_compatible =
        KPI_IMPORT(rt_ofw_machine_is_compatible, 1407);
    rt_ofw_node_is_available = KPI_IMPORT(rt_ofw_node_is_available, 1408);
    rt_ofw_node_is_compatible = KPI_IMPORT(rt_ofw_node_is_compatible, 1409);
    rt_ofw_prop_match = KPI_IMPORT(rt_ofw_prop_match, 1410);
    rt_ofw_node_match = KPI_IMPORT(rt_ofw_node_match, 1411);
    rt_ofw_find_node_by_tag = KPI_IMPORT(rt_ofw_find_node_by_tag, 1412);
    rt_ofw_find_node_by_prop_r = KPI_IMPORT(rt_ofw_find_node_by_prop_r, 1413);
    rt_ofw_find_node_by_name = KPI_IMPORT(rt_ofw_find_node_by_name, 1414);
    rt_ofw_find_node_by_type = KPI_IMPORT(rt_ofw_find_node_by_type, 1415);
    rt_ofw_find_node_by_compatible =
        KPI_IMPORT(rt_ofw_find_node_by_compatible, 1416);
    rt_ofw_find_node_by_ids_r = KPI_IMPORT(rt_ofw_find_node_by_ids_r, 1417);
    rt_ofw_find_node_by_path = KPI_IMPORT(rt_ofw_find_node_by_path, 1418);
    rt_ofw_find_node_by_phandle = KPI_IMPORT(rt_ofw_find_node_by_phandle, 1419);
    rt_ofw_get_parent = KPI_IMPORT(rt_ofw_get_parent, 1420);
    rt_ofw_get_child_by_tag = KPI_IMPORT(rt_ofw_get_child_by_tag, 1421);
    rt_ofw_get_child_by_compatible =
        KPI_IMPORT(rt_ofw_get_child_by_compatible, 1422);
    rt_ofw_get_child_count = KPI_IMPORT(rt_ofw_get_child_count, 1423);
    rt_ofw_get_available_child_count =
        KPI_IMPORT(rt_ofw_get_available_child_count, 1424);
    rt_ofw_get_next_node = KPI_IMPORT(rt_ofw_get_next_node, 1425);
    rt_ofw_get_next_parent = KPI_IMPORT(rt_ofw_get_next_parent, 1426);
    rt_ofw_get_next_child = KPI_IMPORT(rt_ofw_get_next_child, 1427);
    rt_ofw_get_next_available_child =
        KPI_IMPORT(rt_ofw_get_next_available_child, 1428);
    rt_ofw_get_cpu_node = KPI_IMPORT(rt_ofw_get_cpu_node, 1429);
    rt_ofw_get_next_cpu_node = KPI_IMPORT(rt_ofw_get_next_cpu_node, 1430);
    rt_ofw_get_cpu_state_node = KPI_IMPORT(rt_ofw_get_cpu_state_node, 1431);
    rt_ofw_get_cpu_id = KPI_IMPORT(rt_ofw_get_cpu_id, 1432);
    rt_ofw_get_cpu_hwid = KPI_IMPORT(rt_ofw_get_cpu_hwid, 1433);
    rt_ofw_get_alias_node = KPI_IMPORT(rt_ofw_get_alias_node, 1434);
    rt_ofw_get_alias_id = KPI_IMPORT(rt_ofw_get_alias_id, 1435);
    rt_ofw_get_alias_last_id = KPI_IMPORT(rt_ofw_get_alias_last_id, 1436);
    rt_ofw_map_id = KPI_IMPORT(rt_ofw_map_id, 1437);
    rt_ofw_append_child = KPI_IMPORT(rt_ofw_append_child, 1438);
    rt_ofw_append_prop = KPI_IMPORT(rt_ofw_append_prop, 1439);
    rt_ofw_parse_phandle = KPI_IMPORT(rt_ofw_parse_phandle, 1440);
    rt_ofw_parse_phandle_cells = KPI_IMPORT(rt_ofw_parse_phandle_cells, 1441);
    rt_ofw_count_phandle_cells = KPI_IMPORT(rt_ofw_count_phandle_cells, 1442);
    rt_ofw_get_prop_fuzzy_name = KPI_IMPORT(rt_ofw_get_prop_fuzzy_name, 1443);
    rt_ofw_get_prop = KPI_IMPORT(rt_ofw_get_prop, 1444);
    rt_ofw_prop_read_u8_array_index =
        KPI_IMPORT(rt_ofw_prop_read_u8_array_index, 1445);
    rt_ofw_prop_read_u16_array_index =
        KPI_IMPORT(rt_ofw_prop_read_u16_array_index, 1446);
    rt_ofw_prop_read_u32_array_index =
        KPI_IMPORT(rt_ofw_prop_read_u32_array_index, 1447);
    rt_ofw_prop_read_u64_array_index =
        KPI_IMPORT(rt_ofw_prop_read_u64_array_index, 1448);
    rt_ofw_prop_read_string_array_index =
        KPI_IMPORT(rt_ofw_prop_read_string_array_index, 1449);
    rt_ofw_prop_count_of_size = KPI_IMPORT(rt_ofw_prop_count_of_size, 1450);
    rt_ofw_prop_index_of_string = KPI_IMPORT(rt_ofw_prop_index_of_string, 1451);
    rt_ofw_prop_next_u32 = KPI_IMPORT(rt_ofw_prop_next_u32, 1452);
    rt_ofw_prop_next_string = KPI_IMPORT(rt_ofw_prop_next_string, 1453);
    rt_ofw_parse_object = KPI_IMPORT(rt_ofw_parse_object, 1454);
    rt_ofw_console_setup = KPI_IMPORT(rt_ofw_console_setup, 1455);
    rt_ofw_bootargs_select = KPI_IMPORT(rt_ofw_bootargs_select, 1456);

    rt_pic_attach_irq = KPI_IMPORT(rt_pic_attach_irq, 1468);
    rt_pic_detach_irq = KPI_IMPORT(rt_pic_detach_irq, 1469);
    rt_pic_irq_enable = KPI_IMPORT(rt_pic_irq_enable, 1470);
    rt_pic_irq_disable = KPI_IMPORT(rt_pic_irq_disable, 1471);
    rt_pic_irq_ack = KPI_IMPORT(rt_pic_irq_ack, 1472);
    rt_pic_irq_mask = KPI_IMPORT(rt_pic_irq_mask, 1473);
    rt_pic_irq_unmask = KPI_IMPORT(rt_pic_irq_unmask, 1474);
    rt_pic_irq_eoi = KPI_IMPORT(rt_pic_irq_eoi, 1475);
    rt_pic_irq_set_priority = KPI_IMPORT(rt_pic_irq_set_priority, 1476);
    rt_pic_irq_get_priority = KPI_IMPORT(rt_pic_irq_get_priority, 1477);
    rt_pic_irq_set_affinity = KPI_IMPORT(rt_pic_irq_set_affinity, 1478);
    rt_pic_irq_get_affinity = KPI_IMPORT(rt_pic_irq_get_affinity, 1479);
    rt_pic_irq_set_triger_mode = KPI_IMPORT(rt_pic_irq_set_triger_mode, 1480);
    rt_pic_irq_get_triger_mode = KPI_IMPORT(rt_pic_irq_get_triger_mode, 1481);
    rt_pic_irq_send_ipi = KPI_IMPORT(rt_pic_irq_send_ipi, 1482);

    rt_clk_find = KPI_IMPORT(rt_clk_find, 1498);
    rt_clk_find_by_id = KPI_IMPORT(rt_clk_find_by_id, 1499);
    rt_clk_find_by_index = KPI_IMPORT(rt_clk_find_by_index, 1500);
    rt_clk_find_array = KPI_IMPORT(rt_clk_find_array, 1501);
    rt_clk_create = KPI_IMPORT(rt_clk_create, 1502);
    rt_clk_delete = KPI_IMPORT(rt_clk_delete, 1503);
    rt_clk_register = KPI_IMPORT(rt_clk_register, 1504);
    rt_clk_unregister = KPI_IMPORT(rt_clk_unregister, 1505);
    rt_clk_get_name = KPI_IMPORT(rt_clk_get_name, 1506);
    rt_clk_set_parent = KPI_IMPORT(rt_clk_set_parent, 1507);
    rt_clk_get_parent = KPI_IMPORT(rt_clk_get_parent, 1508);
    rt_clk_enable = KPI_IMPORT(rt_clk_enable, 1509);
    rt_clk_disable = KPI_IMPORT(rt_clk_disable, 1510);
    rt_clk_get_status = KPI_IMPORT(rt_clk_get_status, 1511);
    rt_clk_set_rate = KPI_IMPORT(rt_clk_set_rate, 1512);
    rt_clk_get_rate = KPI_IMPORT(rt_clk_get_rate, 1513);
    rt_clk_round_rate = KPI_IMPORT(rt_clk_round_rate, 1514);
    rt_clk_prepare = KPI_IMPORT(rt_clk_prepare, 1515);
    rt_clk_unprepare = KPI_IMPORT(rt_clk_unprepare, 1516);
    rt_clk_prepare_enable = KPI_IMPORT(rt_clk_prepare_enable, 1517);
    rt_clk_disable_unprepare = KPI_IMPORT(rt_clk_disable_unprepare, 1518);
    rt_clk_array_prepare = KPI_IMPORT(rt_clk_array_prepare, 1519);
    rt_clk_array_unprepare = KPI_IMPORT(rt_clk_array_unprepare, 1520);
    rt_clk_array_enable = KPI_IMPORT(rt_clk_array_enable, 1521);
    rt_clk_array_disable = KPI_IMPORT(rt_clk_array_disable, 1522);
    rt_clk_array_prepare_enable = KPI_IMPORT(rt_clk_array_prepare_enable, 1523);
    rt_clk_array_disable_unprepare =
        KPI_IMPORT(rt_clk_array_disable_unprepare, 1524);
    rt_clk_put = KPI_IMPORT(rt_clk_put, 1525);
    rt_ofw_get_clk_by_name = KPI_IMPORT(rt_ofw_get_clk_by_name, 1526);
    rt_clk_fixed_rate_register = KPI_IMPORT(rt_clk_fixed_rate_register, 1527);
    rt_clk_fixed_rate_unregister =
        KPI_IMPORT(rt_clk_fixed_rate_unregister, 1528);
    rt_clk_fixed_factor_register =
        KPI_IMPORT(rt_clk_fixed_factor_register, 1529);
    rt_clk_fixed_factor_unregister =
        KPI_IMPORT(rt_clk_fixed_factor_unregister, 1530);
    of_clk_add_provider = KPI_IMPORT(of_clk_add_provider, 1531);
    rt_clk_get_by_name = KPI_IMPORT(rt_clk_get_by_name, 1532);

    rt_completion_init = KPI_IMPORT(rt_completion_init, 1544);
    rt_completion_wait = KPI_IMPORT(rt_completion_wait, 1545);
    rt_completion_wait_noisr = KPI_IMPORT(rt_completion_wait_noisr, 1546);
    rt_completion_wait_flags = KPI_IMPORT(rt_completion_wait_flags, 1547);
    rt_completion_wait_flags_noisr =
        KPI_IMPORT(rt_completion_wait_flags_noisr, 1548);
    rt_completion_done = KPI_IMPORT(rt_completion_done, 1549);
    rt_completion_wakeup = KPI_IMPORT(rt_completion_wakeup, 1550);
    rt_completion_wakeup_by_errno =
        KPI_IMPORT(rt_completion_wakeup_by_errno, 1551);

    rt_condvar_init = KPI_IMPORT(rt_condvar_init, 1552);
    rt_condvar_timedwait = KPI_IMPORT(rt_condvar_timedwait, 1553);
    rt_condvar_signal = KPI_IMPORT(rt_condvar_signal, 1554);
    rt_condvar_broadcast = KPI_IMPORT(rt_condvar_broadcast, 1555);

    rt_data_queue_init = KPI_IMPORT(rt_data_queue_init, 1556);
    rt_data_queue_push = KPI_IMPORT(rt_data_queue_push, 1557);
    rt_data_queue_pop = KPI_IMPORT(rt_data_queue_pop, 1558);
    rt_data_queue_peek = KPI_IMPORT(rt_data_queue_peek, 1559);
    rt_data_queue_reset = KPI_IMPORT(rt_data_queue_reset, 1560);
    rt_data_queue_deinit = KPI_IMPORT(rt_data_queue_deinit, 1561);
    rt_data_queue_len = KPI_IMPORT(rt_data_queue_len, 1562);

    rt_pipe_create = KPI_IMPORT(rt_pipe_create, 1563);
    rt_pipe_open = KPI_IMPORT(rt_pipe_open, 1564);
    rt_pipe_read = KPI_IMPORT(rt_pipe_read, 1565);
    rt_pipe_write = KPI_IMPORT(rt_pipe_write, 1566);
    rt_pipe_control = KPI_IMPORT(rt_pipe_control, 1567);
    rt_pipe_close = KPI_IMPORT(rt_pipe_close, 1568);
    rt_pipe_delete = KPI_IMPORT(rt_pipe_delete, 1569);

    rt_rbb_init = KPI_IMPORT(rt_rbb_init, 1570);
    rt_rbb_get_buf_size = KPI_IMPORT(rt_rbb_get_buf_size, 1571);
    rt_rbb_create = KPI_IMPORT(rt_rbb_create, 1572);
    rt_rbb_destroy = KPI_IMPORT(rt_rbb_destroy, 1573);
    rt_rbb_blk_alloc = KPI_IMPORT(rt_rbb_blk_alloc, 1574);
    rt_rbb_blk_put = KPI_IMPORT(rt_rbb_blk_put, 1575);
    rt_rbb_blk_get = KPI_IMPORT(rt_rbb_blk_get, 1576);
    rt_rbb_blk_size = KPI_IMPORT(rt_rbb_blk_size, 1577);
    rt_rbb_blk_buf = KPI_IMPORT(rt_rbb_blk_buf, 1578);
    rt_rbb_blk_free = KPI_IMPORT(rt_rbb_blk_free, 1579);
    rt_rbb_blk_queue_get = KPI_IMPORT(rt_rbb_blk_queue_get, 1580);
    rt_rbb_blk_queue_len = KPI_IMPORT(rt_rbb_blk_queue_len, 1581);
    rt_rbb_blk_queue_buf = KPI_IMPORT(rt_rbb_blk_queue_buf, 1582);
    rt_rbb_blk_queue_free = KPI_IMPORT(rt_rbb_blk_queue_free, 1583);
    rt_rbb_next_blk_queue_len = KPI_IMPORT(rt_rbb_next_blk_queue_len, 1584);

    rt_ringbuffer_init = KPI_IMPORT(rt_ringbuffer_init, 1585);
    rt_ringbuffer_reset = KPI_IMPORT(rt_ringbuffer_reset, 1586);
    rt_ringbuffer_put = KPI_IMPORT(rt_ringbuffer_put, 1587);
    rt_ringbuffer_put_force = KPI_IMPORT(rt_ringbuffer_put_force, 1588);
    rt_ringbuffer_putchar = KPI_IMPORT(rt_ringbuffer_putchar, 1589);
    rt_ringbuffer_putchar_force = KPI_IMPORT(rt_ringbuffer_putchar_force, 1590);
    rt_ringbuffer_get = KPI_IMPORT(rt_ringbuffer_get, 1591);
    rt_ringbuffer_peek = KPI_IMPORT(rt_ringbuffer_peek, 1592);
    rt_ringbuffer_getchar = KPI_IMPORT(rt_ringbuffer_getchar, 1593);
    rt_ringbuffer_data_len = KPI_IMPORT(rt_ringbuffer_data_len, 1594);
    rt_ringbuffer_create = KPI_IMPORT(rt_ringbuffer_create, 1595);
    rt_ringbuffer_destroy = KPI_IMPORT(rt_ringbuffer_destroy, 1596);

    __wqueue_default_wake = KPI_IMPORT(__wqueue_default_wake, 1597);
    rt_wqueue_add = KPI_IMPORT(rt_wqueue_add, 1598);
    rt_wqueue_remove = KPI_IMPORT(rt_wqueue_remove, 1599);
    rt_wqueue_wait = KPI_IMPORT(rt_wqueue_wait, 1600);
    rt_wqueue_wait_killable = KPI_IMPORT(rt_wqueue_wait_killable, 1601);
    rt_wqueue_wait_interruptible =
        KPI_IMPORT(rt_wqueue_wait_interruptible, 1602);
    rt_wqueue_wakeup = KPI_IMPORT(rt_wqueue_wakeup, 1603);
    rt_wqueue_wakeup_all = KPI_IMPORT(rt_wqueue_wakeup_all, 1604);

    rt_work_init = KPI_IMPORT(rt_work_init, 1605);
    rt_workqueue_create = KPI_IMPORT(rt_workqueue_create, 1606);
    rt_workqueue_destroy = KPI_IMPORT(rt_workqueue_destroy, 1607);
    rt_workqueue_dowork = KPI_IMPORT(rt_workqueue_dowork, 1608);
    rt_workqueue_submit_work = KPI_IMPORT(rt_workqueue_submit_work, 1609);
    rt_workqueue_cancel_work = KPI_IMPORT(rt_workqueue_cancel_work, 1610);
    rt_workqueue_cancel_work_sync =
        KPI_IMPORT(rt_workqueue_cancel_work_sync, 1611);
    rt_workqueue_cancel_all_work =
        KPI_IMPORT(rt_workqueue_cancel_all_work, 1612);
    rt_workqueue_urgent_work = KPI_IMPORT(rt_workqueue_urgent_work, 1613);

    rt_work_submit = KPI_IMPORT(rt_work_submit, 1614);
    rt_work_urgent = KPI_IMPORT(rt_work_urgent, 1615);
    rt_work_cancel = KPI_IMPORT(rt_work_cancel, 1616);

    rt_i2c_driver_register = KPI_IMPORT(rt_i2c_driver_register, 1628);
    rt_i2c_device_register = KPI_IMPORT(rt_i2c_device_register, 1629);
    rt_i2c_bus_device_device_init =
        KPI_IMPORT(rt_i2c_bus_device_device_init, 1630);
    rt_i2c_bus_device_register = KPI_IMPORT(rt_i2c_bus_device_register, 1631);
    rt_i2c_bus_device_find = KPI_IMPORT(rt_i2c_bus_device_find, 1632);
    rt_i2c_transfer = KPI_IMPORT(rt_i2c_transfer, 1633);
    rt_i2c_control = KPI_IMPORT(rt_i2c_control, 1634);
    rt_i2c_master_send = KPI_IMPORT(rt_i2c_master_send, 1635);
    rt_i2c_master_recv = KPI_IMPORT(rt_i2c_master_recv, 1636);

    rt_spi_driver_register = KPI_IMPORT(rt_spi_driver_register, 1648);
    rt_spi_device_register = KPI_IMPORT(rt_spi_device_register, 1649);
    rt_spi_bus_register = KPI_IMPORT(rt_spi_bus_register, 1650);
    rt_spi_bus_attach_device = KPI_IMPORT(rt_spi_bus_attach_device, 1651);
    rt_spi_bus_attach_device_cspin =
        KPI_IMPORT(rt_spi_bus_attach_device_cspin, 1652);
    rt_spi_bus_configure = KPI_IMPORT(rt_spi_bus_configure, 1653);
    rt_spi_take_bus = KPI_IMPORT(rt_spi_take_bus, 1654);
    rt_spi_release_bus = KPI_IMPORT(rt_spi_release_bus, 1655);
    rt_spi_take = KPI_IMPORT(rt_spi_take, 1656);
    rt_spi_release = KPI_IMPORT(rt_spi_release, 1657);
    rt_spi_configure = KPI_IMPORT(rt_spi_configure, 1658);
    rt_spi_send_then_recv = KPI_IMPORT(rt_spi_send_then_recv, 1659);
    rt_spi_send_then_send = KPI_IMPORT(rt_spi_send_then_send, 1660);
    rt_spi_transfer = KPI_IMPORT(rt_spi_transfer, 1661);
    rt_spi_sendrecv8 = KPI_IMPORT(rt_spi_sendrecv8, 1662);
    rt_spi_sendrecv16 = KPI_IMPORT(rt_spi_sendrecv16, 1663);
    rt_spi_transfer_message = KPI_IMPORT(rt_spi_transfer_message, 1664);

    rt_hw_dac_register = KPI_IMPORT(rt_hw_dac_register, 1752);
    rt_dac_write = KPI_IMPORT(rt_dac_write, 1753);
    rt_dac_enable = KPI_IMPORT(rt_dac_enable, 1754);
    rt_dac_disable = KPI_IMPORT(rt_dac_disable, 1755);

    rt_hw_watchdog_register = KPI_IMPORT(rt_hw_watchdog_register, 1767);

    rt_syscon_find_by_ofw_phandle =
        KPI_IMPORT(rt_syscon_find_by_ofw_phandle, 1779);
}
