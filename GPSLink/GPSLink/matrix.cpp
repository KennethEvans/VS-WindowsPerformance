// matrix.c *** Functions of Matrix Class

// $Log: matrix.cc,v $
// Revision 1.3  1996/10/07 20:43:38  evans
// Fixed typo in Matrix::diagonalmatrix that made it not work right for a
// column matrix.
//
// Revision 1.2  1996/06/25  18:23:07  evans
// Added optional imax argument to statistics() to return index of maxabs.
//
// Revision 1.1.1.1  1996/03/22  12:38:27  evans
// Imported files.
//

#include "StdAfx.h"

#include <float.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "matrix.h"
#include "utils.h"

#define PRINTFORMAT "% #11.4g"

const int NUMBUFLENGTH=32;

/**************************** constructors **************************/
/********************************************************************/
Matrix::Matrix()
// Default constructor: Zero matrix(1,1)
{
	nrows=ncols=1;
	mm=matrix(1,1);
	err=ERR_NOTINIT;
}

Matrix::Matrix(int rows, int cols, double value)
// A matrix(rows,cols) initialized with value
{                                                      
	if(rows < 1 || cols < 1) {
		errMsg(_T("Invalid size: (%d,%d)"),rows,cols);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	nrows=rows;
	ncols=cols;
	err=0;
	mm=matrix(nrows,ncols);
	initialize(value);
}

Matrix::Matrix(const Matrix &other)
// Copy constructor
{
	int i,j;

	nrows=other.nrows;
	ncols=other.ncols;
	err=other.err;
	mm=matrix(nrows,ncols);
	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) mm[i][j]=other.mm[i][j];
	}
}

