// Methods for CMap

#include "stdafx.h"

#define MAXWIDTH 10000
#define PIXFALLBACK 500

#include "CMap.h"
#include "utils.h"

/**************************************************************************/
/**************************** CMapPoint ***********************************/
/**************************************************************************/

CMapPoint::CMapPoint(void) :
latitude(0.0),
longitude(0.0),
xPix(0.0),
yPix(0.0),
ident(NULL)
{
}

CMapPoint::CMapPoint(double latitudeIn, double longitudeIn,
					 double xPixIn, double yPixIn, TCHAR *identIn) :
latitude(latitudeIn),
longitude(longitudeIn),
xPix(xPixIn),
yPix(yPixIn),
ident(NULL)
{
	if(identIn) {
		int len=_tcslen(identIn);
		ident=new TCHAR[len+1];
		if(ident) {
			_tcscpy(ident,identIn);
		}
	}
}

CMapPoint::~CMapPoint(void)
{
	if(ident) delete [] ident;
}

void CMapPoint::setIdent(TCHAR *identIn)
{
	if(identIn) {
		int len=_tcslen(identIn);
		ident=new TCHAR[len+1];
		if(ident) {
			_tcscpy(ident,identIn);
		}
	} else {
		delete [] ident;
		ident=NULL;
	}
}

void CMapPoint::setPoint(double latitudeIn, double longitudeIn,
						 double xPixIn, double yPixIn, TCHAR *identIn)
{
	latitude=latitudeIn;
	longitude=longitudeIn;
	xPix=xPixIn;
	yPix=yPixIn;
	if(identIn) {
		int len=_tcslen(identIn);
		ident=new TCHAR[len+1];
		if(ident) {
			_tcscpy(ident,identIn);
		}
	} else {
		delete [] ident;
		ident=NULL;
	}
}

/**************************************************************************/
/**************************** CMap ****************************************/
/**************************************************************************/

CMap::CMap(void) :
a(2,2,0.0),
ainv(2,2,0.0),
b(2,1),
det(0.0),
latMin(0.0),
latMax(0.0),
gmtOffsetHr(0),
calibrationValid(FALSE),
hFile(INVALID_HANDLE_VALUE),
hWnd(NULL),
hBitmap(NULL),
hDCMem(NULL),
fileName(NULL)
{
}

CMap::CMap(HWND hWndIn, TCHAR *fileNameIn, TCHAR *baseFileName) :
a(2,2),
ainv(2,2),
b(2,1),
latMin(0.0),
latMax(0.0),
gmtOffsetHr(0),
det(0.0),
calibrationValid(FALSE),
hFile(INVALID_HANDLE_VALUE),
hWnd(hWndIn),
hBitmap(NULL),
hDCMem(NULL),
fileName(NULL)
{

	// Build a filename
	if(fileNameIn == NULL) {
		fileName=NULL;
	} else if(!baseFileName) {
		// No base filename, just use the given filename
		int len=_tcslen(fileNameIn);
		fileName=new TCHAR[len+1];
		if(fileName) {
			_tcscpy(fileName,fileNameIn);
		}
	} else {
		// Check if the fileName has a drive or dir
		if(_tcschr(fileNameIn,':') || _tcschr(fileNameIn,'\\')) {
			// Use the given filename
			int len=_tcslen(fileNameIn);
			fileName=new TCHAR[len+1];
			if(fileName) {
				_tcscpy(fileName,fileNameIn);
			}
		} else {
			// A simple filename, use the drive and dir of the base
			// file name
			TCHAR pathBuffer[_MAX_PATH];

			_tcscpy(pathBuffer,baseFileName);
			TCHAR *ptr=_tcsrchr(pathBuffer,'\\');
			if(ptr) {
				*ptr='\0';
				_tcscat(pathBuffer,_T("\\"));
				_tcscat(pathBuffer,fileNameIn);
			} else {
				// No dir separator, just use the file name
				_tcscpy(pathBuffer,fileNameIn);
			}
			int len=_tcslen(pathBuffer);
			fileName=new TCHAR[len+1];
			if(fileName) {
				_tcscpy(fileName,pathBuffer);
			}
		}
	}

	// Make a memory DC
	if(hWnd) {
		HDC hdc=GetDC(hWnd);
		hDCMem=CreateCompatibleDC(hdc);
		ReleaseDC(hWnd,hdc);
	}

	// Get the image
	BOOL status=readPictureFile();
	if(!status) {
		errMsg(_T("Cannot read image file: %s"),fileName);
	}

	// Select the bitmap into the memory DC
	if(hDCMem && hBitmap) {
		SelectObject(hDCMem,hBitmap);
	}
}

