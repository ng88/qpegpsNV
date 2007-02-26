This is beta version with A. Karkhov additions.
Tested only on Zaurus SL-C860 under QTopia (no known bugs).
Known problems with Open Zaurus.
Use runcompat utility to start on Open Zaurus.

Whats new in 0.9.2.1:
1. Added "map mode" to browsing maps (with or without GPS receiver).
2. Added saving/deleting places/waypoints.
3. Added setting destination point (qpegps will display bearing and distance to destination).
4. Added measuring distances between any points on the maps.
5. Added scale line.

Whats new in 0.9.2.2:
1. Some minor fixes.
2. Added downloading multiple map files which cover some area (region) from www.expedia.com

Whats new in 0.9.2.3:
1. minor bug fixes
2. In menu which is available by <OK> button now added two items first: Switch view (full screen or normal)  second: Switch mode (GPS or Map)
3. In GPS mode now you also can click to any point on the map and you will see dialog with point position (coordinates) and distance from current position to the pointed point, also in this dialog you can save pointed position in places.txt file (by pressing "Save" button) or Set up pointed position as destination (by pressing "Destination" button).
4. You can switch between full screen and normal view by pressing <Cancel> / <Esc> button

Whats new in 0.9.2.3.1:

1. According to user's requests now you can select qpeGPS startup mode. For this you can check "Map mode" check box for startup in Map mode or uncheck "Map mode" check box for startup in GPS mode.
2. Also you can choose startup "place" (position) by choosing "place" in listbox near "Map mode" check box.

Whats new in 0.9.2.3.2:

1. New feature: Now available drawing of the saved places (waypoints) on the moving map. For switching on this mode you can press "OK" button and chose "Draw places" menu item. You can switching off this mode you can press "OK" button and chose "Don't draw places" menu item. 
2. New feature: Now available animation of the saved tracks by the moving map. For start of the animation you can (in "Map mode") press "OK" button and chose "Animate track" menu item. Then you can choose track file (saved in /home/QtPalmtop/qpegps/tracks/ directory). Also you can choose "Time accelerator" which is accelerate animation time. For example if you set "Time acceleration:"=10 time in animation will be 10 times faster then real time.You can setup "Time Zone: GMT+" field you can enter time shifting from GMT (Grinvich Mean Time). Also if you will check "Shortcut large time interval" all time intervals (accelerated not original) larger then 1 minutes will be converted to 5 second. This is useful if you animate track with long term stops.
 To stop animation you can in "Map mode" press "OK" button and chose "Stop track animation" menu item. 
3. In map mode even if map is not available you can move coordinates by arrows buttons. This is useful to return back to area which is covered by map.
4. One additional source (http://www.multimap.com) added to download many maps. But this source genereate only low resolution maps.
5. Now qpegps save altitude in the places file. But places.txt file format now is not compatible with previous version. Sorry...
6. Known problem. This is a problem in Cacko ROM v 1.23. setting 4800 baud rate frozen Zaurus (stty 4800 < ttyS3). This is the problem with serial_cs driver, For solving problem in the console enter command:
$ cardctl ident 0
product info: "TFAC  ", "CF30   GPS", "2001", "  "
function: 2 (serial)

Then copy product info strings to the file:  /etc/bluetooth/serial and add to the end of the string such substring: ":baud_base 1000000 spd_cust divisor 24"
For this example whole string will:
"TFAC  ", "CF30   GPS", "2001", "  ":baud_base 1000000 spd_cust divisor 24
After it you must use 38400 speed instead of 4800. So for 4800 GPS card working on /dev/ttyS3 port use gpsd Args: -p /dev/ttyS3 -s 38400.
So you must use 8 times higher speed.

Whats new in 0.9.2.3.3:

1. International encoding for saving places now is available. Places (in places.txt file) now storing in the UTF8 encoding.

2. Help file now also work.

3. Many peoples reported problem with wakeup CF GPS cards after suspending (pressing power "off/on" button). If you have a problems with resuming qpegps/gpsd operations (after soft off/on Data Status: gpsd: is ERR and GPS: is red). Typically it happens in Cacko ROM v 1.22 or later. 
You can try to add in the /etc/apm.d/bluetooth file (or in the some another such file) at the end of "resume)" section such four commands:
sleep 3
 killall -SIGCONT qpegps
sleep 12
 killall -SIGUSR1 qpegps

 4. If you have problems with storing places (waypoints) and maps from the console run such commsnds:
 chmod uoga=rw /opt/QtPalmtop/qpegps/maps
chmod uoga=rw /opt/QtPalmtop/qpegps/maps/maps.txt
chmod uoga=rw /opt/QtPalmtop/qpegps/maps/sources.txt
chmod uoga=rw /opt/QtPalmtop/qpegps/tracks
chmod uoga=rw /opt/QtPalmtop/qpegps/tracks/places.txt