Matrix::Matrix(const Matrix &other, int rows, int cols,
			   int startrow, int startcol)
			   // Submatrix(rows,cols) starting at startrow, startcol of other
			   // startrow and startcol may be non-positive
			   // Zero filled if extends beyond other
{
	int i,j,i1,j1;

	if(rows < 1 || cols < 1) {
		errMsg(_T("Invalid size: (%d,%d)"),rows,cols);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	nrows=rows;
	ncols=cols;
	err=0;
	mm=matrix(nrows,ncols);
	for(i=1; i <= nrows; i++) {
		i1=startrow-1+i;
		if(i1 < 1) for(j=1; j <= ncols; j++) mm[i][j]=0;
		else if(i1 > other.nrows) for(j=1; j <= ncols; j++) mm[i][j]=0;
		else {
			for(j=1; j <= ncols; j++) {
				j1=startcol+j-1;
				if(j1 < 1) mm[i][j]=0;
				else if(j1 > other.ncols) mm[i][j]=0;
				else mm[i][j]=other.mm[i1][j1];
			}
		}
	}
}

Matrix::Matrix(int rows, int cols, char type)
// Identity matrix(rows,cols) if type='I'
{
	int i,j;

	if(rows < 1 || cols < 1) {
		errMsg(_T("Invalid size: (%d,%d)"),rows,cols);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	nrows=rows;
	ncols=cols;
	err=0;
	mm=matrix(nrows,ncols);
	switch(type) {
	case 'I':     // Identity matrix
	case 'i':
		if(nrows != ncols) {
			errMsg(_T("Identity matrix must be square: (%d,%d)"),
				nrows,ncols);
			err=ERR_NOTINIT;
		}
		else {
			for(i=1; i <= nrows; i++) {
				for(j=1; j <= ncols; j++) {
					if(i != j) mm[i][j]=0.;
					else mm[i][i]=1.;
				}
			}
		}
		break;
	default:
		errMsg(_T("Matrix::Matrix: Unknown type: %c"),type);
		err=ERR_NOTINIT;
		break;
	}
}

Matrix::Matrix(int rows, int cols, char type, Matrix v)
// Diagonal matrix(n,n) with v on the diagonal if type='D'
// n is the larger of rows, cols
// v must be a single row or column matrix
{
	int n,i;

	if(rows < 1 || cols < 1) {
		errMsg(_T("Invalid size: (%d,%d)"),rows,cols);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Use the larger of rows and columns
	if(rows >= cols) {
		nrows=ncols=n=nrows;
		err=0;
		mm=matrix(n,n);
	}
	else {
		nrows=ncols=n=cols;
		err=0;
		mm=matrix(n,n);
	}
	initialize();

	switch(type) {
	case 'D':
	case 'd':
		if(v.nrows == 1) {
			for(i=1; i <= n; i++) {
				if(i <= v.ncols) mm[i][i]=v.mm[1][i];
				else mm[i][i]=0.;
			}
		}
		else if(v.ncols == 1) {
			for(i=1; i <= n; i++) {
				if(i <= v.ncols) mm[i][i]=v.mm[i][1];
				else mm[i][i]=0.;
			}
		}
		else {
			errMsg(_T("Can only make a diagonal matrix from a ")
				_T("matrix with a single row or column: ")
				_T("(%d,%d)"),v.nrows,v.ncols);
			err=ERR_NOTINIT;
		}
		break;
	default:
		errMsg(_T("Matrix::Matrix: Unknown type: %c"),type);
		err=ERR_NOTINIT;
		break;
	}
}

Matrix::Matrix(char *filename)
// Read matrix from a binary file
{
	FILE *file;
	char id[sizeof(MATRIXID)];
	int i,j;

	// Open file
	file=fopen(filename,"r");
	if(!file) {
		errMsg(_T("Cannot open %s"),filename);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Read ID
	fread(id,sizeof(MATRIXID),1,file);
	if(ferror(file)) {
		errMsg(_T("Cannot read %s"),filename);
		fclose(file);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	id[sizeof(MATRIXID)-1]='\0';
	// Check ID
	if(strcmp(id,MATRIXID) != 0) {
		errMsg(_T("Not a valid matrix file: %s"),filename);
		fclose(file);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Read rows and columns
	fread((char *)&nrows,sizeof(nrows),1,file);
	fread((char *)&ncols,sizeof(ncols),1,file);
	if(ferror(file)) {
		errMsg(_T("Cannot read %s"),filename);
		fclose(file);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	if(nrows <= 0 || ncols <= 0) {
		errMsg(_T("Invalid size (%d,%d) matrix in %s"),nrows,ncols,filename);
		fclose(file);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Allocate matrix
	mm=matrix(nrows,ncols);
	err=0;
	initialize(ERRVAL);
	// Read elements
	for(i=1; i <=nrows; i++) {
		for(j=1; j <= ncols; j++) {
			fread((char *)&mm[i][j],sizeof(double),1,file);
		}
	}
	if(ferror(file)) {
		err=ERR_BADVAL;
		errMsg(_T("Error reading %s"),filename);
	}
	// Close file and return
	fclose(file);
	return;
}

Matrix::Matrix(char *filename, char type, int rows, int cols)
// Read matrix from a text file
//   From Mathematica file if type='M'
//   From rows and columns file if type='T'
// Will scan file for number of rows and/or columns if given as 0
// Will zero fill missing rows and columns
// Will discard rows or columns from the file beyond the given values
{
	FILE *file;
	int c,zrows,zcols,zcolsmax,level,done,nchars=0,comment=0;
	char buf[NUMBUFLENGTH];

	// Open file
	file=fopen(filename,"r");
	if(!file) {
		errMsg(_T("Cannot open %s"),filename);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Determine number of rows and columns
	if(rows < 1 || cols < 1) {     // Scan file
		switch(type) {
	case 'M':     // Mathematica file
	case 'm':
		level=done=zrows=zcols=zcolsmax=0;
		while((c=getc(file)) != EOF) {
			if(done) break;
			switch(c) {
	case '{':
		if(++level == 1) {
		}
		else if(level == 2) {
			zrows++;
			zcols=0;
		}
		else {
			errMsg(_T("Too many {'s: %s"),filename);
			fclose(file);
			nrows=ncols=1;
			mm=matrix(1,1);
			err=ERR_NOTINIT;
			return;
		}
		break;
	case '}':
		if(--level == 0) {
			done=1;
		}
		else if(level == 1) {
			zcols++;
			if(zcolsmax < zcols) zcolsmax=zcols;     // Longest row
		}
		else {
			errMsg(_T("Too many }'s: %s"),filename);
			fclose(file);
			nrows=ncols=1;
			mm=matrix(1,1);
			err=ERR_NOTINIT;
			return;
		}
		break;
	case ',':
		if(level == 2) zcols++;
		break;
			}
		}     // End while
		break;     // End of 'M' case
	case 'T':     // Text file
	case 't':
		level=zrows=zcols=zcolsmax=0;

		while((c=getc(file)) != EOF) {
			switch(c) {
	case ' ':     // Whitespace
	case '\t':
		if(nchars <= 0) break;
	case ',':     // Delimiters other than whitespace
	case ';':
		if(comment) break;
		level=1;
		nchars=0;
		break;
	case '\n':     // End of row or comment
		level=comment=0;
		if(zcolsmax < zcols) zcolsmax=zcols;
		if( nchars != 0) {
			nchars=0;
		}
		zcols=0;
		break;
	case '#':
		comment=1;
		break;
	default:
		if(comment) break;
		if(level == 0) {
			zrows++;
			zcols++;
			nchars=0;
		}
		else if(level == 1) {
			zcols++;
			nchars=0;
		}
		level=2;
		nchars++;
		break;
			}
		}     // End while
		break;     // End of 'T' case
		}     // End switch on type
	}     // End scanning file for rows and columns
	// Use the scanned values only if no values were given
	if(rows < 1) nrows=zrows;
	else nrows=rows;
	if(cols < 1) ncols=zcolsmax;
	else ncols=cols;
	// Check rows and columns
	if(nrows <= 0 || ncols <= 0) {
		errMsg(_T("Invalid size (%d,%d) matrix in %s"),nrows,ncols,filename);
		fclose(file);
		nrows=ncols=1;
		mm=matrix(1,1);
		err=ERR_NOTINIT;
		return;
	}
	// Allocate matrix
	mm=matrix(nrows,ncols);
	err=0;
	initialize(0);
	// Read elements
	fseek(file,0,0);     /* Beginning of file */
	switch(type) {
	case 'M':     // Mathematica file
	case 'm':
		level=done=zrows=zcols=0;

		while((c=getc(file)) != EOF) {
			if(done) break;
			switch(c) {
	case '{':
		if(++level == 1) {
		}
		else if(level == 2) {
			zrows++;
			zcols=0;
			nchars=0;
		}
		break;
	case '}':
		if(--level == 0) {
			done=1;
		}
		else if(level == 1) {
			zcols++;
			buf[nchars]='\0';
			if(zrows <= nrows && zcols <= ncols) {
				char *ptr;

				if(ptr=strstr(buf,"*10^")) {     // Convert Mathematica notation
					*ptr='e';
					strcpy(ptr+1,ptr+4);
				}
				mm[zrows][zcols]=atof(buf);
				//			printf("%c %d %d %s %g\n",c,zrows,zcols,buf,mm[zrows][zcols]);
			}
			nchars=0;
		}
		break;
	case ',':
		if(level == 2) {
			zcols++;
			buf[nchars]='\0';
			if(zrows <= nrows && zcols <= ncols) {
				char *ptr;

				if(ptr=strstr(buf,"*10^")) {     // Convert Mathematica notation
					*ptr='e';
					strcpy(ptr+1,ptr+4);
				}
				mm[zrows][zcols]=atof(buf);
				//			printf("%c %d %d %s %g\n",c,zrows,zcols,buf,mm[zrows][zcols]);
			}
			nchars=0;
		}
		break;
	default:
		if(level == 2) {
			if(nchars >= (NUMBUFLENGTH-1)) {
				errMsg(_T("Text number exceeded %d characters"),NUMBUFLENGTH);
				fclose(file);
				nrows=ncols=1;
				mm=matrix(1,1);
				err=ERR_NOTINIT;
				return;
			}
			buf[nchars++]=c;
		}
		break;
			}
		}
		break;
	case 'T':     // Text file
	case 't':
		level=zrows=zcols=0;

		while((c=getc(file)) != EOF) {
			switch(c) {
	case ' ':     // Whitespace
	case '\t':
		if(nchars <= 0) break;
	case ',':     // Delimiters other than whitespace
	case ';':
		if(comment) break;
		level=1;
		buf[nchars]='\0';
		if(zrows <= nrows && zcols <= ncols)
			mm[zrows][zcols]=atof(buf);
		nchars=0;
		break;
	case '\n':     // End of row or comment
		level=comment=0;
		if( nchars != 0) {
			buf[nchars]='\0';
			if(zrows <= nrows && zcols <= ncols)
				mm[zrows][zcols]=atof(buf);
			nchars=0;
		}
		zcols=0;
		break;
	case '#':
		comment=1;
		break;
	default:
		if(comment) break;
		if(level == 0) {
			zrows++;
			zcols++;
			nchars=0;
		}
		else if(level == 1) {
			zcols++;
			nchars=0;
		}
		level=2;
		if(nchars >= (NUMBUFLENGTH-1)) {
			errMsg(_T("Text number exceeded %d characters"),NUMBUFLENGTH);
			fclose(file);
			nrows=ncols=1;
			mm=matrix(1,1);
			err=ERR_NOTINIT;
			return;
		}
		buf[nchars++]=c;
		break;
			}
		}
		if( nchars != 0) {     // File did not end with \n
			buf[nchars]='\0';
			if(zrows <= nrows && zcols <= ncols)
				mm[zrows][zcols]=atof(buf);
			nchars=0;
		}
		break;
	}
	// Close file and return
	fclose(file);
	return;
}

/**************************** destructors ***************************/
/********************************************************************/

Matrix::~Matrix()
{
	freematrix(mm,nrows,ncols);
	nrows=ncols=0;
	err=ERR_DELETED;
}

/**************************** operators *****************************/
/********************************************************************/

/**************************** operator= *****************************/
Matrix &Matrix::operator=(const Matrix &other)
// Assignment operator
{
	int i,j;

	// Check for identity assignment
	if(&other == this) return *this;
	// Delete old matrix
	freematrix(mm,nrows,ncols);
	// Create a new one equal to the other
	nrows=other.nrows;
	ncols=other.ncols;
	err=other.err;
	mm=matrix(nrows,ncols);
	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) mm[i][j]=other.mm[i][j];
	}
	return *this;
}

/**************************** operator== ****************************/
int Matrix::operator==(const Matrix &other)
{
	int i,j;

	// Check for identity
	if(&other == this) return 1;
	// Check rows and cols
	if(nrows != other.nrows) return 0;
	if(ncols != other.ncols) return 0;
	if(err != other.err) return 0;
	// Check each element
	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++)
			if(mm[i][j] != other.mm[i][j]) return 0;
	}
	return 1;
}

/**************************** operator!= ****************************/
int Matrix::operator!=(const Matrix &other)
{
	return !(*this == other);
}

/**************************** operator+ *****************************/
Matrix Matrix::operator+(const Matrix &second)
{
	if(nrows != second.nrows || ncols !=second.ncols) {
		errMsg(_T("Cannot add matrices of different sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}
	Matrix sum(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			sum.mm[i][j]=mm[i][j]+second.mm[i][j];
		}
	}
	if(err || sum.err || second.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator+ *****************************/
Matrix Matrix::operator+(double second)
{
	Matrix sum(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			sum.mm[i][j]=mm[i][j]+second;
		}
	}
	if(err || sum.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator+*****************************/
Matrix operator+(double first, const Matrix &second)
// Not a member of the Matrix class
{
	Matrix sum(second.nrows,second.ncols);
	int i,j;

	for(i=1; i <= second.nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			sum.mm[i][j]=first+second.mm[i][j];
		}
	}
	if(sum.err || second.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator+= ****************************/
Matrix &Matrix::operator+=(const Matrix &second)
{
	if(nrows != second.nrows || ncols !=second.ncols) {
		errMsg(_T("Cannot add matrices of different sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]+=second.mm[i][j];
		}
	}
	if(err || second.err) err=ERR_CALC;
	return *this;
}

/**************************** operator+= ****************************/
Matrix &Matrix::operator+=(double second)
{
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]+=second;
		}
	}
	if(err) err=ERR_CALC;
	return *this;
}

/**************************** operator- *****************************/
Matrix Matrix::operator-(const Matrix &second)
{
	if(nrows != second.nrows || ncols !=second.ncols) {
		errMsg(_T("Cannot subtract matrices of different sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}
	Matrix sum(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			sum.mm[i][j]=mm[i][j]-second.mm[i][j];
		}
	}
	if(err || sum.err || second.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator- *****************************/
Matrix Matrix::operator-(double second)
{
	Matrix sum(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			sum.mm[i][j]=mm[i][j]-second;
		}
	}
	if(err || sum.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator- *****************************/
Matrix operator-(double first, const Matrix &second)
{
	Matrix sum(second.nrows,second.ncols);
	int i,j;

	for(i=1; i <= second.nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			sum.mm[i][j]=first-second.mm[i][j];
		}
	}
	if(sum.err || second.err) sum.err=ERR_CALC;
	return(sum);
}

/**************************** operator- *****************************/
Matrix Matrix::operator-()
// Unary minus
{
	Matrix neg(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			neg.mm[i][j]=-mm[i][j];
		}
	}
	if(err || neg.err) neg.err=ERR_CALC;
	return(neg);
}

/**************************** operator-= ****************************/
Matrix &Matrix::operator-=(const Matrix &second)
{
	if(nrows != second.nrows || ncols !=second.ncols) {
		errMsg(_T("Cannot subtract matrices of different sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]-=second.mm[i][j];
		}
	}
	if(err || second.err) err=ERR_CALC;
	return *this;
}

/**************************** operator-= ****************************/
Matrix &Matrix::operator-=(double second)
{
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]-=second;
		}
	}
	if(err) err=ERR_CALC;
	return *this;
}

/**************************** operator* *****************************/
Matrix Matrix::operator*(const Matrix &second)
{
	if(ncols != second.nrows) {
		errMsg(_T("Cannot multiply matrices of incompatible sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}

	Matrix prod(nrows,second.ncols);
	int i,j,k;
	double sum;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			sum=0.;
			for(k=1; k <= ncols; k++) {
				sum+=mm[i][k]*second.mm[k][j];
			}
			prod.mm[i][j]=sum;
		}
	}
	if(err || prod.err || second.err) prod.err=ERR_CALC;
	return(prod);
}

/**************************** operator* *****************************/
Matrix Matrix::operator*(double second)
{
	Matrix prod(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			prod.mm[i][j]=mm[i][j]*second;
		}
	}
	if(err || prod.err) prod.err=ERR_CALC;
	return(prod);
}

/**************************** operator* *****************************/
Matrix operator*(double first, const Matrix &second)
{
	Matrix prod(second.nrows,second.ncols);
	int i,j;

	for(i=1; i <= second.nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			prod.mm[i][j]=first*second.mm[i][j];
		}
	}
	if(prod.err || second.err) prod.err=ERR_CALC;
	return(prod);
}

/**************************** operator/ *****************************/
Matrix Matrix::operator/(double second)
{
	Matrix prod(nrows,ncols);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			prod.mm[i][j]=mm[i][j]/second;
		}
	}
	if(err || prod.err) prod.err=ERR_CALC;
	return(prod);
}

/**************************** operator/ *****************************/
Matrix operator/(double first, const Matrix &second)
{
	Matrix prod(second.nrows,second.ncols);
	int i,j;

	for(i=1; i <= second.nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			prod.mm[i][j]=first/second.mm[i][j];
		}
	}
	if(prod.err || second.err) prod.err=ERR_CALC;
	return(prod);
}

/**************************** operator*= ****************************/
Matrix &Matrix::operator*=(const Matrix &second)
{
	if(ncols != second.nrows) {
		errMsg(_T("Cannot multiply matrices of incompatible sizes: ")
			_T("(%d,%d), (%d,%d)"),
			nrows,ncols,second.nrows,second.ncols);
		err=ERR_CALC;
		return *this;
	}

	Matrix prod(nrows,second.ncols);
	int i,j,k;
	double sum;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= second.ncols; j++) {
			sum=0.;
			for(k=1; k <= ncols; k++) {
				sum+=mm[i][k]*second.mm[k][j];
			}
			prod.mm[i][j]=sum;
		}
	}
	if(err || prod.err || second.err) prod.err=ERR_CALC;
	return(*this=prod);
}

/**************************** operator*= ****************************/
Matrix &Matrix::operator*=(double second)
{
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]*=second;
		}
	}
	if(err) err=ERR_CALC;
	return *this;
}

/**************************** operator/= ****************************/
Matrix &Matrix::operator/=(double second)
{
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			mm[i][j]/=second;
		}
	}
	if(err) err=ERR_CALC;
	return *this;
}
/**************************** functions *****************************/
/********************************************************************/

