/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Птн Ноя 12 18:58:55 MSK 2004
    copyright            : (C) 2004 by Alexander Karkhov
    email                : shuak@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//ifdef HAVE_CONFIG_H
//include <config.h>
//endif


#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define UNIX

#ifdef UNIX
#define DIR_SEPARATION_CHAR '/'
#else
#define DIR_SEPARATION_CHAR '\\'
#endif

  // Setup possible scales
int  _scales[] = {1000,1500,2000,3000,5000,7500,10000,15000,20000,30000,50000,75000,
              100000,150000,200000,300000,500000,750000,1000000,1500000,2000000,3000000,
              5000000,7500000,10000000,15000000,20000000,30000000,50000000,75000000,0};

int _EXPEDIAALTS[] = { 1, 3, 6, 12, 25, 50, 150, 800, 2000, 7000, 12000};

// Set defaults and get options from command line
double _lat,_lon,_slat,_endlat,_slon,_endlon,_waypoint,_area,_unit,_mapdir,_debug,_force,_version,_man,_help,_areax,_areay;
int _failcount = 0;
int _scale = 100000;
char _CONFIG_DIR [150]   = "maps/"; 
char _KOORD_FILE [] = "maps.txt"; // Should we allow config of this?  	//  FIX BY KARKHOV for qpegps default file name use cat maps.txt >> /..../maps.txt to join files for few separatly downloaded regions
char _FILEPREFIX[20]   = "m";
char talkwget[]="-q"; 
char AddiCommand[400]   = "";

int _BASE_HRES	=1332;	//  FIX BY KARKHOV
int _BASE_VRES	=1332;	//  FIX BY KARKHOV
double _RES_MULTUPLYER=0.440; 	//  FIX BY KARKHOV Increase this number to increase resolution
double _h_res; 	//  FIX BY KARKHOV
double _v_res;	//  FIX BY KARKHOV
bool _wld;	//  FIX BY KARKHOV
bool _nowld;	//  FIX BY KARKHOV
bool  _polite=0, inmin=1;

double _LAT_OVERLAP_DIVIDER;  //  FIX BY KARKHOV Increase this number to reduce overlaping
double _LON_OVERLAP_DIVIDER; //  FIX BY KARKHOV Increase this number to reduce overlaping

// Setup up some constants
int _EXPEDIAFACT   = 3950;
double _DIFF          = 0.000001;
double _RADIUS_KM     = 6371.01;
double _LAT_DIST_KM   = 110.87;
double _KM2NAUTICAL   = 0.54;
double  _KM2MILES	= 0.62137119;

char	*tmphtm="tmp.htm";
size_t	readed=0;
char	signature[40]="\"http://mc.multimap.com/cs/";
FILE 	*mfile;
char	*endaddr,*begaddr;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subroutines
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int get_scales(int tscale) {
   // OK lets figure out what scales they want
   // '////////'  - just that scale
   // '>////////' - scales above and including the number given
   // '<////////' - scales below and including the number given
   // '////////,////////,////////' - a list of scales to download
   // '////////-////////' - scales from first to last
   //
   int SCALE=2000000000, i=0;

while (_scales[i]!=0){
        if ( abs(_scales[i]-tscale) < abs(SCALE-tscale) ) SCALE=_scales[i];
  i++;
  }
   return SCALE;
} //End get_scales

int file_count (double  _slat, double  _slon,double  _endlat, double  _endlon) {
   int _count;
   double _k,_klat = 0; ////// FIX BY CAMEL 	//  FIX BY KARKHOV for dynamic overlaping
   double _klon =0;
   double _long, _lati;
  _count=0;
//   foreach my _scale (@{_SCALES_TO_GET_ref}) {
  
      _k = _DIFF * _scale*_RES_MULTUPLYER;  	//  FIX BY KARKHOV for  change Resolution (*_RES_MULTUPLYER);
      _lati = _slat;
      while (_lati < _endlat) {
         _long = _slon;
         while (_long < _endlon) {
      _klon = ( _k - (_k / _LON_OVERLAP_DIVIDER) ) * ( 1.25/fabs(cos(3.14592*_lati/180))); ////// FIX BY CAMEL	//  FIX BY KARKHOV for dynamic overlaping and reduce overlaping (*1,3) and
            _long += _klon; ////// FIX BY CAMEL
            _count++; 
         }
   	_klat = ( _k*1000 - (_k*1000 / _LAT_OVERLAP_DIVIDER) )/780; ////// FIX BY CAMEL 	//  FIX BY KARKHOV for dynamic overlaping
//        _klat = (_k - (_k / _LAT_OVERLAP_DIVIDER))*1.2; ////// FIX BY CAMEL 	//  FIX BY KARKHOV for dynamic overlaping
	 _lati += _klat; ////// FIX BY CAMEL
         _long = _slon; ////// FIX BY CAMEL
      }
//   }
   return _count;
}

