@echo off

del Build\Inform7.pdb

pushd \Programs
"\Program Files\Zip\zip" -j \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\COPYING
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\MakeDist.bat
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\BuildDate\*.vcproj Adv\Inform7\BuildDate\*.cpp
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\res\* Adv\Inform7\Inform7\*.cpp Adv\Inform7\Inform7\*.c Adv\Inform7\Inform7\*.h Adv\Inform7\Inform7\*.rc Adv\Inform7\Inform7\*.vcproj Adv\Inform7\Inform7\*.sln
"\Program Files\Zip\zip" -r \Temp\Windows_UI_source.zip Adv\Inform7\Inform7\Scintilla\*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Installer\Inform7.* Adv\Inform7\Installer\*.bmp Adv\Inform7\Installer\*.txt
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Installer\PlugIn\*.cpp Adv\Inform7\Installer\PlugIn\*.sln Adv\Inform7\Installer\PlugIn\*.vcproj
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Frotz\*.cpp Adv\Inform7\Interpreters\Frotz\*.vcproj Adv\Inform7\Interpreters\Frotz\*.sln
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Glulxe\*.cpp Adv\Inform7\Interpreters\Glulxe\*.h Adv\Inform7\Interpreters\Glulxe\*.vcproj Adv\Inform7\Interpreters\Glulxe\*.sln Adv\Inform7\Interpreters\Glulxe\Makefile
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Git\*.cpp Adv\Inform7\Interpreters\Git\Makefile
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Interpreters\Test\*.inf
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Inform7\Icon\*.cpp Adv\Inform7\Icon\*.vcproj Adv\Inform7\Icon\*.sln Adv\Inform7\Icon\bitmaps\*.*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Blorb\*.c Adv\Frotz\Blorb\*.h
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Frotz\Generic\*.c Adv\Frotz\Generic\*.h Adv\Frotz\Generic\*.txt
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Glulxe\*.c Adv\Glk\Glulxe\*.h Adv\Glk\Glulxe\README*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Git\*.c Adv\Glk\Git\*.h Adv\Glk\Git\README*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\Include\g*.h
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Adv\Glk\GlkDll\*.c Adv\Glk\GlkDll\Glk*.cpp Adv\Glk\GlkDll\Glk*.h
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Libraries\mfc\* Libraries\libpng\* Libraries\zlib\* Libraries\jpeg\* Libraries\hunspell\*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Libraries\scalegfx\*.h Libraries\scalegfx\*.cpp Libraries\scalegfx\*.def Libraries\scalegfx\Makefile
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Libraries\libmodplug\*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Libraries\libvorbis\*
"\Program Files\Zip\zip" -r \Temp\Windows_UI_source.zip Libraries\libvorbis\include\* Libraries\libvorbis\lib\*
"\Program Files\Zip\zip" \Temp\Windows_UI_source.zip Libraries\libogg\*
"\Program Files\Zip\zip" -r \Temp\Windows_UI_source.zip Libraries\libogg\include\* Libraries\libogg\src\*
popd

pushd Installer
"\Program Files\NSIS\makensis" Inform7.nsi
move I7_*_Windows.exe \Temp
popd

