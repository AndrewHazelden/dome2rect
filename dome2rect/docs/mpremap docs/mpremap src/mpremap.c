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
#include "getopt.h"
#include "ppmstream.h"
#include "string2.h"

#include <fcntl.h>


#define MPREMAP_VERSION "0.2a"

void stitchImages( char** files, int nim, char *pano, char *script, int stitch);
void extractImages( char** files, int nim, char *frame, char *script);
void addAlphato( Image *im );
int preprocess(const char* s, const char* d);
void MakePano2( TrformStr *TrPtr, aPrefs *aP );
void ExtractStill2( TrformStr *TrPtr , aPrefs *aP );
void SetInvMakeParams2( struct fDesc *stack, struct MakeParams *mp, Image *im ,Image *pn, int color );
void SetMakeParams2( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color );



int _fisheye2 = 10;

#define PrintError(x) {fprintf(stderr,x);fprintf(stderr,"\n");fflush(stderr);}



void usage(){
	fprintf(stderr, "mpremap version %s\n", MPREMAP_VERSION );
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "mpremap -f scriptfile -o output  file1 file2 ...\n");
	fprintf(stderr, "To process piped PPM-stream:\n");
	fprintf(stderr, "mpremap -f scriptfile -o - -\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "-f Scriptfile\n");
	fprintf(stderr, "-o Output, - stdout\n");
	fprintf(stderr, "-s stitch\n");
	fprintf(stderr, "-e extract\n");
	fprintf(stderr, "-b num blur using num images (only extract mode)\n");
	fprintf(stderr, "file1,.. input, - stdin\n");
}


int quality=80;
int nblur=1;
int idx_in = 0;


int main( int argc, char **argv ){

	char** files=NULL;				// The list of files to stitch
	char*  pano=NULL;				// The output
	char*  script="script.txt";		// Scriptfile
	int    nim = 0,c;				// Number of images
	int	   stitch=FALSE;
	int	   extract=FALSE;

	while( (c = getopt (argc, argv, "eso:f:F:q:b:h")) != EOF ){
		switch (c){
			case 'o': 
				pano = optarg;
				break;
			case 'f':
				script=optarg;
				break;
			case 'F':
				script=tmpnam(NULL);
				sfileout( sreplace(optarg, "_:", " \n"), script );
				break;
			case 's':
				stitch=TRUE;
				break;
			case 'e':
				extract=TRUE;
				break;
			case 'b':
				nblur = atoi(optarg);
				break;
			case 'q':
				quality=atoi(optarg);
				if(quality<0 || quality>100)
					quality=80;
				break;
			case 'h': 
				usage();
				exit(0);
		}
	}

	while (optind < argc){
		files = (char**) realloc(files, (++nim)*sizeof(char*));
		files[nim-1]=argv[optind++];
	}			
			
	// Check Scriptfile
	if( pano==NULL || script==NULL || files==NULL ){
		usage();
		exit(0);
	}

#ifdef __Win__
	setmode(fileno(stdin), O_BINARY);
	setmode(fileno(stdout), O_BINARY);
#endif

	if(extract)
		extractImages(  files, nim, pano, script );
	else
		stitchImages( files, nim, pano, script, stitch );
	return 0;
}



// Save image to file
// name "-" ppm,stdout
// extension ".jpg" file jpeg
// default ppm
int saveImage(char* fname, Image* im){
	static int idx=0;
	
	if(strcmp(fname, "-")){
		char* c=(char*)malloc(strlen(fname)+10);
		sprintf(c,fname,idx++);
		if(sendswith(c,".jpg", TRUE)){
			writeJPEG(im,(fullPath *)c,quality,0);
		}else{
			FILE *out = fopen(c,"wb");
			if(out==NULL || putImageToPPMStream( out, im )){
				PrintError( "Could not save result");
				return -1;
			}
			fclose(out);
		}
		free(c);
	}else{
		if(putImageToPPMStream( stdout, im )){
			PrintError( "Could not save result");
			return -1;
		}
	}
	return 0;
}
	

Image *extractImage(aPrefs* a, Image* pano){
	TrformStr  	Tr;
	int			destheight, destwidth;
	
	Tr.dest = (Image*)malloc(sizeof(Image));
	SetImageDefaults( Tr.dest );		
				
	Tr.src		= pano;
	Tr.mode		= _honor_valid;	
	Tr.success 	= 1;


	destheight 			= a->im.height;
	destwidth 			= a->im.width;

	if( SetDestImage( &Tr, destwidth, destheight) != 0){
		// fprintf(stderr,"Could not allocate %ld bytes",Tr.dest->dataSize );
		return NULL;
	}
	CopyPosition( Tr.dest, &(a->im) );

	Tr.mode					|= _honor_valid;
	if( a->pano.hfov == 360.0 )
		Tr.mode				|= _wrapX;
			

	Tr.interpolator = a->interpolator;
	Tr.gamma	    = a->gamma;

	ExtractStill2( &Tr,  a );
	if(Tr.success)
		return Tr.dest;
	else{
		myfree((void**)Tr.dest->data);
		free(Tr.dest);
		return NULL;
	}
}

