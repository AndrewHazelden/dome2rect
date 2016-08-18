@ECHO OFF
@title cubic2gearvrmono v1.6 - immersive transformations

echo cubic2gearvrmono v1.6 - 2016-08-18
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo cubic2gearvrmono converts 6 cubemap formatted image sequences
echo into a GearVR cubic format using Imagemagick.
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

REM Image extension:
@set input_ext=jpg

REM Output image - 6 extracted cubemap faces per camera view:
REM Cube Views - enter the initital file name prefix like "back" for "back.1.jpg"
@set input_back=input\back
@set input_bottom=input\bottom
@set input_front=input\front
@set input_left=input\left
@set input_right=input\right
@set input_top=input\top

REM output image folder
@set outputFolder=C:\dome2rect\output

REM Output image - final stitched cubic GearVR horizontal strip
@set output_gearvr=%outputFolder%\gearvr

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)

REM Move to the base dome2rect folder
cd C:\dome2rect\

echo.

FOR /L %%G IN (%start_frame%, %step_by_frames%, %end_frame%) DO (

  REM Checking if the source left and right image exists
  if exist %input_left%.%%G.%input_ext% if exist %input_right%.%%G.%input_ext% if exist %input_top%.%%G.%input_ext% if exist %input_bottom%.%%G.%input_ext% if exist %input_back%.%%G.%input_ext% if exist %input_front%.%%G.%input_ext% (
  
    echo.
    echo Processing Frame %%G
    echo.
    
    echo Merging Cubic Images
    REM Build the 6 cubic faces into a GearVR horizontal strip layout
    REM Note: The ^ carets are for escaping the closing parentheses in the Imagemagick commands since they are happening inside the batch script's do loop
    bin\imagemagick\imconvert.exe %input_left%.%%G.%input_ext% %input_right%.%%G.%input_ext% ( %input_top%.%%G.%input_ext% -rotate 180 ^) ( %input_bottom%.%%G.%input_ext% -rotate 180 ^) %input_back%.%%G.%input_ext% %input_front%.%%G.%input_ext% +append %output_gearvr%.%%G.%input_ext%
    
    echo Saving Image: %output_gearvr%.%%G.%input_ext%
  ) else (
    echo Warning: One of the cubic input images were not found.
  )
)

echo.
echo Sequence Processing Complete
echo.

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output_gearvr%.%%d.%input_ext%