/**************************** avg ***********************************/
double Matrix::avg()
{
	int i,j;
	double n=(double)nrows*(double)ncols;
	double sum=0.;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++)
			sum+=mm[i][j];
	}

	return sum/n;
}
/**************************** determinant ***************************/
double Matrix::determinant()
{
	if(ncols != nrows) {
		errMsg(_T("Can only find a determinant for a square matrix: ")
			_T("(%d,%d)"),nrows,ncols);
		return(0.);
	}
	Matrix am=*this;
	double det;
	int *indx=ivector(nrows);

	ludcmp(am.mm,nrows,indx,det);
	freeivector(indx,nrows);
	return(det);
}
/**************************** deletecols ****************************/
Matrix Matrix::deletecols(int istart, int iend)
{
	if(istart < 1) istart=1;
	if(iend > ncols) iend=ncols;
	int n=iend-istart+1;
	if(n <= 0) return *this;
	if(n == ncols) {
		errMsg(_T("Cannot delete all columns: (%d,%d)"),nrows,ncols);
		err=ERR_CALC;
		return *this;
	}

	Matrix am(nrows,ncols-n);
	int i,j,ja;

	for(ja=j=1; j <= ncols; j++) {
		if(j < istart || j > iend) {
			for(i=1; i <=nrows; i++)
				am.mm[i][ja]=mm[i][j];
			ja++;
		}
	}
	if(err || am.err) am.err=ERR_CALC;
	return am;
}
/**************************** deleterows *******************/
Matrix Matrix::deleterows(int istart, int iend)
{
	if(istart < 1) istart=1;
	if(iend > nrows) iend=nrows;
	int n=iend-istart+1;
	if(n <= 0) return *this;
	if(n == nrows) {
		errMsg(_T("Cannot delete all rows: (%d,%d)"),nrows,ncols);
		err=ERR_CALC;
		return *this;
	}

	Matrix am(nrows-n,ncols);
	int i,ia,j;

	for(ia=i=1; i <= nrows; i++) {
		if(i < istart || i > iend) {
			for(j=1; j <=ncols; j++)
				am.mm[ia][j]=mm[i][j];
			ia++;
		}
	}
	if(err || am.err) am.err=ERR_CALC;
	return am;
}
/**************************** diagonalmatrix ************************/
Matrix Matrix::diagonalmatrix()
{
	int n,i;

	// Use the larger of rows and columns
	if(nrows >= ncols) n=nrows;
	else n=ncols;
	Matrix am(n,n);

	if(nrows == 1) {
		for(i=1; i <= n; i++) {
			if(i <= ncols) am.mm[i][i]=mm[1][i];
			else am.mm[i][i]=0.;
		}
	}
	else if(ncols == 1) {
		for(i=1; i <= n; i++) {
			if(i <= nrows) am.mm[i][i]=mm[i][1];
			else am.mm[i][i]=0.;
		}
	}
	else {
		errMsg(_T("Can only make a diagonal matrix from a ")
			_T("matrix with a single row or column: ")
			_T("(%d,%d)"),nrows,ncols);
		am.err=ERR_CALC;
	}
	if(err || am.err) am.err=ERR_CALC;
	return am;
}
/**************************** getval ********************************/
double Matrix::getval(int row, int col)
// Safe version
{
	static double zero=0.;
	if(row < 1 || row > nrows) {
		errMsg(_T("Matrix::getval: row=%d is out of range (%d,%d)"),
			row,nrows,ncols);
		return(zero);
	}
	if(col < 1 || col > ncols) {
		errMsg(_T("Matrix::getval: col=%d is out of range (%d,%d)"),
			col,nrows,ncols);
		return(zero);
	}
	return(mm[row][col]);
}
/**************************** initialize ****************************/
void Matrix::initialize(double val)
{
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) mm[i][j]=val;
	}
}
/**************************** inverse *******************************/
Matrix Matrix::inverse()
{
	double det;

	Matrix ym=(*this).inverse(det);
	return(ym);
}
Matrix Matrix::inverse(double &det)
{
	Matrix ym=*this;

	if(ncols != nrows) {
		errMsg(_T("Can only find inverse for a square matrix: ")
			_T("(%d,%d)"),nrows,ncols);
		ym.err=ERR_CALC;
		return(ym);
	}

	Matrix am=*this;

	double *col=vector(nrows);
	int *indx=ivector(nrows);
	int i,j;

	ludcmp(am.mm,nrows,indx,det);
	if(det) {
		for(j=1; j <= nrows; j++) {
			for(i=1; i <= nrows; i++) col[i]=0.;
			col[j]=1.;
			lubksb(am.mm,nrows,indx,col);
			for(i=1; i <= nrows; i++) ym.mm[i][j]=col[i];
		}
	}
	else {
		ym.err=ERR_CALC;
	}
	freeivector(indx,nrows);
	freevector(col,nrows);
	if(err || ym.err) am.err=ERR_CALC;
	return(ym);
}
/**************************** max ***********************************/
double Matrix::maxval()
{
	int i,j;
	double limit=-DBL_MAX,temp;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			temp=mm[i][j];
			if(temp > limit) limit=temp;
		}
	}

	return limit;
}
/**************************** maxabs ********************************/
double Matrix::maxabs()
{
	int i,j;
	double limit=-DBL_MAX,temp;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			temp=fabs(mm[i][j]);
			if(temp > limit) limit=temp;
		}
	}

	return limit;
}
/**************************** min ***********************************/
double Matrix::minval()
{
	int i,j;
	double limit=DBL_MAX,temp;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			temp=mm[i][j];
			if(temp < limit) limit=temp;
		}
	}

	return limit;
}
/**************************** print *********************************/
void Matrix::print(char *msg, char *fmt,  char *filename)
{
	int i,j;
	char format[20];

	// Determine format
	if(!fmt) strcpy(format,PRINTFORMAT);
	else strncpy(format,fmt,20);
	// Determine destination
	if(filename[0]) {     // To file
		FILE*file;

		// Open file
		file=fopen(filename,"w");
		if(!file) {
			errMsg(_T("Cannot open %s"),filename);
			return;
		}
		// Write
		if(*msg) fprintf(file,"# %s:\n",msg);
		for(i=1; i <= nrows; i++) {
			for(j=1; j <= ncols; j++) {
				fprintf(file,format,mm[i][j]);
			}
			fprintf(file,"\n");
		}
		// Check errors and close file
		if(ferror(file)) errMsg(_T("Error writing %s"),filename);
		fclose(file);
	}
	else {     // To stdout
		// Write
		if(*msg) printf("%s:\n",msg);
		for(i=1; i <= nrows; i++) {
			for(j=1; j <= ncols; j++) {
				printf(format,mm[i][j]);
			}
			printf("\n");
		}
	}
}
/**************************** pseudoinverse *************************/
Matrix Matrix::pseudoinverse()
// PseudoInverse=(at*a)^(-1)*at
{
	Matrix pseudo=*this;
	Matrix trans=pseudo.transpose();
	double det;

	pseudo=(trans*pseudo).inverse(det)*trans;
	return(pseudo);
}
/**************************** rms ***********************************/
double Matrix::rms()
{
	int i,j;
	double n=(double)nrows*(double)ncols;
	double sum2=0.;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			sum2+=(mm[i][j]*mm[i][j]);
		}
	}

	return sqrt(sum2/n);
}
/**************************** setval ********************************/
void Matrix::setval(int row, int col, double val)
// Safe version
{
	static double zero=0.;
	if(row < 1 || row > nrows) {
		errMsg(_T("Matrix::setval: row=%d is out of range (%d,%d)"),
			row,nrows,ncols);
		return;
	}
	if(col < 1 || col > ncols) {
		errMsg(_T("Matrix::setval: col=%d is out of range (%d,%d)"),
			col,nrows,ncols);
		return;
	}
	mm[row][col]=val;
}
/**************************** singularvalues ************************/
void Matrix::singularvalues(Matrix &u, Matrix &w, Matrix &v)
{
	Matrix am;
	int i,j;

	if(nrows < ncols) {     // Fill rows to make it square
		freematrix(am.mm,am.nrows,am.ncols);
		am.nrows=am.ncols=ncols;
		am.mm=matrix(am.nrows,am.ncols);
		for(i=1; i <= am.nrows; i++) {
			if(i <= nrows) {
				for(j=1; j <= am.ncols; j++)
					am.mm[i][j]=mm[i][j];
			}
			else {
				for(j=1; j <= am.ncols; j++)
					am.mm[i][j]=0.;
			}
		}
	}
	else am=*this;

	Matrix um(am.nrows,ncols);
	Matrix wm(1,ncols);
	Matrix vm(ncols,ncols);


	svdcmp(am.mm,am.nrows,ncols,wm.mm[1],vm.mm);
	u=am.transpose();
	w=wm;
	v=vm.transpose();
	// Sort in descending order of singular values
	int n=w.ncols;
	int *indx=ivector(n);

	hsort(w.mm[1],indx,n);

	double *vtemp=vector(n);
	for(j=1; j<=n; j++) vtemp[j]=w.mm[1][j];
	for(j=1; j<=n; j++) w.mm[1][j]=vtemp[indx[n-j+1]];
	freevector(vtemp,n);

	double **mtemp=new double *[n];
	mtemp--;
	for(i=1; i <= n; i++) mtemp[i]=u.mm[i];
	for(i=1; i <= n; i++) u.mm[i]=mtemp[indx[n-i+1]];

	for(i=1; i <= n; i++) mtemp[i]=v.mm[i];
	for(i=1; i <= n; i++) v.mm[i]=mtemp[indx[n-i+1]];
	mtemp++;
	delete [] mtemp;

	freeivector(indx,n);
}
/**************************** statistics ****************************/
void Matrix::statistics(double &avg, double &maxabs, double &rms,
						double &stddev)
{
	int imax;

	(*this).statistics(avg,maxabs,rms,stddev,imax);
}
void Matrix::statistics(double &avg, double &maxabs, double &rms,
						double &stddev, int &imax)
{
	int i,j;
	double n=(double)nrows*(double)ncols;
	double limit=-DBL_MAX,temp;
	double sum=0.,sum2=0.;

	if(n < 1) {
	}

	imax=1;
	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			temp=mm[i][j];
			sum+=temp;
			sum2+=(temp*temp);
			temp=fabs(temp);
			if(temp > limit) {
				imax=i;
				limit=temp;
			}
		}
	}

	avg=sum/n;
	maxabs=limit;
	temp=sum2/n;
	rms=sqrt(temp);
	stddev=sqrt(temp-avg*avg);
}
/**************************** stddev ********************************/
double Matrix::stddev()
{
	int i,j;
	double n=(double)nrows*(double)ncols;
	double sum=0.,sum2=0.,temp;

	if(n < 1) {
	}

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) {
			temp=mm[i][j];
			sum+=temp;
			sum2+=(temp*temp);
		}
	}
	temp=sum/n;

	return sqrt(sum2/n-temp*temp);
}
/**************************** submatrix *****************************/
Matrix Matrix::submatrix(int rows, int cols, int rowstart, int colstart)
{
	Matrix am(*this,rows,cols,rowstart,colstart);
	return am;
}
/**************************** transpose *****************************/
Matrix Matrix::transpose()
{
	Matrix trans(ncols,nrows);
	int i,j;

	for(i=1; i <= nrows; i++) {
		for(j=1; j <= ncols; j++) trans.mm[j][i]=mm[i][j];
	}
	if(err || trans.err) trans.err=ERR_CALC;
	return(trans);
}
/**************************** write *********************************/
void Matrix::write(char *filename)
{
	FILE *file;
	int i,j;

	// Open file
	file=fopen(filename,"w");
	if(!file) {
		errMsg(_T("Cannot open %s"),filename);
		return;
	}
	// Write ID
	fwrite(MATRIXID,sizeof(MATRIXID),1,file);
	if(ferror(file)) {
		errMsg(_T("Cannot write %s"),filename);
		fclose(file);
		return;
	}
	// Write rows and columns
	fwrite((char *)&nrows,sizeof(nrows),1,file);
	fwrite((char *)&ncols,sizeof(ncols),1,file);
	if(ferror(file)) {
		errMsg(_T("Cannot write %s"),filename);
		fclose(file);
		return;
	}
	// Write elements
	for(i=1; i <=nrows; i++) {
		for(j=1; j <= ncols; j++) {
			fwrite((char *)&mm[i][j],sizeof(double),1,file);
		}
	}
	if(ferror(file)) errMsg(_T("Error writing %s"),filename);
	// Close file and return
	fclose(file);
	return;
}

/**************************** utilities *****************************/
/********************************************************************/
