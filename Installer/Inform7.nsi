; Install script for Windows Inform 7

!include "MUI2.nsh"
!include "Inform7.nsh"

Name "Windows Inform 7"
Caption "Windows Inform 7 (${BUILD}) Setup"
BrandingText "NullSoft Install System"
Unicode true
ManifestDPIAware true

SetCompressor /SOLID lzma
OutFile "I7_${BUILD}_Windows.exe"

InstallDir "$PROGRAMFILES64\Inform 7"
InstallDirRegKey HKLM "SOFTWARE\David Kinder\Inform\Install64" "Directory"

!define MUI_ICON "..\Inform7\res\Inform7.ico"
!define MUI_UNICON "..\Inform7\res\Inform7.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "Back.bmp"
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH

!define MUI_WELCOMEPAGE_TEXT "Setup will guide you through the installation of Inform 7, a design system for interactive fiction based on natural language.$\r$\n$\r$\n$_CLICK"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW SetWelcomeBitmap
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_PAGE_CUSTOMFUNCTION_SHOW SetFinishBitmap
!define MUI_FINISHPAGE_RUN $INSTDIR\Inform7.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Function ".onInit"
  InitPluginsDir
  System::Call USER32::GetDpiForSystem()i.r0
  ${If} $0 > 96
    File "/oname=$PLUGINSDIR\Side.bmp" "Side120.bmp"
  ${Else}
    File "/oname=$PLUGINSDIR\Side.bmp" "Side96.bmp"
  ${EndIf}
FunctionEnd

Function "SetWelcomeBitmap"
  ${NSD_SetImage} $mui.WelcomePage.Image "$PLUGINSDIR\Side.bmp" $mui.WelcomePage.Image.Bitmap
FunctionEnd

Function "SetFinishBitmap"
  ${NSD_SetImage} $mui.FinishPage.Image "$PLUGINSDIR\Side.bmp" $mui.FinishPage.Image.Bitmap
FunctionEnd

Section "DoInstall"

  SetOutPath "$INSTDIR"

  ; Remove old libcef files
  Delete "$INSTDIR\natives_blob.bin"
  Delete "$INSTDIR\Chrome\*.pak"

  File /r "..\Build\*.*"
  WriteUninstaller "Uninstall.exe"

  SetShellVarContext all
  CreateShortCut "$SMPROGRAMS\Inform 7.lnk" "$INSTDIR\Inform7.exe"
  SetShellVarContext current
  
  WriteRegStr HKLM "SOFTWARE\David Kinder\Inform\Install64" "Directory" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "DisplayName" "Inform 7 (64-bit)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "DisplayIcon" "$INSTDIR\Inform7.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "DisplayVersion" ${BUILD}
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "Publisher" "David Kinder"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64" "NoRepair" 1

SectionEnd

Section "Uninstall"

  RMDir /r "$INSTDIR\Books"
  RMDir /r "$INSTDIR\Chrome"
  RMDir /r "$INSTDIR\Compilers"
  RMDir /r "$INSTDIR\Dictionaries"
  RMDir /r "$INSTDIR\Documentation"
  RMDir /r "$INSTDIR\Images"
  RMDir /r "$INSTDIR\Internal"
  RMDir /r "$INSTDIR\Interpreters"
  RMDir /r "$INSTDIR\Retrospective"
  RMDir /r "$INSTDIR\Samples"
  RMDir /r "$INSTDIR\Symbols"
  RMDir /r "$INSTDIR\Web"

  Delete "$INSTDIR\Inform7.exe"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\chrome_elf.dll"
  Delete "$INSTDIR\libcef.dll"
  Delete "$INSTDIR\icudtl.dat"
  Delete "$INSTDIR\snapshot_blob.bin"
  Delete "$INSTDIR\v8_context_snapshot.bin"
  Delete "$INSTDIR\Inform7.VisualElementsManifest.xml"
  RMDir "$INSTDIR"

  SetShellVarContext all
  Delete "$SMPROGRAMS\Inform 7.lnk"
  SetShellVarContext current

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inform 7 x64"

SectionEnd

