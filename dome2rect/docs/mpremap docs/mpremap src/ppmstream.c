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


int  readPPMFileHeader(file_spec src, Image *im);


	

inline void switchBytes(char* b, long size){
	char c;
	while(size-- > 0){
		c 		= *b;
		*b 		= *(b+1);
		*(b+1)	= c;
		b		+=2;
	}
}

int getImageFromPPMStream( FILE* src, Image* im){
	long count;
	int c;
	unsigned char* d;

	int rh = readPPMFileHeader(src, im);
	if(rh < 0){  
		PrintError("Error Reading File Header");
		return -1;
	}else if(rh>0){ // No more data
		return 1;
	}

	im->bytesPerLine 	= im->width * im->bitsPerPixel/8;
	im->dataSize		= im->bytesPerLine * im->height;
	
	im->data			= (unsigned char**) mymalloc( im->width * im->height * im->bitsPerPixel/24 * 4 );
	if ( im->data == NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}
	
	count = 	im->width * im->height * im->bitsPerPixel/24 * 3;
	
	myread(src,count,*(im->data));
	
	if( count != im->width * im->height * im->bitsPerPixel/24 * 3 )
	{
		PrintError("Error: Read %d instead of %d bytes",
			count, im->width * im->height * im->bitsPerPixel/24 * 3 );
		return -1;
	}
#ifndef BIGENDIAN
	if(im->bitsPerPixel == 48){
		switchBytes((char*)*(im->data), (long)im->width * im->height * 3);
	}
#endif

	ThreeToFourBPP( im );
	return 0;
}


int putImageToPPMStream( FILE* out, Image*im ){
	int x,y;
	int bps2 = im->bitsPerPixel > 32 ; // 2 bytes per sample ?
	int fourBPP=( im->bitsPerPixel == 32 || im->bitsPerPixel == 64);

	// Make Filesize dividable by 16 (ffmpeg ? problem)

	char fmt[64];
	sprintf(fmt, "%ld %ld\n%d\n", im->width, im->height, (bps2 ? 255 : 65535));
	int size = 3 + strlen(fmt)+3*im->width*im->height*(bps2+1);
	int pad = 16-size%16;

	fprintf(out, "P6\n");
	while(pad-- >0)
		fputc('\n',out);
	fprintf(out, "%s", fmt);
	for(y=0; y<im->height; y++){
		char* data = (char*)(*im->data)+y*im->bytesPerLine;
		char dummy;
		if(bps2){
			for(x=0; x<im->width; x++){
				if(fourBPP){
					data++;data++;
				}
#ifndef BIGENDIAN
				dummy=*data++;fputc(*data++,out);fputc(dummy,out);
				dummy=*data++;fputc(*data++,out);fputc(dummy,out);
				dummy=*data++;fputc(*data++,out);fputc(dummy,out);
#else
				fputc(*data++,out);fputc(*data++,out);
				fputc(*data++,out);fputc(*data++,out);
				fputc(*data++,out);fputc(*data++,out);
#endif
			}
		}else{
			for(x=0; x<im->width; x++){
				if(fourBPP){
					data++;
				}
				fputc(*data++,out);
				fputc(*data++,out);
				fputc(*data++,out);
			}
		}
	}
	fflush(out);
	return 0;
}
						
int readPPMFileHeader(file_spec src, Image *im)
{
	int 			i;
	char 			smallBuf[32], c;
	long 			count = 1;
	
	/* read the file header (including height and width) */
	im->height = -1;
	while(im->height == -1)
	{
		myread(src,count,&c); 
		if( count != 1 ) return 1;
		switch(c)
		{
			case '#':
				/* comment line -- skip it */
				while(c != 0x0A	&& c != 0x0D && count == 1)
					myread(src,count,&c); 
				break;
			case ' ': case '\012': case '\t': case '\015':
				/* random whitespace... just ignore it. */
				break;
			case 'P':
				/* magic number */
				myread(src,count,&c); 
				if(c != '6' )
				{
					PrintError("Bad magic number in input file");
					return(-1);
				}
				/* there should be one whitespace character */
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				/* read width */
				for(i=0;isdigit(c) && (i+1 < sizeof(smallBuf));i++)
				{
					smallBuf[i] = c;
					myread(src,count,&c); 
					if( count != 1 ) return  -1;
				}
				if(!isspace(c))
				{
					PrintError("Bad input file format");
					return -1;
				}
				smallBuf[i] = 0;
				im->width = atoi(smallBuf);
				/* read height */
				myread(src,count,&c); 
				if( count != 1 ) return  -1;
				for(i=0;isdigit(c) && (i+1 < sizeof(smallBuf));i++)
				{
					smallBuf[i] = c;
					myread(src,count,&c); 
					if( count != 1 ) return  -1;
				}
				if(!isspace(c))
				{
					PrintError("Bad input file format");
					return -1;
				}
				smallBuf[i] = 0;
				im->height = atoi(smallBuf);
				/* read numColors */
				myread(src,count,&c); 
				if( count != 1 ) return  -1;
				for(i=0;isdigit(c) && (i+1 < sizeof(smallBuf));i++)
				{
					smallBuf[i] = c;
					myread(src,count,&c); 
					if( count != 1 ) return  -1;
				}
				if(!isspace(c))
				{
					PrintError("Bad input file format");
					return -1;
				}
				smallBuf[i] = 0;
				if(atoi(smallBuf) == 255)
					im->bitsPerPixel = 24;
				else if(atoi(smallBuf) == 65535)
					im->bitsPerPixel = 48;
				else
					PrintError("Warning: Wrong Colordepth!");
				break;
			default:
					//PrintError("Bad input file format");
					return(1); // No error, just finish
				break;
		}
	}
			
	/* the header has been taken care of.  The rest of the file is just image data. */	
	return(0);
}