#if 0
// splitpath version
CMap::CMap(HWND hWndIn, TCHAR *fileNameIn, TCHAR *baseFileName) :
a(2,2),
ainv(2,2),
b(2,1),
latMin(0.0),
latMax(0.0),
gmtOffsetHr(0),
det(0.0),
calibrationValid(FALSE),
hFile(INVALID_HANDLE_VALUE),
hWnd(hWndIn),
hBitmap(NULL),
hDCMem(NULL),
fileName(NULL)
{

	// Build a filename
	if(fileNameIn == NULL) {
		fileName=NULL;
#ifdef UNDER_CE
	} else {
		// Use the given filename
		int len=_tcslen(fileNameIn);
		fileName=new TCHAR[len+1];
		if(fileName) {
			_tcscpy(fileName,fileNameIn);
		}
#else
	} else if(!baseFileName) {
		// No base filename, just use the given filename
		int len=_tcslen(fileNameIn);
		fileName=new TCHAR[len+1];
		if(fileName) {
			_tcscpy(fileName,fileNameIn);
		}
	} else {
		// Make a full path using parts from the base filename
		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];

		_tsplitpath(fileNameIn,drive,dir,fname,ext);
		// Check if the fileName has a drive or dir
		if(drive[0] != '\0' || dir[0] != '\0') {
			// Use the given filename
			int len=_tcslen(fileNameIn);
			fileName=new TCHAR[len+1];
			if(fileName) {
				_tcscpy(fileName,fileNameIn);
			}
		} else {
			// A simple filename, use the drive and dir of the base
			// file name
			TCHAR pathBuffer[_MAX_PATH];

			_tsplitpath(baseFileName,drive,dir,NULL,NULL);
			_tmakepath(pathBuffer,drive,dir,fname,ext);
			int len=_tcslen(pathBuffer);
			fileName=new TCHAR[len+1];
			if(fileName) {
				_tcscpy(fileName,pathBuffer);
			}
		}
#endif
	}

	// Make a memory DC
	if(hWnd) {
		HDC hdc=GetDC(hWnd);
		hDCMem=CreateCompatibleDC(hdc);
		ReleaseDC(hWnd,hdc);
	}

	// Get the image
	BOOL status=readPictureFile();
	if(!status) {
		errMsg(_T("Cannot read image file: %s"),fileName);
	}

	// Select the bitmap into the memory DC
	if(hDCMem && hBitmap) {
		SelectObject(hDCMem,hBitmap);
	}
}
#endif

CMap::~CMap(void)
{
	if(hFile == INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile=INVALID_HANDLE_VALUE;
	}
	if(hDCMem) {
		DeleteDC(hDCMem);
		hDCMem=NULL;
	}
	if(hBitmap) {
		DeleteObject(hBitmap);
		hBitmap=NULL;
	}
	if(fileName) {
		delete [] fileName;
		fileName=NULL;
	}

	// Clear the pointList and delete the points
	clearPoints();
}

TCHAR *CMap::getShortFileName(void) const
{
	// If it is NULL, return NULL
	if(fileName == NULL) return fileName;

	// Check if the fileName has a drive or dir
	if(_tcschr(fileName,':') || _tcschr(fileName,'\\')) {
		// Find the start of the short part
		TCHAR *ptr=_tcsrchr(fileName,'\\');
		return ptr+1;
	} else {
		// It is already short, use it
		return fileName;
	}
}

void CMap::clearPoints(void)
{
	while(pointList.count() > 0) {
		CMapPoint *pLast=pointList.last();
		pointList.remove(*pLast);
		delete pLast;
	}
}

void CMap::setLatLonLimits(double latitudeMin, double latitudeMax,
						   double longitudeMin, double longitudeMax)
{
	latMin=latitudeMin;
	latMax=latitudeMax;
	lonMin=longitudeMin;
	lonMax=longitudeMax;
}

BOOL CMap::addPoint(double latitude, double longitude,
					double xPix, double yPix, TCHAR *ident)
{
	CMapPoint *point=new CMapPoint(latitude,longitude,xPix,yPix,ident);
	if(!point) {
		errMsg(_T("Cannot create CMapPoint for %s"),
			ident?ident:_T("<No ident>"));
		return FALSE;
	}
	pointList.add(*point);
	return TRUE;
}

