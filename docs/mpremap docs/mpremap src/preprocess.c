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
#include "filter.h"
#include <math.h>
#include "string2.h"

#define LOOP "LOOP"
#define PAN  "PAN"
#define SKIP "SKIP"
#define SPLINE "SPLINE"
#define GRID "GRID"


int imageLine(Image* im, stBuf *sbuf, char* cmd);
double* schwenk(double x1, double x2, double tau, int n);
char* getArg(char* s, int n);
static int ReadImageDescription( Image *imPtr, stBuf *sPtr, char *line );
double* spline(double* x, double* y, int n, double vl, double vr);

// preprocess file in s, write to d
int preprocess(const char* s, const char* d){
#define LSIZE 256
	char line[LSIZE], iline[LSIZE];
	FILE *sf   = fopen(s, "r");
	FILE *df   = fopen(d, "w");
	
	if(sf==NULL || df==NULL)
		return -1;
	
	while( fgets(line, LSIZE, sf)!= NULL ){
		if( sstartswith(line, LOOP, FALSE) ){
			char **lines=NULL, *arg;
			int nlines=0;
			int c,i;
			arg = getArg(line, 0);
			if(arg==NULL) return -1;
			c = atoi( arg );
			free(arg);
			while( fgets(line, LSIZE, sf)!= NULL && !sstartswith(line, "}", FALSE) ){
				lines = (char**)realloc(lines,(++nlines)*sizeof(char*));
				lines[nlines-1] = strdup(line);
			}
			while(c-- >0){
				int i;
				for(i=0; i<nlines; i++){
					fputs(lines[i],df);
					strcpy(iline,lines[i]);
				}
			}
			for(i=0; i<nlines; i++)
				free(lines[i]);
			free(lines);
		}else if( sstartswith(line, PAN, FALSE) ){
			Image start, ziel;
			stBuf sbuf;
			char *c;
			int n, i;
			double tau =1.0, *y, *p, *r, *v;
			
			SetImageDefaults(&start);
			SetImageDefaults(&ziel);
			
			
			c = getArg(line, 0);
			if(c==NULL) return -1;
			n = atoi(c);
			free(c);
			
			c = getArg(line, 1);
			if(c!=NULL){
				tau=atof(c);
				free(c);
			}
			// Last iline is start
			if( ReadImageDescription( &start, &sbuf, iline ))
				return -1;
			c =  fgets(line, LSIZE, sf);
			if(c==NULL || ReadImageDescription( &ziel, &sbuf, line ))
					return -1;
			y = schwenk( start.yaw, ziel.yaw, tau, n);
			p = schwenk( start.pitch, ziel.pitch, tau, n);
			r = schwenk( start.roll, ziel.roll, tau, n);
			v = schwenk( start.hfov, ziel.hfov, tau, n);
			for(i=0; i<n; i++){
				ziel.yaw = y[i];
				ziel.pitch = p[i];
				ziel.roll = r[i];
				ziel.hfov = v[i];
				imageLine(&ziel, &sbuf, line);
				fputs(line, df);
				strcpy(iline, line);
			}
			free(y); free(p); free(r); free(v);
		}else if( sstartswith(line, SPLINE, FALSE) ){
			char *c;
			Image im;
			stBuf sbuf;
			int n=0,i;
			double *X, *Y, *y, *p, *r, *v;
			
			
			// Determine number of args
			while((c=getArg(line,n)) != NULL){
				free(c);
				n++;
			}
			n++;
			X = (double*)malloc(n*sizeof(double));
			X[0] = 0;
			for(i=1; i<n; i++){
				c = getArg(line,i-1);
				X[i] = X[i-1] + atof(c);
				free(c);
			}
			y=(double*)malloc(n*sizeof(double));
			p=(double*)malloc(n*sizeof(double));
			r=(double*)malloc(n*sizeof(double));
			v=(double*)malloc(n*sizeof(double));
			
			// Read lines
			// start at previous line
			for(i=0; i<n; i++){
				if(i==0)
					strcpy(line,iline);
				else
					fgets(line, LSIZE, sf);
			
				SetImageDefaults(&im);
				if( ReadImageDescription( &im, &sbuf, line ))
					return -1;
				y[i]=im.yaw;
				p[i]=im.pitch;
				r[i]=im.roll;
				v[i]=im.hfov;
			}
			Y = spline(X, y, n, 0., 0.); free(y); y=Y;
			Y = spline(X, p, n, 0., 0.); free(p); p=Y;
			Y = spline(X, r, n, 0., 0.); free(r); r=Y;
			Y = spline(X, v, n, 0., 0.); free(v); v=Y;
			
			n = (int)(X[n-1]-X[0]+1.);

			for(i=0; i<n; i++){
				im.yaw = y[i];
				im.pitch = p[i];
				im.roll = r[i];
				im.hfov = v[i];
				imageLine(&im, &sbuf, line);
				fputs(line, df);
				strcpy(iline, line);
			}

			free(y); free(p); free(r); free(v);free(X);
		}else if( sstartswith(line, SKIP, FALSE) ){
			Image dummy;
			stBuf sbuf;
			char *c;
			int n;
			
			c = getArg(line, 0);
			if(c==NULL) return -1;
			n = atoi(c);
			free(c);
			
			SetImageDefaults( &dummy );
			strcpy(dummy.name,"skip");
			SetStitchDefaults( &sbuf );
			imageLine(&dummy, &sbuf, line);
			while(n-- >0){
				fputs(line, df);
			}
		}else if( sstartswith(line, GRID, FALSE) ){
		// GRID syntax: pitch_down, pitch_up, n_vert yaw_left, yaw_right, n_hor
			int n_vert, n_hor,i,k;
			double pitch_down, pitch_up, yaw_left, yaw_right, dp, dy;
			char* c;
			Image im;
			stBuf sbuf;
			
			if( (c = getArg(line, 0)) ==NULL) return -1;
			pitch_down = atof(c);
			free(c);
			if( (c = getArg(line, 1)) ==NULL) return -1;
			pitch_up = atof(c);
			free(c);
			if( (c = getArg(line, 2)) ==NULL) return -1;
			n_vert = atoi(c);
			free(c);
			if( (c = getArg(line, 3)) ==NULL) return -1;
			yaw_left = atof(c);
			free(c);
			if( (c = getArg(line, 4)) ==NULL) return -1;
			yaw_right = atof(c);
			free(c);
			if( (c = getArg(line, 5)) ==NULL) return -1;
			n_hor = atoi(c);
			free(c);
			
			// Read and parse image line
			fgets(line, LSIZE, sf);
			
			SetImageDefaults(&im);
			if( ReadImageDescription( &im, &sbuf, line ))
				return -1;
			
			dy =  (yaw_right - yaw_left)  / (n_hor-1);
			dp =  (pitch_up - pitch_down) / (n_vert-1);
			for(i=0; i<n_vert; i++){
				im.pitch = i * dp + pitch_down;
				for(k=0; k<n_hor; k++){
					im.yaw = k*dy + yaw_left;
					if(i==0 && k==0)
						strcpy(im.name,"");
					else
						strcpy(im.name,"same");						
					imageLine(&im, &sbuf, line);
					fputs(line, df);
					strcpy(iline, line);					
				}
			}			
		}else{
			fputs(line,df);
			strcpy(iline,line);
		}
	}
	fclose(sf);
	fclose(df);
	return 0;
}


