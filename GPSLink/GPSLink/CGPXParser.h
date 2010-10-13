// Header for CGPXParser

#include "stdafx.h"
#include "GPSLink.h"

#ifndef _INC_GPX_PARSER_H
#define _INC_GPX_PARSER_H

#define GPX_CHECK_NAME 0x01
#define GPX_CHECK_DESC 0x02
#define GPX_CHECK_SYM  0x04
#define GPX_CHECK_LAT  0x08
#define GPX_CHECK_LON  0x10
#define GPX_CHECK_ELE  0x20
#define GPX_CHECK_TIME 0x40

class CGPXParser
{
  public:
    CGPXParser(TCHAR *fileNameIn);
    ~CGPXParser(void);

    void clear(void);
    void parse(DOMNode *node);
    BOOL validate(TCHAR *type, unsigned flag);
    CWaypoint *createWaypoint(BOOL checkedIn);
    CRoute *createRoute(BOOL checkedIn);
    CTrack *createTrack(BOOL checkedIn);
    CTrackpoint *createTrackpoint(BOOL checkedIn, BYTE startTrack);

    int getNItem() const { return nItem; }
    void setNItem(int nItemIn) { nItem=nItemIn; }
    void resetNItem() { nItem=0; }
    TCHAR *getFileName() const { return szFileName; }
    void setFileName(TCHAR * szFileNameIn) { szFileName=szFileNameIn; }
    double getLat() const { return lat; }
    double getLon() const { return lon; }
    double getEle() const { return ele; }
    char *getName() const { return name; }
    char *getDesc() const { return desc; }
    char *getSym() const { return sym; }
    char *getTime() const { return time; }

    static time_t parseTime(const char *cdatastr, int *microsecs); 
    static time_t mkgmtime(struct tm *t);


  private:
    int nItem;
    TCHAR *szFileName;
    double lat;
    double lon;
    double ele;
    char *name;
    char *desc;
    char *sym;
    char *time;
};



#endif // _INC_GPX_PARSER_H
