# Zintercom  
##### Zigbee Intercom Automator  
This device is designed to control the matrix intercom using Zigbee.

Using zigbee2mqtt you can:
1. Receive notification when the intercom rings. (also support **direct bind**)
2. Mute the sound on the intercom handset.
3. Automatically or manually open the door or hang up when the intercom rings.

There are 4 work modes:  
1. `Never` - ordinary work mode, use handset to control  
2. `Once` - one time open door when intercom rings  
3. `Always` - open door when intercom rings  
4. `Drop` - hangs up all intercom rings, right after start  

You can change the operating mode uisng z2m or by short press the button. (< 1000ms)  
After pressing the button, the LED will flash at 4 Hz. 
The number of flashes indicates the current operating mode.  

You can control sound mode using z2m or by long press the button. (1000ms < X < 5000ms)  
After pressing the button, the LED will flash at 1.5 Hz.  
One flash - OFF, two flashes - ON.  

  
  
## Important info
#### 1
In Gerber PCB v1.0 committed on Apr 14, there are some errors in the silkscreen layers:
1. 'Line-' and 'Phone-' are reversed.
2. '+' and '-' are also reversed.  

Just plug by swapping.  
  
#### 2
The problem of cross-calls to other apartments:  
it was revealed that the intercom circuit is closed through the reverse diode of the u2 transitor (thanks to @Alex_AW)  
to prevent it, you need to add a diode according to the diagram:  
![](/images/fix_error_rings.png)  

#### 3
If you want to use the battery version without a handset, do not turn off the sound in z2m, but use an external resistor to emulate the intercom handset, according to the diagram:  
![](/images/battery_no_handset.png)    
   
   
### How to join
Reset to FN by reboot device 5 times with interval less than 10 seconds.  
Reset to FN by press and hold BTN for 5 seconds.  

LED will flash during reset.  

![](/images/z2m_dashboard.gif)  


### How to add device into zigbe2mqtt
Use [external converters](https://www.zigbee2mqtt.io/information/configuration.html#external-converters-configuration) feature

Converter file located [here](https://github.com/diyruz/Zintercom/blob/master/converters/DIYRuZ_Zintercom.js)  

![](/images/z2m_exposes.png)  

### Schematic

![](/hardware/Schematic_Zintercom.png)  


### PCB size  

`57.5mm x 27.3mm`  
![](/images/dimensions.png)  


### DC Power  

5-10 V  
  
Support `router` and `end device` modes.  
Install `E18-MS1PA1`, `E18-MS1PA2` or `E18-MS1-PCB`.   

![](/images/front_side.png)  

![](/images/back_DC_5-9V.png)  


### Battery Power  

2 * AAA batteries  
  
Support only `end device` mode.  
Install `E18-MS1-PCB` only!  

Do not turn off `sound` because it turns U2 forever, which will drain the battery much faster.

Since the device is in sleep mode, it cannot receive commands.  
But you have the ability to set the button mode or at the time of the call.  

If any commands are in the coordinator's queue, they will be executed after a button is pressed or a call is received.

![](/images/back_2xAAA.png)  

##### The mode set by the Zigbee command `overwrites` the mode set by the button.

### Binding
The device supports direct binding of an incoming call to the onOff cluster.  

For example, you can turn on the light while a call comes to the intercom, for notification in mute mode.


### Files to reproduce
* [Gerbers and BOM](https://github.com/diyruz/Zintercom/tree/master/hardware) by [xyzroe](https://t.me/xyzroe)  
* [Firmware](https://github.com/diyruz/Zintercom/releases) by [xyzroe](https://t.me/xyzroe)  
* [Case stl](https://www.thingiverse.com/thing:4866356) by [dreamertwo](https://t.me/dreamertwo) 

![](https://cdn.thingiverse.com/assets/0b/2f/09/d3/37/large_display_2021-05-22_22-55-43.JPG)

### Inspired by
The original scheme of the intercom opener by [Alexander Vaidurov](https://easyeda.com/Alex_AW/domofon-with-battery)  
Various hardware solutions by [Jager](https://modkam.ru)  
Firmware for different Zigbee devices by [Anonymous](https://github.com/nurikk/)  
