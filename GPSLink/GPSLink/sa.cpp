// Street Atlas routines for GPSLink

// These routines are mostly based on those in G7TO/W

#include "StdAfx.h"

#include "sa.h"

#define G7INT short

#define M_PI 3.14159265358979323846
#define Pi (double)M_PI

#define DEF_TXT_COLOR 2
#define DEF_TRK_COLOR 2
#define DEF_RTE_COLOR 1
#define DEF_WPT_COLOR 2

char *Trailer1= "CDMLTypeSetting";

struct _Const2 {
	G7INT a; G7INT b; G7INT c; G7INT d;
	G7INT e; G7INT f; G7INT g; G7INT h;
	G7INT i; G7INT j; G7INT k; G7INT l;
	G7INT m; G7INT n; G7INT o; G7INT p;
	G7INT q;
	G7INT a1; G7INT a2; G7INT a3; G7INT a4; G7INT a5; G7INT a6;
	G7INT a8; G7INT a9; G7INT aa; G7INT ab; G7INT ac; G7INT ad;
	G7INT ae; G7INT af; G7INT ag; G7INT ah; G7INT ai; G7INT aj;
	G7INT ak; G7INT al; G7INT am; G7INT an; G7INT ao; G7INT ap;
	G7INT aq; G7INT ar; G7INT as; G7INT at; G7INT au; G7INT av;
	G7INT aw; G7INT ax; G7INT ay; G7INT az; G7INT ba; G7INT bb;
	G7INT bc; G7INT bd;
	char qqa;
	char qqa1[9];
	char qq;
	char qq1[9];
	G7INT be; G7INT bf;
} Const2 = {
	0,0,0,0x3ff0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0x3ff0,
	0x7fff,
	1,1,0,0,10000,10000,
	1,1,6500,6500,9700,9700,
	1,1,200,200,9700,800,
	1,1,0,0,7500,10000,
	1,1,7500,0,10000,10000,
	1,1,200,200,7200,800,
	4,1,
	9,'c','h','a','i','n','.','c','h','n',
	9,'c','h','a','i','n','.','c','h','n',
	1,1
};
struct _Const3 {
	G7INT a; G7INT b; G7INT c; G7INT d;
} Const3 = {
	0,5,0x200,0x766c
};

struct _Const4 {
	G7INT a; G7INT b; G7INT c;
	G7INT d; G7INT e; G7INT f;
	G7INT i; G7INT h; G7INT g;
	G7INT j;
} Const4 = {
	1,0x3fff,0,
	3,0,1,
	1,0x3fff,0,
	0x102
};

struct _Trailer {
	G7INT a; G7INT b; G7INT c;
	G7INT d; G7INT e; G7INT f;
	G7INT i; G7INT h;
} Trailer = {
	3,0,1,
	1,0x3fff,0,
	0x100,0
};

struct _Trailersa6 {
	unsigned G7INT a1; unsigned G7INT b1; unsigned G7INT c1; unsigned G7INT d1;
	unsigned G7INT k;
} Trailersa6 = {
	0x7fb1, 0x0539, 0x0000, 0x0000, 0 
};

