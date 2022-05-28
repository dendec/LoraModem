import serial
import pytest
import time

modem0 = serial.Serial('/dev/ttyUSB0', 115200, timeout=60)
modem1 = serial.Serial('/dev/ttyUSB1', 115200, timeout=60)

def send(ser, data):
    ser.write(data)

def receive(ser, size):
    return ser.read(size)

def assert_equals(actual, expected):
    assert actual == expected

def test_transmission_simple():
    payload = b'test'
    send(modem0, payload)
    actual = receive(modem1, 4)
    assert_equals(actual, payload)
    send(modem1, payload)
    actual = receive(modem0, 4)
    assert_equals(actual, payload)

def test_transmission_allchars():
    payload = bytes([*range(0, 250, 1)])
    send(modem0, payload)
    actual = receive(modem1, 250)
    assert_equals(actual, payload)

def test_transmission_long_message():
    length = 1000
    payload = bytes( [85] * length )
    now = time.time_ns()
    send(modem0, payload)
    actual = receive(modem1, length)
    latency = (time.time_ns() - now) / 1e9
    bitrate = length / latency
    print("\n{}, {}, {}".format(length, latency, bitrate))
    assert_equals(len(actual), len(payload))
    assert_equals(actual, payload)

#@pytest.mark.skip(reason="can take a lot of time")
def test_transmission_by_length():
    errors_count = 0
    wrong_length_errors_count = 0
    for length in range(5, 1000, 5):
        payload = bytes([85] * length)
        now = time.time_ns()
        send(modem0, payload)
        actual = receive(modem1, length)
        latency = (time.time_ns() - now) / 1e9
        bitrate = length / latency
        print("{}, {}, {}".format(length, latency, bitrate))
        if actual != payload:
            errors_count += 1
            if len(actual) != len(payload):
                wrong_length_errors_count += 1
    print("errors {}".format(errors_count))
    print("wrong_length_errors_count {}".format(wrong_length_errors_count))