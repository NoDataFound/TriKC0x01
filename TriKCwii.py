#!/usr/bin/python
#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#|T|R|i|K|C|-|C|o|r|y|K|e|n|n|e|d|y|1|3|3|7|
#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#
# Connect a Nintendo Wii Remote via Bluetooth
# and  read the button states in Python.
#
# Date   : 11/03/2014
# -----------------------
# Import required Python libraries
# -----------------------
import cwiid
import time

button_delay = 0.1

print 'Press 1 + 2 on your Wii Remote now ...'
time.sleep(1)

# Connect to the Wii Remote. If it times out
# then quit.
try:
  wii=cwiid.Wiimote()
except RuntimeError:
  print "Error opening wiimote connection"
  quit()

print 'Wii Remote connected...\n'
print 'Press some buttons!\n'
print 'Press PLUS and MINUS together to disconnect and quit.\n'

wii.rpt_mode = cwiid.RPT_BTN
 
while True:

  buttons = wii.state['buttons']

  # If Plus and Minus buttons pressed
  # together then rumble and quit.
  if (buttons - cwiid.BTN_PLUS - cwiid.BTN_MINUS == 0):  
    print '\nClosing connection ...'
    wii.rumble = 1
    time.sleep(1)
    wii.rumble = 0
    exit(wii)  
  
  # Check if other buttons are pressed by
  # doing a bitwise AND of the buttons number
  # and the predefined constant for that button.
  if (buttons & cwiid.BTN_LEFT):
    print 'Left pressed'
    time.sleep(button_delay)         

  if(buttons & cwiid.BTN_RIGHT):
    print 'Right pressed'
    time.sleep(button_delay)          

  if (buttons & cwiid.BTN_UP):
    print 'Up pressed'        
    time.sleep(button_delay)          
    
  if (buttons & cwiid.BTN_DOWN):
    print 'Down pressed'      
    time.sleep(button_delay)  
    
  if (buttons & cwiid.BTN_1):
    print 'Button 1 pressed'
    time.sleep(button_delay)          

  if (buttons & cwiid.BTN_2):
    print 'Button 2 pressed'
    time.sleep(button_delay)          

  if (buttons & cwiid.BTN_A):
    print 'Button A pressed'
    time.sleep(button_delay)          

  if (buttons & cwiid.BTN_B):
    print 'Button B pressed'
    time.sleep(button_delay)          

  if (buttons & cwiid.BTN_HOME):
    print 'Home Button pressed'
    time.sleep(button_delay)           
    
  if (buttons & cwiid.BTN_MINUS):
    print 'Minus Button pressed'
    time.sleep(button_delay)   
    
  if (buttons & cwiid.BTN_PLUS):
    print 'Plus Button pressed'
    time.sleep(button_delay)
