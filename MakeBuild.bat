@echo off
copy Distribution\inform\inform6\Tangled\inform6.exe Build\Compilers
copy Distribution\inform\inform7\Tangled\inform7.exe Build\Compilers
xcopy /s /q /i /y Distribution\inform\inform7\Internal Build\Internal