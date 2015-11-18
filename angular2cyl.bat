@ECHO OFF
@title angular2cyl v1.4 - immersive transformations

echo angular2cyl v1.4 - November 18, 2015
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo angular2cyl converts an angular fisheye image sequence into the cylinder 
echo format using the moving panorama program by Helmut Dersch and FFMPEG.
echo ----------------------------------------------------------------------
echo Check out the PTStitcher wiki for the script syntax:
echo http://wiki.panotools.org/PTStitcher

rem Note: mpremap accepts piped .pnm image streams

REM All image sequences start on frame number 0 (eg. 0.jpg)

REM Use the exact image name for a single frame (eg. image.0009.jpg)
REM Use %%d.jpg for an unpadded image sequence (eg:  9.jpg)
REM Use %%.4d.jpg for a padded image sequence (eg: 0009.jpg)

REM Process a single frame for testing:
@set ptscript=angular2cyl
@set input=input\angular360.jpg
REM @set input=input\zosma.0001.jpg
@set output=output\sequence.%%d.jpg

REM Process a 4 digit padded image sequence:
REM @set ptscript=angular2cyl
REM @set input=input\zosma.0001.jpg
REM @set input=input\zosma.%%.4d.jpg
REM @set output=output\sequence.%%d.jpg

REM Move to the base dome2rect folder
cd C:\dome2rect\

REM Run the remapping program
bin\mpremap.exe -f scripts\%ptscript% -o %output% %input%

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output%