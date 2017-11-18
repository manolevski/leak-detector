import sys
import glob
import sqlite3 as lite
import serial

def serial_port():
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            ser = serial.Serial(port, 9600)
            line = ser.readline()
            if ':' in line:
                ser.close()
                result.append(port)
        except (OSError, serial.SerialException):
            pass
    if len(result) > 1:
        raise EnvironmentError('Multiple devices not supported.')
    elif len(result) == 0:
        raise EnvironmentError('Device not found.')
    return result[0]

if __name__ == '__main__':
    portName = serial_port()
    device = serial.Serial(portName, 9600)

    with lite.connect('storage.db') as con:
        cur = con.cursor()
        cur.execute("CREATE TABLE IF NOT EXISTS Sensors("
                    "ID INT PRIMARY KEY,"
                    "SensorID INT NOT NULL,"
                    "Value REAL NOT NULL,"
                    "TimeStamp INT NOT NULL)")
    while True:
        data = device.readline().split(":")

        with lite.connect('storage.db') as con:
            cur = con.cursor()
            cur.execute("INSERT INTO Sensors (SensorID,Value,TimeStamp)"
                        "VALUES (?, ?, current_timestamp)", (data[0], data[1]))