Image* cutrect(Image* im, PTRect* s){
	int y;
	Image *output = (Image*)malloc(sizeof(Image));
	memcpy(output, im, sizeof(Image));					
	output->width = s->right-s->left;
	output->height = s->bottom-s->top;
	output->bytesPerLine = output->width*output->bitsPerPixel/8;
	output->dataSize = output->bytesPerLine*output->height;
	output->data = (unsigned char**)mymalloc( output->dataSize );
	if(output->data == NULL)
		return NULL;

	for(y=0; y<output->height; y++){
		memcpy( (*output->data)+y*output->bytesPerLine,
				(*im->data)+(y+s->top)*im->bytesPerLine+s->left*im->bitsPerPixel/8,
				output->bytesPerLine);
	}
	return output;
}

inline int hasmoved( Image *a, Image *b ){
	return 	a->hfov != b->hfov || 
			a->yaw  != b->yaw  || 
			a->pitch != b->pitch || 
			a->roll != b->roll;
} 

aPrefs* interpolate_position(aPrefs *a, aPrefs* b, int n){
	aPrefs* ap = (aPrefs*)malloc(n*sizeof(aPrefs));
	int i;
	Image *im;
	for(i=0; i<n; i++){	
		memcpy(&ap[i], a, sizeof(aPrefs));
		im = &ap[i].im;
		im->yaw 	= a->im.yaw 	+ i*(b->im.yaw-a->im.yaw)	/(double)n;
		im->pitch 	= a->im.pitch 	+ i*(b->im.pitch-a->im.pitch)/(double)n;
		im->roll 	= a->im.roll 	+ i*(b->im.roll-a->im.roll)	/(double)n;
		im->hfov 	= a->im.hfov 	+ i*(b->im.hfov-a->im.hfov)	/(double)n;
	}
	return ap;
}

Image* imaverage(Image** dim, int n){
	Image *im = (Image*)malloc(sizeof(Image));
	int x,y,i,fs=dim[0]->bitsPerPixel==32 || dim[0]->bitsPerPixel==64;
	unsigned char** d=(unsigned char**)malloc(n*sizeof(unsigned char*)), *dst;
	memcpy(im, dim[0], sizeof(Image));
	im->data=(unsigned char**)mymalloc(im->dataSize);
	
	for(y=0; y<im->height; y++){
		for(i=0; i<n; i++){
			d[i] = (unsigned char*)*(dim[i]->data) + y*dim[i]->bytesPerLine;
		}
		dst = (unsigned char*)*im->data + y*im->bytesPerLine;;
		for(x=0; x<im->width; x++){
			double r=0,g=0,b=0;
			for(i=0; i<n; i++){
				if(fs)d[i]++;
				r += *(d[i]++);
				g += *(d[i]++);
				b += *(d[i]++);
			}
			r /= n; g /= n; b /= n;
			if(fs)dst++;
			DBL_TO_UC( *dst, r ); dst++;
			DBL_TO_UC( *dst, g ); dst++;
			DBL_TO_UC( *dst, b ); dst++;
		}
	}
	free(d);
	return im;
}