struct _Trailersa8 {
	unsigned short a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,aa,ab,ac,ad,ae,af;
	unsigned short b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,ba,bb,bc,bd,be,bf;
	unsigned short c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,ca,cb,cc,cd,ce,cf;
	unsigned short d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,da,db,dc,dd,de,df;
	unsigned short e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef;
	unsigned short f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,fa,fb,fc,fd,fe,ff;
	unsigned short g0,g1,g2,g3,g4,g5,g6,g7,g8,g9,gg,gb,gc,gd,ge,gf;
	unsigned short h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,hh,hb,hc,hd,he,hf;
	unsigned short j0,j1,j2,j3,j4,j5,j6,j7,j8,j9,jj,jb,jc,jd,je,jf;
	unsigned short k0,k1,k2,k3,k4,k5,k6,k7,k8,k9,kk,kb,kc,kd,ke,kf;
	unsigned short l0,l1,l2,l3,l4,l5,l6,l7,l8,l9,ll,lb,lc,ld,le,lf;
	unsigned short m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mm,mb,mc,md,me,mf;
	unsigned short n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,nn,nb,nc,nd;
} Trailersa8 = {
	0x0003,0x0000,0x0001,0x0001,0x3FFF,0x0000,0x0102,0x0000,
	0x000A,0x0000,0x0001,0x0001,0x7FFF,0x0000,0x000A,0x0000,
	0x0001,0x0001,0x7FFF,0x0000,0x8E7F,0x009F,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x7FB2,0x0539,
	0x0000,0x0000,0x0100,0x0000,0x0100,0x0000,0x0100,0x0000,
	0x0100,0x0000,0x0100,0x0000,0x0100,0x0000,0x0100,0x0000,
	0x0100,0x0000,0x0100,0x0000,0x0100,0x0000,0x0100,0x0000,
	0x0100,0x0000,0x0100,0x0000,0x0000,0x0000,0x0000,
};

struct _Trailer1a {
	unsigned G7INT a1; unsigned G7INT b1; unsigned G7INT c1; unsigned G7INT d1;
	unsigned G7INT a2; unsigned G7INT b2; unsigned G7INT c2; unsigned G7INT d2;
	unsigned G7INT a3; unsigned G7INT b3; unsigned G7INT c3; unsigned G7INT d3;
	unsigned G7INT a4;
} Trailer1a = {
	0x7fb1, 0x0539, 0x540b, 0x7079,
	0x2065, 0x6964, 0x6c61, 0x676f,
	0x0004, 0x0000, 0xffff, 0x0002,
	0x000f
};

// Trailer from Brent that appears to work as well as the trailer
// above
unsigned G7INT TrailerX[5]={ 0x7fb1,0x0539,0,0,0};

struct _Trailer1x {
	unsigned G7INT a1; unsigned G7INT b1; unsigned G7INT c1; unsigned G7INT d1;
	unsigned G7INT a2; unsigned G7INT b2; unsigned G7INT c2; unsigned G7INT d2;
	unsigned G7INT a3; unsigned G7INT b3; unsigned G7INT c3;
} Trailer1x = {
	0x3C00, 0x0000, 0x8001, 0x4400,
	0x0000, 0x8001, 0x4800, 0x0000,
	0x8001, 0x4C00, 0x0000
};

struct _sa4mapline {
	unsigned long latR;     // 4
	unsigned long lonR;     // 4
	unsigned G7INT ItemNum; // 2
	unsigned G7INT Const1;  // 2
	unsigned char paramlen; // 1
	G7INT LineColor;        // 2
	G7INT LineWidth;        // 2
	G7INT LineStyle;        // 2
	G7INT Points;           // 2
	double lat;             // 8 (unused)
	double lon;             // 8 (unused)
} sa4mapline;

struct _sa3symbol {
	unsigned long latR;     // 4
	unsigned long lonR;     // 4
	G7INT ItemNum;          // 2
	unsigned G7INT Const;   // 2
	unsigned char paramlen; // 1
	G7INT ObjCode;          // 2
	G7INT ObjSize;          // 2
	G7INT ObjColor;         // 2
	G7INT TextSize;         // 2
	G7INT TextColor;        // 2
	G7INT TextAlign;        // 2
	unsigned char TextLen;  // 1 (Unused)
	unsigned char *Text;    // 4 (Unused)
	double lat;             // 8 (Unused)
	double lon;             // 8 (Unused)
} sa3symbol;

struct _sa3mapnote {
	unsigned long latR;
	unsigned long lonR;
	G7INT ItemNum;
	unsigned G7INT Const;
	unsigned char paramlen;
	G7INT TextSize;
	G7INT TextColor;
	G7INT TextJustification;
	G7INT TextXoffset;
	G7INT TextYoffset;
	unsigned char TextLen;
	unsigned char *Text;
	double lat;
	double lon;
} sa3mapnote;

