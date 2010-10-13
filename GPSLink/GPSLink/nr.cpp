//////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 The University of Chicago, as Operator of Argonne
// National Laboratory.
// This file is distributed subject to a Software License Agreement found
// in the file LICENSE that is included with this distribution. 
//////////////////////////////////////////////////////////////////////////

// nr.cpp *** Numerical Recipies routines for Matrix class
// Based on routines in W. H. Press, et al., Numerical Recipies in C
//   Float replaced by double
//   C++ memory allocation
//   Message handler is nrerrmsg (exits), nrwarnmsg (stays)
//   Improvements / Modifications

#include "StdAfx.h"

#if 0
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include "nr.h"
#include "utils.h"
const int NR_END=0;
const double TINY=1.0e-20;

// Inline macros

inline double DMAX(double a, double b)
{
	double evala=a,evalb=b;
	return (evala > evalb)?evala:evalb;
}

inline double DSQR(double a)
{
	double evala=a;
	return (evala == 0.0)?0.0:evala*evala;
}

inline int IMIN(int a, int b)
{
	int evala=a,evalb=b;
	return (evala < evalb)?evala:evalb;
}

inline int IMAX(int a, int b)
{
	int evala=a,evalb=b;
	return (evala > evalb)?evala:evalb;
}

inline double SIGN(double a, double b)
{
	double evala=a,evalb=b;
	return (evalb >= 0.0)?fabs(evala):-fabs(evala);
}


