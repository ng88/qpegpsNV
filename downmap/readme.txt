This is utility to downloading multiple map files which cover some area (region) from www.expedia.com

Requrements:
1. Utilities: wget, gif2png
OS / Platform:
1. ARM,  QT, Zaurus (directory: arm_QT)
2. i386, Linux (directory: i386_Linux)
3. i386, Windows (you can compile it)

Usage:
1. unzip downmap_arm_i386_Linux.zip
2. run Console (terminal)
3. enter (example):
downmap -sla 36.5 -ela 59.0 -slo -9.5 -elo 19.0 -sc 300000 -md /home/QtPalmtop/qpegps/maps/ -P m
or:
downmap -la 47 -slo 1 -a 200x100 -sc 300000 -md /home/QtPalmtop/qpegps/maps/ -P m

run "downmap -h" for help



if you use proxy enter:
run Console (terminal) and enter:
  export http_proxy=http://yourpoxy:<port number>
   then run
   downmap <params>

   Example:
   # export http_proxy=http://mypoxy:8080\n
   # downmap ...\n;
where: mypoxy -  proxy IP ddress or proxy name
	     8080 - port on proxy


03.06.05: Small mistake fix.
I the downmap help (downmap -h) corrected mistake in the help string:
"downmap -la 47 -slo 1 -sc 300000 -a 3000x3000 -md /home/QtPalmtop/qpegps/maps/ -P m"
changed to:
"downmap -la 47 -lo 1 -sc 300000 -a 3000x3000 -md /home/QtPalmtop/qpegps/maps/ -P m"