struct _sa3text {
	unsigned long latR;
	unsigned long lonR;
	G7INT ItemNum;
	unsigned G7INT Const;
	unsigned char paramlen;
	G7INT TextSize;
	G7INT TextColor;
	unsigned char TextLen;
	unsigned char *Text;
	double lat;
	double lon;
} sa3text;

struct _sa4maparea {
	unsigned long latR;
	unsigned long lonR;
	unsigned G7INT ItemNum;
	unsigned G7INT Const1;
	unsigned char paramlen;
	G7INT area_draw_line;
	G7INT area_draw_fill;
	G7INT LineColor;
	G7INT LineWidth;
	G7INT Const3;
	G7INT area_fill_color;
	G7INT area_fill_style;
	G7INT Points;
	double lat;
	double lon;
} sa4maparea;

struct _sa4mapcircle {
	unsigned long latR;
	unsigned long lonR;
	unsigned G7INT ItemNum;
	unsigned G7INT Const1;
	unsigned char paramlen;
	G7INT area_draw_line;
	G7INT area_draw_fill;
	G7INT LineColor;
	G7INT LineWidth;
	G7INT Const3;
	G7INT area_fill_color;
	G7INT area_fill_style;
	G7INT const4;
	unsigned long latCntr;
	unsigned long lonCntr;
	unsigned long latPnt;
	unsigned long lonPnt;
} sa4mapcircle;

struct _sa4route {
	unsigned long startLo;
	unsigned long startLa;
	unsigned long startLo1;
	unsigned long startLa1;
	unsigned G7INT unk1;
	unsigned G7INT unk2;
	unsigned G7INT unk3;
	unsigned G7INT unk4;
	unsigned G7INT unk5;
	unsigned G7INT unk6;
	unsigned long st_name_l;
	char *st_name;
	unsigned long st_st_name_l;
	char *st_st_name;
	unsigned long finishLo;
	unsigned long finishLa;
	unsigned long finishLo1;
	unsigned long finishLa1;
	unsigned G7INT unk7;
	unsigned G7INT unk8;
	unsigned G7INT unk9;
	unsigned G7INT unk10;
	unsigned G7INT unk11;
	unsigned G7INT unk12;
	unsigned long fi_name_l;
	char *fi_name;
	unsigned long fi_st_name_l;
	char *fi_st_name;
	unsigned long NumObjs;
} sa4route;

struct _sa4routepts {
	unsigned long startLo;
	unsigned long startLa;
	unsigned long startLo1;
	unsigned long startLa1;
	unsigned long checksum;
	unsigned G7INT unk3;
	unsigned G7INT unk4;
	unsigned G7INT unk5;
	unsigned G7INT unk6;
	unsigned long st_name_l;
	char *st_name;
	unsigned long st_st_name_l;
	char *st_st_name;
} sa4routepts;

struct _MapConstM {
	unsigned G7INT a; unsigned G7INT b; unsigned G7INT c;
	unsigned G7INT d; unsigned G7INT e; unsigned G7INT f;
	unsigned G7INT g;
	unsigned G7INT h; unsigned G7INT i; unsigned G7INT j;
	unsigned G7INT k; unsigned G7INT l; unsigned G7INT m;
	unsigned G7INT n; unsigned G7INT o; unsigned G7INT p;
	unsigned G7INT q; unsigned G7INT r; unsigned G7INT s;
	unsigned G7INT t; unsigned G7INT u; unsigned G7INT v;
} MapConstM = {
	0x0003,0x0000,0x0001,
	0x0001,0x3FFF,0x0000,
	0x0102,
	0x0000,0x000A,0x0000,
	0x0001,0x0001,0x7FFF,
	0x0000,0x000A,0x0000,
	0x0001,0x0001,0x7FFF,
	0x0000,0x8E7F,0x009F    // 0x8E7E for SA5, 0x8E7F for SA6
};

struct _MapConstM0 {
	G7INT a; G7INT b; G7INT c; G7INT d; G7INT e;
	G7INT f; G7INT g; G7INT h; G7INT i; G7INT j;
	G7INT k; G7INT l; G7INT m; G7INT n; G7INT o;
	G7INT p; G7INT q; G7INT r; G7INT s; G7INT t;
	G7INT u; G7INT v;
} MapConstM0 = {
	0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000
};