BOOL CMap::setPoint(CMapPoint *pPoint, double latitudeIn,
					double longitudeIn, double xPixIn, double yPixIn, TCHAR *identIn)
{
	// Find the block by parsing the list (inefficient)
	tsDLIterBD<CMapPoint> iter(pointList.first());
	tsDLIterBD<CMapPoint> eol;
	while(iter != eol) {
		CMapPoint *pPoint0=iter;
		if(pPoint == pPoint0) {
			pPoint->setPoint(latitudeIn,longitudeIn,xPixIn,yPixIn,identIn);
			return TRUE;
		}
		++iter;
	}

	return FALSE;
}

BOOL CMap::deletePoint(CMapPoint *pPoint, double latitudeIn,
					   double longitudeIn, double xPixIn, double yPixIn, TCHAR *identIn)
{
	// Find the block by parsing the list (inefficient)
	tsDLIterBD<CMapPoint> iter(pointList.first());
	tsDLIterBD<CMapPoint> eol;
	while(iter != eol) {
		CMapPoint *pPoint0=iter;
		if(pPoint == pPoint0) {
			pointList.remove(*pPoint);
			delete pPoint;
			return TRUE;
		}
		++iter;
	}

	return FALSE;
}

void CMap::calculatePixel(double latitude, double longitude,
						  double *xPix, double *yPix)
{
	Matrix X(2,1,0.0),x;

	if(!calibrationValid) {
		errMsg(_T("Calibration is invalid"));
		*xPix=*yPix=0.0;
		return;
	}
	if(det) {
		X.setval(1,1,latitude);
		X.setval(2,1,longitude);
		x=ainv*(X-b);
		*xPix=x.getval(1,1);
		*yPix=x.getval(2,1);
	} else {
		errMsg(_T("Determinant is zero"));
		*xPix=*yPix=0.0;
	}
}

void CMap::calculateLatLon(double xPix, double yPix,
						   double *latitude, double *longitude)
{
	Matrix X,x(2,1,0.0);

	if(!calibrationValid) {
		errMsg(_T("Calibration is invalid"));
		*longitude=*latitude;
		return;
	}
	x.setval(1,1,xPix);
	x.setval(2,1,yPix);
	X=a*x+b;
	*latitude=X.getval(1,1);
	*longitude=X.getval(2,1);
}

