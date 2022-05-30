import re
import serial
import time
import pytest

from test_common import send, send_command, assert_equals

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.5)

def assert_address(actual):
    assert re.match(r'[0-9A-F]{4}', actual) is not None

def assert_ok(actual):
    assert_equals(actual, 'OK')

def assert_empty(actual):
    assert_equals(actual, '')

def test_AT_succeeds():
    assert_ok(send_command(ser, b'AT\n'))

def test_AT_fails():
    assert_empty(send_command(ser, b'AT1\n'))
    assert_empty(send_command(ser, b'AT=1\n'))
    assert_empty(send_command(ser, b'AT?\n'))

def test_ATZ():
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_CONF():
    assert_equals(send_command(ser, b'AT+CONF\n')[5:], "434.0,500.0,9,7,10,0")

def test_AT_CONF_set_fails():
    assert_empty(send_command(ser, b'AT+CONF=434.0\n'))

def test_AT_ADDR():
    assert_address(send_command(ser, b'AT+ADDR\n'))

def test_AT_ADDR_set_fails():
    assert_empty(send_command(ser, b'AT+ADDR=FACE\n'))

def test_AT_FREQ():
    assert_equals(send_command(ser, b'AT+FREQ\n'), '434.0')

def test_AT_FREQ_set():
    for freq in range(410, 525, 5):
        assert_ok(send_command(ser, f"AT+FREQ={freq:.1f}\n".encode()))
        assert_equals(send_command(ser, b'AT+FREQ\n'), f"{freq:.1f}")
        assert_equals(send_command(ser, b'AT+CONF\n')[5:], f"{freq:.1f},500.0,9,7,10,0")
        print(freq)
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_FREQ_set_wrong():
    assert_equals(send_command(ser, b'AT+FREQ=409.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')
    assert_equals(send_command(ser, b'AT+FREQ=526.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')
    assert_equals(send_command(ser, b'AT+FREQ=-434.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')

def test_AT_BANDWIDTH():
    assert_equals(send_command(ser, b'AT+BW\n'), '500')

def test_AT_BANDWIDTH_set():
    bw_map = {7: 7.8, 10: 10.4, 15: 15.6, 20: 20.8, 31: 31.2, 41: 41.7, 62: 62.5, 125: 125.0, 250: 250.0, 500: 500.0}
    for bw in bw_map.keys():
        assert_ok(send_command(ser, f"AT+BW={bw}\n".encode()))
        assert_equals(send_command(ser, b'AT+BW\n'), f"{bw}")
        assert_equals(send_command(ser, b'AT+CONF\n')[5:], f"434.0,{bw_map[bw]},9,7,10,0")
        print(bw)
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_BANDWIDTH_set_wrong():
    assert_equals(send_command(ser, b'AT+BW=100.0\n'), 'Invalid bandwidth. Valid values: 7, 10, 15, 20, 31, 41, 62, 125, 250, 500')
    assert_equals(send_command(ser, b'AT+BW=501.0\n'), 'Invalid bandwidth. Valid values: 7, 10, 15, 20, 31, 41, 62, 125, 250, 500')
    assert_equals(send_command(ser, b'AT+BW=-500.0\n'), 'Invalid bandwidth. Valid values: 7, 10, 15, 20, 31, 41, 62, 125, 250, 500')

def test_AT_SFACTOR():
    assert_equals(send_command(ser, b'AT+SF\n'), '9')

def test_AT_SFACTOR_set():
    for sf in range(7, 13, 1):
        assert_ok(send_command(ser, f"AT+SF={sf}\n".encode()))
        assert_equals(send_command(ser, b'AT+SF\n'), f"{sf}")
        assert_equals(send_command(ser, b'AT+CONF\n')[5:], f"434.0,500.0,{sf},7,10,0")
        print(sf)
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_SFACTOR_set_wrong():
    assert_equals(send_command(ser, b'AT+SF=0\n'), 'Invalid spread factor. Valid values: 7, 8, 9, 10, 11, 12')
    assert_equals(send_command(ser, b'AT+SF=4\n'), 'Invalid spread factor. Valid values: 7, 8, 9, 10, 11, 12')
    assert_equals(send_command(ser, b'AT+SF=-5\n'), 'Invalid spread factor. Valid values: 7, 8, 9, 10, 11, 12')

def test_AT_RATE():
    assert_equals(send_command(ser, b'AT+RATE\n'), '7')

def test_AT_RATE_set():
    for r in range(5, 9, 1):
        assert_ok(send_command(ser, f"AT+RATE={r}\n".encode()))
        assert_equals(send_command(ser, b'AT+RATE\n'), f"{r}")
        assert_equals(send_command(ser, b'AT+CONF\n')[5:], f"434.0,500.0,9,{r},10,0")
        print(r)
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_RATE_set_wrong():
    assert_equals(send_command(ser, b'AT+RATE=0\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')
    assert_equals(send_command(ser, b'AT+RATE=4\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')
    assert_equals(send_command(ser, b'AT+RATE=-5\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')

def test_AT_POWER():
    assert_equals(send_command(ser, b'AT+POW\n'), '10')

def test_AT_POWER_set():
    for p in range(2, 18, 1):
        assert_ok(send_command(ser, f"AT+POW={p}\n".encode()))
        assert_equals(send_command(ser, b'AT+POW\n'), f"{p}")
        assert_equals(send_command(ser, b'AT+CONF\n')[5:], f"434.0,500.0,9,7,{p},0")
        print(p)
    assert_ok(send_command(ser, b'ATZ\n'))

def test_AT_POWER_set_wrong():
    assert_equals(send_command(ser, b'AT+POW=-10\n'), 'Invalid power. Valid values: 2-17,20')
    assert_equals(send_command(ser, b'AT+POW=18\n'), 'Invalid power. Valid values: 2-17,20')
    assert_equals(send_command(ser, b'AT+POW=30\n'), 'Invalid power. Valid values: 2-17,20')

@pytest.mark.skip(reason="can have unpredicted value")
def test_AT_SCAN():
    assert_equals(send_command(ser, b'AT+SCAN\n'), '')

def test_AT_WIFI():
    assert_ok(send_command(ser, b'ATZ\n'))
    assert_equals(send_command(ser, b'AT+WIFI\n'), '1,1,,')

def test_AT_WIFI():
    assert_ok(send_command(ser, b'AT+WIFI=0\n'))
    assert_equals(send_command(ser, b'AT+WIFI\n'), '0,1,,')
    assert_ok(send_command(ser, b'AT+WIFI=1, 0\n'))
    assert_equals(send_command(ser, b'AT+WIFI\n'), '1,1,,')
    assert_ok(send_command(ser, b'AT+WIFI= 1, 6,test_ssid,supersecure\n'))
    assert_equals(send_command(ser, b'AT+WIFI\n'), '1,6,test_ssid,*')
    assert_ok(send_command(ser, b'ATZ\n'))
    assert_equals(send_command(ser, b'AT+WIFI\n'), '1,1,,')

def test_AT_STAT():
    send_command(ser, b'AT+RST\n')
    ser.read(size=256)
    assert_equals(send_command(ser, b'AT+STAT\n'), '0,0')
    send(ser, b'test123')
    time.sleep(0.2)
    assert_equals(send_command(ser, b'AT+STAT\n'), '7,0')