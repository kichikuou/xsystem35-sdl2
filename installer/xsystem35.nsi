ManifestDPIAware true
SetCompressor /SOLID lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Japanese.nlf"

Page license
Page directory
Page components
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

LicenseData ..\COPYING

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
Function RefreshShellIcons
	System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
		(${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

Section
	SetOutPath $INSTDIR
	FILE ${BUILDDIR}\src\xsystem35.exe
	!include ${BUILDDIR}\dlls.nsi
	SetOutPath $INSTDIR\fonts
	FILE /x CMakeLists.txt ..\fonts\*.*
	WriteUninstaller $INSTDIR\uninstall.exe

	WriteRegStr HKLM "Software\Kichikuou\xsystem35\profile" "ttfont_gothic" "$INSTDIR\fonts\MTLc3m.ttf"
	WriteRegStr HKLM "Software\Kichikuou\xsystem35\profile" "ttfont_mincho" "$INSTDIR\fonts\mincho.otf"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\xsystem35" "DisplayName" "xsystem35 (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\xsystem35" "UninstallString" '"$INSTDIR\uninstall.exe"'
SectionEnd

LangString FileAssociation ${LANG_ENGLISH} "Associate with .ALD file type"
LangString FileAssociation ${LANG_JAPANESE} ".ALD ファイルに関連付け"
Section "$(FileAssociation)"
	WriteRegStr HKCR ".ald" "" "System35Archive"
	WriteRegStr HKCR "System35Archive" "" "AliceSoft System 3.x archive"
	WriteRegStr HKCR "System35Archive\DefaultIcon" "" "$INSTDIR\xsystem35.exe,0"
	WriteRegStr HKCR "System35Archive\shell\open\command" "" '"$INSTDIR\xsystem35.exe" "%1"'
	Call RefreshShellIcons
SectionEnd

LangString DesktopIcon ${LANG_ENGLISH} "Create Desktop icon"
LangString DesktopIcon ${LANG_JAPANESE} "デスクトップショートカットを作成"
Section "$(DesktopIcon)"
	CreateShortcut $DESKTOP\xsystem35.lnk $INSTDIR\xsystem35.exe
SectionEnd

LangString StartMenu ${LANG_ENGLISH} "Create Start menu entry"
LangString StartMenu ${LANG_JAPANESE} "スタートメニューに登録"
Section "$(StartMenu)"
	CreateDirectory $SMPROGRAMS\xsystem35
	CreateShortcut $SMPROGRAMS\xsystem35\xsystem35.lnk $INSTDIR\xsystem35.exe
	CreateShortcut $SMPROGRAMS\xsystem35\uninstall.lnk $INSTDIR\uninstall.exe
SectionEnd

Section "Uninstall"
	Delete $DESKTOP\xsystem35.lnk
	RMDir /r $SMPROGRAMS\xsystem35

	DeleteRegKey HKCR ".ald"
	DeleteRegKey HKCR "System35Archive"

	DeleteRegKey HKLM "Software\Kichikuou\xsystem35"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\xsystem35"

	RMDir /r $INSTDIR\fonts
	Delete $INSTDIR\uninstall.exe
	Delete $INSTDIR\xsystem35.exe
	!include ${BUILDDIR}\dlls-uninstall.nsi
	RMDir $INSTDIR
SectionEnd