void extractImages( char** files, int nim, char *frame, char *script){
	int 		i=0, nr=0;
	Image		source,  *dest;
	aPrefs 		*aPtr = NULL, aPref_h;
	
			
	// make temporary copy of scriptfile
	char *tscript = tmpnam(NULL);
	// filecopy(script, tscript);
	preprocess(script, tscript);

	
	SetImageDefaults( &source );
	source.data = NULL;

	while(TRUE){
	
		if(aPtr==NULL) // Not null if continue case (see below)
			aPtr = readAdjustLine((fullPath*)tscript );
		if(aPtr == NULL){
			//filecopy(script, tscript);
			preprocess(script, tscript);
			aPtr = readAdjustLine( (fullPath*)tscript );
			if(aPtr == NULL){
				PrintError("Could not read Scriptfile");
				return;
			}
		}else{
			if(i==0)
				memcpy(&aPref_h, aPtr, sizeof(aPrefs));
		}
		i++;
				
		
		
		
		// Load new file unless image name == "same"
		if(strcmp(aPtr->im.name, "same") || i==1){ // Not same: load new image

			if(source.data != NULL){
				myfree((void**)source.data);
				source.data = NULL;
			}		


			if( strcmp(files[0], "-") ){ 	// Filenames given
				if( strchr( files[0], '%' ) != NULL){ // Numbered Images
					char* c=(char*)malloc(strlen(files[0])+10);
					sprintf(c,files[0],idx_in++);
					if( readImage( &source,  (fullPath*)c  ) ){ // No more images, return
						if(idx_in == 1){ // No images: error
							fprintf(stderr,"Could not read image %s",c);
						}
						free(c);
						return;
					}
					free(c);
				}else{					
					if( nr >= nim ) break; 		// No more input images
					if( readImage( &source,  (fullPath*)(files[nr++])  ) ){
						fprintf(stderr,"Could not read image %s",files[nr-1]);
						return;
					}
				}
			}else{ // read file from stdin
				int rh = getImageFromPPMStream( stdin, &source );
				if(rh<0){ // Error
					PrintError("Error reading from stream.");
					return;
				}else if(rh>0){ // No more Images in stream
					break;
				}
			}		
		}
		
		// Skip image if image name == "skip"
		if(strcmp(aPtr->im.name, "skip")==0){
			if(aPtr != NULL){
				free(aPtr);
				aPtr=NULL;
			}
			continue;
		} 

		if( aPtr->im.width == 0 ){
				aPtr->im.width = 320 ;
		}
		if(  aPtr->im.height == 0 ){
				aPtr->im.height = aPtr->im.width * 3 / 4;
		}
				
		// Set pano-params to src-image irrespective of prefs
		aPtr->pano.width	= source.width;					//	width of panorama
		aPtr->pano.height	= source.height;				//  height of panorama
			
		CopyPosition( &source, &(aPtr->pano) );
		addAlphato( &source ); 


		if( nblur > 1 && hasmoved( &aPtr->im, &aPref_h.im ) ){
			int k;
			aPrefs* aps = interpolate_position(aPtr, &aPref_h, nblur);
			Image **dim = (Image**)malloc(nblur*sizeof(Image*));
			for(k=0; k<nblur; k++){
				fprintf(stderr, "mpremap: converting image %d blur %d\n",i,k);
				fflush(stderr);
				dim[k] = extractImage(&aps[k],&source);
			}
			fprintf(stderr, "mpremap: blurring image %d\n",i);
			fflush(stderr);
			dest = imaverage(dim, nblur);
			for(k=0; k<nblur; k++){
				myfree((void**)dim[k]->data);
				free(dim[k]);
			}
			free(dim);
			free(aps);
		}else{		
			fprintf(stderr, "mpremap: converting image %d\n",i);
			fflush(stderr);
			dest = 	extractImage(aPtr,&source);
		}

		if(dest==NULL){
			PrintError("Error converting image.");
			return;
		}
			
				
		// Output result here
		// Crop Image if required
		if( aPtr->im.cP.cutFrame ){
			Image* out = cutrect(dest, &aPtr->im.selection);
			if(out==NULL){
				PrintError("Error cutting rectangle.");
				return;
			}
			myfree((void**)dest->data);
			free(dest);
			dest = out;
		}

		// Output Image
		if( saveImage(frame, dest) ){
			PrintError( "Error saving result");
			return;
		}
					
		myfree((void**)dest->data);
		free(dest);
		if(aPtr != NULL){	
			memcpy(&aPref_h, aPtr, sizeof(aPrefs));			
			free(aPtr);
			aPtr=NULL;
		}
	} // Tr.success 


	// Clean up; delete tempfiles
	mydelete((fullPath*)tscript);
				
}

Image * cutImage(Image* im, PTRect* s){
	int y;
	Image* output = (Image*)malloc(sizeof(Image));
	memcpy(output, im, sizeof(Image));					
	output->width = s->right-s->left;
	output->height = s->bottom-s->top;
	output->bytesPerLine = output->width*output->bitsPerPixel/8;
	output->dataSize = output->bytesPerLine*output->height;
	output->data = (unsigned char**)mymalloc( output->dataSize );
	if(output->data == NULL){
		PrintError("Not enough memory");
		exit(0);
	}
	for(y=0; y<output->height; y++){
		memcpy( (*output->data)+y*output->bytesPerLine,
				(*im->data)+(y+s->top)* im->bytesPerLine+s->left*im->bitsPerPixel/8,
						output->bytesPerLine);
	}
	return output;
}
		
Image *insertImage(aPrefs* a, Image* im){
	TrformStr  	Tr;
	
	Tr.dest = (Image*)malloc(sizeof(Image));
	SetImageDefaults( Tr.dest );		
				
	Tr.src		= im;
	Tr.mode		= _honor_valid;	
	Tr.success 	= 1;

	Tr.dest->height = a->pano.height;
	Tr.dest->width 	= a->pano.width;

    Tr.dest->bitsPerPixel = im->bitsPerPixel;
    Tr.dest->bytesPerLine = Tr.dest->width* Tr.dest->bitsPerPixel/8;
    Tr.dest->dataSize =     Tr.dest->bytesPerLine *     Tr.dest->height;
    Tr.dest->data = (unsigned char**)mymalloc(     Tr.dest->dataSize );
	if(    Tr.dest->data == NULL){
		PrintError("Not enough memory");
		exit(0);
	}

    Tr.dest->selection.left = 0;
    Tr.dest->selection.right = Tr.dest->width;
	Tr.dest->selection.top = 0;
	Tr.dest->selection.bottom = Tr.dest->height;
	CopyPosition( Tr.dest, &(a->pano) );
    a->pano.width = Tr.dest->width;
    a->pano.height = Tr.dest->height;

	Tr.mode					|= _honor_valid;
	if( a->pano.hfov == 360.0 )
		Tr.mode				|= _wrapX;
			

	Tr.interpolator = a->interpolator;
	Tr.gamma	    = a->gamma;


	MakePano2( &Tr,  a );

	if(Tr.success)
		return Tr.dest;
	else{
		myfree((void**)Tr.dest->data);
		free(Tr.dest);
		return NULL;
	}
}
	
