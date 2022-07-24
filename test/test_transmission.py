import serial
import pytest
import random
import time
from test_common import send, send_command, assert_equals
from os.path import exists

modem0 = serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
modem1 = serial.Serial('/dev/ttyUSB1', 115200, timeout=10)

def send(ser, data):
    ser.write(data)

def assert_equals(actual, expected):
    assert actual == expected

def test_transmission_simple():
    payload = b'test'
    send(modem0, payload)
    actual = modem1.read(4)
    assert_equals(actual, payload)
    send(modem1, payload)
    actual = modem0.read(4)
    assert_equals(actual, payload)

def test_transmission_allchars():
    payload = bytes([*range(0, 250, 1)])
    send(modem0, payload)
    actual = modem1.read(250)
    assert_equals(actual, payload)

def test_transmission_long_message():
    len_ok, msg_ok = transmit(1000)
    assert len_ok
    assert msg_ok

@pytest.mark.skip(reason="takes a lot of time")
def test_transmission_by_length():
    for sf in range(6, 13, 1):
        set_SF(sf)
        for rate in range(5, 9, 1):
            set_RATE(rate)
            for bw in [500, 250, 125, 62, 41, 31, 20, 15, 10, 7]:
                set_BW(bw)
                conf = send_command(modem0, b'AT+CONF\n')
                print("\n{}".format(conf))
                filename = f"{conf}.csv"
                if (exists(filename)):
                    continue
                f = open(f"{conf}.csv", "w")
                f.write(f"length, latency, bitrate, len_ok, msg_ok\n")
                failures = 0
                for length in range(1, 251, 1):
                    latency, bitrate, msg_ok, len_ok = transmit(length)
                    print(f"{length}, {latency}, {bitrate}, {len_ok}, {msg_ok}")
                    while not msg_ok and failures < 10:
                        latency, bitrate, msg_ok, len_ok = transmit(length)
                        print(f"{length}, {latency}, {bitrate}, {len_ok}, {msg_ok}")
                        failures = failures + 1
                    if failures == 10:
                        break
                    else:
                        failures = 0
                    f.write(f"{length}, {latency}, {bitrate}, {len_ok}, {msg_ok}\n")
                f.close()

def transmit(length):
    payload = bytes( random.sample(list(range(1, 256))*length, length) )
    now = time.time_ns()
    send(modem0, payload)
    actual = modem1.read(length)
    latency = (time.time_ns() - now) / 1e9
    bitrate = int(length * 8 / latency)
    return [int(latency * 1000), bitrate, actual == payload, len(actual) == length]

def set_SF(sf):
    update_modems_config(f"AT+SF={sf}\n".encode())

def set_RATE(r):
    update_modems_config(f"AT+RATE={r}\n".encode())

def set_BW(bw):
    update_modems_config(f"AT+BW={bw}\n".encode())

def reset(): 
    update_modems_config(b'AT+RST\n')
    time.sleep(5)
    modem0.read(size=256)
    modem1.read(size=256)

def update_modems_config(at_command):
    send_command(modem0, at_command)
    send_command(modem1, at_command)