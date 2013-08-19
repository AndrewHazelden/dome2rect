MSRC = MPRemap.java Options.java ptyesno.java pttext.java
NSRC = Notepad.java ElementTreePanel.java
SRC = $(MSRC) 

CLS = MPRemap.class 'MPRemap$$1.class'  'MPRemap$$PipeRunner.class' \
       'MPRemap$$PrintRunner.class' ptyesno.class Options.class pttext.class 


MPRemap.jar : Makefile MainClass $(SRC)
	javac  -O  $(SRC)
#	jar cfm@ ../MPRemap.jar MainClass < classes  
	jar cfm0 ../MPRemap.jar MainClass $(CLS) help  

