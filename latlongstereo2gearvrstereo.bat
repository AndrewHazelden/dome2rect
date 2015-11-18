@ECHO OFF
@title latlongstereo2gearvrstereo v1.4 - immersive transformations

echo latlongstereo2gearvrstereo v1.4 - November 18, 2015
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo latlongstereo2gearvrstereo converts a left and right latlong stereo
echo formatted image sequence into a GearVR stereo cubic format using the
echo moving panorama program by Helmut Dersch and Imagemagick.
echo ----------------------------------------------------------------------
echo Check out the PTStitcher wiki for the script syntax:
echo http://wiki.panotools.org/PTStitcher

REM Note: mpremap accepts piped .pnm image streams

REM Image sequences are processed with unpadded sequence frame extensions.

REM Start/End Sequence Frame Range:
REM @set start_frame=1
REM @set end_frame=30
@set start_frame=1
@set end_frame=1

@set step_by_frames=1

REM The cubemap extracted face resolution is defined in each of the PT Stitcher scripts on the "p" line
REM Example: p f0 w1024 h1024 v90

REM PT Stitcher Scripts
@set ptscript_back=latlong2cubemap_back
@set ptscript_bottom=latlong2cubemap_bottom
@set ptscript_front=latlong2cubemap_front
@set ptscript_left=latlong2cubemap_left
@set ptscript_right=latlong2cubemap_right
@set ptscript_top=latlong2cubemap_top

REM Input left and right stereo pair image filename prefix without the left "_L" and right "_R" view additions
REM Example: If the stereo image filename was "CubeX_LatLong_L.1.jpg" and "CubeX_LatLong_R.1.jpg" you would enter "CubeX_LatLong"
@set input=input\CubeX_LatLong


REM Output image extension:
@set output_ext=jpg

REM Output image - 6 extracted cubemap faces per Left/Right camera view:
REM Right Cube Views
@set output_back_R=output\cubemap_back_R
@set output_bottom_R=output\cubemap_bottom_R
@set output_front_R=output\cubemap_front_R
@set output_left_R=output\cubemap_left_R
@set output_right_R=output\cubemap_right_R
@set output_top_R=output\cubemap_top_R
REM Left Cube Views
@set output_back_L=output\cubemap_back_L
@set output_bottom_L=output\cubemap_bottom_L
@set output_front_L=output\cubemap_front_L
@set output_left_L=output\cubemap_left_L
@set output_right_L=output\cubemap_right_L
@set output_top_L=output\cubemap_top_L


REM Output image - final stitched cubic GearVR stereo horitzontal strip
@set output_gearvr=output\gearvr_stereo

REM Move to the base dome2rect folder
cd C:\dome2rect\

echo.

FOR /L %%G IN (%start_frame%, %step_by_frames%, %end_frame%) DO (

  REM Checking if the source left and right image exists
  if exist %input%_L.%%G.%output_ext% if exist %input%_R.%%G.%output_ext%  (
  
    echo.
    echo Processing Frame %%G
    echo.
    
    echo Back View
    bin\mpremap.exe -f scripts\%ptscript_back% -o %output_back_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_back% -o %output_back_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo Bottom View
    bin\mpremap.exe -f scripts\%ptscript_bottom% -o %output_bottom_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_bottom% -o %output_bottom_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo Front View
    bin\mpremap.exe -f scripts\%ptscript_front% -o %output_front_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_front% -o %output_front_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo left View
    bin\mpremap.exe -f scripts\%ptscript_left% -o %output_left_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_left% -o %output_left_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo Right View
    bin\mpremap.exe -f scripts\%ptscript_right% -o %output_right_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_right% -o %output_right_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo Top View
    bin\mpremap.exe -f scripts\%ptscript_top% -o %output_top_L%.%%G.%output_ext% %input%_L.%%G.%output_ext%
    bin\mpremap.exe -f scripts\%ptscript_top% -o %output_top_R%.%%G.%output_ext% %input%_R.%%G.%output_ext%

    echo Merging Cubic Images
    REM Build the 6 cubic faces into a GearVR horizontal strip layout
    REM Note: The ^ carets are for escaping the closing parentheses in the Imagemagick commands since they are happening inside the batch script's do loop
    bin\imagemagick\imconvert.exe %output_left_R%.%%G.%output_ext% %output_right_R%.%%G.%output_ext% ( %output_top_R%.%%G.%output_ext% -rotate 180 ^) ( %output_bottom_R%.%%G.%output_ext% -rotate 180 ^) %output_back_R%.%%G.%output_ext% %output_front_R%.%%G.%output_ext% %output_left_L%.%%G.%output_ext% %output_right_L%.%%G.%output_ext% ( %output_top_L%.%%G.%output_ext% -rotate 180 ^) ( %output_bottom_L%.%%G.%output_ext% -rotate 180 ^) %output_back_L%.%%G.%output_ext% %output_front_L%.%%G.%output_ext% +append %output_gearvr%.%%G.%output_ext%
    
    echo Saving Stereo Image: %output_gearvr%.%%G.%output_ext%
  ) else (
    echo Warning: The %input%_L.%%G.%output_ext% or %input%_R.%%G.%output_ext% image was not found.
  )
)

echo.
echo Sequence Processing Complete
echo.

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output_gearvr%.%%d.%output_ext%