void stitchImages( char** files, int nim, char *frame, char *script, int stitch){
	int 		i=0, nr=0;
	Image		source,  *dest=NULL;
	aPrefs 		*aPtr = NULL, aPref_h;

	int			colcorrect = 0, brcorrect = 0, cabcorrect = 0; // Color correction requested?

				
	// make temporary copy of scriptfile
	char *tscript = tmpnam(NULL);
	preprocess(script, tscript);
	
	SetImageDefaults( &source );
	source.data = NULL;

	while(TRUE){
		if(aPtr==NULL) // Not null if continue case (see below)
			aPtr = readAdjustLine((fullPath*)tscript );
		if(aPtr == NULL){
			preprocess(script, tscript);
			aPtr = readAdjustLine( (fullPath*)tscript );
			if(aPtr == NULL){
				PrintError("Could not read Scriptfile");
				return;
			}
		}else{
			if(i==0)
				memcpy(&aPref_h, aPtr, sizeof(aPrefs));
		}
		i++;

		// Load new file unless image name == "same"
		if(strcmp(aPtr->im.name, "same") || i==1){ // Not same: load new image

			if(source.data != NULL){
				myfree((void**)source.data);
				source.data = NULL;
			}		


			if( strcmp(files[0], "-") ){ 	// Filenames given
				if( strchr( files[0], '%' ) != NULL){ // Numbered Images
					char* c=(char*)malloc(strlen(files[0])+10);
					sprintf(c,files[0],idx_in++);
					if( readImage( &source,  (fullPath*)c  ) ){ // No more images, return
						if(idx_in == 1){ // No images: error
							fprintf(stderr,"Could not read image %s",c);
						}
						free(c);
						return;
					}
					free(c);
				}else{
					if( nr >= nim ) break; 		// No more input images
					if( readImage( &source,  (fullPath*)(files[nr++])  ) ){
						fprintf(stderr,"Could not read image %s",files[nr-1]);
						return;
					}
				}
			}else{ // read file from stdin
				int rh = getImageFromPPMStream( stdin, &source );
				if(rh<0){ // Error
					PrintError("Error reading from stream.");
					return;
				}else if(rh>0){ // No more Images in stream
					break;
				}
			}		
		}

		
		// Skip image if image name == "skip"
		if(strcmp(aPtr->im.name, "skip")==0){
			if(aPtr != NULL){
				free(aPtr);
				aPtr=NULL;
			}
			continue;
		} 

		

		colcorrect = aPtr->sBuf.colcorrect;
		if( aPtr->pano.cP.radial )
			brcorrect  = (int)aPtr->pano.cP.radial_params[0][2] + 1;
		if( aPtr->pano.cP.horizontal )
			cabcorrect = (int)aPtr->pano.cP.horizontal_params[0] + 1;


		
		// Crop Image if required
		if( aPtr->im.cP.cutFrame ){
			if( CropImage( &source, &aPtr->im.selection ) == 0 ){ // Cropping 
				aPtr->im.selection.top = aPtr->im.selection.bottom =
				aPtr->im.selection.right = aPtr->im.selection.left = 0;
				aPtr->im.cP.cutFrame = FALSE;
			}
		}	

		aPtr->im.width	= source.width;
		aPtr->im.height	= source.height;

		if( aPtr->pano.width == 0 && aPtr->im.hfov != 0.0){
			aPtr->pano.width = aPtr->im.width * aPtr->pano.hfov / aPtr->im.hfov;
			aPtr->pano.width/=10; aPtr->pano.width*=10;
		}
		if( aPtr->pano.height == 0 )
			aPtr->pano.height = aPtr->pano.width/2;

		CopyPosition( &source,  &(aPtr->im) );
		memcpy( &source.selection, &aPtr->im.selection, sizeof(PTRect) );

		addAlphato( &source ); 

		if(dest!=NULL){
			myfree((void**)dest->data);
			free(dest);
		}

		fprintf(stderr, "mpremap: converting image %d\n",i); 
		fflush(stderr);
		dest = insertImage(aPtr, &source);
		
		
		if( dest == NULL ) {
			PrintError("Error converting image");
			return;
		}
		
		
		// Stitch images; Proceed only if panoramic image valid

		if( *(aPtr->sBuf.srcName) != 0 ){ // We have to merge in one images
		// Load the bufferimage
			if( LoadBufImage( &aPtr->pano, aPtr->sBuf.srcName, 1 ) != 0 ){
				 goto _insert_exit;
			}

			if( HaveEqualSize( &aPtr->pano, dest )){
				// At this point we have two valid, equally sized images						
				// Do Colour Correction on one or both  images
				DoColorCorrection( dest, &aPtr->pano, aPtr->sBuf.colcorrect & 3);
				if( merge( dest , &aPtr->pano, aPtr->sBuf.feather, 0, aPtr->sBuf.seam ) != 0 ){
					PrintError( "Error merging images. Keeping Source" );
				}
			}				
			myfree( (void**)aPtr->pano.data );
		} // src != 0
					
		if( *(aPtr->sBuf.destName) != 0 ){ // save buffer image
			if( SaveBufImage( dest, aPtr->sBuf.destName )  )
				PrintError( "Could not save to Buffer. Most likely your disk is full");
		}
		
		if(!stitch){ // Output result here
			// Crop Image if required
			if( aPtr->pano.cP.cutFrame ){
				Image* output = cutImage(dest, &aPtr->pano.selection);
				myfree((void**)dest->data);
				free(dest);
				dest = output;
			}
			
			// Output Image
			if( saveImage(frame, dest) ){
				PrintError( "Error saving result");
				return;
			}
			
		}
		if(aPtr != NULL){
			free(aPtr);
			aPtr=NULL;
		}
		
	} // Tr.success 

	if(stitch){ // Output result here
		if( saveImage(frame, dest) ){
			PrintError( "Error saving result");
			return;
		}
	}

	
_insert_exit:
	// Clean up; delete tempfiles
	mydelete((fullPath*)"pano12.buf");
	mydelete((fullPath*)tscript);
				
}