void CMap::calibrate(void)
{
	int nPoints=pointList.count();
	int row=0;

	// Check for calibration points
	calibrationValid=TRUE;
	det=0.0;

	// Reset the current matrices
	a=Matrix(2,2,0.0);
	b=Matrix(2,1,0.0);

	// Define the calibration matrix and vector
	int nPoints2=max(2*nPoints,6);
	double lat0,lat1,lon0,lon1,xPix0,xPix1,yPix0,yPix1;
	Matrix aa(nPoints2,6,0.0);
	Matrix bb(nPoints2,1,0.0);
	Matrix xx,aainv;
	Matrix u,v,w;
	Matrix vt,wi;
	tsDLIterBD<CMapPoint> iter(pointList.first());
	tsDLIterBD<CMapPoint> eol;
	int i=0;
	while(iter != eol) {
		row=2*i+1;
		aa.setval(row,1,iter->getXPix());
		aa.setval(row,2,iter->getYPix());
		aa.setval(row,5,1.0);
		bb.setval(row,1,iter->getLatitude());
		row++;
		aa.setval(row,3,iter->getXPix());
		aa.setval(row,4,iter->getYPix());
		aa.setval(row,6,1.0);
		bb.setval(row,1,iter->getLongitude());
		i++;
		iter++;
	}

	// Get fallbacks for fewer than 3 points
	if(nPoints < 4) {
		if(latMax != latMin) {
			// Use entered values
			lat0=latMax;
			lat1=latMin;
		} else {
			// Use maximum possible values
			lat0=90.0;
			lat1=-90.0;
		}
		if(lonMax != lonMin) {
			lon0=lonMin;
			lon1=lonMax;
		} else {
			// Use maximum possible values
			lon0=-180.0;
			lon1=180.0;
		}
		// Always use upper left
		xPix0=0;
		yPix0=0;
		// If there is a bitmap, use its values
		if(hBitmap) {
			BITMAP bmp=getBitmap();
			xPix1=max(bmp.bmWidth-1,PIXFALLBACK);
			yPix1=max(bmp.bmHeight-1,PIXFALLBACK);

		} else {
			xPix1=PIXFALLBACK;
			yPix1=PIXFALLBACK;
		}
	}

	// Fill in values to make at least two points and 4 rows
	if(nPoints == 0) {
		row++;
		aa.setval(row,1,xPix0);
		aa.setval(row,2,yPix0);
		aa.setval(row,5,1.0);
		bb.setval(row,1,lat0);
		row++;
		aa.setval(row,3,xPix0);
		aa.setval(row,4,yPix0);
		aa.setval(row,6,1.0);
		bb.setval(row,1,lon0);

		row++;
		aa.setval(row,1,xPix1);
		aa.setval(row,2,yPix0);
		aa.setval(row,5,1.0);
		bb.setval(row,1,lat0);
		row++;
		aa.setval(row,3,xPix1);
		aa.setval(row,4,yPix0);
		aa.setval(row,6,1.0);
		bb.setval(row,1,lon1);

		row++;
		aa.setval(row,1,xPix1);
		aa.setval(row,2,yPix1);
		aa.setval(row,5,1.0);
		bb.setval(row,1,lat1);
		row++;
		aa.setval(row,3,xPix1);
		aa.setval(row,4,yPix1);
		aa.setval(row,6,1.0);
		bb.setval(row,1,lon1);
	} else if(nPoints == 1) {
		// See if point used is upper left
		if(aa.getval(1,1) != xPix0 && aa.getval(2,2) == yPix0 &&
			bb.getval(1,1) == lat0 && bb.getval(2,1) == lon0) {
				// Is upper left, use upper right
				row++;
				aa.setval(row,1,xPix1);
				aa.setval(row,2,yPix0);
				aa.setval(row,5,1.0);
				bb.setval(row,1,lat0);
				row++;
				aa.setval(row,3,xPix1);
				aa.setval(row,4,yPix0);
				aa.setval(row,6,1.0);
				bb.setval(row,1,lon1);
		} else {
			// Use upper left
			row++;
			aa.setval(row,1,xPix0);
			aa.setval(row,2,yPix0);
			aa.setval(row,5,1.0);
			bb.setval(row,1,lat0);
			row++;
			aa.setval(row,3,xPix0);
			aa.setval(row,4,yPix0);
			aa.setval(row,6,1.0);
			bb.setval(row,1,lon0);
		}
		// See if point used is lower right
		if(aa.getval(1,1) != xPix1 && aa.getval(2,2) == yPix1 &&
			bb.getval(1,1) == lat1 && bb.getval(2,1) == lon1) {
				// Is lower right, use lower left
				row++;
				aa.setval(row,1,xPix0);
				aa.setval(row,2,yPix1);
				aa.setval(row,5,1.0);
				bb.setval(row,1,lat1);
				row++;
				aa.setval(row,3,xPix0);
				aa.setval(row,4,yPix1);
				aa.setval(row,6,1.0);
				bb.setval(row,1,lon0);
		} else {
			// Use lower right
			row++;
			aa.setval(row,1,xPix1);
			aa.setval(row,2,yPix1);
			aa.setval(row,5,1.0);
			bb.setval(row,1,lat1);
			row++;
			aa.setval(row,3,xPix1);
			aa.setval(row,4,yPix1);
			aa.setval(row,6,1.0);
			bb.setval(row,1,lon1);
		}
	} else if(nPoints == 2) {
		// Use other corner of the given points
		row++;
		aa.setval(row,1,aa.getval(1,1));
		aa.setval(row,2,aa.getval(3,2));
		aa.setval(row,5,1.0);
		bb.setval(row,1,bb.getval(3,1));
		row++;
		aa.setval(row,3,aa.getval(1,1));
		aa.setval(row,4,aa.getval(3,2));
		aa.setval(row,6,1.0);
		bb.setval(row,1,bb.getval(2,1));
	}

	// Get the singular values
	aa.singularvalues(u,w,v);
	vt=v.transpose();
	wi=1./w;
	aainv=vt*wi.diagonalmatrix()*u;

	// Calculate the coefficients
	xx=aainv*bb;

	a.setval(1,1,xx.getval(1,1));
	a.setval(1,2,xx.getval(2,1));
	a.setval(2,1,xx.getval(3,1));
	a.setval(2,2,xx.getval(4,1));
	b.setval(1,1,xx.getval(5,1));
	b.setval(2,1,xx.getval(6,1));

	// Calculate ainv;
	det=a.determinant();
	a.singularvalues(u,w,v);
	vt=v.transpose();
	wi=1./w;
	ainv=vt*wi.diagonalmatrix()*u;

	if(!det || aa.geterror() || ainv.geterror()) {
		errMsg(_T("Calibration failed"));
		calibrationValid=FALSE;
	}
}

