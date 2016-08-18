@ECHO OFF
@title Image Sequence To Movie  v1.6 

echo Image Sequence To Movie v1.6 - 2016-08-18
echo script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo This script will take an image sequence along with an 
echo audio file and create a mp4 movie file using ffmpeg.
echo ----------------------------------------------------------------------

REM Variables
REM -------------------------------

REM Image Sequence To Movie Program Paths
REM Note: You need to add "" quotes around MovieSequencePath values if there are spaces in the filepath
@set MovieSequencePath=C:\dome2rect

REM @set imagemagickTool="C:\Program Files\ImageMagick-6.9.3-Q16\convert.exe"
REM @set ffmpegtool="C:\ffmpeg\bin\ffmpeg.exe"
@set imagemagickTool="C:\dome2rect\bin\imconvert.exe"
@set ffmpegtool="C:\dome2rect\bin\ffmpeg.exe"

REM Sound File
REM @set SoundFile="C:\dome2rect\sounds\cinematic-musical-sting-braam.wav"

REM Image Sequence Frame Rate
@set frameRate=30

REM Image Sequence Starting Frame Number
@set startFrame=0
REM @set startFrame=1

REM input image folder
@set inputFolder=%MovieSequencePath%\input
REM @set inputFolder=C:\dome2rect\input

REM output image folder
@set outputFolder=%MovieSequencePath%\output
REM @set outputFolder=C:\dome2rect\output

REM input image extension (with the frame padding level defined using the %%04d formatting):
@set inputSequence=%inputFolder%\zosma.%%04d.jpg
REM @set inputSequence=%inputFolder%\rect.%%04d.jpg

REM input audio track
@set inputAudio= 
REM @set inputAudio=-i "%inputFolder%\audio.aif"
REM @set inputAudio=-i "%inputFolder%\audio.wav"


REM The output movie format should be set to an mp4 to match the default ffmpeg settings
@set outputMovie="%outputFolder%\encoded_movie.mp4"

REM FFmpeg Options holds the string of options to be used when processing the imagery. 
REM Note: If you need to use parenthesis () for order of operations, make sure to use a carat before the 
REM closing parenthesis ( -r 30 ^) so it is handled correctly by the windows command prompt.
@set ffmpegOptions= 

REM The ffmpeg x264 compressor requires that footage by divisible by 2 on the height axis
REM Note: This code fix is from a stack overflow tip
@set ffmpegImageHeightFix=-vf "scale=trunc(iw/2)*2:trunc(ih/2)*2"

REM The ffmpeg video codec is set to MP4 by default
@set ffmpegVideoCodec=-f mp4 -vcodec libx264  -preset fast -maxrate 20000k -bufsize 20000k -pix_fmt yuv420p -crf 18

REM The ffmpeg audio codec is AAC
@set ffmpegAudioCodec= 
REM @set ffmpegAudioCodec=-acodec aac -strict -2

REM The ffmpeg overwrite file option lets you choose if you want to preserve existing files in the directory
@set ffmpegOverwrite=-y

REM Note: cd /D changes the drive letter and the directory
cd /D %inputFolder%

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)

echo Loading Sequence: %inputSequence% - Encoding Movie: %outputMovie%

echo FFmpeg Launch String: %ffmpegtool% %ffmpegOptions% %inputAudio% -framerate %frameRate% -f image2 -start_number %startFrame% -i %inputSequence% -r %frameRate% %ffmpegOverwrite% %ffmpegImageHeightFix% %ffmpegAudioCodec% %ffmpegVideoCodec% %outputMovie%

REM Run the current movie through ffmpeg.exe and save the result to the output folder
%ffmpegtool% %ffmpegOptions% %inputAudio% -framerate %frameRate% -f image2 -start_number %startFrame% -i %inputSequence% -r %frameRate% %ffmpegOverwrite% %ffmpegImageHeightFix% %ffmpegAudioCodec% %ffmpegVideoCodec% %outputMovie%

echo.
echo Sequence Processing Complete
echo.

cd %outputFolder%

REM Play the job complete BRAAM sound effect
REM powershell -c (New-Object Media.SoundPlayer %SoundFile%).PlaySync();

PAUSE

REM echo Loading frames into viewer...
REM C:\dome2rect\review.bat %outputMovie%