// Add an alpha channel to the image, assuming rectangular or circular shape
// subtract frame 

void addAlphato( Image *im ){
	int 		x,y,xc,yc,rc,rcs,BPP,delx,dely;
	UCHAR		*dx, *dy;
	PTRect		s;

	memcpy(&s, &im->selection, sizeof(PTRect));
	if( s.bottom == 0 ) s.bottom = im->height;
	if( s.right == 0 ) s.right = im->width;
	
	if( im->bitsPerPixel == 32 ){
#define PIXEL_TYPE UCHAR
		BPP = 4;
		if( im->format != _fisheye_circ ){ // Rectangle valid
			for(y=0,dy=*im->data; y<s.top; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=s.bottom,dy=*im->data+s.bottom*im->bytesPerLine; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<s.left; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=s.right,dx=dy+s.right*BPP; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;				
		}else{ //Circle inside selection
			xc = (s.right+s.left)/2;
			yc = (s.bottom+s.top)/2;
			rc = (s.left-s.right)/2;
			rcs = rc*rc;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP){
					dely = y - yc; delx = x - xc;
					if( (dely*dely + delx*delx) > rcs )
						*(PIXEL_TYPE*)dx = 0;
				}
		}
	}else if( im->bitsPerPixel == 64 ){
#undef PIXEL_TYPE
#define PIXEL_TYPE USHORT
		BPP = 8;
		if( im->format != _fisheye_circ ){ // Rectangle valid
			for(y=0,dy=*im->data; y<s.top; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=s.bottom,dy=*im->data+s.bottom*im->bytesPerLine; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<s.left; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=s.right,dx=dy+s.right*BPP; x<im->width; x++,dx+=BPP)
					*(PIXEL_TYPE*)dx = 0;				
		}else{ //Circle inside selection
			xc = (s.right+s.left)/2;
			yc = (s.bottom+s.top)/2;
			rc = (s.left-s.right)/2;
			rcs = rc*rc;
			for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
				for(x=0,dx=dy; x<im->width; x++,dx+=BPP){
					dely = y - yc; delx = x - xc;
					if( (dely*dely + delx*delx) > rcs )
						*(PIXEL_TYPE*)dx = 0;
				}
		}
	}
}



//////////////////////// Some modifications to adjust.c in pano12 ////////////////////////////////

static int		CheckMakeParams( aPrefs *aP)
{
	
	if( (aP->pano.format == _rectilinear) && (aP->pano.hfov >= 180.0) )
	{
		PrintError("Rectilinear Panorama can not have 180 or more degrees field of view.");
		return -1;
	}
	if( (aP->im.format == _rectilinear) && (aP->im.hfov >= 180.0) )
	{
		PrintError("Rectilinear Image can not have 180 or more degrees field of view.");
		return -1;
	}
	if( (aP->mode & 7) == _insert ){
		if( (aP->im.format == _fisheye_circ || 	aP->im.format == _fisheye_ff) &&
		    (aP->im.hfov > MAX_FISHEYE_FOV) ){
				fprintf(stderr,"Fisheye lens processing limited to fov <= %lg\n", MAX_FISHEYE_FOV);
				return -1;
		}
	}

	return 0;
	
}

void sphere_fisheye2( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double phi, theta, rho;

	rho = sqrt( x_dest*x_dest + y_dest*y_dest );
	
	theta = 2.0 * asin( rho/(2.0*((double*)params)[0]) );
	phi   = atan2( y_dest , x_dest );

	*x_src = ((double*)params)[0] * theta * cos( phi );
	*y_src = ((double*)params)[0] * theta * sin( phi );
}



void fisheye2_sphere( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double rho, phi, theta;

	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / ((double*)params)[0];
	phi   = atan2( y_dest , x_dest );
	
	rho = 2.0 * ((double*)params)[0] * sin( theta / 2.0 );
	
	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
}


// Set inverse Makeparameters depending on adjustprefs, color and source image

