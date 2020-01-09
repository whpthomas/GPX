import gpx
gpx.connect("/dev/ttyACM0", 0, "/home/pi/gpx.ini")
gpx.write("M72 P1")
gpx.disconnect()

