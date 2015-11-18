@ECHO OFF
@title latlong2verticalcross v1.4 - immersive transformations

echo latlong2verticalcross v1.4 - November 18, 2015
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo latlong2verticalcross converts a latlong formatted image sequence
echo into a cubic vertical cross format using the moving panorama program by 
echo Helmut Dersch and Imagemagick.
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
@set output_back=output\cubemap_back
@set output_bottom=output\cubemap_bottom
@set output_front=output\cubemap_front
@set output_left=output\cubemap_left
@set output_right=output\cubemap_right
@set output_top=output\cubemap_top

REM This is the cubemap extracted face resolution and is defined in each of the PT Stitcher scripts on the "p" line
@set cubemap_image_width_px=1024

REM Output image - final stitched vertical cross
@set output_cube_vertical=output\verticalcross

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
    REM Build the 6 cubic faces into a vertical cross layout
    REM Note: The ^ carets are for escaping the closing parentheses in the Imagemagick commands since they are happening inside the batch script's do loop
    bin\imagemagick\imconvert.exe ( -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black %output_top%.%%G.%output_ext% -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black +append ^) ( %output_left%.%%G.%output_ext% %output_front%.%%G.%output_ext% %output_right%.%%G.%output_ext% +append ^) ( -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black %output_bottom%.%%G.%output_ext% -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black +append ^) ( -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black ( %output_back%.%%G.%output_ext% -rotate 180 ^) -size %cubemap_image_width_px%x%cubemap_image_width_px% xc:black +append ^) -background black -append %output_cube_vertical%.%%G.%output_ext%
    echo Saving Image: %output_cube_vertical%.%%G.%output_ext%
  ) else (
    echo Warning: %input%.%%G.%output_ext% image was not found.
  )
)

echo.
echo Sequence Processing Complete
echo.

REM echo Loading frames into viewer...
PAUSE
REM C:\dome2rect\review.bat %output_cube_vertical%.%%d.%output_ext%
