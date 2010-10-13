// Header for Street Atlas routines

#ifndef _INC_SA_H
#define _INC_SA_H

typedef enum {
    SA_3=3,
    SA_4,
    SA_5,
    SA_6,
    SA_7,
    SA_8,
} SAVersion;

// Function prototypes

void setSAVer(SAVersion ver);
void setSALimits(double latMin, double lonMin, double latMax, double lonMax,
  int numObjs);
BOOL writeSAHeader(FILE *out);
BOOL writeSATrailer(FILE *out);
BOOL writeSAWaypoint(FILE *out, double lat, double lon, char *ident);
BOOL writeSARouteWaypoint(FILE *out, double lat, double lon, char *ident);
BOOL writeSARouteHeader(FILE *out, double lat, double lon, short points);
BOOL writeSATrackHeader(FILE *out, double lat, double lon, short points);
BOOL writeSATrackpoint(FILE *out, double lat, double lon);

#endif // _INC_SA_H
