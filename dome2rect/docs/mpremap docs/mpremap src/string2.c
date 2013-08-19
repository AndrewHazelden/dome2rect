/* string2 - String processing 
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Replace characters in src according to rule 
// specified in from and to

char* sreplace(char* src, const char* from, const char* to){
	const char *f, *t;
	char *s;
	s=src;
	while(*s != 0){
		f=from; t=to;
		while(*f!=0){
			if(*s==*f)
				*s=*t;
			f++;t++;
		}
		s++;
	}
	return src;
}	

// String s1 starts with s2 ?
int sstartswith(const char* s1, const char* s2, int ignorecase){
	const char *b;
	int s1_len = strlen(s1), s2_len = strlen(s2);
	if(s1_len<s2_len)
		return 0;
	b=s2+s2_len;
	if(ignorecase){
		while(s2<b){
			if(tolower(*s2++)!=tolower(*s1++))
				return 0;
		}
	}else{
		while(s2<b){
			if( (*s2++)!= (*s1++) )
				return 0;
		}
	}
	return 1;
}

// String s1 ends with s2 ?
int sendswith(const char* s1, const char* s2, int ignorecase){
	const char *a, *b;
	int s1_len = strlen(s1), s2_len = strlen(s2);
	if(s1_len<s2_len)
		return 0;
	b=s2+s2_len-1;
	a=s1+s1_len-1;
	if(ignorecase){
		while(b>=s2){
			if(tolower(*b--)!=tolower(*a--))
				return 0;
		}
	}else{
		while(b>=s2){
			if( (*b--)!= (*a--) )
				return 0;
		}
	}
	return 1;
}

// Print string to file	
int sfileout(const char* s, const char* f){
	FILE *sf = fopen(f, "w");
	if(sf!= NULL){
		fprintf(sf,s);
		fclose(sf);
		return 0;
	}
	return -1;
}