BOOL CMap::readPictureFile(void)
{
	HRESULT hres;
	BYTE szBuffer[1024]={0};
	DecompressImageInfo dii;
	HDC hdc;
	int iBitDepth;

	if(!fileName || !hDCMem) return FALSE;

	// Open the file
	if(hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile=INVALID_HANDLE_VALUE;
	}
	hFile=CreateFile(fileName,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		DWORD dwError=GetLastError();
		sysErrMsg(_T("Could not open image file:"));
		return FALSE;
	}

	// Get the DC of the specified window
	hdc=GetDC(hWnd);
	if(!hdc) return FALSE;

	// Fill in the 'DecompressImageInfo' structure
	dii.dwSize=sizeof(DecompressImageInfo);  // Size of this structure
	dii.pbBuffer=szBuffer;  // Pointer to the buffer to use for data
	dii.dwBufferMax=1024;   // Size of the buffer
	dii.dwBufferCurrent=0;  // The amount of data which is current in the buffer
	dii.phBM=&hBitmap;      // Pointer to the bitmap returned (can be NULL)
	dii.ppImageRender=NULL; // Pointer to an IImageRender object  // (can be NULL)

	// The WIN32 version only handles BitDepth = (2,4,8,16,24).
	// Using 24 for 32 seems to work.
	iBitDepth=GetDeviceCaps(hdc,BITSPIXEL);
	if(iBitDepth > 24) iBitDepth=24;
	dii.iBitDepth=iBitDepth; // Bit depth of the output image

	dii.lParam=(LPARAM)this; // User parameter for callback functions
	dii.hdc=hdc;  // HDC to use for retrieving palettes
	dii.iScale=100; // Scale factor (1 - 100)
	dii.iMaxWidth=MAXWIDTH; // Maximum width of the output image
	dii.iMaxHeight=MAXWIDTH; // Maxumum height of the output image
	dii.pfnGetData=CMap::getImageData; // Callback function to get image data
	dii.pfnImageProgress=CMap::imageProgress; // Callback function to
	// Notify caller of progress decoding the image
	dii.crTransparentOverride=(UINT)(-1); // If this color is not
	// (UINT)-1, it will override the transparent color in the image with
	// this color. (GIF ONLY)

	// Step 6: Call DecompressImageIndirect
	hres=DecompressImageIndirect(&dii);

	// Clean up 
	CloseHandle(hFile);
	hFile=INVALID_HANDLE_VALUE;
	ReleaseDC(hWnd,hdc);

	return TRUE;
}

DWORD CALLBACK CMap::getImageData(LPSTR szBuffer, DWORD dwBufferMax,
								  LPARAM lParam)
{
	DWORD dwNumberOfBytesRead;
	CMap *pMap=(CMap *)lParam;

	if(!pMap) return 0;
	if(pMap->hFile == INVALID_HANDLE_VALUE) return 0;

	ReadFile(pMap->hFile,szBuffer,dwBufferMax,&dwNumberOfBytesRead,NULL);

	return dwNumberOfBytesRead;
}

void CALLBACK CMap::imageProgress(IImageRender *pRender, BOOL bComplete,
								  LPARAM lParam)
{
#if 0
	CMap *pMap=(CMap *)lparam;

	if(bComplete) {
		;//Do whatever when done processing the image
	}
#endif
}

BITMAP CMap::getBitmap(void) const
{
	BITMAP bmp;

	// Zero it
	memset(&bmp,0,sizeof(BITMAP));

	// Get it from the handle
	if(hBitmap) {
		GetObject(hBitmap,sizeof(BITMAP),&bmp);
	}
	return bmp;
}

void CMap::getMatrices(Matrix &aIn, Matrix &ainvIn, Matrix &bIn) const
{
	aIn=a;
	ainvIn=ainv;
	bIn=b;    
}
