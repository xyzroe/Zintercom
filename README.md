# Zintercom  
## Zigbee Intercom Automator

### How to join
Reset to FN rebooting device 5 times with interval less than 10 seconds, led will start flashing during reset
Reset to FN by pressing and holding button(1) 5 seconds



### How to add device into zigbe2mqtt
Use [external converters](https://www.zigbee2mqtt.io/information/configuration.html#external-converters-configuration) feature

Converter file located [here](./converters/DIY_Zintercom.js)

### PCB size  
  
`57.5mm x 27.3mm`  
<img src="./images/dimensions.png" width="60%">


### DC Power  
  
Support `router` and `end device` modes.  
Install `E18-MS1PA1`, `E18-MS1PA2` or `E18-MS1-PCB`.  
<img src="./images/front_side.png" width="70%">
<img src="./images/back_DC_5-9V.png" width="35%">
<img src="./images/back_microUSB.png" width="35%">


### Battery Power  
  
Support only `end device` mode.  
Install `E18-MS1-PCB` only!  
<img src="./images/back_2xAAA.png" width="35%">
<img src="./images/back_CR2032.png" width="35%">
