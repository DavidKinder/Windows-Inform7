; Install script for Windows Inform 7

!include "MUI2.nsh"
!include "Inform7.nsh"

Name "Windows Inform 7"
Caption "Windows Inform 7 (${BUILD}) Setup"
BrandingText "NullSoft Install System"

SetCompressor /SOLID lzma
OutFile "I7_${BUILD}_Windows.exe"

InstallDir "$PROGRAMFILES\Inform 7"
InstallDirRegKey HKLM "SOFTWARE\David Kinder\Inform\Install" "Directory"

!define MUI_ICON "..\Inform7\res\BigLogo.ico"
!define MUI_UNICON "..\Inform7\res\BigLogo.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "Back.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP "Side.bmp"
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Inform 7"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "SOFTWARE\David Kinder\Inform\Install" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu"
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\Inform7.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "DoInstall"

  SetOutPath "$INSTDIR"
  RMDir /r "$INSTDIR\Documentation"
  RMDir /r "$INSTDIR\Images"
  RMDir /r "$INSTDIR\Inform7"
  RMDir /r "$INSTDIR\Library"
  File /r "..\Build\*.*"
  WriteUninstaller "Uninstall.exe"
  CallInstDLL $INSTDIR\Install.dll ImageAlpha

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    Delete "$SMPROGRAMS\$STARTMENU_FOLDER\License.lnk"
    IfFileExists "$SMPROGRAMS\$STARTMENU_FOLDER" icons
    SetShellVarContext current
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    Delete "$SMPROGRAMS\$STARTMENU_FOLDER\License.lnk"
icons:
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Inform 7.lnk" "$INSTDIR\Inform7.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Licence.lnk" "$INSTDIR\Documentation\licences\licence.html"
    SetShellVarContext current
  !insertmacro MUI_STARTMENU_WRITE_END
  
  WriteRegStr HKLM "SOFTWARE\David Kinder\Inform\Install" "Directory" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7" "DisplayName" "Inform 7"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7" "UninstallString" '"$INSTDIR\Uninstall.exe"'

SectionEnd

Section "Uninstall"

  RMDir /r "$INSTDIR\Compilers"
  RMDir /r "$INSTDIR\Dictionaries"
  RMDir /r "$INSTDIR\Documentation"
  RMDir /r "$INSTDIR\Images"
  RMDir /r "$INSTDIR\Inform7"
  RMDir /r "$INSTDIR\Interpreters"
  RMDir /r "$INSTDIR\Library"
  RMDir /r "$INSTDIR\Web"

  Delete "$INSTDIR\Inform7.exe"
  Delete "$INSTDIR\Inform7.pdb"
  Delete "$INSTDIR\Install.dll"
  Delete "$INSTDIR\ScaleGfx.dll"
  Delete "$INSTDIR\BugReport.txt"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $R0
  SetShellVarContext all
  Delete "$SMPROGRAMS\$R0\Inform 7.lnk"
  Delete "$SMPROGRAMS\$R0\Licence.lnk"
  RMDir "$SMPROGRAMS\$R0"
  SetShellVarContext current
  Delete "$SMPROGRAMS\$R0\Inform 7.lnk"
  Delete "$SMPROGRAMS\$R0\Licence.lnk"
  RMDir "$SMPROGRAMS\$R0"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7"

SectionEnd

