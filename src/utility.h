#ifndef __UTILITY_H__
#define __UTILITY_H__


#include <time.h>
#include <math.h>

#define INT64 52 - 4
#define INT32 23 - 4

int approxf(double actual, double expected, int bits) ;

double calculateAngularDistance( double a1, double d1, double a2, double d2 );

double hms2deg( double Hours, double Minutes, double Seconds ); 

double dms2deg(double Degrees, double Minutes, double Seconds );

void   deg2dms( double DecimalDegrees, int *Degrees, int *Minutes, int *Seconds );

double JulianDate( struct tm tm_val );

double J2000( double JD );

double getLocalSiderealTime( double lon, double J2000 );

double getHourAngle( double RightAscension, double LocalSideRealTime );

double getAltitude( double Latitude, double Declination, double HourAngle );

double getAzimuth( double Latitude, double Declination, double HourAngle );


#endif
