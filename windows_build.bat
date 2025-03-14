REM @echo off

REM store current directory
set current_dir=%cd%
set ue_dir=%UEPATH%
set uat_file=%ue_dir%\..\..\Build\BatchFiles\RunUAT.bat

echo %uat_file%

"%uat_file%" BuildCookRun -project=%current_dir%\RootMRICNR.uproject -noP4 -platform=Win64 -clientconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory=%current_dir% -nocompileeditor -CookAll -Clean