char* getArg(char* s, int n){
	char *c, *d, *rp, *r;
	// find '('
	c = s;
	while(*c!='(' && *c!=0)
		c++;
	if(*c==0) return NULL;
	c++;
	// find n ','
	while(n-- > 0){
		while(*c !=',' && *c!= 0)
			c++;
		if(*c==0) return NULL;
		c++;
	}
	// find ',' or ')'
	d=c;
	while(*d!=',' && *d!=')' && *d!=0)
		d++;
	if(*d==0) return NULL;
	rp = (char*)malloc(d-c+1);
	r = rp;
	while(c!=d)
		*r++=*c++;
	*r=0;
	return rp;
}


double* schwenk(double x1, double x2, double tau, int n){
	double tm = n / 2.0;
	double xm = (x2+x1)/2.0;
	double v, t;
	if(tau <= 0.)
		tau = 1e-5;
		
	v = (xm-x1)/(tm-tau*(1.0-exp(-tm/tau)));
	double* s=(double*)malloc(n*sizeof(double));
	s[n-1]=x2;
	for(t=1.0; t<=n/2.0; t+=1.0){
		double inc = v*t - v*tau*(1.0-exp(-t/tau));
		s[(int)t-1] = x1 + inc;
		s[n-1-(int)t] = x2 - inc;
	}
	return s;
}

