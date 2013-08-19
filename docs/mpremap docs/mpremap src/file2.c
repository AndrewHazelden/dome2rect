/* file2 - file handling 
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


int filecopy(const char* s, const char* d){
	int c;
	FILE *sf   = fopen(s, "r");
	FILE *df   = fopen(d, "w");
	if(sf==NULL || df==NULL)
		return -1;
	while((c=fgetc(sf)) !=-1)
		fputc(c,df);
	fclose(sf);
	fclose(df);
	return 0;
}