void 	SetInvMakeParams2( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color )
{

	int 		i;
	double		a,b;							// field of view in rad


	a =	 DEG_TO_RAD( im->hfov );	// field of view in rad		
	b =	 DEG_TO_RAD( pn->hfov );


	SetMatrix( 	DEG_TO_RAD( im->pitch ), 
				0.0, 
				DEG_TO_RAD( im->roll ), 
				mp->mt, 
				1 );


	if(pn->format == _rectilinear)									// rectilinear panorama
	{
		mp->distance 	= (double) pn->width / (2.0 * tan(b/2.0));
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(2.0 * tan(a/2.0))) * ((double)im->width/(double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 

		}else if(im->format == _mirror|| im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(4.0 * sin(a/4.0))) * ((double)im->width/(double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 
		}else{ 																//  pamoramic or fisheye image
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width/ (double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 
		}
	}
	else if(pn->format == _fisheye2){
		mp->distance 	= (double) pn->width / (4.0 * sin(b/4.0));
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(2.0 * tan(a/2.0))) * ((double)im->width/(double) pn->width)
						   * 4.0 * sin(b/4.0) / b;

		}else if(im->format == _mirror|| im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(4.0 * sin(a/4.0))) * ((double)im->width/(double) pn->width)
						   * 4.0 * sin(b/4.0) / b;
		}else{ 																//  pamoramic or fisheye image
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width/ (double) pn->width)
						   * 4.0 * sin(b/4.0) / b; 
		}
	}
	else																// equirectangular or panoramic 
	{
		mp->distance 	= ((double) pn->width) / b;
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * (a /(2.0 * tan(a/2.0))) * ((double)im->width)/ ((double) pn->width); 

		}else if(im->format == _mirror|| im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * (a /(4.0 * sin(a/4.0))) * ((double)im->width)/ ((double) pn->width); 
		}
		else 																//  pamoramic or fisheye image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width)/ ((double) pn->width); 
		}
	}
	mp->shear[0] 	= -im->cP.shear_x / im->height;
	mp->shear[1] 	= -im->cP.shear_y / im->width;
	
	mp->scale[0] = 1.0 / mp->scale[0];
	mp->scale[1] 	= mp->scale[0];
	mp->horizontal 	= -im->cP.horizontal_params[color];
	mp->vertical 	= -im->cP.vertical_params[color];
	for(i=0; i<4; i++)
		mp->rad[i] 	= im->cP.radial_params[color][i];
	mp->rad[5] = im->cP.radial_params[color][4];
	
	switch( im->cP.correction_mode & 3 )
	{
		case correction_mode_radial: mp->rad[4] = ((double)(im->width < im->height ? im->width : im->height) ) / 2.0;break;
		case correction_mode_vertical: 
		case correction_mode_deregister: mp->rad[4] = ((double) im->height) / 2.0;break;
	}

	mp->rot[0]		= mp->distance * PI;								// 180¡ in screenpoints
	mp->rot[1]		= im->yaw *  mp->distance * PI / 180.0; 			//    rotation angle in screenpoints

	mp->perspect[0] = (void*)(mp->mt);
	mp->perspect[1] = (void*)&(mp->distance);




	i = 0;	// Stack counter
		
		// Perform radial correction
	if( im->cP.shear )
	{
		SetDesc( stack[i],shear,			mp->shear		); i++;
	}
		
	if ( im->cP.horizontal )
	{
		SetDesc(stack[i],horiz,				&(mp->horizontal)); i++;
	}
	if (  im->cP.vertical)
	{
		SetDesc(stack[i],vert,				&(mp->vertical)); 	i++;
	}
	if(   im->cP.radial )
	{
		switch( im->cP.correction_mode & 3)
		{
			case correction_mode_radial:   SetDesc(stack[i],inv_radial,mp->rad); 	i++; break;
			case correction_mode_vertical: SetDesc(stack[i],inv_vertical,mp->rad); 	i++; break;
			case correction_mode_deregister: break;
		}
	}
	
	SetDesc(	stack[i],	resize,				mp->scale		); i++; // Scale image
	
	if(im->format 		== _rectilinear)									// rectilinear image
	{
		SetDesc(stack[i],	sphere_tp_rect,		&(mp->distance)	); i++;	// 
	}
	else if	(im->format 	== _panorama)									//  pamoramic image
	{
		SetDesc(stack[i],	sphere_tp_pano,		&(mp->distance)	); i++;	// Convert panoramic to spherical
	}
	else if	(im->format 	== _equirectangular)							//  PSphere image
	{
		SetDesc(stack[i],	sphere_tp_erect,	&(mp->distance)	); i++;	// Convert Psphere to spherical
	}
	else if	(im->format 	== _mirror)							//  PSphere image
	{
		SetDesc(stack[i],	sphere_mirror,	&(mp->distance)	); i++;	// Convert mirror to spherical
	}
	else if	(im->format 	== _fisheye2)							//  PSphere image
	{
		SetDesc(stack[i],	sphere_fisheye2,	&(mp->distance)	); i++;	// Convert mirror to spherical
	}


	SetDesc(	stack[i],	persp_sphere,		mp->perspect	); i++;	// Perspective Control spherical Image
	SetDesc(	stack[i],	erect_sphere_tp,	&(mp->distance)	); i++;	// Convert spherical image to equirect.
	SetDesc(	stack[i],	rotate_erect,		mp->rot			); i++;	// Rotate equirect. image horizontally

	if(pn->format == _rectilinear)									// rectilinear panorama
	{
		SetDesc(stack[i],	rect_erect,		&(mp->distance)	); i++;	// Convert rectilinear to spherical
	}
	else if(pn->format == _panorama)
	{
		SetDesc(stack[i],	pano_erect,		&(mp->distance)	); i++;	// Convert rectilinear to spherical
	}
	else if(pn->format == _fisheye_circ || pn->format == _fisheye_ff )
	{
		SetDesc(stack[i],	sphere_tp_erect,		&(mp->distance)	); i++;	// Convert rectilinear to spherical
	}
	else if(pn->format == _fisheye2 )
	{
		SetDesc(stack[i],	sphere_tp_erect,		&(mp->distance)	); i++;	// Convert rectilinear to spherical
		SetDesc(stack[i],	fisheye2_sphere,		&(mp->distance)	); i++;	// Convert mirror to spherical
	}
	
	stack[i].func = (trfn)NULL;
}