int imageLine(Image* im, stBuf *sbuf, char* cmd){
	int format=0;
	char line[64];
	switch( im->format ){
			case _rectilinear 		: format = 0; break;
			case _panorama    		: format = 1; break;
			case _fisheye_circ		: format = 2; break;
			case _fisheye_ff  		: format = 3; break;
			case _equirectangular 		: format = 4; break;
			case _mirror 			: format = 7; break;
			default: format = im->format; break;
	}
	sprintf( cmd, "o f%d w%ld h%ld r%f p%f y%f v%f ", format, im->width, im->height, im->roll, im->pitch, im->yaw, im->hfov );
	if(im->cP.radial ){
		sprintf( line, "a%f b%f c%f ", im->cP.radial_params[0][3], im->cP.radial_params[0][2], im->cP.radial_params[0][1]);
		strcat( cmd, line );
	}
	if( im->cP.shear ){
		sprintf( line, "g%f t%f ", im->cP.shear_x, im->cP.shear_y);
		strcat( cmd, line );
	}

	if( im->cP.horizontal ){
			sprintf( line, "d%f ",im->cP.horizontal_params[0]); 
			strcat( cmd, line );
	}
	if( im->cP.vertical ){
			sprintf( line, "e%f ",im->cP.vertical_params[0]);
			strcat( cmd, line );
	}

	if( im->selection.bottom != 0 || im->selection.right != 0 ){
			if( im->cP.cutFrame ){
				sprintf( line, " C%ld,%ld,%ld,%ld ",im->selection.left, im->selection.right,
						       im->selection.top, im->selection.bottom );
			}else{
				sprintf( line, " S%ld,%ld,%ld,%ld ",im->selection.left, im->selection.right,
						       im->selection.top, im->selection.bottom );
			}
			strcat( cmd, line );
	}
	
	if( im->name != NULL && strlen(im->name)>0 ){
		sprintf( line, "n%s ", im->name );
		strcat( cmd, line );
	}
	

	strcat( cmd, "\n");
	return 0;
}




#define MY_SSCANF( str, format, ptr )		if( sscanf( str, format, ptr ) != 1 )   \
												return -1;							\

#define READ_VAR(format, ptr )		nextWord2( buf, &ch );			\
									MY_SSCANF( buf, format, ptr );

// Fill 'word' with word starting at (*ch). Advance *ch

void nextWord2( register char* word, char** ch )
{
	register char *c;
	c = *ch;
	
	c++;
	if( *c == '\"' )
	{
		c++;
		while( *c != '\"' && *c != 0 )
			*word++ = *c++;
	}
	else
	{
		while( !isspace(*c) && *c != 0 )
		{
			*word++ = *c++;
		}
	}
	*word = 0;
	*ch  = c;
}


// Parse a line describing a single image

