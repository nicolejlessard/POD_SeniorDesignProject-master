import serial

ser = serial.Serial('/dev/rfcomm0', 9600)

while True:
    result = ser.readline()
    result = result.translate(None, ';') # remove ; from string
    humidity, temp, moisture = result.split(",")
    #print humidity + ", " + temp + ", " + moisture + ";"
    print result
