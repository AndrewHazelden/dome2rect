@ECHO OFF
@title latlong2cubemap3x2 v1.6 - immersive transformations

echo latlong2cubemap3x2 v1.6 - 2016-08-18
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo latlong2cubemap3x2 converts a latlong formatted image sequence
echo into a 3x2 cubemap format using the moving panorama program by 
echo Helmut Dersch and Imagemagick.
echo ----------------------------------------------------------------------
echo Check out the PTStitcher wiki for the script syntax:
echo http://wiki.panotools.org/PTStitcher

REM Note: mpremap accepts piped .pnm image streams

REM Image sequences are processed with unpadded sequence frame extensions.

REM output image folder
@set outputFolder=C:\dome2rect\output

REM Start/End Sequence Frame Range:
REM @set start_frame=1
REM @set end_frame=30
@set start_frame=1
@set end_frame=1

@set step_by_frames=1

REM PT Stitcher Scripts
@set ptscript_back=latlong2cubemap_back
@set ptscript_bottom=latlong2cubemap_bottom
@set ptscript_front=latlong2cubemap_front
@set ptscript_left=latlong2cubemap_left
@set ptscript_right=latlong2cubemap_right
@set ptscript_top=latlong2cubemap_top

REM Input image filename prefix
@set input=input\latlong_sequence

REM Output image extension:
@set output_ext=jpg

REM Output image - 6 extracted cubemap faces:
@set output_back=%outputFolder%\back
@set output_bottom=%outputFolder%\bottom
@set output_front=%outputFolder%\front
@set output_left=%outputFolder%\left
@set output_right=%outputFolder%\right
@set output_top=%outputFolder%\top

REM Output image - final stitched 3x2 cubemap
@set output_cube3x2=%outputFolder%\cubemap3x2

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)

REM Move to the base dome2rect folder
cd C:\dome2rect\

echo.

FOR /L %%G IN (%start_frame%, %step_by_frames%, %end_frame%) DO (

  REM Checking if the source image exists
  if exist %input%.%%G.%output_ext% (
  
    echo.
    echo Processing Frame %%G
    echo.
    
    echo Back View
    bin\mpremap.exe -f scripts\%ptscript_back% -o %output_back%.%%G.%output_ext% %input%.%%G.%output_ext%

    echo Bottom View
    bin\mpremap.exe -f scripts\%ptscript_bottom% -o %output_bottom%.%%G.%output_ext% %input%.%%G.%output_ext%

    echo Front View
    bin\mpremap.exe -f scripts\%ptscript_front% -o %output_front%.%%G.%output_ext% %input%.%%G.%output_ext%

    echo left View
    bin\mpremap.exe -f scripts\%ptscript_left% -o %output_left%.%%G.%output_ext% %input%.%%G.%output_ext%

    echo Right View
    bin\mpremap.exe -f scripts\%ptscript_right% -o %output_right%.%%G.%output_ext% %input%.%%G.%output_ext%

    echo Top View
    bin\mpremap.exe -f scripts\%ptscript_top% -o %output_top%.%%G.%output_ext% %input%.%%G.%output_ext%
    
    echo Merging Cubic Images
    REM Build the 6 cubic faces into a cubemap3x2 layout
    REM Note: The ^ carets are for escaping the closing parentheses in the Imagemagick commands since they are happening inside the batch script's do loop
    bin\imagemagick\imconvert.exe ( %output_front%.%%G.%output_ext% %output_right%.%%G.%output_ext% %output_back%.%%G.%output_ext% +append ^) ( %output_left%.%%G.%output_ext% %output_top%.%%G.%output_ext% %output_bottom%.%%G.%output_ext% +append ^) -background black -append %output_cube3x2%.%%G.%output_ext%
    echo Saving Image: %output_cube3x2%.%%G.%output_ext%
  ) else (
    echo Warning: %input%.%%G.%output_ext% image was not found.
  )
)

echo.
echo Sequence Processing Complete
echo.

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output_cube3x2%.%%d.%output_ext%
