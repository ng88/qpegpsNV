Requrements:
1. perl
2. wget

Usage:
1. unzip download_maps.zip
2. cd download
3. set file execution rights for file example.sh:
	chmod 0555 example.sh
4. set file execution rights for file gps_map.pl:
	chmod 0555 gps_map.pl
5. for examle download run: ./example.sh
or use:
	./gps_map.pl -h
to help

if you use proxy enter:
	 http_proxy=http://192.168.0.79:8080
where: 192.168.0.79 -  192.168.0.79 IP ddress or name
	     8080 - port on 192.168.0.79