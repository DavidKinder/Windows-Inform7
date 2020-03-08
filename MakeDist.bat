@echo off

del Build\Inform7.pdb

pushd \Programs
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\COPYING
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\MakeDist.bat
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\BuildDate\*.vcxproj Adv\Inform7\BuildDate\*.cpp
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\res\* Adv\Inform7\Inform7\*.cpp Adv\Inform7\Inform7\*.h Adv\Inform7\Inform7\*.rc Adv\Inform7\Inform7\*.vcxproj Adv\Inform7\Inform7\*.filters Adv\Inform7\Inform7\*.sln
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\Scintilla\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Installer\Inform7.* Adv\Inform7\Installer\*.bmp
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Frotz\*.cpp Adv\Inform7\Interpreters\Frotz\*.vcproj Adv\Inform7\Interpreters\Frotz\*.sln
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Glulxe\*.cpp Adv\Inform7\Interpreters\Glulxe\*.h Adv\Inform7\Interpreters\Glulxe\*.vcproj Adv\Inform7\Interpreters\Glulxe\*.sln Adv\Inform7\Interpreters\Glulxe\Makefile
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Git\*.cpp Adv\Inform7\Interpreters\Git\Makefile
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Test\*.inf
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Icon\*.cpp Adv\Inform7\Icon\*.vcproj Adv\Inform7\Icon\*.sln Adv\Inform7\Icon\bitmaps\*.*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Blorb\*.c Adv\Frotz\Blorb\*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Generic\*.c Adv\Frotz\Generic\*.h Adv\Frotz\Generic\*.txt
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Glulxe\*.c Adv\Glk\Glulxe\*.h Adv\Glk\Glulxe\README*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Git\*.c Adv\Glk\Git\*.h Adv\Glk\Git\README*
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Include\g*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\GlkDll\*.c Adv\Glk\GlkDll\Glk*.cpp Adv\Glk\GlkDll\Glk*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Libraries\mfc\* Libraries\scalegfx\*.h Libraries\scalegfx\*.cpp
"%ProgramFiles(x86)%\Zip\zip" \Temp\Windows_UI_source.zip Libraries\libmodplug\*
popd

pushd Installer
"%ProgramFiles(x86)%\NSIS\makensis" Inform7.nsi
move I7_*_Windows.exe \Temp
popd