double min_to_deg(double DDMMmm)
{double min;
  min=(DDMMmm-int(DDMMmm))/0.6;
  if (min > 1.0) { printf("\n*** Bad coordinates format. Use -f Dd switch for degrease only format: DD.dddd. \n");  exit(7);}
  return (double)int(DDMMmm)+min;
}

char usage[] = "downmap [-la <latitude DD>] [-lo <longitude DD>]  [-slat, --start-lat <start latitude DD>] [-elat, --end-lat <end latitude D>] [-slon, --start-lon <start longitude D>] [-elon, --end-lon <end longitude D>]  [-sc <SCALE>] [-a <#>] [-p] [-md <DIR>] [-P <PREFIX>] [-h]\n\n Download maps covering region from www.multimap.com\n  <-la <latitude D>> - Takes a latitude in format specified by -f switch and uses that as the latitude for the centerpoint of the area to be covered. This and '-lo', or '-sla', '-ela', '-slo', '-elo' is required.\n   <-lo <longitude D>> - Takes a longitude in format specified by -f switch and uses that as the longitude for the centerpoint of the area to be covered. This and '-la', or '-sla', '-ela', '-slo', '-elo' is required.\n  <-sla, --start-lat <start latitude D>> - Takes a latitude in format specified by -f switch and uses that as the start latitude for the area to be covered. This, '-ela', '-slo' and '-elo' or '-la' and '-lo' is required.\n  <-ela --end-lat <end latitude D>> - Takes a latitude in format specified by -f switch and uses that as the end latitude for the area to be covered. This, '-sla', '-slo' and '-elo' or '-la' and '-lo' is required.\n  <-slo --start-lon <start longitude D>> - Takes a longitude in format  specified by -f switch and uses that as the start longitude for the area to be covered. This, '-sla', '-ela' and '-elo' or '-la' and '-lo' is required.\n  <-elo, --end-lon <end longitude D>> - Takes a longitude in format  specified by -f switch and uses that as the end longitude for the area to be covered. This, '-sla', '-ela' and '-slo' or '-la' and '-lo' is required.\n  <-sc <SCALE>> - Scales of map(s) to download. Default: 300000.\n  <-a <#>> - Area to cover. number of kilometers square around the centerpoint. You can use a single number for square area. Or you can use '#x#' to do a rectangle, where the first number is distance in km (East-West direction) and the second number is distance km (Nodr-South).\n  <-md <DIR>> - Override the default mapdir (maps/) with this value.\n  <-P <PREFIX>> - Takes a prefix string to be used as the start of all saved map files. Default: \"m\".\n  <-wld, --wld> - Do not use roadmap. Use relief map everywere.\n  <-nowld, --nowld> - Do not use relief map everywere.\n  <-h> Prints the usage page and exits.\n  <-v> - shows wget output (for debug).\n <-f DM, -f Dd> - Specify Coordinates format. Where: -f DM (default) - means format DD.MMmm (degrese and aafter point minutes). -f Dd means format DD.dddd degrease in fixed decimal format;\n <-c> - specify additional parsms for wget command. <-si> - specify signature for search in downloaded html file.\n\n\n";
char example[] = "\nExamples:\n  downmap -sla 36.5 -ela 59.0 -slo -9.5 -elo 19.0 -sc 300000 -md /home/QtPalmtop/qpegps/maps/ -P m -r 1\n  downmap -la 47 -lo 1 -sc 300000 -a 3000x3000 -md /home/QtPalmtop/qpegps/maps/ -P m\n  If you need use proxy run Console (terminal) and enter:\n  export http_proxy=http://yourpoxy:<port number>\n then run downmap\n Example:\n# export http_proxy=http://mypoxy:8080\n# downmap\n";

struct params {
  char name[20];
  } par_index[] = { {"-la"}, {"-lo"}, {"--start-lat"}, {"--end-lat"}, {"--start-lon"}, {"--end-lon"}, {"-sc"}, {"-a"}, {"-p"}, {"-md"}, {"-P"}, {"-h"}, {"-v"}, {"-sla"} , {"-ela"} , {"-slo"} , {"-elo"} , {"-f"}, {"-si"},  {"-R"},  {"-c"} ,  {"-C"}, {"-wld"},  {"-WLD"}, {"-r"}  };