// Function prototypes
static double fm_diskLL(unsigned long l);
static unsigned long to_diskLL(double l);
static void setObjParams(void);

// Global variables

static const double Degree=Pi/180.0;
short ShowGrids=1,NumObjs=0;
short TextColor=DEF_TXT_COLOR,TextSize=0,TextAlign=2;
short WptTextColor=DEF_WPT_COLOR,WptTextSize=0,WptTextAlign=2;
short WptObjCode=14,WptObjColor=DEF_WPT_COLOR,WptObjSize=0;
short RteTextColor=DEF_RTE_COLOR,RteTextSize=0,RteTextAlign=4;
short RteObjCode=46,RteObjColor=DEF_RTE_COLOR,RteObjSize=0;
short RteLineWidth=1,RteLineColor=DEF_RTE_COLOR,RteLineStyle=0;
short TrkLineWidth=1,TrkLineColor=DEF_TRK_COLOR,TrkLineStyle=0;
short LineWidth=1,LineColor=0,LineStyle=0;
short CirLineWidth=0,CirLineColor=0,CirFillStyle=0;
short CirLineDraw=1,CirFillArea=0,CirFillColor=0;
short ObjCode=0,ObjSize=0,ObjColor=0;
SAVersion saVer=SA_8;
short saObjNumber=0;    // Current SA item number
short Id=0x100,MagC=4,Mapfeatures=0x3fff;
unsigned long Const1=0x1a2b3c4dL;
unsigned long LatConst=0x6ae187b1L;
unsigned long LonConst=0x494ae39bL;
double setlat=1000.0;
double setlon=1000.0;
short setmag=0;

// double LatC,LonC;

char align[8][3]={"nw","n","ne","w","e","sw","s","se"};
char sizes[5][3]={"vs","s","m","l","vl"};
char colors[5][7]={"black","red","blue","green","yellow"};
char width[5][3]={"vt","t","m","k","vk"};
char style[3][3]={"n","ha","*h"};
char noyes[2][2]={"n","y"};

// Get LL from SA LatR, LonR
static double fm_diskLL(unsigned long l)
{
	unsigned long ff7f,m80;

	ff7f=0x7fffffL;
	m80=0x80000000L;

	if(l<=m80 ) 
		return (double)((m80-l)/(double)ff7f);
	else
		return -(double)((l-m80)/(double)ff7f);
}

// Get SA LatR, LonR from LL
static unsigned long to_diskLL(double l)
{
	unsigned long ff7f,m80;

	ff7f=0x7fffffL;
	m80=0x80000000L;

	return (unsigned long)(m80-(unsigned long)(l*(double)ff7f));
}

static void setObjParams(void)
{
	sa3symbol.TextAlign=TextAlign;
	sa3symbol.TextColor=TextColor;
	sa3symbol.ObjColor=ObjColor;
	sa3symbol.ObjCode=ObjCode;
	sa3symbol.ObjSize=ObjSize;
	sa3symbol.TextSize=TextSize;
	sa4mapline.LineColor=LineColor;
	sa4mapline.LineStyle=LineStyle;
	sa4mapline.LineWidth=LineWidth;
	sa4mapcircle.area_draw_line=CirLineDraw;
	sa4mapcircle.area_draw_fill=CirFillArea;
	sa4mapcircle.LineColor=CirLineColor;
	sa4mapcircle.LineWidth=CirLineWidth;
	sa4mapcircle.area_fill_color=CirFillColor;
	sa4mapcircle.area_fill_style=CirFillStyle;
}

void setSAVer(SAVersion ver)
{
	saVer=ver;
}

