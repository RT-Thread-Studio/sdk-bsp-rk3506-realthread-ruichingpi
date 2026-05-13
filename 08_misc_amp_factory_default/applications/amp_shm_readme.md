# SHM Demo

## 测试中断收发

- Linux发中断，RT-Thread等中断

  ```shell
  RT-Thread $ amp_shm_test amp-shm wait_notify
  linux # amp_shm_device_app /dev/amp_shm send_notify
  ```

  

- Linux等中断，RT-Thread发中断

  ```shell
  linux #  amp_shm_device_app /dev/amp_shm wait_notify
  RT-Thread $ amp_shm_test amp-shm send_notify
  ```

  

## 测试数据收发

- Linux写数据，RT-Thread读数据

  ```shell
  linux # amp_shm_device_app /dev/amp_shm write 0 "hello"
  RT-Thread $ amp_shm_test amp-shm read  0 16
  ```

  RT-Thread 有打印，字符与 Linux 写入的相同。

  

- Linux读数据，RT-Thread写数据

  ```shell
  RT-Thread $ amp_shm_test amp-shm write 0 "world"
  linux # amp_shm_device_app /dev/amp_shm read  0 16
  ```

  Linux 有打印，字符与 RT-Thread 写入相同。

