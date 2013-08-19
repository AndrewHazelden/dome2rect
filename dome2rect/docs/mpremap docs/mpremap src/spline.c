/* mpremap - remap motion pictures
   Copyright (C) 2007 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>

#undef DEBUG

#ifdef DEBUG
#endif

void pivot(double **a, double *c, int n, int k){
	int i,pivot;
	double maxa, mt;
	
	pivot = k;	
	maxa = fabs(a[k][k]);
	for(i=k+1; i<n; i++){
		mt = fabs(a[i][k]);
		if( mt>maxa ){
			maxa=mt;
			pivot=i;
		}
	}
	if(pivot != k){
		for(i=k; i<n; i++){
			mt = a[pivot][i];
			a[pivot][i]=a[k][i];
			a[k][i]=mt;
		}
		mt=c[pivot];
		c[pivot]=c[k];
		c[k]=mt;
	}
}
					

	
double* gauss(double **a, double *c, int n){
	double *x = (double*)malloc(n*sizeof(double));
	double f,s;
	int i,j,k;
	
	// Vorwärtseliminierung
	for(k=0; k<n-1; k++){
		pivot(a,c,n,k);
		for(i=k+1; i<n; i++){
			f = a[i][k]/a[k][k];
			for(j=k+1; j<n; j++){
				a[i][j] -= f*a[k][j];
			}
			c[i] -= f*c[k];
		}
	}
	
	// Rückwärtseinsetzen
	x[n-1] = c[n-1]/a[n-1][n-1];
	for(i=n-2; i>=0; i--){
		s=0.;
		for(j=i+1; j<n; j++)
			s += a[i][j]*x[j];
		x[i] = (c[i] - s)/ a[i][i];
	}
	
	return x;
}

double splineval(double *a,double *b, double *c, double* d, double* xp, int n, double x){
	int in=0,i;
	double u;
	for(i=0; i<n-1; i++)
		if(x>xp[i])
			in=i;
	u = x-xp[in];
	return a[in]+b[in]*u+c[in]*u*u+d[in]*u*u*u;
}


double* spline(double* x, double* y, int n, double vl, double vr){
	// Coefficients for Spline polynomials
	double *a,*b,*c,*d, *s;
	// Gleichungssystem
	double **A,*C,h1,h2,*yr;
	int i,k,np;
	
	a = (double*)malloc(n*sizeof(double));
	b = (double*)malloc(n*sizeof(double));
	c = (double*)malloc(n*sizeof(double));
	d = (double*)malloc(n*sizeof(double));

	A = (double**)malloc(n*sizeof(double*));
	for( i=0; i<n; i++){
		A[i] = (double*)malloc(n*sizeof(double));
		for(k=0;k<n;k++)
			A[i][k]=0.0;
	}
	
	C = (double*)malloc( n*sizeof(double));
	
	
	for( i=0; i<n; i++)
		a[i] = y[i];
		
	for( i=1; i<n-1; i++ ){
		h1 = x[i]-x[i-1]; 
		h2 = x[i+1]-x[i];
		A[i][i-1] = h1;
		A[i][i+1] = h2;
		A[i][i] = 2*(h1+h2);
		C[i] = 3*( (a[i+1]-a[i])/h2 - (a[i]-a[i-1])/h1 );
	}
	
	// Randbedingung
	// v links
	A[0][0] = -2.*(x[1]-x[0])/3.;
	A[0][1] = -(x[1]-x[0])/3.;
	C[0] = vl - (a[1]-a[0])/(x[1]-x[0]);

	// v rechts
	h1 = x[n-1]-x[n-2];
	A[n-1][n-2] = h1*h1/3.;
	A[n-1][n-1] = h1*h1*2./3.;
	C[n-1] = vr-a[n-1]+a[n-2];
	
	
	s = gauss( A, C, n);
	//for(i=0; i<n; i++)
		//fprintf(stderr,"s[%d]=%f\n",i,s[i]);
	
	for(i=0; i<n; i++){
		c[i] = s[i];
	}

	for(i=0; i<n-1; i++){ 
		b[i] = (a[i+1]-a[i])/(x[i+1]-x[i])-(2*c[i]+c[i+1])*(x[i+1]-x[i])/3.;
		d[i] = (c[i+1]-c[i])/(3*(x[i+1]-x[i]));
		//fprintf(stderr,"%f %f %f %f\n",a[i],b[i],c[i],d[i]);
	}
	
	
	//// Einsetzen von Werten
	//// Mit start und ziel
	
	np = (int)(x[n-1]-x[0]+1.);
	yr = (double*)malloc(np*sizeof(double));
	for(i=0; i<np; i++){
		yr[i] = splineval(a,b,c,d,x,n, i+x[0]);
		//fprintf(stderr,"yr[%d]=%f %f\n",i,yr[i],i+x[0]);
	}	
	free(a);
	free(b);
	free(c);
	free(d);
	for(i=0; i<n; i++)
		free(A[i]);
	free(A);
	free(C);
	return yr;
}
	
	
	
	
	
	
		
	
	
	

		
				
