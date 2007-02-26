mkdir w_europ_3000
cd      w_europ_3000
echo
echo w_europ_3000
cp  ../gps_map.pl  ./gps_map.pl
./gps_map.pl -start-lat 36.5 -end-la 59.0 -start-lo -9.5 -end-lo 19.0 -sc 3000000
cd ..
mkdir usa_3000
cd      usa_3000
echo
echo usa_3000
cp  ../gps_map.pl  ./gps_map.pl
./gps_map.pl -start-lat 25.5 -end-la 51.0 -start-lo -124.5 -end-lo -66.0 -sc 3000000
cd ..
mkdir rus_w_3000
cd      rus_w_3000
echo
echo rus_w_3000
cp  ../gps_map.pl  ./gps_map.pl
./gps_map.pl -start-lat 53.5 -end-la 61.5 -start-lo 28.5 -end-lo 44.0 -sc 3000000 -wld
cd ..

