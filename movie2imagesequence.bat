@ECHO OFF
@title Movie To Image Sequence v1.5 

echo Movie To Image Sequence Script v1.5 - 2016-03-28
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo This script will extract image sequence frames from all of the mp4
echo movie files in the input folder using ffmpeg.
echo ----------------------------------------------------------------------

REM Variables
REM -------------------------------

REM Movie To Image Sequence Program Paths
REM Note: You need to add "" quotes around MovieSequencePath values if there are spaces in the filepath
@set MovieSequencePath=C:\dome2rect

REM @set imagemagickTool="C:\Program Files\ImageMagick-6.9.3-Q16\convert.exe"
REM @set ffmpegtool="C:\ffmpeg\bin\ffmpeg.exe"
@set imagemagickTool="C:\dome2rect\bin\imconvert.exe"
@set ffmpegtool="C:\dome2rect\bin\ffmpeg.exe"

REM Sound File
REM @set SoundFile="C:\dome2rect\sounds\cinematic-musical-sting-braam.wav"

REM input image extension:
@set inputExt=mp4

REM output image extension:
@set outputExt=png
REM @set outputExt=tif

REM input image folder
@set inputFolder=%MovieSequencePath%\input
REM @set inputFolder=C:\dome2rect\input

REM output image folder
@set outputFolder=%MovieSequencePath%\output
REM @set outputFolder=C:\dome2rect\output

REM FFmpeg Options holds the string of options to be used when processing the imagery. 
REM Note: If you need to use parenthesis () for order of operations, make sure to use a carat before the 
REM closing parenthesis ( -r 25 ^) so it is handled correctly by the windows command prompt.
REM The ffmpegOptions defaults are Frame Rate = -r 29.97 and  Maximum Image Quality = -q:v 1 
@set ffmpegOptions=-r 29.97 -q:v 1 

REM ------------------------------------------------------
REM    Do a conversion on a folder
REM ------------------------------------------------------

REM Run the "for" loop from inside the input folder
REM Note: cd /D changes the drive letter and the directory
cd /D %inputFolder%

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)
 

REM Note: To find out how the FOR command works type "help for" in the command prompt.
for %%I in (*.%inputExt%) do (

  echo Loading: %inputFolder%\%%I  - Saving: %outputFolder%\%%~nI.%outputExt%

  REM Run the current movie through ffmpeg.exe and save the result to the output folder
  %ffmpegtool% -i %%I %ffmpegOptions% %outputFolder%\%%~nI.%%04d.%outputExt% 
)

echo.
echo Sequence Processing Complete
echo.

cd %outputFolder%

REM Play the job complete BRAAM sound effect
REM powershell -c (New-Object Media.SoundPlayer %SoundFile%).PlaySync();

PAUSE