// Extract image from pano in TrPtr->src 
// using parameters in prefs (ignore image parameters
// in TrPtr)

void ExtractStill2( TrformStr *TrPtr , aPrefs *aP )
{
	struct 	MakeParams	mp;
	fDesc 	stack[15], fD;		// Parameters for execute 

	int 	k, kstart, kend, color;

	TrPtr->success = 1;

	if( CheckMakeParams( aP) != 0)
	{
		TrPtr->success = 0;
		return;
	}

		

	if( isColorSpecific( &(aP->im.cP) ) )			// Color dependent
	{
		kstart 	= 1; kend	= 4;
	}
	else 															// Color independent
	{
		kstart	= 0; kend	= 1;
	}
				
	for( k = kstart; k < kend; k++ )
	{
		color = k-1; if( color < 0 ) color = 0;
		SetInvMakeParams2( stack, &mp,  &(aP->im) , &(aP->pano), color );
		
		if( TrPtr->success != 0)
		{
			fD.func = execute_stack; fD.param = stack;
			transForm( TrPtr,  &fD , k);
		}
	}
}


// Set Makeparameters depending on adjustprefs, color and source image

void SetMakeParams2( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color )
{
	int 		i;
	double		a,b;						// field of view in rad


	a	=	 DEG_TO_RAD( im->hfov );	// field of view in rad		
	b	=	 DEG_TO_RAD( pn->hfov );

	SetMatrix(  	- DEG_TO_RAD( im->pitch ), 
					0.0, 
					- DEG_TO_RAD( im->roll ), 
					mp->mt, 
					0 );


	if(pn->format == _rectilinear)									// rectilinear panorama
	{
		mp->distance 	= (double) pn->width / (2.0 * tan(b/2.0));
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(2.0 * tan(a/2.0))) * ((double)im->width/(double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 

		}else if(im->format == _mirror || im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(4.0 * sin(a/4.0))) * ((double)im->width/(double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 
		}else{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width/ (double) pn->width)
						   * 2.0 * tan(b/2.0) / b; 
		}
	}
	else if(pn->format == _fisheye2){
		mp->distance 	= (double) pn->width / (4.0 * sin(b/4.0));
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(2.0 * tan(a/2.0))) * ((double)im->width/(double) pn->width)
						   * 4.0 * sin(b/4.0) / b; 

		}else if(im->format == _mirror || im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * 
						   (a /(4.0 * sin(a/4.0))) * ((double)im->width/(double) pn->width)
						   * 4.0 * sin(b/4.0) / b; 
		}else{ 																//  pamoramic or fisheye image
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width/ (double) pn->width)
						   * 4.0 * sin(b/4.0) / b; 
		}
	}
	else																// equirectangular or panoramic or fisheye
	{
		mp->distance 	= ((double) pn->width) / b;
		if(im->format == _rectilinear)										// rectilinear image
		{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * (a /(2.0 * tan(a/2.0))) * ((double)im->width)/ ((double) pn->width); 

		}else if(im->format == _mirror || im->format == _fisheye2){
			mp->scale[0] = ((double)pn->hfov / im->hfov) * (a /(4.0 * sin(a/4.0))) * ((double)im->width)/ ((double) pn->width); 
		}else{
			mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)im->width)/ ((double) pn->width); 
		}
	}
	mp->scale[1] 	= mp->scale[0];


	mp->shear[0] 	= im->cP.shear_x / im->height;
	mp->shear[1] 	= im->cP.shear_y / im->width;
	mp->rot[0]		= mp->distance * PI;								// 180¡ in screenpoints
	mp->rot[1]		= -im->yaw *  mp->distance * PI / 180.0; 			//    rotation angle in screenpoints
	mp->perspect[0] = (void*)(mp->mt);
	mp->perspect[1] = (void*)&(mp->distance);
			
	for(i=0; i<4; i++)
		mp->rad[i] 	= im->cP.radial_params[color][i];
	mp->rad[5] = im->cP.radial_params[color][4];

	if( (im->cP.correction_mode & 3) == correction_mode_radial )
		mp->rad[4] 	= ( (double)( im->width < im->height ? im->width : im->height) ) / 2.0;
	else
		mp->rad[4] 	= ((double) im->height) / 2.0;
		

	mp->horizontal 	= im->cP.horizontal_params[color];
	mp->vertical 	= im->cP.vertical_params[color];
	
	i = 0;

	if(pn->format == _rectilinear)									// rectilinear panorama
	{
		SetDesc(stack[i],	erect_rect,		&(mp->distance)	); i++;	// Convert rectilinear to equirect
	}
	else if(pn->format == _panorama)
	{
		SetDesc(stack[i],	erect_pano,		&(mp->distance)	); i++;	// Convert panoramic to equirect
	}
	else if(pn->format == _fisheye_circ || pn->format == _fisheye_ff)
	{
		SetDesc(stack[i],	erect_sphere_tp,		&(mp->distance)	); i++;	// Convert panoramic to sphere
	}
	else if(pn->format == _fisheye2 )
	{
		SetDesc(stack[i],	sphere_fisheye2,		&(mp->distance)	); i++;	// Convert mirror to spherical
		SetDesc(stack[i],	erect_sphere_tp,		&(mp->distance)	); i++;	// Convert panoramic to sphere
	}

	SetDesc(	stack[i],	rotate_erect,		mp->rot			); i++;	// Rotate equirect. image horizontally
	SetDesc(	stack[i],	sphere_tp_erect,	&(mp->distance)	); i++;	// Convert spherical image to equirect.
	SetDesc(	stack[i],	persp_sphere,		mp->perspect	); i++;	// Perspective Control spherical Image

	if(im->format 		== _rectilinear)									// rectilinear image
	{
		SetDesc(stack[i],	rect_sphere_tp,		&(mp->distance)	); i++;	// Convert rectilinear to spherical
	}
	else if	(im->format 	== _panorama)									//  pamoramic image
	{
		SetDesc(stack[i],	pano_sphere_tp,		&(mp->distance)	); i++;	// Convert panoramic to spherical
	}
	else if	(im->format 	== _equirectangular)							//  PSphere image
	{
		SetDesc(stack[i],	erect_sphere_tp,	&(mp->distance)	); i++;	// Convert PSphere to spherical
	}
	else if(im->format == _mirror){
		SetDesc(stack[i],	mirror_sphere,	&(mp->distance)	); i++;	// Convert mirror to spherical
	}
	else if(im->format == _fisheye2){
		SetDesc(stack[i],	fisheye2_sphere,	&(mp->distance)	); i++;	// Convert mirror to spherical
	}

	SetDesc(	stack[i],	resize,				mp->scale		); i++; // Scale image
	
	if( im->cP.radial )
	{
		switch( im->cP.correction_mode & 3 )
		{
			case correction_mode_radial:    SetDesc(stack[i],radial,mp->rad); 	  i++; break;
			case correction_mode_vertical:  SetDesc(stack[i],vertical,mp->rad);   i++; break;
			case correction_mode_deregister:SetDesc(stack[i],deregister,mp->rad); i++; break;
		}
	}
	if (  im->cP.vertical)
	{
		SetDesc(stack[i],vert,				&(mp->vertical)); 	i++;
	}
	if ( im->cP.horizontal )
	{
		SetDesc(stack[i],horiz,				&(mp->horizontal)); i++;
	}
	if( im->cP.shear )
	{
		SetDesc( stack[i],shear,			mp->shear		); i++;
	}

	stack[i].func  = (trfn)NULL;


}


