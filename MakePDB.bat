@echo off
Tools\cv2pdb\cv2pdb Build\Compilers\cBlorb.exe Build\Compilers\cBlorb.exe Build\Symbols\cBlorb.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
Tools\cv2pdb\cv2pdb Build\Compilers\inform6.exe Build\Compilers\inform6.exe Build\Symbols\inform6.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
Tools\cv2pdb\cv2pdb Build\Compilers\intest.exe Build\Compilers\intest.exe Build\Symbols\intest.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
Tools\cv2pdb\cv2pdb Build\Compilers\ni.exe Build\Compilers\ni.exe Build\Symbols\ni.pdb
if %errorlevel% neq 0 exit /b %errorlevel%

