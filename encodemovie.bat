@ECHO OFF
@title Encode Movie v1.5 - immersive transformations

echo Encode Movie v1.5 - 2016-03-29
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo This script will use ffmpeg to re-encode a mp4/mov/mkv/avi movie file
echo into a compact mp4 movie that is easy to scrub in the timeline and playback.
echo ----------------------------------------------------------------------

REM Variables
REM -------------------------------

REM Dome2rect Program Path
REM Note: You need to add "" quotes around MovieSequencePath values if there are spaces in the filepath
@set MovieSequencePath=C:\dome2rect

REM @set imagemagickTool="C:\Program Files\ImageMagick-6.9.3-Q16\convert.exe"
REM @set ffmpegtool="C:\ffmpeg\bin\ffmpeg.exe"
@set imagemagickTool="C:\dome2rect\bin\imconvert.exe"
@set ffmpegtool="C:\dome2rect\bin\ffmpeg.exe"

REM Sound File
REM @set SoundFile="C:\dome2rect\sounds\cinematic-musical-sting-braam.wav"


REM input movie folder
@set inputFolder=%MovieSequencePath%\input
REM @set inputFolder=C:\dome2rect\input

REM output movie folder
@set outputFolder=%MovieSequencePath%\output
REM @set outputFolder=C:\dome2rect\output


REM Movie file to re-encode: (You can specify a MOV/MP4/MKV/AVI File here)
@set inputMovie=%inputFolder%\input-movie.mp4

REM The output movie format should be set to an MP4 to match the default ffmpegOptions settings
@set outputMovie=%outputFolder%\encoded-movie.mp4

REM FFmpeg Options holds the string of options to be used when processing the imagery. 
REM Note: If you need to use parenthesis () for order of operations, make sure to use a carrat before the 
REM closing parenthsis ( -r 24 ^) so it is handled correctly by the windows command prompt.
@set ffmpegOptions=-r 30 -vcodec libx264 -preset fast -maxrate 20000k -bufsize 20000k -pix_fmt yuv420p -crf 18 -acodec aac -strict -2 -y

REM Run the "for" loop from inside the input folder
REM Note: cd /D changes the drive letter and the directory
cd /D %inputFolder%

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)

echo Loading Movie: %inputMovie% - Encoding Movie: %outputMovie%

REM Run the current movie through ffmpeg.exe and save the result to the output folder
IF exist %inputMovie% (
  %ffmpegtool% -i %inputMovie% %ffmpegOptions% %outputMovie%  
) ELSE (
  echo The %inputMovie% file does not exist on disk
)

echo.
echo Sequence Processing Complete
echo.

cd %outputFolder%

REM Play the job complete BRAAM sound effect
REM powershell -c (New-Object Media.SoundPlayer %SoundFile%).PlaySync();

PAUSE

REM echo Loading frames into viewer...
REM C:\dome2rect\review.bat %outputMovie%

