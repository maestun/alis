echo off
IF %1&==& GOTO Erreur
IF %2&==& GOTO Erreur
IF %0==INSTA GOTO :lecture
Cls
echo  
echo 	To install this game on your hard disk, you must
echo 	have at least 3 Mbytes of free disk space.
echo 
pause

md %2\ISHAR2
copy %0.BAT %2\ISHAR2\INSTA.BAT
%2
cd %2\ISHAR2
INSTA %1 %2 
goto :fin


:lecture
echo 
:rec1
echo ***** Insert DISK A in drive %1
pause
if not exist %1\main.io goto :rec1
xcopy %1\*.*
if errorlevel 2 goto :erreur2

echo 
:rec5
echo ***** Insert DISK B in drive %1
pause
if not exist %1\vport.io goto :rec5
xcopy %1\*.*
if errorlevel 2 goto :erreur2
goto :reussi


:reussi
echo 
echo     Installation finished.
echo     To play, choose START.EXE from the ISHAR2 sub-directory.
echo 
pause
Goto Fin


:erreur2
echo 
echo	***** Procedure interrupted. *****
echo	***** Installation failed.   *****
echo 
pause
goto :fin


:Erreur
echo 
echo       INSTALL Source_Drive Destination_Drive
echo       Ex: INSTALL A: C:
echo 
:Fin