#if defined(WIN32)
// On WIN32 use this instead
#define nrerrmsg errMsg
#define nrwarnmsg infoMsg
#else
/**************************** nrerrmsg ******************************/
void nrerrmsg(TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	_ftprintf(stderr,"\nAborting\n");
	exit(1);
}
/**************************** nrwarnmsg *****************************/
void nrwarnmsg(TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	_vstprintf(stderr,format,args);
	va_end(args);
	_ftprintf(stderr,"\n");
}
#endif
/**************************** hsort *********************************/
void hsort(double arrin[],int indx[],int n)
{
	int l,j,ir,indxt,i;
	double q;

	if(n == 1) {
		indx[1]=1;
		return;
	}

	for(j=1; j <= n; j++) indx[j]=j;
	l=(n>>1)+1;
	ir=n;
	for(;;) {
		if(l > 1) q=arrin[(indxt=indx[--l])];
		else {
			q=arrin[(indxt=indx[ir])];
			indx[ir]=indx[1];
			if(--ir == 1) {
				indx[1]=indxt;
				return;
			}
		}
		i=l;
		j=l<<1;
		while(j <= ir) {
			if(j < ir && arrin[indx[j]] < arrin[indx[j+1]]) j++;
			if(q < arrin[indx[j]]) {
				indx[i]=indx[j];
				j+=(i=j);

			}
			else  j=ir+1;
		}
		indx[i]=indxt;
	}
}
/**************************** lubskb ********************************/
void lubksb(double **a, int n, int *indx, double b[])
{
	int i,ii=0,ip,j;
	double sum;

	for (i=1;i<=n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n;i>=1;i--) {
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}
/**************************** ludcmp ********************************/
void ludcmp(double **a, int n, int *indx, double &det)
{
	int i,imax,j,k;
	double aamax,big,dum,sum,temp;
	double *vv;

	vv=vector(1,n);
	det=1.0;
	for (i=1;i<=n;i++) {
		aamax=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > aamax) aamax=temp;
		if (aamax == 0.0) {
			errMsg(_T("ludcmp: Singular matrix"));
			det=0.;
			return;
		}
		vv[i]=1.0/aamax;
	}
	for (j=1;j<=n;j++) {
		for (i=1;i<j;i++) {
			sum=a[i][j];
			for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++) {
			sum=a[i][j];
			for (k=1;k<j;k++)
				sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=1;k<=n;k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			det=-det;
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n) {
			dum=1.0/(a[j][j]);
			for (i=j+1;i<=n;i++) a[i][j] *= dum;
		}
	}
	// Find determinant
	for(i=1; i <= n; i++) det*=a[i][i];
	// Check size
	if(fabs(det)/aamax < TINY) {
		errMsg(_T("ludcmp: Determinant=%g is small")
			_T(" relative to largest diagonal element"),det);
	}
	// Clean up and return
	freevector(vv,1,n);
}
/**************************** pythag ********************************/
double pythag(double a, double b)
{
	double absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+DSQR(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+DSQR(absa/absb)));
}
/**************************** svdcmp ********************************/
void svdcmp(double **a, int m, int n, double w[], double **v)
{
	double pythag(double a, double b);
	int flag,i,its,j,jj,k,l,nm;
	double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

	rv1=vector(1,n);
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm=DMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=IMIN(m,n);i>=1;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f=(s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((double)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((double)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((double)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=pythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its == 30) {
				nrerrmsg(_T("svdcmp: No convergence in 30 svdcmp iterations"));
				break;
			}
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	freevector(rv1,1,n);
}
/**************************** vector ********************************/
double *vector(long nh)
/* allocate a double vector with subscript range v[1..nh] */
{
	return vector(1,nh);
}
double *vector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v=new double[nh-nl+1+NR_END];
	if (!v) {
		nrerrmsg(_T("vector: Allocation failure"));
		return v;
	}
	return v-nl+NR_END;
}
/**************************** ivector *******************************/
int *ivector(long nh)
/* allocate an int vector with subscript range v[1..nh] */
{
	return ivector(1,nh);
}
int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v=new int[nh-nl+1+NR_END];
	if (!v) {
		nrerrmsg(_T("ivector: Allocation failure"));
		return v;
	}
	return v-nl+NR_END;
}
/**************************** lvector *******************************/
long *lvector(long nh)
/* allocate a long vector with subscript range v[1..nh] */
{
	return lvector(1,nh);
}
long *lvector(long nl, long nh)
/* allocate a long vector with subscript range v[nl..nh] */
{
	long *v=new long[nh-nl+1+NR_END];
	if (!v) {
		nrerrmsg(_T("lvector: Allocation failure"));
		return v;
	}
	return v-nl+NR_END;
}
/**************************** matrix ********************************/
double **matrix(long nrh, long nch)
/* allocate a double matrix with subscript range m[1..nrh][1..nch] */
{
	return matrix(1,nrh,1,nch);
}
double **matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i,nrow=nrh-nrl+1,ncol=nch-ncl+1;

	/* allocate pointers to rows */
	double **m=new double *[nrow+NR_END];
	if (!m) {
		nrerrmsg(_T("matrix: Allocation failure 1"));
		return m;
	}
	m+=NR_END;
	m-=nrl;
	/* allocate rows and set pointers to them */
	for(i=nrl; i <= nrh; i++) {
		m[i]=new double [ncol+NR_END];
		if (!m[i]) {
			nrerrmsg(_T("matrix: Allocation failure, element %d"),i);
		} else {
			m[i]+=NR_END;
			m[i]-=ncl;
		}
	}	
	/* return pointer to array of pointers to rows */
	return m;
}
/**************************** freevector ****************************/
void freevector(double *v, long nh)
/* free a double vector allocated with vector() */
{
	freevector(v,1,nh);
}
void freevector(double *v, long nl, long)
/* free a double vector allocated with vector() */
{
	delete [] (v+nl-NR_END);
}
/**************************** freeivector ***************************/
void freeivector(int *v, long nh)
/* free an int vector allocated with ivector() */
{
	freeivector(v,1,nh);
}
void freeivector(int *v, long nl, long)
/* free an int vector allocated with ivector() */
{
	delete [] (v+nl-NR_END);
}
/**************************** freelvector ***************************/
void freelvector(int *v, long nh)
/* free a long vector allocated with lvector() */
{
	freelvector(v,1,nh);
}
void freelvector(int *v, long nl, long)
/* free a long vector allocated with lvector() */
{
	delete [] (v+nl-NR_END);
}
/**************************** freematrix ****************************/
void freematrix(double **m, long nrh, long nch)
/* free a double matrix allocated by matrix() */
{
	freematrix(m,1,nrh,1,nch);
}
void freematrix(double **m, long nrl, long nrh, long ncl, long)
/* free a double matrix allocated by matrix() */
{
	long i;

	for(i=nrl; i <= nrh; i++) {
		delete [] (m[i]+ncl-NR_END);
	}	
	delete [] (m+nrl-NR_END);
}
