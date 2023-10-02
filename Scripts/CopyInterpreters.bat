@echo off
copy ..\Distribution\inform\inform6\Tests\Assistants\dumb-frotz\dumb-frotz.exe ..\Build\Compilers\frotz.exe
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\frotz.exe ..\Build\Compilers\frotz.exe ..\Build\Compilers\frotz.pdb
del ..\Build\Compilers\frotz.pdb
copy ..\Distribution\inform\inform6\Tests\Assistants\dumb-glulx\glulxe\glulxe.exe ..\Build\Compilers\glulxe.exe
..\Tools\cv2pdb\cv2pdb ..\Build\Compilers\glulxe.exe ..\Build\Compilers\glulxe.exe ..\Build\Compilers\glulxe.pdb
del ..\Build\Compilers\glulxe.pdb
