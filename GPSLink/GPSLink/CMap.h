// Header file for CMap class

#if !defined(_CMAP_H)
#define _CMAP_H

#include "stdafx.h"

#include "tsDLList.h"
#include "matrix.h"

class CMapPoint : public tsDLNode<CMapPoint>
{
  public:
    CMapPoint(void);
    CMapPoint(double latitudeIn, double longitudeIn,
      double xPixIn, double yPixIn, TCHAR *identIn=NULL);
    ~CMapPoint(void);
    double getLatitude(void) const { return latitude; }
    void setLatitude(double latitudeIn) { latitude=latitudeIn; }
    double getLongitude(void) const { return longitude; }
    void setLongitude(double longitudeIn) { longitude=longitudeIn; }
    void setXPix(double xPixIn) { xPix=xPixIn; }
    double getXPix(void) const { return xPix; }
    void setYPix(double yPixIn) { yPix=yPixIn; }
    double getYPix(void) const { return yPix; }
    TCHAR *getIdent(void) const { return ident; }
    void setIdent(TCHAR *identIn);
    void setPoint(double latitudeIn, double longitudeIn,
      double xPixIn, double yPixIn, TCHAR *identIn);
    
  private:
    double latitude;
    double longitude;
    double xPix;
    double yPix;
    TCHAR *ident;
};

class CMap
{
  public:
    CMap(void);
    CMap(HWND hWndIn, TCHAR *fileNameIn, TCHAR *baseFileName=NULL);
    ~CMap(void);
    void clearPoints(void);
    void calculatePixel(double latitude, double longitude,
      double *xPix, double *yPix);
    void calculateLatLon(double xPix, double yPix,
      double *latitude, double *longitudeIn);
    void calibrate(void);
    BOOL addPoint(double latitudeIn, double longitudeIn,
      double xPixIn, double yPixIn, TCHAR *identIn=NULL);
    void setLatLonLimits(double latitudeMin, double latitudeMax,
      double longitudeMin, double longitudeMax);
    BOOL setPoint(CMapPoint *pPoint, double latitudeIn,
      double longitudeIn, double xPixIn, double yPixIn, TCHAR *identIn);
    BOOL deletePoint(CMapPoint *pPoint, double latitudeIn,
      double longitudeIn, double xPixIn, double yPixIn, TCHAR *identIn);
    BOOL bitmapIsValid(void) const { return hBitmap?TRUE:FALSE; }
    BITMAP getBitmap(void) const;
    HDC getHDCMem(void) const { return hDCMem; }
    int getCount(void) const { return pointList.count(); }
    const tsDLList<CMapPoint> *getList(void) const { return &pointList; }
    TCHAR *getShortFileName(void) const;
    TCHAR *getFileName(void) const { return fileName; }
    double getGMTOffsetHr(void) const { return gmtOffsetHr; }
    void setGMTOffsetHr(double gmtOffsetHrIn) { gmtOffsetHr=gmtOffsetHrIn; }
    void getMatrices(Matrix &aIn, Matrix &ainvIn, Matrix &bIn) const;
    BOOL isValid(void) const { return calibrationValid; }

  private:
    BOOL readPictureFile(void);
    static DWORD CALLBACK getImageData(LPSTR szBuffer, DWORD dwBufferMax,
      LPARAM lParam);
    static void CALLBACK imageProgress(IImageRender *pRender, BOOL bComplete,
      LPARAM lParam);

  private:
    HANDLE hFile;
    HWND hWnd;
    HBITMAP hBitmap;
    HDC hDCMem;
    TCHAR *fileName;
    Matrix a,ainv,b;
    double det;
    double latMin,latMax,lonMin,lonMax;
    double gmtOffsetHr;
    BOOL calibrationValid;
    tsDLList<CMapPoint> pointList;
};    

#endif // !defined(_CMAP_H)
