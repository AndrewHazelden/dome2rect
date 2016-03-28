@ECHO OFF
@title Alpha Extract v1.5

echo Alpha Extract v1.5 - 2016-03-28
echo Script by Andrew Hazelden
echo ----------------------------------------------------------------------
echo Alpha Extract will extract a transparent mask from footage
echo and save it into an RGB image file.
echo ----------------------------------------------------------------------

REM Variables
REM -------------------------------

REM alphaextract Program Paths
REM Note: You need to add "" quotes around alphaextractPath values if there are spaces in the filepath
@set alphaextractPath=C:\dome2rect
@set imagemagickTool="C:\dome2rect\bin\imagemagick\imconvert.exe"

REM Sound File
REM @set alphaextractSoundFile="C:\dome2rect\sounds\cinematic-musical-sting-braam.wav"

REM input image extension:
@set inputExt=png

REM output image extension:
REM @set outputExt=png
@set outputExt=jpg

REM input image folder
@set inputFolder=%alphaextractPath%\input
REM @set inputFolder=C:\dome2rect\input

REM output image folder
@set outputFolder=%alphaextractPath%\output
REM @set outputFolder=C:\dome2rect\input

REM Imagemagick Options holds the string of options to be used when processing the imagery. 
REM Note: If you need to use parenthesis () for order of operations, make sure to use a carrat before the 
REM closing parenthsis ( -rotate 180 ^) so it is handled correctly by the windows command prompt.
@set imagemagickOptions=-alpha extract -alpha on

REM ------------------------------------------------------
REM    Do an image conversion on a folder
REM ------------------------------------------------------

REM Run the "for" loop from inside the input folder
REM Note: cd /D changes the drive letter and the directory
cd /D %inputFolder%

REM Create the output folder if it doesn't exist
IF exist %outputFolder% ( echo The %outputFolder% folder exists ) ELSE ( mkdir %outputFolder% && echo The %outputFolder% folder has been created)

REM Note: To find out how the FOR command works type "help for" in the command prompt.
for %%I in (*.%inputExt%) do (

  echo Loading: %inputFolder%\%%I  - Saving: %outputFolder%\%%~nI.%outputExt%

  REM Run the current image through imconvert.exe and save the result to the output folder
  %imagemagickTool% %%I %imagemagickOptions% %outputFolder%\%%~nI.%outputExt%
)

echo.
echo Sequence Processing Complete
echo.

cd %outputFolder%

REM Play the job complete BRAAM sound effect
REM powershell -c (New-Object Media.SoundPlayer %alphaextractSoundFile%).PlaySync();

PAUSE
