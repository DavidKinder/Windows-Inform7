@echo off
pushd \Programs
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\Windows_UI_source.zip Adv\Inform7\COPYING
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\MakeRelease.bat
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\BuildDate\*.vcxproj Adv\Inform7\BuildDate\*.cpp
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\res\* Adv\Inform7\Inform7\*.cpp Adv\Inform7\Inform7\*.h Adv\Inform7\Inform7\*.rc Adv\Inform7\Inform7\*.vcxproj Adv\Inform7\Inform7\*.filters Adv\Inform7\Inform7\*.sln
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\Scintilla\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Installer\Inform7.* Adv\Inform7\Installer\*.bmp
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\*.sln
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Frotz\*.cpp Adv\Inform7\Interpreters\Frotz\*.vcxproj
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Glulxe\*.cpp Adv\Inform7\Interpreters\Glulxe\*.h Adv\Inform7\Interpreters\Glulxe\*.vcxproj
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Git\*.cpp Adv\Inform7\Interpreters\Git\*.vcxproj
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Test\*.inf
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Icon\*.cpp Adv\Inform7\Icon\*.vcxproj Adv\Inform7\Icon\*.sln Adv\Inform7\Icon\bitmaps\*.*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Blorb\*.c Adv\Frotz\Blorb\*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Generic\*.c Adv\Frotz\Generic\*.h Adv\Frotz\Generic\*.txt
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glulxe\Generic\*.c Adv\Glulxe\Generic\*.h Adv\Glulxe\Generic\README*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Git\*.c Adv\Git\*.h Adv\Git\README*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Include\g*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\GlkDll\*.c Adv\Glk\GlkDll\Glk*.cpp Adv\Glk\GlkDll\Glk*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Libraries\mfc\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Libraries\libmodplug\*
popd

pushd Installer
"%ProgramFiles(x86)%\NSIS\makensis" Inform7.nsi
move Inform*Windows*.exe \Temp
popd

