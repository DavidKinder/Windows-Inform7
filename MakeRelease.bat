@echo off

if %1. == . goto NoRelease

pushd Build
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\Inform_%1_Windows_zipped_files.zip *.*
popd

pushd Installer
"%ProgramFiles(x86)%\NSIS\makensis" Inform7.nsi
move Inform_Windows.exe Inform_%1_Windows.exe
"%ProgramFiles(x86)%\Zip\zip" \Temp\Inform_%1_Windows_installer.zip Inform_%1_Windows.exe
del Inform_%1_Windows.exe
popd
goto Done

:NoRelease
echo No release argument given, should be something like 10_1_0

:Done

