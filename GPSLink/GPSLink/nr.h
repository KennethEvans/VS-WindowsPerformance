//////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 The University of Chicago, as Operator of Argonne
// National Laboratory.
// This file is distributed subject to a Software License Agreement found
// in the file LICENSE that is included with this distribution. 
//////////////////////////////////////////////////////////////////////////

// nr.h *** Header for Numerical Recipies routines for Matrix class

#if !defined(_NR_H)
#define _NR_H

void nrerrmsg(TCHAR *format,...);
void nrwarnmsg(TCHAR *format,...);
void lubksb(double **a ,int n,int *indx,double b[]);
void ludcmp(double **a,int n,int *indx,double &det);
void hsort(double arrin[],int indx[],int n);
double pythag(double a,double b);
void svdcmp(double **a,int m,int n,double w[],double **v);
double *vector(long nh);
double *vector(long nl,long nh);
int *ivector(long nh);
int *ivector(long nl,long nh);
long *lvector(long nh);
long *lvector(long nl,long nh);
double **matrix(long nrh,long nch);
double **matrix(long nrl,long nrh,long ncl,long nch);
void freevector(double *v,long nh);
void freevector(double *v,long nl,long nh);
void freeivector(int *v,long nh);
void freeivector(int *v,long nl,long nh);
void freelvector(int *v,long nh);
void freelvector(int *v,long nl,long nh);
void freematrix(double **m,long nrh,long nch);
void freematrix(double **m,long nrl,long nrh,long ncl,long nch);


#endif // !defined(_NR_H)