int main(unsigned int argc, char *argv[])
{                                                                          
   double _k, _klat, _klon, _lati, _long;
   int _mapscale, status;
	char filename  [50]= "";
	char	command[300];
    char *pos;
   FILE *mapsf;

#define IMPOSSIBLE 999999999
_scale=3000000;
_areax=_areay=_lat=_lon=_slat=_endlat=_slon=_endlon=IMPOSSIBLE;

unsigned int i, j, param;
_RES_MULTUPLYER=1.5;

for (i=1;i<argc;i++)
{
//  str_upper(argv[i]);
  param=IMPOSSIBLE;
    for (j=0; j<(sizeof(par_index)/sizeof(struct params)); j++) {
      if ( strcmp(argv[i],&par_index[j].name[0]) == 0 )
        { param=j;
        break; }
    }
    if ( param==IMPOSSIBLE ) { printf("\n*** Unknow parameter: %s\n",argv[i]); return 4;}
    switch (param)
    {
      case 0: sscanf(argv[i+1],"%lf",&_lat); i++; break; // -la
      case 1: sscanf(argv[i+1],"%lf",&_lon); i++; break; // -lo
      case 13:
      case 2: sscanf(argv[i+1],"%lf",&_slat); i++; break;  // -sla
      case 14:
      case 3: sscanf(argv[i+1],"%lf",&_endlat); i++; break; // -ela
      case 15:
      case 4: sscanf(argv[i+1],"%lf",&_slon);  i++; break;  // -slo
      case 16:
      case 5: sscanf(argv[i+1],"%lf",&_endlon); i++; break; // -elo
      case 6: sscanf(argv[i+1],"%i",&_scale); i++; break; // -sc
      case 7:									// -a #x#
      		if ( (pos=strchr(argv[i+1],'x')) == NULL) {sscanf(argv[i+1],"%lf",&_areax); _areay=_areax;}
                              else { *pos=' '; sscanf(argv[i+1],"%lf%lf",&_areax,&_areay);}
          i++;
          break;
      case 8:   _polite=1; break;		// -p
      case 9:   if (strlen(argv[i+1])>=(sizeof(_CONFIG_DIR)-2)) { printf("\n*** Maps directory path too long.\n"); return 5; } // -md xxxx
               sscanf(argv[i+1],"%s",&_CONFIG_DIR[0]);
               if ( _CONFIG_DIR [strlen(&_CONFIG_DIR[0])-1] != DIR_SEPARATION_CHAR )
                {  _CONFIG_DIR [strlen(&_CONFIG_DIR[0])]=DIR_SEPARATION_CHAR;
                _CONFIG_DIR [strlen(&_CONFIG_DIR[0])+1]=0;
                }
               i++;
	       break;
      case 10:  if (strlen(argv[i+1])>=(sizeof(_FILEPREFIX)-1)) { printf("\n*** File prefix too long.\n"); return 6; }		// -P xxxx
               sscanf(argv[i+1],"%s",&_FILEPREFIX[0]); i++; break;
      case 11:  printf("%s", &usage[0]); printf("%s", &example[0]); return 0;
      				break;
      case 12:  talkwget[0]=0; break;		//  -v
      case 17:  switch(*(argv[i+1]+1)) {	// -f Dx
                      case 'D': case 'd':  inmin=0; break;
                      case 'M': case 'm':  inmin=1;  break;
                      default: printf("\n*** Bad format switch. Use -f Dd or -f DM.\n");  return 7;
                      }
                      i++;
                      break;
      case 19: case 18:  strncpy(&signature[0],argv[i+1],39); 		// -si
                      printf("\n*** Signature changed to: %s.\n",&signature[0]); 
                      i++;
                      break;
      case 20: case 21:                            // -c xxxxx
                 if (strlen(argv[i+1])>=(sizeof(AddiCommand)-2)) { printf("\n*** Maps directory path too long.\n"); return 8; } // -md xxxx
                         sscanf(argv[i+1],"%s",&AddiCommand[strlen(AddiCommand)]);   strcat(&AddiCommand[0]," ");
      
                         printf("\n Additional parameter to WGET: %s\n",&AddiCommand[0]);
                      i++;
                      break;
      case 22: case 23:                            // -wld
                 _wld=1;      
                         printf("\n Using World geografic map view.\n");
                      i++;
                      break;
	case 24:    i++;
                      break;

      default:
        printf("\n*** Unknow parameter: %s\n",argv[i]);
      break;
      }
    }

    printf("Coordinate format set to ");
     if (inmin) printf("DD.MMmm\n");
       else printf("DD.dddd\n");

    _h_res=900; //_BASE_HRES*_RES_MULTUPLYER; 	//  FIX BY KARKHOV
    _v_res=900;	//_BASE_VRES*_RES_MULTUPLYER;
    _LON_OVERLAP_DIVIDER=10*_RES_MULTUPLYER/1.5;
    _LAT_OVERLAP_DIVIDER=10;

    if (_areax != IMPOSSIBLE )
      {
        printf("Downloading in directory: %s  maps which cover area: %.0fx%.0fkm. with Centralpoint:\n", &_CONFIG_DIR[0], _areax, _areay);
        if (_areax > 40000 || _areay > 40000 )  {printf ("\n*** Too large coverege area. \n"); return 14; }
        if (_lat == IMPOSSIBLE )  {printf ("\n*** Please add Central point Latitude:  -la D\n"); return 8; }
        if (_lon == IMPOSSIBLE ) {printf ("\n*** Please add Central point Longitude:  -lo D\n"); return 9; }
        if (inmin) {
                     _lat=min_to_deg(_lat);
                     _lon=min_to_deg(_lon);
        }
        printf ("  Longitude=%.3f, Latitude=%.3f    ", _lon, _lat); 
        _k=(_areay*180)/(6390*M_PI);
        _slat=_lat-_k;
        _endlat=_lat+_k;
        _k=(_areax*180)/(6390*M_PI*fabs(cos(_lat*M_PI/180)+0.00000001));
        if (_k>360) _k=360;
        _slon=_lon-_k;
        _endlon=_lon+_k;
      }
    else
    {
        printf("Downloading in directory: %s  maps which cover area between borders:\n", &_CONFIG_DIR[0]);
        if (_slat == IMPOSSIBLE )  {printf ("\n*** Please add Start Latitude:  -start-lat D\n"); return 10; }
        if (_slon == IMPOSSIBLE )  {printf ("\n*** Please add Start Longitude:  -start-lon D\n"); return 11; }
        if (_endlat == IMPOSSIBLE )  {printf ("\n*** Please add End Latitude:  -end-lat D\n"); return 12; }
        if (_endlon == IMPOSSIBLE )  {printf ("\n*** Please add End Longitude:  -end-lon D\n"); return 13; }
        if (inmin) {
                     _slat=min_to_deg(_slat);
                     _slon=min_to_deg(_slon);
                     _endlat=min_to_deg(_endlat);
                     _endlon=min_to_deg(_endlon);
        }
        printf ("  Start Latitude=%.3f,  Start Longitude=%.3f End Latitude=%.3f End Longitude=%.3f    ", _slat, _slon, _endlat, _endlon); 
    }
    if (_slat>_endlat) {printf("*** Error  start latitude must be lower then end latitude.\n"); return 14;}
    if (_slon>_endlon) {printf("*** Error  start longitude must be lower then end longitude.\n"); return 15;}

   // Get the list of scales we need
double  _SCALES_TO_GET_ref = get_scales(_scale);
   int _count = file_count(_slat,_slon,_endlat,_endlon);
_scale=int( _SCALES_TO_GET_ref);

   // Setup k
   _k = 0; //  FIX BY KARKHOV for reduce change Resolution (*_RES_MULTUPLYER) & _mapscale instead of _scale
   _klat = 0;
   _klon = 0;
   _lati = _slat;
   _mapscale=0;

   sprintf(&command[0],"%s%s",&_CONFIG_DIR [0], &_KOORD_FILE[0]);
   mapsf=fopen(&command[0],"a+");
   if (mapsf==NULL)
   {
    mkdir(&_CONFIG_DIR [0],0777);
    chmod(&_CONFIG_DIR [0],0777);
    mapsf=fopen(&command[0],"a+");
    if (mapsf==NULL)
      { printf("can't open mapfile: %s\n",&command[0]);
        return 1; /* can't open mapfile */
      }
   }
  _mapscale=_scale;
  
  printf ("with requested scale=%i and actual scale=%i\n", _scale, _mapscale);
  printf( "You are about to download about %i file(s).\nYou are violating the map servers copyright!\nDownloading files:\n",_count);

   _k = _DIFF * _mapscale * _RES_MULTUPLYER;  	//  FIX BY KARKHOV for reduce change Resolution (*_RES_MULTUPLYER) & _mapscale instead of _scale
   _klat = ( _k*1000 - (_k*1000 / _LAT_OVERLAP_DIVIDER) )/780; ////// FIX BY CAMEL 	//  FIX BY KARKHOV for dynamic overlaping
//   printf("_k=%f _klat=%f\n",_k,_klat);

char *fbuff=(char *)malloc(40000);
   
   while (_lati < _endlat)
   {
       _long = _slon;
       _klon=0;
      while (_long < _endlon)
      {
  _long += _klon;
  if ( _klon == 0 )  _klon = (_k - (_k / _LON_OVERLAP_DIVIDER) ) * (1.25/fabs(cos(3.14592*_lati/180))); ////// FIX BY CAMEL 	//  FIX BY KARKHOV for dynamic overlaping and  reduce overlaping (*1.25) and

  if (_scale <=1000) break;
	sprintf((char *)&filename[0]  , "%s%i-%.4f-%.4f",_FILEPREFIX,(int)rint(((double)_mapscale)/1000.0), _lati,_long);

	sprintf((char *)&command,"wget -nd -t 7 %s %s -O %s%s http://www.multimap.com/map/browse.cgi?lat=%f\\&lon=%f\\&scale=%i\\&icon=x",
			&talkwget[0], &AddiCommand[0], &_CONFIG_DIR [0], &tmphtm[0],_lati,_long,_mapscale);
//	printf("Command: \"%s\"\n",command);
  if ((status=system(&command[0])) != 0 )
    {
      if (status==-1)      { printf ("*** Can't execute wget.\n"); _failcount++; fclose(mapsf); return 2; }
      else printf("Download problem. If you need use proxy run Console (terminal) and enter:\n  export http_proxy=http://yourpoxy:<port number>\n then run downmap\n Example:\n# export http_proxy=http://mypoxy:8080\n# downmap\n");
    _failcount++;
     }
    else {
//	if(changesign)

	readed=0;
	sprintf((char *)&command,"%s%s", &_CONFIG_DIR [0], &tmphtm[0]);
	mfile=fopen(&command[0],"r");
	if (mfile==NULL) 
		{ printf("Fatal error: can't open downloaded file \"%s\"\n",&tmphtm[0]); _failcount++;}	
	else {
	readed=fread(fbuff,1,40000,mfile);
	fclose(mfile);
//	printf("readed= \"%i\"\n",readed);
	*(fbuff+readed)=0;
//	printf("Readed: \"%s\"\n",fbuff);
	begaddr=strstr(fbuff,&signature[0]);
	if (begaddr==NULL) 
		{ printf("Fatal error no \"%s\" in the file.\n",signature); _failcount++;}	
	else {	begaddr++;
		
		endaddr=strstr(begaddr,"W500H300.gif");
		memcpy(endaddr,"W900H900.gif",12);
		endaddr=strchr(begaddr,'"');
		*endaddr=0;
//		printf("Found: \"%s\" in the file.",begaddr); 
		sprintf((char *)&command,"wget -nd -t 7 %s %s -O %s%s.gif %s",&talkwget[0], &AddiCommand[0], &_CONFIG_DIR [0], &filename[0],begaddr);
		if ((status=system(&command[0])) != 0 ) 
		   { printf("Error Can't download gif file (%s.gif) to target directoy (%s)\n",  &filename[0], &_CONFIG_DIR[0]);
		   _failcount++; 
		   }
          	sprintf(&command[0],"gif2png -O -d %s%s.gif", &_CONFIG_DIR [0], &filename[0]);
          	if ((status=system(&command[0])) == -1 )
          	 {printf ("Can't execute gif2png. Check is gif2png utility available and is download directory exist and readable/writable.\n");
          	 _failcount++; fclose(mapsf); return 3;
           	}
           	else fprintf( mapsf, "FRITZ %s.png %i %i %i %.6f %.6f\r\n",&filename[0], _mapscale, int(_h_res), int(_v_res), _lati, _long);
     	}
     	}
	}
    if (!_polite) sleep(1);
      }
      _lati += _klat; ////// FIX BY CAMEL
      _long = _slon; ////// FIX BY CAMEL
   }
   sprintf((char *)&command,"%s%s", &_CONFIG_DIR [0], &tmphtm[0]);
 remove(&command[0]);
free(fbuff);
printf( "\nFailcount: %i\n",_failcount);
fclose(mapsf);
if (_failcount!=0) return 127;
else return 0;
}