static int ReadImageDescription( Image *imPtr, stBuf *sPtr, char *line )
{
	Image im;
	stBuf sBuf;
	char *ch = line;
	char buf[256];
	int	 i;

	
	memcpy( &im, 	imPtr,	 sizeof(Image) );
	memcpy( &sBuf, 	sPtr,	 sizeof(stBuf ));
	im.cP.radial = FALSE;
	
	while( *ch != 0)
	{
		switch(*ch)
		{
			case 'v':	READ_VAR( "%lf", &im.hfov );
						break;
			case 'a':	READ_VAR( "%lf", &(im.cP.radial_params[0][3]));
						im.cP.radial 	= TRUE;
						break;
			case 'b':	READ_VAR("%lf", &(im.cP.radial_params[0][2]));
						im.cP.radial 	= TRUE;
						break;
			case 'c':	READ_VAR("%lf", &(im.cP.radial_params[0][1]));
						im.cP.radial	= TRUE;
						break;
			case 'f':	READ_VAR( "%ld", &im.format );
						if( im.format == _panorama || im.format == _equirectangular )
							im.cP.correction_mode |= correction_mode_vertical;
						break;
			case 'y':	READ_VAR( "%lf", &im.yaw);
						break;
			case 'p':	READ_VAR( "%lf", &im.pitch);
						break;
			case 'r':	READ_VAR( "%lf", &im.roll);
						break;
			case 'd':	READ_VAR("%lf", &(im.cP.horizontal_params[0]));
						im.cP.horizontal 	= TRUE;
						break;
			case 'e':	READ_VAR("%lf", &(im.cP.vertical_params[0]));
						im.cP.vertical = TRUE;
						break;
			case 'g':	READ_VAR("%lf", &(im.cP.shear_x));
						im.cP.shear 	= TRUE;
						break;
			case 't':	READ_VAR("%lf", &(im.cP.shear_y));
						im.cP.shear = TRUE;
						break;
			case '+':	nextWord2( buf, &ch );
						sprintf( sBuf.srcName, "%s", buf);
						break;
			case '-':	nextWord2( buf, &ch );
						sprintf( sBuf.destName, "%s", buf );
						break;
			case 'u':	READ_VAR( "%d", &(sBuf.feather) );
						break;
			case 's':	READ_VAR( "%d", &sBuf.seam );
						{
							if(sBuf.seam != _dest)
								sBuf.seam = _middle;
						}
						break;
			case 'w':	READ_VAR( "%ld", &im.width );
						break;
			case 'h':	READ_VAR("%ld", &im.height);
						break;
			case 'o':	ch++;
						im.cP.correction_mode |= correction_mode_morph;
						break;
			case 'k':  // Colour correction
						READ_VAR( "%d", &i );
						// i &= 3;
						sBuf.colcorrect |= i&3;
						sBuf.colcorrect += (i+1)*4;
						break;	
			case 'm':  // Frame
						ch++;
						switch( *ch )
						{
							case 'x':
								READ_VAR( "%d", &im.cP.fwidth );
								im.cP.cutFrame = TRUE;
								break;
							case 'y':
								READ_VAR( "%d", &im.cP.fheight );
								im.cP.cutFrame = TRUE;
								break;
							default:
								ch--;
								READ_VAR( "%d", &(im.cP.frame) );
								im.cP.cutFrame = TRUE;											
								break;
						}
						break;
			case 'n':  // Name string (used for panorama format)
						nextWord2( buf, &ch );
						strcpy( im.name, buf );
						break;	
			case 'S':  nextWord2( buf, &ch );		
				   sscanf( buf, "%ld,%ld,%ld,%ld", &im.selection.left, &im.selection.right, &im.selection.top, &im.selection.bottom );
				   break;
			case 'C':  nextWord2( buf, &ch );		
				   sscanf( buf, "%ld,%ld,%ld,%ld", &im.selection.left, &im.selection.right, &im.selection.top, &im.selection.bottom );
				   im.cP.cutFrame = TRUE;
				   break;
			default: 	ch++;
						break;
		}
	}
	
	// Set 4th polynomial parameter
					
	im.cP.radial_params[0][0] = 1.0 - ( im.cP.radial_params[0][3] + im.cP.radial_params[0][2]
																  + im.cP.radial_params[0][1] ) ;
	
	SetEquColor( &im.cP );
	SetCorrectionRadius( &im.cP );	
	// Do checks
	
	// appears ok
	
	memcpy( imPtr,	&im, sizeof(Image) );
	memcpy( sPtr,	&sBuf, sizeof(stBuf ) );
	return 0;
}
