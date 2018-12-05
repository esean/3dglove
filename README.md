# 3dglove


Virtual reality haptic glove, venture finalist in 2015 Montery Bay Startup Challenge

Software on Mac used the Leap Motion controller to determine user hand location and fed that info into a physics engine to determine where the hand collided with virtual shapes. That collision information was then used to vibrate the glove at the location and with correct collision intensity to provide haptic feedback to user about touching objects in 3D virtual space.

Glove hardware provided 24 points of haptic info per hand (ie, 24 vibrators), driven by a 24-channel 12-bit LED driver TLC5947. Driver updated via FTDI SPI MPSSE cable, shipping 288bits to LED driver at ~100Hz update rate.

https://www.santacruzsentinel.com/2015/04/30/santa-cruz-hopefuls-vie-for-20000-in-startup-challenge/

Another team member posted a video showing the Mac UI and Leap motion controller, but the glove is not being used in this demo,

https://www.youtube.com/watch?v=leHrdxnxjvA

