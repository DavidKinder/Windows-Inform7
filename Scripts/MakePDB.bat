@echo off
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\inblorb.exe ..\Build\Compilers\inblorb.exe ..\Build\Compilers\inblorb.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\inform6.exe ..\Build\Compilers\inform6.exe ..\Build\Compilers\inform6.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\inform7.exe ..\Build\Compilers\inform7.exe ..\Build\Compilers\inform7.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\intest.exe ..\Build\Compilers\intest.exe ..\Build\Compilers\intest.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\inbuild.exe ..\Build\Compilers\inbuild.exe ..\Build\Compilers\inbuild.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\frotz.exe ..\Build\Compilers\frotz.exe ..\Build\Compilers\frotz.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
del ..\Build\Compilers\frotz.pdb
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\glulxe.exe ..\Build\Compilers\glulxe.exe ..\Build\Compilers\glulxe.pdb
if %errorlevel% neq 0 exit /b %errorlevel%
del ..\Build\Compilers\glulxe.pdb
