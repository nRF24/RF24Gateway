#!/usr/bin/env python
#
# TMRh20 - 2015
# A simple example of controlling a RF24Ethernet based sensor node via a python script
# Turn a light on in the evening and off in the morning according to a schedule
#

import urllib2
from datetime import datetime
import time
import syslog

######### Configuration #########

scheduleON = 21 #The hour (24hr clock) to turn the system on (21:00)
scheduleOFF = 6 #The hour (24hr clock) to turn the system off (06:00)
sensorIP = "10.10.2.4" #The IP of a sensor node running one of the server examples

#################################

requestON = "http://" + sensorIP + ":1000/ON"
requestOFF = "http://" + sensorIP + ":1000/OFF"
lightState = 2
		
while 1:

    #Get the current hour
    currentHour = datetime.now().hour

	#Check to see if the system should be off
    if (currentHour >= scheduleON or currentHour < scheduleOFF) and lightState != 1:
        
        result = 0;
		#Connect to our sensor at 10.10.3.44:1000/ and request OFF
        try:
            response = urllib2.urlopen(requestOFF,None,15) #15 second time-out
            result = response.getcode()
        except urllib2.HTTPError, e:
            syslog.syslog('HTTPError = ' + str(e.code))
        except urllib2.URLError, e:
            syslog.syslog('URLError = ' + str(e.reason))
        except Exception:
            import traceback
            syslog.syslog('generic exception: ' + traceback.format_exc())
        #Log errors and successful results to /var/log/syslog
        if result == 200:
            syslog.syslog( "Light off at %s\n" % datetime.now())
			#Only change the light state if successful
            lightState = 1

	#Else, check to see if the system should be on
    elif currentHour >= scheduleOFF and lightState != 0:
        result = 0;

        try:
            response = urllib2.urlopen(requestON,None,15) #15 second time-out
            result = response.getcode()
        except urllib2.HTTPError, e:
            syslog.syslog('HTTPError = ' + str(e.code))
        except urllib2.URLError, e:
            syslog.syslog('URLError = ' + str(e.reason))
        except Exception:
            import traceback
            syslog.syslog('generic exception: ' + traceback.format_exc())
        #Log errors and successful results to /var/log/syslog
        if result == 200: 
            syslog.syslog("Light on at %s\n" % datetime.now())
			#Only change the light state if successful
            lightState = 0
            
    #Wait 1 minute between retries and status checks
    time.sleep(60)
	