void setSALimits(double latMin, double lonMin, double latMax, double lonMax,
				 int numObjs)
{
	INT rt,magla,maglo;
	double xx,yy,ff;

	// Transfer the number of objects
	NumObjs=numObjs;

	// Determine the magnification and LL center
	if(saVer == SA_5 || saVer == SA_6 || saVer == SA_8) magla=maglo=18;
	else magla=maglo=16;
	xx=(latMax-latMin)/2.0;
	xx=xx*70.0;
	rt=19;
	while(--rt>3) {
		if((26500.0/pow(2.0,(double)rt)) > xx) {
			magla=rt;
			break;
		}
	}
	xx=(latMax+latMin)/2.0;
	yy=(lonMax-lonMin)/2.0;
	ff=yy*10.0;
	xx=xx*Degree;
	xx=cos(xx);
	yy=yy*60.0+ff; // fudge factor
	rt=19;
	while(--rt>3) {
		if(29800.0/pow(2.0,(double)rt)/xx > yy) {
			maglo=rt;
			break;
		}
	}
	setlat=(latMax+latMin)/2.0;
	setlon=(lonMax+lonMin)/2.0;
	if(setlon==0.0) {
		setlon=100.0;
		setmag=4;
	}
	if(setlat==0.0) {
		setlat=40.0;
		setmag=4;
	}
	if(setmag==0) setmag=min(magla,maglo);
	if(saVer == SA_5 || saVer == SA_6 || saVer == SA_8)
		if(setmag>18) setmag=18;
		else
			if(setmag>16) setmag=16;
	if(setmag<1) setmag=1;
}

BOOL writeSAHeader(FILE *out)
{
	unsigned char len;
	char buffer[80];

	if(!out) return FALSE;

	// Set the current parameters into the structures
	setObjParams();

	// Zero the current object number (G7ToWin seems to use 1)
	saObjNumber=0;

	// Write appropriate header info
	if(saVer == SA_4 || saVer == SA_5 || saVer == SA_6 || saVer == SA_8) {
		Id=0x102;
		Const2.bc=6;
		Const2.qq1[0]=Const2.qqa1[0]='C';
	}
	fwrite(&Id,2,1,out);
	fwrite(&Const1,4,1,out);
	strcpy(buffer,"GPSLData");
	len=(unsigned char)strlen(buffer);
	fwrite(&len,1,1,out);
	fwrite(buffer,len,1,out);
	if(setlat!=1000.0) LatConst=to_diskLL(setlat);
	fwrite(&LatConst,4,1,out);
	if(setlon!=1000.0) LonConst=to_diskLL(-setlon);
	fwrite(&LonConst,4,1,out);
	if(setmag) MagC=setmag;
	fwrite(&MagC,2,1,out);
	fwrite(&Const2,134,1,out);
	fwrite(&Mapfeatures,2,1,out);
	fwrite(&Const3,8,1,out);
	fwrite(&ShowGrids,2,1,out);
	if(saVer == SA_6 || saVer == SA_8) Const4.j=0x102;
	else Const4.j=0x100;
	fwrite(&Const4,20,1,out);

	// Write the number of objects
	fwrite(&NumObjs,2,1,out);

	return TRUE;
}

BOOL writeSATrailer(FILE *out)
{
	long zero=0;

	if(!out) return FALSE;

	if(saVer == SA_5)	{
		fwrite(&Trailersa6,9,1,out);
	} else if(saVer == SA_5) {
		fwrite(&zero,4,1,out);
		fwrite(&zero,4,1,out);
		fwrite(&Trailersa6,9,1,out);
	} else if(saVer == SA_8) {
		// Note 205 bytes, not 206
		fwrite(&Trailersa8,205,1,out);
	}

	return TRUE;
}

BOOL writeSAWaypoint(FILE *out, double lat, double lon, char *ident)
{
	short ItemType=2;

	if(!out) return FALSE;

	sa3symbol.ItemNum=saObjNumber++;
	sa3symbol.latR=to_diskLL(lat);
	sa3symbol.lonR=to_diskLL(-lon);
	sa3symbol.Const=0x8000;
	sa3symbol.paramlen=0;

	sa3symbol.ObjCode=WptObjCode;
	sa3symbol.ObjSize=WptObjSize;
	sa3symbol.ObjColor=WptObjColor;

	sa3symbol.TextColor=WptTextColor;
	sa3symbol.TextSize=WptTextSize;
	sa3symbol.TextAlign=WptTextAlign;
	sa3symbol.TextLen=(unsigned char)strlen(ident);

	fwrite(&ItemType,2,1,out);
	// Break this up because of 8-byte alignment padding
	fwrite(&sa3symbol,12,1,out);
	fwrite(&sa3symbol.paramlen,1,1,out);
	fwrite(&sa3symbol.ObjCode,12,1,out);
	fwrite(&sa3symbol.TextLen,1,1,out);
	fwrite(ident,sa3symbol.TextLen,1,out);

	return TRUE;
}

