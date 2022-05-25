import re
import serial
import time
import pytest

ser = serial.Serial('/dev/heltec', 115200, timeout=0.5)

def send(data):
    ser.write(data)

def send_command(command):
    send(command)
    result = ser.read(size=256).decode("utf-8")
    result = re.sub(r'\[VERBOSE\].+', '', result).strip()
    return result

def assert_address(actual):
    assert re.match(r'[0-9A-F]{4}', actual) is not None

def assert_equals(actual, expected):
    assert actual == expected

def assert_ok(actual):
    assert_equals(actual, 'OK')

def assert_empty(actual):
    assert_equals(actual, '')

def test_AT_succeeds():
    assert_ok(send_command(b'AT\n'))

def test_AT_fails():
    assert_empty(send_command(b'AT1\n'))
    assert_empty(send_command(b'AT=1\n'))
    assert_empty(send_command(b'AT?\n'))

def test_ATZ():
    assert_ok(send_command(b'ATZ\n'))

def test_AT_CONF():
    assert_equals(send_command(b'AT+CONF\n')[5:], "434.0,500.0,9,7,10,0")

def test_AT_CONF_set_fails():
    assert_empty(send_command(b'AT+CONF=434.0\n'))

def test_AT_ADDR():
    assert_address(send_command(b'AT+ADDR\n'))

def test_AT_ADDR_set_fails():
    assert_empty(send_command(b'AT+ADDR=FACE\n'))

def test_AT_FREQ():
    assert_equals(send_command(b'AT+FREQ\n'), '434.0')

def test_AT_FREQ_set():
    assert_ok(send_command(b'AT+FREQ=433.0\n'))
    assert_equals(send_command(b'AT+FREQ\n'), '433.0')
    assert_equals(send_command(b'AT+CONF\n')[5:], "433.0,500.0,9,7,10,0")
    assert_ok(send_command(b'ATZ\n'))

def test_AT_FREQ_set_wrong():
    assert_equals(send_command(b'AT+FREQ=409.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')
    assert_equals(send_command(b'AT+FREQ=526.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')
    assert_equals(send_command(b'AT+FREQ=-434.0\n'), 'Invalid frequency. Valid values: 410.0-525.0')

def test_AT_BANDWIDTH():
    assert_equals(send_command(b'AT+BANDWIDTH\n'), '500.0')

def test_AT_BANDWIDTH_set():
    assert_ok(send_command(b'AT+BANDWIDTH=250.0\n'))
    assert_equals(send_command(b'AT+BANDWIDTH\n'), '250.0')
    assert_equals(send_command(b'AT+CONF\n')[5:], "434.0,250.0,9,7,10,0")
    assert_ok(send_command(b'ATZ\n'))

def test_AT_BANDWIDTH_set_wrong():
    assert_equals(send_command(b'AT+BANDWIDTH=100.0\n'), 'Invalid bandwidth. Valid values: 62.5, 125.0, 250.0, 500.0')
    assert_equals(send_command(b'AT+BANDWIDTH=501.0\n'), 'Invalid bandwidth. Valid values: 62.5, 125.0, 250.0, 500.0')
    assert_equals(send_command(b'AT+BANDWIDTH=-500.0\n'), 'Invalid bandwidth. Valid values: 62.5, 125.0, 250.0, 500.0')

def test_AT_SFACTOR():
    assert_equals(send_command(b'AT+SFACTOR\n'), '9')

def test_AT_SFACTOR_set():
    assert_ok(send_command(b'AT+SFACTOR=7\n'))
    assert_equals(send_command(b'AT+SFACTOR\n'), '7')
    assert_equals(send_command(b'AT+CONF\n')[5:], "434.0,500.0,7,7,10,0")
    assert_ok(send_command(b'ATZ\n'))

def test_AT_SFACTOR_set_wrong():
    assert_equals(send_command(b'AT+SFACTOR=0\n'), 'Invalid spread factor. Valid values: 6, 7, 8, 9, 10, 11, 12')
    assert_equals(send_command(b'AT+SFACTOR=4\n'), 'Invalid spread factor. Valid values: 6, 7, 8, 9, 10, 11, 12')
    assert_equals(send_command(b'AT+SFACTOR=-5\n'), 'Invalid spread factor. Valid values: 6, 7, 8, 9, 10, 11, 12')


def test_AT_RATE():
    assert_equals(send_command(b'AT+RATE\n'), '7')

def test_AT_RATE_set():
    assert_ok(send_command(b'AT+RATE=5\n'))
    assert_equals(send_command(b'AT+RATE\n'), '5')
    assert_equals(send_command(b'AT+CONF\n')[5:], "434.0,500.0,9,5,10,0")
    assert_ok(send_command(b'ATZ\n'))

def test_AT_RATE_set_wrong():
    assert_equals(send_command(b'AT+RATE=0\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')
    assert_equals(send_command(b'AT+RATE=4\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')
    assert_equals(send_command(b'AT+RATE=-5\n'), 'Invalid code rate. Valid values: 5, 6, 7, 8')

def test_AT_POWER():
    assert_equals(send_command(b'AT+POWER\n'), '10')

def test_AT_POWER_set():
    assert_ok(send_command(b'AT+POWER=7\n'))
    assert_equals(send_command(b'AT+POWER\n'), '7')
    assert_equals(send_command(b'AT+CONF\n')[5:], "434.0,500.0,9,7,7,0")
    assert_ok(send_command(b'ATZ\n'))

def test_AT_POWER_set_wrong():
    assert_equals(send_command(b'AT+POWER=-10\n'), 'Invalid power. Valid values: -3-17')
    assert_equals(send_command(b'AT+POWER=18\n'), 'Invalid power. Valid values: -3-17')
    assert_equals(send_command(b'AT+POWER=30\n'), 'Invalid power. Valid values: -3-17')

@pytest.mark.skip(reason="can have unpredicted value")
def test_AT_SCAN():
    assert_equals(send_command(b'AT+SCAN\n'), '')

def test_AT_WIFI():
    assert_ok(send_command(b'ATZ\n'))
    assert_equals(send_command(b'AT+WIFI\n'), '1,1,,')

def test_AT_WIFI():
    assert_ok(send_command(b'AT+WIFI=0\n'))
    assert_equals(send_command(b'AT+WIFI\n'), '0,1,,')
    assert_ok(send_command(b'AT+WIFI=1, 0\n'))
    assert_equals(send_command(b'AT+WIFI\n'), '1,1,,')
    assert_ok(send_command(b'AT+WIFI= 1, 6,test_ssid,supersecure\n'))
    assert_equals(send_command(b'AT+WIFI\n'), '1,6,test_ssid,*')
    assert_ok(send_command(b'ATZ\n'))
    assert_equals(send_command(b'AT+WIFI\n'), '1,1,,')

def test_AT_STAT():
    send_command(b'AT+RESTART\n')
    time.sleep(1)
    assert_equals(send_command(b'AT+STAT\n'), '0,0')
    send(b'test123')
    time.sleep(0.2)
    assert_equals(send_command(b'AT+STAT\n'), '7,0')