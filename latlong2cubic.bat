@ECHO OFF
@title latlong2cubic v1.4 - immersive transformations

echo latlong2cubic v1.4 - November 18, 2015
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo latlong2cubic converts a latlong formatted image sequence
echo into the cubic faces format using the moving panorama program by 
echo Helmut Dersch and FFMPEG.
echo ----------------------------------------------------------------------
echo Check out the PTStitcher wiki for the script syntax:
echo http://wiki.panotools.org/PTStitcher

REM Note: mpremap accepts piped .pnm image streams

REM All image sequences start on frame number 0 (eg. 0.jpg)

REM Use the exact image name for a single frame (eg. image.0009.jpg)
REM Use %%d.jpg for an unpadded image sequence (eg:  9.jpg)
REM Use %%.4d.jpg for a padded image sequence (eg: 0009.jpg)

REM PT Stitcher Scripts
@set ptscript_back=latlong2cubemap_back
@set ptscript_bottom=latlong2cubemap_bottom
@set ptscript_front=latlong2cubemap_front
@set ptscript_left=latlong2cubemap_left
@set ptscript_right=latlong2cubemap_right
@set ptscript_top=latlong2cubemap_top

REM Still images
@set input=input\latlong.jpg

@set output_back=output\cubemap_back.jpg
@set output_bottom=output\cubemap_bottom.jpg
@set output_front=output\cubemap_front.jpg
@set output_left=output\cubemap_left.jpg
@set output_right=output\cubemap_right.jpg
@set output_top=output\cubemap_top.jpg

REM Moving images
REM @set input=input\latlong.%%.4d.jpg
REM @set input=input\latlong_sequence.%%.1d.jpg

REM @set output_back=output\cubemap_back.%%.4d.jpg
REM @set output_bottom=output\cubemap_bottom.%%.4d.jpg
REM @set output_front=output\cubemap_front.%%.4d.jpg
REM @set output_left=output\cubemap_left.%%.4d.jpg
REM @set output_right=output\cubemap_right.%%.4d.jpg
REM @set output_top=output\cubemap_top.%%.4d.jpg

REM Move to the base dome2rect folder
cd C:\dome2rect\

REM Run the remapping program
echo Back View
bin\mpremap.exe -f scripts\%ptscript_back% -o %output_back% %input%

echo Bottom View
bin\mpremap.exe -f scripts\%ptscript_bottom% -o %output_bottom% %input%

echo Front View
bin\mpremap.exe -f scripts\%ptscript_front% -o %output_front% %input%

echo left View
bin\mpremap.exe -f scripts\%ptscript_left% -o %output_left% %input%

echo Right View
bin\mpremap.exe -f scripts\%ptscript_right% -o %output_right% %input%

echo Top View
bin\mpremap.exe -f scripts\%ptscript_top% -o %output_top% %input%

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output_front%
