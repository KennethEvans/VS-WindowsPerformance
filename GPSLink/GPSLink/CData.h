// Header file for CData classes

#if !defined(_CDATA_H)
#define _CDATA_H

#include "stdafx.h"

#include "tsDLList.h"
#include "garmin.h"

// Safe packet size is 4 non-stuffed +2*(2 stuffed + sizeof(data))
// Non-stuffed are DLE[0], PacketID[1], DLE[n-2], ETX[n-1]
// Stuffed are DataSize[2], Checksum[n-3], data[3 thru n-4]
#define MAXPACKETSIZE(dataSize) (8+dataSize)
// Size that should accomodate any packet
#define DATA_BUFSIZE 512

// Use to convert bytes in the buffer to other formats
#define DVAL(x) (*(double *)(x))
#define FVAL(x) (*(float *)(x))
#define LVAL(x) (*(long *)(x))
#define SVAL(x) (*(short *)(x))
#define BVAL(x) (*(x))
#define CVAL(x) ((char *)(x))
#define CPYN(x,y,n) (memcpy(&(x),(y),(n)))
#define CPY8(x,y) ((x),memcpy(&(x),(y),8))
#define CPY4(x,y) ((x),memcpy(&(x),(y),4))
#define CPY2(x,y) ((x),memcpy(&(x),(y),2))
#define CPY1(x,y) ((x),memcpy(&(x),(y),1))

typedef enum {
    TYPE_WPT,
    TYPE_RTE,
    TYPE_TRK,
    TYPE_TPT,
    TYPE_POS
} DataType;

class CBlockList;
class CBlock;
class CWaypoint;
class CTrack;
class CTrackpoint;
class CPosition;
class CCommand;

class CBlockList : public tsDLList<CBlock>
{
  public:
    CBlockList(void);
    CBlockList(const CBlockList &old);
    ~CBlockList(void);
    CBlockList &operator=(const tsDLList<CBlock> &old);
    unsigned getCount(void) const { return tsDLList<CBlock>::count(); }
#if 0
    CBlock *first(void) const { return tsDLList<CBlock>::first(); }
    CBlock *last(void) const { return tsDLList<CBlock>::last(); }
#endif    
#if 0
    CBlock *getBlock(const int pos, BOOL removeBlock=FALSE);
#endif
    BOOL removeBlock(CBlock *pBlock, BOOL deleteBlock=TRUE);
    BOOL addBlock(CBlock *pBlock);
    BOOL insertBlockBefore(CBlock *pBlock, CBlock *pTarget);
    BOOL insertBlockAfter(CBlock *pBlock, CBlock *pTarget);
    void empty(BOOL removeBlocks=TRUE);
    int getPos(const CBlock *pBlock) const;
    void dump(void) const;

  protected:
    BOOL copy(const tsDLList<CBlock> &old);
};    

class CBlock : public tsDLNode<CBlock>
{
  public:
    CBlock(DataType typeIn);
    CBlock(const CBlock &old);
    virtual ~CBlock(void);
    CBlock &operator=(const CBlock &old);
    int getType(void) const { return type; }
    CBlockList *getParentList(void) const { return parentList; }
    void setParentList(CBlockList *parentListIn) { parentList=parentListIn; }
    int getIndex(void) const { return index; }
    void setIndex(int indexIn) { index=indexIn; }
    double getDistance(void) const { return distance; }
    void setDistance(double distanceIn) { distance=distanceIn; }
    BOOL isChecked(void) const { return checked; }
    void setChecked(BOOL newChecked) { checked=newChecked; }

  // Virtual functions
    virtual double getLatitude(void) const {return 0.0; }
    virtual double getLongitude(void) const {return 0.0; }
    virtual double getAltitude(void) const=0;
    virtual char *getIdent(void) const=0;
    virtual BOOL setIdent(char *newIdent)=0;
    virtual void setLatitude(double latitudeIn)=0;
    virtual void setLongitude(double longitudeIn)=0;
    virtual void setAltitude(double altitudeIn)=0;
    virtual BOOL sendData(PacketType type)=0;
    virtual CBlock *clone(void) const=0;
    
  protected:
    BOOL copy(const CBlock &old);

  protected:
    CBlockList *parentList;
    int type;
    int index;
    double distance;
    BOOL checked;
};

class CWaypoint : public CBlock
{
  public:
    CWaypoint(BOOL checkedIn);
    CWaypoint(BOOL checkedIn, char *name, double latitude,
      double longitude, double altitude, char *symbol);
    CWaypoint(const CWaypoint &old);
    ~CWaypoint(void);
    CWaypoint &operator=(const CWaypoint &old);
    BOOL setData(BYTE *data, BYTE nData);
    BOOL setLinkData(BYTE *data, BYTE nData);

