def send(ser, data):
    ser.write(data)

def send_command(ser, command):
    send(ser, command)
    result = ser.readline().decode("utf-8").strip()
    return result

def assert_equals(actual, expected):
    assert actual == expected