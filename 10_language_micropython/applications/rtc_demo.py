from machine import I2C
import time

i2c = I2C(0)
ADDR = 0x51

def hym8563s_write(reg, val):
    buf = bytes([reg, val])
    i2c.writeto(ADDR, buf)

def bcd2dec(bcd):
    return ((bcd >> 4) * 10) + (bcd & 0x0F)

def hym8563s_read(reg, length=1):
    return i2c.readfrom_mem(ADDR, reg, length)

def hym8563s_set_time():
    hym8563s_write(0x01, 0x0A)
    hym8563s_write(0x02, 0x56)  # 秒
    hym8563s_write(0x03, 0x59)  # 分
    hym8563s_write(0x04, 0x23)  # 时
    hym8563s_write(0x05, 0x31)  # 日
    hym8563s_write(0x07, 0x12)  # 月
    hym8563s_write(0x08, 0x25)  # 年
    hym8563s_write(0x06, 0x01)  # 星期
    hym8563s_write(0x01, 0x0A)

def hym8563s_get_time():
    data = hym8563s_read(0x02, 7)
    sec    = bcd2dec(data[0] & 0x7F)
    minute = bcd2dec(data[1] & 0x7F)
    hour   = bcd2dec(data[2] & 0x3F)
    day    = bcd2dec(data[3] & 0x3F)
    week   = data[4] & 0x07
    month  = bcd2dec(data[5] & 0x1F)
    year   = bcd2dec(data[6])
    return "20%02d-%02d-%02d  %02d:%02d:%02d  week:%d" % (year, month, day, hour, minute, sec, week)

def run_example():
    hym8563s_set_time()
    for i in range(10):
        print(hym8563s_get_time())
        time.sleep(1)

run_example()