BOOL writeSARouteWaypoint(FILE *out, double lat, double lon, char *ident)
{
	short ItemType=2;

	if(!out) return FALSE;

	sa3symbol.ItemNum=saObjNumber++;
	sa3symbol.latR=to_diskLL(lat);
	sa3symbol.lonR=to_diskLL(-lon);
	sa3symbol.Const=0x8000;
	sa3symbol.paramlen=0;

	sa3symbol.ObjCode=RteObjCode;
	sa3symbol.ObjSize=RteObjSize;
	sa3symbol.ObjColor=RteObjColor;

	sa3symbol.TextColor=RteTextColor;
	sa3symbol.TextSize=RteTextSize;
	sa3symbol.TextAlign=RteTextAlign;
	sa3symbol.TextLen=(unsigned char)strlen(ident);

	fwrite(&ItemType,2,1,out);
	// Break this up because of 8-byte alignment padding
	fwrite(&sa3symbol,12,1,out);
	fwrite(&sa3symbol.paramlen,1,1,out);
	fwrite(&sa3symbol.ObjCode,12,1,out);
	fwrite(&sa3symbol.TextLen,1,1,out);
	fwrite(ident,sa3symbol.TextLen,1,out);

	return TRUE;
}

BOOL writeSARouteHeader(FILE *out, double lat, double lon, short points)
{
	short ItemType=7;

	if(!out) return FALSE;

	sa4mapline.ItemNum=saObjNumber++;
	sa4mapline.latR=to_diskLL(lat);
	sa4mapline.lonR=to_diskLL(-lon);
	sa4mapline.Const1=0x8000;
	sa4mapline.paramlen=0;
	sa4mapline.LineColor=RteLineColor;
	sa4mapline.LineWidth=RteLineWidth;
	sa4mapline.LineStyle=RteLineStyle;
	sa4mapline.Points=points;

#if 0
	_sa4mapline *ptr=&sa4mapline;
#endif

	fwrite(&ItemType,2,1,out);
	// Break this up because of 8-byte alignment padding
	fwrite(&sa4mapline,12,1,out);
	fwrite(&sa4mapline.paramlen,1,1,out);
	fwrite(&sa4mapline.LineColor,8,1,out);

	return TRUE;
}

BOOL writeSATrackHeader(FILE *out, double lat, double lon, short points)
{
	short ItemType=7;

	if(!out) return FALSE;

	sa4mapline.ItemNum=saObjNumber++;
	sa4mapline.latR=to_diskLL(lat);
	sa4mapline.lonR=to_diskLL(-lon);
	sa4mapline.Const1=0x8000;
	sa4mapline.paramlen=0;
	sa4mapline.LineColor=TrkLineColor;
	sa4mapline.LineWidth=TrkLineWidth;
	sa4mapline.LineStyle=TrkLineStyle;
	sa4mapline.Points=points;

#if 0
	_sa4mapline *ptr=&sa4mapline;
#endif

	fwrite(&ItemType,2,1,out);
	// Break this up because of 8-byte alignment padding
	fwrite(&sa4mapline,12,1,out);
	fwrite(&sa4mapline.paramlen,1,1,out);
	fwrite(&sa4mapline.LineColor,8,1,out);

	return TRUE;
}

BOOL writeSATrackpoint(FILE *out, double lat, double lon)
{
	if(!out) return FALSE;

	sa4mapline.latR=to_diskLL(lat);
	sa4mapline.lonR=to_diskLL(-lon);
	fwrite(&sa4mapline,8,1,out);

	return TRUE;
}

