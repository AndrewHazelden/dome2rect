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

char* sreplace(char* src, const char* from, const char* to);
int sendswith(const char* s1, const char* s2, int ignorecase);
int sstartswith(const char* s1, const char* s2, int ignorecase);

int sfileout(const char* s, const char* f);