    virtual double getLatitude(void) const { return lat*SEMI2DEG; }
    virtual double getLongitude(void) const { return lon*SEMI2DEG; }
    virtual double getAltitude(void) const { return alt*M2FT; }
    virtual char *getIdent(void) const { return ident; }
    virtual BOOL setIdent(char *newIdent);
    virtual void setLatitude(double latitudeIn) {
	lat=(long)(latitudeIn/SEMI2DEG+.5); }
    virtual void setLongitude(double longitudeIn) {
	lon=(long)(longitudeIn/SEMI2DEG+.5); }
    virtual void setAltitude(double altitudeIn) {
	alt=(float)(altitudeIn/M2FT); }
    virtual BOOL sendData(PacketType type);
    virtual CBlock *clone(void) const;
    int getSymbol(void) const { return (int)smbl; }
    void setSymbol(char *symbolName);
    void setSymbol(int smblIn) { smbl=smblIn; }
    BOOL sendLinkData(void);
    BOOL isDuplicate(CBlockList *list) const;
    
  private:
    BOOL copy(const CWaypoint &old);

  private:
  // D108_Wpt_type
BYTE wpt_class; /* class (see below) 1 */
BYTE color; /* color (see below) 1 */
BYTE dspl; /* display options (see below) 1 */
BYTE attr; /* attributes (see below) 1 */
short smbl; /* waypoint symbol 2 */
BYTE subclass[18]; /* subclass 18 */
long lat; /* 32 bit semicircle 8 */
long lon;
float alt; /* altitude in meters 4 */
float dpth; /* depth in meters 4 */
float dist; /* proximity distance in meters 4 */
char state[2]; /* state 2 */
char cc[2]; /* country code 2 */
char *ident; /* variable length string 1-51 */
char *comment; /* waypoint user comment 1-51 */
char *facility; /* facility name 1-31 */
char *city; /* city name 1-25 */
char *addr; /* address number 1-51 */
char *cross_road; /* intersecting road label 1-51 */

BOOL link;
WORD link_class;
BYTE link_subclass[18]; /* subclass 18 */
char *link_ident; /* variable length string 1-51 */
};

class CRoute : public CBlock, public CBlockList
{
  public:
    CRoute(BOOL checkedIn);
    CRoute(BOOL checkedIn, char *name);
    CRoute(const CRoute &old);
    ~CRoute(void);
    CRoute &operator=(const CRoute &old);
    BOOL setData(BYTE *data, BYTE nData);

    virtual double getLatitude(void) const;
    virtual double getLongitude(void) const;
    virtual double getAltitude(void) const { return maxAlt*M2FT; }
    virtual char *getIdent(void) const { return rte_ident; }
    virtual BOOL setIdent(char *newIdent);
    virtual void setLatitude(double latitudeIn) {}
    virtual void setLongitude(double longitudeIn) {}
    virtual void setAltitude(double altitudeIn) {
	maxAlt=altitudeIn/M2FT; }
    virtual BOOL sendData(PacketType type);
    virtual CBlock *clone(void) const;
    CBlockList *getBlockList(void) const { return (CBlockList *)this; }
    
  private:
    BOOL copy(const CRoute &old);

  private:
  // D202_Rte_Hdr_Type
    char *rte_ident; /* null-terminated string */
    double maxAlt;
};

class CTrack : public CBlock, public CBlockList
{
  public:
    CTrack(BOOL checkedIn);
    CTrack(BOOL checkedIn, char *name);
    CTrack(const CTrack &old);
    ~CTrack(void);
    BOOL setData(BYTE *data, BYTE nData);
    CTrack &operator=(const CTrack &old);

    virtual double getLatitude(void) const;
    virtual double getLongitude(void) const;
    virtual double getAltitude(void) const { return maxAlt*M2FT; }
    virtual char *getIdent(void) const { return trk_ident; }
    virtual BOOL setIdent(char *newIdent);
    virtual void setLatitude(double latitudeIn) {}
    virtual void setLongitude(double longitudeIn) {}
    virtual void setAltitude(double altitudeIn) {
	maxAlt=altitudeIn/M2FT; }
    virtual BOOL sendData(PacketType type);
    virtual CBlock *clone(void) const;
    CBlockList *getBlockList(void) const { return (CBlockList *)this; }
    
  private:
    BOOL copy(const CTrack &old);

    
  private:
  // D310_Trk_Hdr_Type
  // KE: is boolean = 8-bit integer or unsigned char?  Use BYTE.
    BYTE dspl; /* display on the map? */
    BYTE color; /* color (same as D108) */
    char *trk_ident; /* null-terminated string */
    double maxAlt;
};