// Make a pano in TrPtr->dest (must be allocated and all set!)
// using parameters in aPrefs (ignore image parameters in TrPtr !)

void MakePano2( TrformStr *TrPtr, aPrefs *aP )
{
	struct 	MakeParams	mp;
	fDesc 	stack[15], fD;		// Parameters for execute 
	void	*morph[3];	

	int 	i,k, kstart, kend, color;

	TrPtr->success = 1;
	
	if( CheckMakeParams( aP) != 0)
	{
		TrPtr->success = 0;
		return;
	}


	if(  isColorSpecific( &(aP->im.cP) ) )			// Color dependent
	{
		kstart 	= 1; kend	= 4;
	}
	else 											// Color independent
	{
		kstart	= 0; kend	= 1;
	}
				
	for( k = kstart; k < kend; k++ )
	{
		color = k-1; if( color < 0 ) color = 0;
		SetMakeParams2( stack, &mp, &(aP->im) , &(aP->pano), color );
		
		if( aP->nt > 0 )	// Morphing requested
		{
			morph[0] = (void*)aP->td;
			morph[1] = (void*)aP->ts;
			morph[2] = (void*)&aP->nt;

			i=0; while( stack[i].func != NULL && i<14 ) i++;
			if( i!=14 )
			{
				for(i=14; i>0; i--)
				{
					memcpy( &stack[i], &stack[i-1], sizeof( fDesc ));
				}
				stack[0].func 		= tmorph;
				stack[0].param 		= (void*)morph;
			}
		}
					
			
		
		if( TrPtr->success != 0)
		{
			fD.func = execute_stack; fD.param = stack;
			transForm( TrPtr,  &fD , k);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

