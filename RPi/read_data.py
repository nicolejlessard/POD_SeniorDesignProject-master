import serial
import time as t
import datetime
import sqlite3

db = sqlite3.connect("/home/pi/POD/POD.db")
com = '/dev/rfcomm0'

counter = 0

last_halert = 0
last_talert = 0
last_salert = 0

while True:

    # **** READING DATA ****
    s = serial.Serial(com, baudrate = 9600) # open bluetooth serial port

    if s.isOpen():
        data = s.readline() # read three bytes of data
        data = data.translate(None, ';') # remove ; from string
        humidity, temp, moisture = data.split(",") # split string on ,
    else:
        print "Serial port has not been opened properly"

    if (humidity < 40) | (humidity > 50):
        if datetime.date.today() != last_halert:
            print "Humdity is " + humidity + "%!"
            last_halert = datetime.date.today()
    if (temp < 18.3) | (temp > 23.9):
        if datetime.date.today() != last_talert:
            print "Temperature is " + temp + "C!"
            last_talert = datetime.date.today()
    if (moisture < 30) | (moisture > 60):    
        if datetime.date.today() != last_salert:
            print "Soil moisture is " + moisture + "%!" 
            last_salert = datetime.date.today()

    if counter < 1440:   
        counter = counter+1

    # **** SAVING DATA ****
    db.execute("INSERT INTO temps VALUES ((DATETIME('now','localtime')), "+ humidity + ", " + temp + ", " + moisture + ");")
    db.commit()

    if(counter >= 1440): #1440 iteration is a full 24 hours
        db.execute("DELETE FROM TABLE temps JOIN (SELECT MIN(datetime) AS min_time FROM TABLE) time WHERE temps.datetime = time.min_time")
        db.commit() # remove oldest record (24 hours ago)

    # **** PLOTTING DATA ****