class CTrackpoint : public CBlock
{
  public:
    CTrackpoint(BOOL checkedIn);
    CTrackpoint(BOOL checkedIn, BYTE newTrack, double latitude,
      double longitude, double altitude, char *timeString);
    CTrackpoint(BOOL checkedIn, BYTE newTrack, double latitude,
      double longitude, double altitude, time_t unixTime);
    CTrackpoint(const CTrackpoint &old);
    ~CTrackpoint(void);
    CTrackpoint &operator=(const CTrackpoint &old);
    BOOL setData(BYTE *data, BYTE nData);
    BOOL isNewTrack(void) const { return new_trk?TRUE:FALSE; }
    long getGTime(void) const { return (long)time; }

    virtual double getLatitude(void) const {return lat*SEMI2DEG; }
    virtual double getLongitude(void) const {return lon*SEMI2DEG; }
    virtual double getAltitude(void) const {return alt*M2FT; }
    virtual char *getIdent(void) const {return new_trk?"Start":"Continue"; }
    virtual void setLatitude(double latitudeIn) {
	lat=(long)(latitudeIn/SEMI2DEG+.5); }
    virtual void setLongitude(double longitudeIn) {
	lon=(long)(longitudeIn/SEMI2DEG+.5); }
    virtual void setAltitude(double altitudeIn) {
	alt=(float)(altitudeIn/M2FT); }
    virtual BOOL setIdent(char *newIdent);
    virtual BOOL sendData(PacketType type);
    virtual CBlock *clone(void) const;
    
  private:
    BOOL copy(const CTrackpoint &old);
    
  private:
  // D301_Trk_Point_type
    long lat; /* 32 bit semicircle 8 */
    long lon;
    DWORD time; /* time */
    float alt; /* altitude in meters */
    float dpth; /* depth in meters */
  // KE: is boolean = 8-bit integer or unsigned char?  Use BYTE.
    BYTE new_trk; /* new track segment? */
};

class CPosition : public CBlock
{
  public:
    CPosition(void);
    CPosition(const CPosition &old);
    ~CPosition(void);
    CPosition &operator=(const CPosition &old);
    BOOL setData(BYTE *data, BYTE nData);
    BOOL isUseful() const { return (fix > 2 && fix < 6)?TRUE:FALSE; }
    const TCHAR *getFixName(void) const;
    double getEPE(void) const { return epe*M2FT; }
    double getEPH(void) const { return eph*M2FT; }
    double getEPV(void) const { return epv*M2FT; }
    double getVEast(void) const { return east*MPS2MPH; }
    double getVNorth(void) const { return north*MPS2MPH; }
    double getVUp(void) const { return up*MPS2MPH; }
    long getGTime(void) const {
	return (long)(wn_days*DAY2SEC+tow-leap_scnds+.5); }
    BOOL isValid(void) const { return (lat == DBL_MAX)?FALSE:TRUE; }  

    virtual double getLatitude(void) const {return lat*RAD2DEG; }
    virtual double getLongitude(void) const {return lon*RAD2DEG; }
    virtual double getAltitude(void) const {return alt*M2FT; }
    virtual char *getIdent(void) const {return NULL; }
    virtual BOOL setIdent(char *newIdent);
    virtual void setLatitude(double latitudeIn) {
	lat=(long)(latitudeIn/SEMI2DEG+.5); }
    virtual void setLongitude(double longitudeIn) {
	lon=(long)(longitudeIn/SEMI2DEG+.5); }
    virtual void setAltitude(double altitudeIn) {
	alt=(float)(altitudeIn/M2FT); }
    virtual BOOL sendData(PacketType type) { return TRUE; };
    virtual CBlock *clone(void) const;

  private:
    BOOL copy(const CPosition &old);

  private:
  // D800_Pvt_Data_type
    float alt; /* altitude above WGS 84 ellipsoid (meters) */
    float epe; /* estimated position error, 2 sigma (meters) */
    float eph; /* epe, but horizontal only (meters) */
    float epv; /* epe, but vertical only (meters) */
    WORD fix; /* type of position fix */
    double tow; /* time of week (seconds) */
    double lat; /* latitude and longitude (radians) */
    double lon; /* latitude and longitude (radians) */
    float east; /* velocity east (meters/second) */
    float north; /* velocity north (meters/second) */
    float up; /* velocity up (meters/second) */
    float msl_hght; /* height of WGS 84 ellipsoid above MSL (meters) */
    WORD leap_scnds; /* difference between GPS and UTC (seconds) */
    DWORD wn_days; /* week number days */

};

class CCommand : public tsDLNode<CCommand>
{
  public:
    CCommand(CommandType typeIn);
    ~CCommand(void);
    CommandType getCommand(void) const {return type; }

  private:
    CommandType type;
};

#endif // !defined(_CDATA_H)
