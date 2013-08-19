# Machine = x86-linux
# Machine = ps3-linux
Machine = mingw


ifeq "$(Machine)"  "x86-linux"
  CFLAGS  =  -Wall -O2 
  LDFLAGS = -L/opt/lib -lpano12 -Wl,-rpath=/opt/lib
endif
ifeq "$(Machine)"   "ps3-linux"
  CFLAGS  = -DBIGENDIAN -m32 -Wall -O2 -fpic
  LDFLAGS = -L/opt/lib -lpano12 -Wl,-rpath=/opt/lib
endif
ifeq "$(Machine)" "mingw"
  CFLAGS = -Wall -O2
#  LIB = -L d:/mpremap/bin -lpano12
  LIB = -L../lib -lpano12 -ltiff -ljpeg -lpng -lz c:/mingw/lib/libcomdlg32.a
  # e:/mingw/lib/libpano12.a
endif

CC      = gcc
  

OBJ = mpremap.o preprocess.o spline.o string2.o ppmstream.o
SRC = $(OBJ:%.o=%.c)
HDR = filter.h panorama.h ppmstream.h string2.h


%.o: %.c	
	$(CC) $(CFLAGS) -c $<


mpremap : $(SRC) $(HDR) $(OBJ) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o mpremap $(OBJ) 
	
mpremap.exe	: $(OBJ) Makefile
	windres  -i mpremap.rc -o mpremap_rc.o
	$(CC) $(CFLAGS) -o mpremap.exe mpremap_rc.o $(OBJ) $(LIB) 
	strip mpremap.exe
	mv mpremap.exe d:/mpremap/bin/

clean	:
	rm $(OBJ) mpremap
