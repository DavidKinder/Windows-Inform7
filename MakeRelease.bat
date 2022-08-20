@echo off

pushd Build
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\Inform_10_X_X_Windows_zipped.zip *.*
popd

pushd Installer
"%ProgramFiles(x86)%\NSIS\makensis" Inform7.nsi
move Inform*Windows*.exe \Temp
popd
