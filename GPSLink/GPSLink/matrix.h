// matrix.h *** Header file for Matrix class

/* $Log: matrix.h,v $
 * Revision 1.3  1996/06/25  18:23:10  evans
 * Added optional imax argument to statistics() to return index of maxabs.
 *
 * Revision 1.2  1996/03/27  20:32:10  evans
 * First time.
 *
 * Revision 1.1.1.1  1996/03/22  12:38:26  evans
 * Imported files.
 * */

#ifndef _MATRIX_H_

#define _MATRIX_H_

#include "nr.h"

#define MATRIXID "#MatrixClassFile"

const double ERRVAL=1.111111111111111e+111;
const int ERR_NOTINIT=1;
const int ERR_BADVAL=2;
const int ERR_DELETED=3;
const int ERR_CALC=4;
const int ERR_SET=5;

// Function prototypes

void lubksb(double **a, int n, int *indx, double b[]);
void ludcmp(double **a, int n, int *indx, double *d);

class Matrix {
public:
    Matrix();
    Matrix(const Matrix &other);
    Matrix(const Matrix &other, int rows, int cols,
      int startrow=1, int startcol=1);
    Matrix(int rows, int cols, double value=0.);
    Matrix(int rows, int cols, char type);
    Matrix(int rows, int cols, char type, Matrix v);
    Matrix(char *filename);
    Matrix(char *filename, char type, int rows=0, int cols=0);
    virtual ~Matrix();
    
    Matrix &operator=(const Matrix &other);
    int operator==(const Matrix &other);
    int operator!=(const Matrix &other);
    Matrix operator+(const Matrix &second);
    Matrix operator+(double second);
  friend Matrix operator+(double first, const Matrix &second);
    Matrix &operator+=(const Matrix &second);
    Matrix &operator+=(double second);
    Matrix operator-(const Matrix &second);
    Matrix operator-(double second);
  friend Matrix operator-(double first, const Matrix &second);
    Matrix &operator-=(const Matrix &second);
    Matrix &operator-=(double second);
    Matrix operator-();
    Matrix operator*(const Matrix &second);
    Matrix operator*(double second);
  friend Matrix operator*(double first, const Matrix &second);
    Matrix operator/(double second);
  friend Matrix operator/(double first, const Matrix &second);
    Matrix &operator*=(const Matrix &second);
    Matrix &operator*=(double second);
    Matrix &operator/=(double second);
    
    int rows() const {return nrows;}
    int cols() const {return ncols;}
    double &mval(int row, int col) const {return mm[row][col];}
    
    double avg();
    void clearerror() {err=0;}
    Matrix deletecols(int istart, int iend);
    Matrix deleterows(int istart, int iend);
    double determinant();
    Matrix diagonalmatrix(); 
    double getval(int row, int col);
    void initialize(double val=0.);
    Matrix inverse();
    Matrix inverse(double &det);
    double maxval();
    double maxabs();
    double minval();
    void print(char *msg="", char *fmt=(char *)0, char *filename="");
    Matrix pseudoinverse();
    double rms();
    void seterror() {err=ERR_SET;}
    int geterror() const {return err;}
    void setval(int row, int col, double val);
    void singularvalues(Matrix &u, Matrix &w, Matrix &v);
    void statistics(double &avg, double &maxabs, double &rms, double &stddev,
      int &imax);
    void statistics(double &avg, double &maxabs, double &rms, double &stddev);
    double stddev();
    Matrix submatrix(int rows, int cols, int startrow=1, int startcol=1);
    Matrix transpose();
    void write(char *filename);
    
private:
    int err;
    int nrows;
    int ncols;
    double **mm;
};

#endif
