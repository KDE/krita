!ifndef KRITA_INSTALLER_32 & KRITA_INSTALLER_64
	!error "Either one of KRITA_INSTALLER_32 or KRITA_INSTALLER_64 must be defined."
!endif
!ifdef KRITA_INSTALLER_32 & KRITA_INSTALLER_64
	!error "Only one of KRITA_INSTALLER_32 or KRITA_INSTALLER_64 should be defined."
!endif

!ifndef KRITA_PACKAGE_ROOT
	!error "KRITA_PACKAGE_ROOT should be defined and point to the root of the package files."
!endif

!ifdef KRITA_INSTALLER_64
	!define KRITA_INSTALLER_BITNESS 64
!else
	!define KRITA_INSTALLER_BITNESS 32
!endif

Unicode true
# Enabling DPI awareness creates awful CJK text in some sizes, so don't enable it.
ManifestDPIAware false

# Krita constants (can be overridden in command line params)
!define /ifndef KRITA_VERSION "0.0.0.0"
!define /ifndef KRITA_VERSION_DISPLAY "test-version"
#!define /ifndef KRITA_VERSION_GIT ""
!define /ifndef KRITA_INSTALLER_OUTPUT_DIR ""
!ifdef KRITA_INSTALLER_64
	!define /ifndef KRITA_INSTALLER_OUTPUT_NAME "krita_x64_setup.exe"
!else
	!define /ifndef KRITA_INSTALLER_OUTPUT_NAME "krita_x86_setup.exe"
!endif

# Krita constants (fixed)
!if "${KRITA_INSTALLER_OUTPUT_DIR}" == ""
	!define KRITA_INSTALLER_OUTPUT "${KRITA_INSTALLER_OUTPUT_NAME}"
!else
	!define KRITA_INSTALLER_OUTPUT "${KRITA_INSTALLER_OUTPUT_DIR}\${KRITA_INSTALLER_OUTPUT_NAME}"
!endif
!define KRTIA_PUBLISHER "Krita Foundation"
!ifdef KRITA_INSTALLER_64
	!define KRITA_PRODUCTNAME "Krita (x64)"
	!define KRITA_UNINSTALL_REGKEY "Krita_x64"
!else
	!define KRITA_PRODUCTNAME "Krita (x86)"
	!define KRITA_UNINSTALL_REGKEY "Krita_x86"
!endif

VIProductVersion "${KRITA_VERSION}"
VIAddVersionKey "CompanyName" "${KRTIA_PUBLISHER}"
VIAddVersionKey "FileDescription" "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} Setup"
VIAddVersionKey "FileVersion" "${KRITA_VERSION}"
VIAddVersionKey "InternalName" "${KRITA_INSTALLER_OUTPUT_NAME}"
VIAddVersionKey "LegalCopyright" "${KRTIA_PUBLISHER}"
VIAddVersionKey "OriginalFileName" "${KRITA_INSTALLER_OUTPUT_NAME}"
VIAddVersionKey "ProductName" "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} Setup"
VIAddVersionKey "ProductVersion" "${KRITA_VERSION}"

BrandingText "[NSIS ${NSIS_VERSION}]  ${KRITA_PRODUCTNAME} ${KRITA_VERSION}"

Name "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}"
OutFile ${KRITA_INSTALLER_OUTPUT}
!ifdef KRITA_INSTALLER_64
	InstallDir "$PROGRAMFILES64\Krita (x64)"
!else
	InstallDir "$PROGRAMFILES32\Krita (x86)"
!endif
XPstyle on

ShowInstDetails show
ShowUninstDetails show

Var KritaStartMenuFolder
Var CreateDesktopIcon

!include MUI2.nsh

!define MUI_FINISHPAGE_NOAUTOCLOSE

# Installer Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "license_gpl-3.0.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_PRE  func_ShellExLicensePage_Init
!define MUI_PAGE_HEADER_TEXT "$(ShellExLicensePageHeader)"
!insertmacro MUI_PAGE_LICENSE "license.rtf"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Krita"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Krita"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
!define MUI_STARTMENUPAGE_NODISABLE
!insertmacro MUI_PAGE_STARTMENU Krita $KritaStartMenuFolder
Page Custom func_DesktopShortcutPage_Init
Page Custom func_BeforeInstallPage_Init
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

# Uninstaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Languages
!insertmacro MUI_LANGUAGE "English"

!include Sections.nsh
!include LogicLib.nsh
!include x64.nsh
!include WinVer.nsh
!include WordFunc.nsh

!define KRITA_SHELLEX_DIR "$INSTDIR\shellex"

!include "include\FileExists2.nsh"
!include "include\IsFileInUse.nsh"
!include "krita_versions_detect.nsh"
!include "krita_shell_integration.nsh"

Var KritaMsiProductX86
Var KritaMsiProductX64
Var KritaNsisVersion
Var KritaNsisBitness
Var KritaNsisInstallLocation

Var PrevShellExInstallLocation
Var PrevShellExStandalone

Var UninstallShellExStandalone

Section "-Remove_shellex" SEC_remove_shellex
	${If} $PrevShellExInstallLocation != ""
	${AndIf} $PrevShellExStandalone == 1
	${AndIf} $KritaNsisVersion == ""
	${AndIf} ${FileExists} "$PrevShellExInstallLocation\uninstall.exe"
		push $R0
		DetailPrint "$(RemovingShellEx)"
		SetDetailsPrint listonly
		ExecWait "$PrevShellExInstallLocation\uninstall.exe /S _?=$PrevShellExInstallLocation" $R0
		${If} $R0 != 0
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "$(RemoveShellExFailed)"
			${EndIf}
			SetDetailsPrint both
			DetailPrint "$(RemoveShellExFailed)"
			Abort
		${EndIf}
		Delete "$PrevShellExInstallLocation\uninstall.exe"
		RMDir /REBOOTOK "$PrevShellExInstallLocation"
		SetRebootFlag false
		SetDetailsPrint lastused
		DetailPrint "$(RemoveShellExDone)"
		pop $R0
	${EndIf}
SectionEnd

Section "$(SectionRemoveOldVer)" SEC_remove_old_version
	${If} $KritaNsisInstallLocation != ""
	${AndIf} ${FileExists} "$KritaNsisInstallLocation\uninstall.exe"
		push $R0
		DetailPrint "$(RemovingOldVer)"
		SetDetailsPrint listonly
		ExecWait "$KritaNsisInstallLocation\uninstall.exe /S _?=$KritaNsisInstallLocation" $R0
		${If} $R0 != 0
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "$(RemoveOldVerFailed)"
			${EndIf}
			SetDetailsPrint both
			DetailPrint "$(RemoveOldVerFailed)"
			Abort
		${EndIf}
		Delete "$KritaNsisInstallLocation\uninstall.exe"
		RMDir /REBOOTOK "$KritaNsisInstallLocation"
		SetRebootFlag false
		SetDetailsPrint lastused
		DetailPrint "$(RemoveOldVerDone)"
		pop $R0
	${EndIf}
SectionEnd

Section "-Thing"
	SetOutPath $INSTDIR
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "DisplayName" "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteUninstaller $INSTDIR\uninstall.exe
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "DisplayVersion" "${KRITA_VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "DisplayIcon" "$\"$INSTDIR\shellex\krita.ico$\",0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "URLInfoAbout" "https://krita.org/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                 "Publisher" "${KRTIA_PUBLISHER}"
	#WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	#                   "EstimatedSize" 250000
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                   "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}" \
	                   "NoRepair" 1
	# Registry entries for version recognition
	#   InstallLocation:
	#     Where krita is installed
	WriteRegStr HKLM "Software\Krita" \
	                 "InstallLocation" "$INSTDIR"
	#   Version:
	#     Version of Krita
	WriteRegStr HKLM "Software\Krita" \
	                 "Version" "${KRITA_VERSION}"
	#   x64:
	#     Set to 1 for 64-bit Krita, can be missing for 32-bit Krita
!ifdef KRITA_INSTALLER_64
	WriteRegDWORD HKLM "Software\Krita" \
	                   "x64" 1
!else
	DeleteRegValue HKLM "Software\Krita" "x64"
!endif
	#   StartMenuFolder:
	#     Start Menu Folder
	#     Handled by Modern UI 2.0 MUI_PAGE_STARTMENU
SectionEnd

Section "${KRITA_PRODUCTNAME}" SEC_product_main
	# TODO: Maybe switch to explicit file list?
	File /r /x ffmpeg.exe /x ffmpeg_README.txt /x ffmpeg_LICENSE.txt ${KRITA_PACKAGE_ROOT}\bin
	File /r ${KRITA_PACKAGE_ROOT}\lib
	File /r ${KRITA_PACKAGE_ROOT}\share
	File /r ${KRITA_PACKAGE_ROOT}\python
SectionEnd

Section "-Main_associate"
	CreateDirectory ${KRITA_SHELLEX_DIR}
	${Krita_RegisterFileAssociation} "$INSTDIR\bin\krita.exe"
SectionEnd

Section "-Main_Shortcuts"
	# Placing this after Krita_RegisterFileAssociation to get the icon
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Krita
		CreateDirectory "$SMPROGRAMS\$KritaStartMenuFolder"
		CreateShortcut "$SMPROGRAMS\$KritaStartMenuFolder\${KRITA_PRODUCTNAME}.lnk" "$INSTDIR\bin\krita.exe" "" "$INSTDIR\shellex\krita.ico" 0
		CreateDirectory "$SMPROGRAMS\$KritaStartMenuFolder\Tools"
		CreateShortcut "$SMPROGRAMS\$KritaStartMenuFolder\Tools\$(UninstallKritaLnkFileName).lnk" "$INSTDIR\Uninstall.exe"
	!insertmacro MUI_STARTMENU_WRITE_END
	${If} $CreateDesktopIcon == 1
		# For the desktop icon, keep the name short and omit version info
		CreateShortcut "$DESKTOP\Krita.lnk" "$INSTDIR\bin\krita.exe" "" "$INSTDIR\shellex\krita.ico" 0
	${EndIf}
SectionEnd

Section "$(SectionShellEx)" SEC_shellex
	${If} ${RunningX64}
		${Krita_RegisterComComonents} 64
	${EndIf}
	${Krita_RegisterComComonents} 32

	${Krita_RegisterShellExtension}

	#   ShellExtension\InstallLocation:
	#     Where the shell extension is installed
	#     If installed by Krita installer, this must point to shellex sub-dir
	WriteRegStr HKLM "Software\Krita\ShellExtension" \
	                 "InstallLocation" "$INSTDIR\shellex"
	#   ShellExtension\Version:
	#     Version of the shell extension
	WriteRegStr HKLM "Software\Krita\ShellExtension" \
	                 "Version" "${KRITASHELLEX_VERSION}"
	#   ShellExtension\Standalone:
	#     0 = Installed by Krita installer
	#     1 = Standalone installer
	WriteRegDWORD HKLM "Software\Krita\ShellExtension" \
	                   "Standalone" 0
	#   ShellExtension\KritaExePath:
	#     Path to krita.exe as specified by user or by Krita installer
	#     Empty if not specified
	WriteRegStr HKLM "Software\Krita\ShellExtension" \
	                 "KritaExePath" "$INSTDIR\bin\krita.exe"
SectionEnd

!ifdef HAS_FFMPEG
Section "$(SectionBundledFfmpeg)" SEC_ffmpeg
	File /oname=bin\ffmpeg.exe ${KRITA_PACKAGE_ROOT}\bin\ffmpeg.exe
	File /oname=bin\ffmpeg_LICENSE.txt ${KRITA_PACKAGE_ROOT}\bin\ffmpeg_LICENSE.txt
	File /oname=bin\ffmpeg_README.txt ${KRITA_PACKAGE_ROOT}\bin\ffmpeg_README.txt
SectionEnd
!endif

Section "-Main_refreshShell"
	${RefreshShell}
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	#!insertmacro MUI_DESCRIPTION_TEXT ${SEC_remove_shellex} "Remove previously installed Krita Shell Integration."
	!insertmacro MUI_DESCRIPTION_TEXT ${SEC_remove_old_version} "$(SectionRemoveOldVerDesc)"
	!insertmacro MUI_DESCRIPTION_TEXT ${SEC_product_main} "$(SectionMainDesc)"
	!insertmacro MUI_DESCRIPTION_TEXT ${SEC_shellex} "$(SectionShellExDesc)"
!ifdef HAS_FFMPEG
	!insertmacro MUI_DESCRIPTION_TEXT ${SEC_ffmpeg} "$(SectionBundledFfmpegDesc)"
!endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "un.$(SectionShellEx)"
	${If} $UninstallShellExStandalone == 1
		push $R0
		DetailPrint "$(RemovingShellEx)"
		SetDetailsPrint listonly
		ExecWait "$INSTDIR\shellex\uninstall.exe /S _?=$INSTDIR\shellex" $R0
		${If} $R0 != 0
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "$(RemoveShellExFailed)"
			${EndIf}
			SetDetailsPrint lastused
			SetDetailsPrint both
			DetailPrint "$(RemoveShellExFailed)"
		${EndIf}
		Delete "$INSTDIR\shellex\uninstall.exe"
		RMDir /REBOOTOK "$INSTDIR\shellex"
		SetDetailsPrint lastused
		DetailPrint "$(RemoveShellExDone)"
		pop $R0
	${Else}
		${Krita_UnregisterShellExtension}

		${If} ${RunningX64}
			${Krita_UnregisterComComonents} 64
		${EndIf}
		${Krita_UnregisterComComonents} 32
	${EndIf}
SectionEnd

Section "un.Main_associate"
	# TODO: Conditional, use install log
	${If} $UninstallShellExStandalone != 1
		${Krita_UnregisterFileAssociation}
	${EndIf}
SectionEnd

Section "un.Main_Shortcuts"
	Delete "$DESKTOP\Krita.lnk"
	!insertmacro MUI_STARTMENU_GETFOLDER Krita $KritaStartMenuFolder
	Delete "$SMPROGRAMS\$KritaStartMenuFolder\Tools\Uninstall ${KRITA_PRODUCTNAME}.lnk"
	RMDir "$SMPROGRAMS\$KritaStartMenuFolder\Tools"
	Delete "$SMPROGRAMS\$KritaStartMenuFolder\${KRITA_PRODUCTNAME}.lnk"
	RMDir "$SMPROGRAMS\$KritaStartMenuFolder"
SectionEnd

Section "un.${KRITA_PRODUCTNAME}"
	# TODO: Maybe switch to explicit file list or some sort of install log?
	RMDir /r $INSTDIR\bin
	RMDir /r $INSTDIR\lib
	RMDir /r $INSTDIR\share
	RMDir /r $INSTDIR\python
SectionEnd

Section "un.Thing"
	RMDir /REBOOTOK $INSTDIR\shellex
	DeleteRegKey HKLM "Software\Krita"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${KRITA_UNINSTALL_REGKEY}"
	Delete $INSTDIR\uninstall.exe
	RMDir /REBOOTOK $INSTDIR
SectionEnd

Section "un.Main_refreshShell"
	${RefreshShell}
SectionEnd

Function .onInit
	SetShellVarContext all
	!insertmacro SetSectionFlag ${SEC_product_main} ${SF_RO}
	!insertmacro SetSectionFlag ${SEC_product_main} ${SF_BOLD}
	!insertmacro SetSectionFlag ${SEC_remove_old_version} ${SF_RO}
!ifdef HAS_FFMPEG
	!insertmacro SetSectionFlag ${SEC_ffmpeg} ${SF_RO}
!endif
	StrCpy $CreateDesktopIcon 1 # Create desktop icon by default
	${IfNot} ${AtLeastWin7}
		${IfNot} ${Silent}
			MessageBox MB_OK|MB_ICONSTOP "$(MsgRequireWin7)"
		${EndIf}
		Abort
	${EndIf}

	# Language selection, seems that the order is predefined.
	Push "" # This value is for languages auto count
	Push ${LANG_ENGLISH}
	Push English
	Push A # = auto count languages
	LangDLL::LangDialog "$(^SetupCaption)" "$(SetupLangPrompt)"
	Pop $LANGUAGE
	${If} $LANGUAGE == "cancel"
		Abort
	${Endif}
	# ---

!ifdef KRITA_INSTALLER_64
	${If} ${RunningX64}
		SetRegView 64
	${Else}
		${IfNot} ${Silent}
			MessageBox MB_OK|MB_ICONSTOP "$(Msg64bitOn32bit)"
		${EndIf}
		Abort
	${Endif}
!else
	${If} ${RunningX64}
		SetRegView 64
		${IfNot} ${Silent}
			MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(Msg32bitOn64bit)" \
			           /SD IDYES \
			           IDYES lbl_allow32on64
			Abort
		${EndIf}
		lbl_allow32on64:
	${Endif}
!endif
	# Detect other Krita versions
	${DetectKritaMsi32bit} $KritaMsiProductX86
	${If} ${RunningX64}
		${DetectKritaMsi64bit} $KritaMsiProductX64
		${IfKritaMsi3Alpha} $KritaMsiProductX64
			${IfNot} ${Silent}
				MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "$(MsgKrita3alpha1)" \
				           /SD IDYES \
				           IDYES lbl_removeKrita3alpha
				Abort
			${EndIf}
			lbl_removeKrita3alpha:
			push $R0
			${MsiUninstall} $KritaMsiProductX64 $R0
			${If} $R0 != 0
				${IfNot} ${Silent}
					MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita3alpha1RemoveFailed)"
				${EndIf}
				Abort
			${EndIf}
			pop $R0
			StrCpy $KritaMsiProductX64 ""
		${ElseIf} $KritaMsiProductX64 != ""
			${If} $KritaMsiProductX86 != ""
				${IfNot} ${Silent}
					MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "$(MsgKrita2msiBoth)" \
					           /SD IDYES \
					           IDYES lbl_removeKritaBoth
					Abort
				${EndIf}
				lbl_removeKritaBoth:
				push $R0
				${MsiUninstall} $KritaMsiProductX86 $R0
				${If} $R0 != 0
					${IfNot} ${Silent}
						MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita2msi32bitRemoveFailed)"
					${EndIf}
					Abort
				${EndIf}
				${MsiUninstall} $KritaMsiProductX64 $R0
				${If} $R0 != 0
					${IfNot} ${Silent}
						MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita2msi64bitRemoveFailed)"
					${EndIf}
					Abort
				${EndIf}
				pop $R0
				StrCpy $KritaMsiProductX86 ""
				StrCpy $KritaMsiProductX64 ""
			${Else}
				${IfNot} ${Silent}
					MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "$(MsgKrita2msi64bit)" \
					           /SD IDYES \
					           IDYES lbl_removeKritaX64
					Abort
				${EndIf}
				lbl_removeKritaX64:
				push $R0
				${MsiUninstall} $KritaMsiProductX64 $R0
				${If} $R0 != 0
					${IfNot} ${Silent}
						MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita2msi64bitRemoveFailed)"
					${EndIf}
					Abort
				${EndIf}
				pop $R0
				StrCpy $KritaMsiProductX64 ""
			${EndIf}
		${EndIf}
	${Endif}
	${If} $KritaMsiProductX86 != ""
		${IfNot} ${Silent}
			MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "$(MsgKrita2msi32bit)" \
			           /SD IDYES \
			           IDYES lbl_removeKritaX86
			Abort
		${EndIf}
		lbl_removeKritaX86:
		push $R0
		${MsiUninstall} $KritaMsiProductX86 $R0
		${If} $R0 != 0
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita2msi32bitRemoveFailed)"
			${EndIf}
			Abort
		${EndIf}
		pop $R0
		StrCpy $KritaMsiProductX86 ""
	${EndIf}

	${DetectKritaNsis} $KritaNsisVersion $KritaNsisBitness $KritaNsisInstallLocation
	${If} $KritaNsisVersion != ""
		push $R0
		${VersionCompare} "${KRITA_VERSION}" "$KritaNsisVersion" $R0
		${If} $R0 == 0
			# Same version installed... probably
			${If} $KritaNsisBitness == ${KRITA_INSTALLER_BITNESS}
				# Very likely the same version
				${IfNot} ${Silent}
					MessageBox MB_OK|MB_ICONINFORMATION "$(MsgKritaSameVerReinstall)"
				${EndIf}
			${Else}
				# Very likely the same version but different arch
				${IfNot} ${Silent}
!ifdef KRITA_INSTALLER_64
					MessageBox MB_OK|MB_ICONINFORMATION "$(MsgKritaSameVer32bitReplaceWith64bit)"
!else
					MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgKritaSameVer64bitReplaceWith32bit)"
!endif
				${EndIf}
			${EndIf}
		${ElseIf} $R0 == 1
			# Upgrade
			${If} $KritaNsisBitness == ${KRITA_INSTALLER_BITNESS}
				# Silent about upgrade
			${Else}
				# Upgrade but different arch
				${IfNot} ${Silent}
!ifdef KRITA_INSTALLER_64
					MessageBox MB_OK|MB_ICONINFORMATION "$(MsgKritaDiffVer32bitReplaceWith64bit)"
!else
					MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgKritaDiffVer64bitReplaceWith32bit)"
!endif
				${EndIf}
			${EndIf}
		${ElseIf} $R0 == 2
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "$(MsgKritaNewerAlreadyInstalled)"
			${EndIf}
			Abort
		${Else}
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONSTOP "Error: Unexpected state"
			${EndIf}
			Abort
		${EndIf}
		!insertmacro SetSectionFlag ${SEC_remove_old_version} ${SF_SELECTED}
		# Detect if Krita is running...
		${If} ${IsFileinUse} "$KritaNsisInstallLocation\bin\krita.exe"
			${IfNot} ${Silent}
				MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgKritaRunning)"
			${EndIf}
			Abort
		${EndIf}
		pop $R0
	${Else}
		!insertmacro ClearSectionFlag ${SEC_remove_old_version} ${SF_SELECTED}
		SectionSetText ${SEC_remove_old_version} ""
	${EndIf}

	# Detect standalone shell extension
	# TODO: Would it be possible to update Krita without replacing the standalone shellex?
	ClearErrors
	ReadRegStr $PrevShellExInstallLocation HKLM "Software\Krita\ShellExtension" "InstallLocation"
	#ReadRegStr $PrevShellExVersion HKLM "Software\Krita\ShellExtension" "Version"
	ReadRegDWORD $PrevShellExStandalone HKLM "Software\Krita\ShellExtension" "Standalone"
	#ReadRegStr $PrevShellExKritaExePath HKLM "Software\Krita\ShellExtension" "KritaExePath"
	${If} ${Errors}
		# TODO: Assume no previous version installed or what?
	${EndIf}
	${If} $PrevShellExStandalone == 1
		${IfNot} ${Silent}
			MessageBox MB_YESNO|MB_ICONQUESTION "$(MsgKritaShellExOverwrite)" \
			           /SD IDYES \
			           IDYES lbl_allowremoveshellex
			Abort
		${EndIf}
		lbl_allowremoveshellex:
		#!insertmacro SetSectionFlag ${SEC_remove_shellex} ${SF_SELECTED}
	${Else}
		#!insertmacro ClearSectionFlag ${SEC_remove_shellex} ${SF_SELECTED}
		#SectionSetText ${SEC_remove_shellex} ""
	${EndIf}
FunctionEnd

Function un.onInit
	SetShellVarContext all
!ifdef KRITA_INSTALLER_64
	${If} ${RunningX64}
		SetRegView 64
	${Else}
		Abort
	${Endif}
!else
	${If} ${RunningX64}
		SetRegView 64
	${Endif}
!endif
	ReadRegDWORD $UninstallShellExStandalone HKLM "Software\Krita\ShellExtension" "Standalone"
	${If} ${IsFileinUse} "$INSTDIR\bin\krita.exe"
		${IfNot} ${Silent}
			MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgUninstallKritaRunning)"
		${EndIf}
		Abort
	${EndIf}
FunctionEnd

Function func_ShellExLicensePage_Init
	${IfNot} ${SectionIsSelected} ${SEC_shellex}
		# Skip ShellEx license page if not selected
		Abort
	${EndIf}
FunctionEnd

Var hwndChkDesktopIcon

Function func_DesktopShortcutPage_Init
	push $R0

	nsDialogs::Create 1018
	pop $R0
	${If} $R0 == error
		Abort
	${EndIf}
	!insertmacro MUI_HEADER_TEXT "$(DesktopIconPageHeader)" "$(DesktopIconPageDesc)"

	${NSD_CreateLabel} 0u 0u 300u 20u "$(DesktopIconPageDesc2)"
	pop $R0

	${NSD_CreateCheckbox} 0u 20u 300u 10u "$(DesktopIconPageCheckbox)"
	pop $hwndChkDesktopIcon
	${If} $CreateDesktopIcon == 1
		${NSD_Check} $hwndChkDesktopIcon
	${Else}
		${NSD_Uncheck} $hwndChkDesktopIcon
	${EndIf}
	${NSD_OnClick} $hwndChkDesktopIcon func_DesktopShortcutPage_CheckChange

	nsDialogs::Show

	pop $R0
FunctionEnd

Function func_DesktopShortcutPage_CheckChange
	${NSD_GetState} $hwndChkDesktopIcon $CreateDesktopIcon
	${If} $CreateDesktopIcon == ${BST_CHECKED}
		StrCpy $CreateDesktopIcon 1
	${Else}
		StrCpy $CreateDesktopIcon 0
	${EndIf}
FunctionEnd

Function func_BeforeInstallPage_Init
	push $R0

	nsDialogs::Create 1018
	pop $R0
	${If} $R0 == error
		Abort
	${EndIf}
	!insertmacro MUI_HEADER_TEXT "$(ConfirmInstallPageHeader)" "$(ConfirmInstallPageDesc)"

	${NSD_CreateLabel} 0u 0u 300u 140u "$(ConfirmInstallPageDesc2)"
	pop $R0

	# TODO: Add install option summary for review?

	nsDialogs::Show

	pop $R0
FunctionEnd


# Strings
!define CURRENT_LANG ${LANG_ENGLISH}

# Strings to show in the installation log:
LangString RemovingShellEx ${CURRENT_LANG} "Removing Krita Shell Integration..."
LangString RemoveShellExFailed ${CURRENT_LANG} "Failed to remove Krita Shell Integration."
LangString RemoveShellExDone ${CURRENT_LANG} "Krita Shell Integration removed."
LangString RemovingOldVer ${CURRENT_LANG} "Removing previous version..."
LangString RemoveOldVerFailed ${CURRENT_LANG} "Failed to remove previous version of Krita."
LangString RemoveOldVerDone ${CURRENT_LANG} "Previous version removed."

# Strings for the component selection dialog:
LangString SectionRemoveOldVer ${CURRENT_LANG} "Remove Old Version"
LangString SectionRemoveOldVerDesc ${CURRENT_LANG} "Remove previously installed Krita $KritaNsisVersion ($KritaNsisBitness-bit)."
LangString SectionShellEx ${CURRENT_LANG} "Shell Integration"
LangString SectionShellExDesc ${CURRENT_LANG} "Shell Extension component to provide thumbnails and file properties display for Krita files.$\r$\n$\r$\nVersion: ${KRITASHELLEX_VERSION}"
LangString SectionMainDesc ${CURRENT_LANG} "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}$\r$\n$\r$\nVersion: ${KRITA_VERSION}"
# We don't actually bundle FFmpeg so these are not shown.
LangString SectionBundledFfmpeg ${CURRENT_LANG} "Bundled FFmpeg"
LangString SectionBundledFfmpegDesc ${CURRENT_LANG} "Install a bundled version of FFmpeg for exporting animations."

# Main dialog strings:
LangString SetupLangPrompt ${CURRENT_LANG} "Please select the language to be used for the setup process:"
LangString ShellExLicensePageHeader ${CURRENT_LANG} "License Agreement (Krita Shell Extension)"
LangString DesktopIconPageHeader ${CURRENT_LANG} "Desktop Icon"
LangString DesktopIconPageDesc ${CURRENT_LANG} "Configure desktop shortcut icon."
LangString DesktopIconPageDesc2 ${CURRENT_LANG} "You can choose to create a shortcut icon on the desktop for launching Krita."
LangString DesktopIconPageCheckbox ${CURRENT_LANG} "Create a desktop icon"
LangString ConfirmInstallPageHeader ${CURRENT_LANG} "Confirm Installation"
LangString ConfirmInstallPageDesc ${CURRENT_LANG} "Confirm installation of ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}."
LangString ConfirmInstallPageDesc2 ${CURRENT_LANG} "Setup is ready to install ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}. You may review the install options before you continue.$\r$\n$\r$\n$_CLICK"
LangString UninstallKritaLnkFileName ${CURRENT_LANG} "Uninstall ${KRITA_PRODUCTNAME}"

# Misc. message prompts:
LangString MsgRequireWin7 ${CURRENT_LANG} "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} requires Windows 7 or above."
LangString Msg64bitOn32bit ${CURRENT_LANG} "You are running 32-bit Windows, but this installer installs Krita 64-bit which can only be installed on 64-bit Windows. Please download the 32-bit version on https://krita.org/"
LangString Msg32bitOn64bit ${CURRENT_LANG} "You are trying to install 32-bit Krita on 64-bit Windows. You are strongly recommended to install the 64-bit version of Krita instead since it offers better performance.$\nIf you want to use the 32-bit version for testing, you should consider using the zip package instead.$\n$\nDo you still wish to install the 32-bit version of Krita?"
LangString MsgKrita3alpha1 ${CURRENT_LANG} "Krita 3.0 Alpha 1 is installed. It must be removed before ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} can be installed.$\nDo you wish to remove it now?"
LangString MsgKrita3alpha1RemoveFailed ${CURRENT_LANG} "Failed to remove Krita 3.0 Alpha 1."
LangString MsgKrita2msiBoth ${CURRENT_LANG} "Both 32-bit and 64-bit editions of Krita 2.9 or below are installed.$\nBoth must be removed before ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} can be installed.$\nDo you want to remove them now?"
LangString MsgKrita2msi32bitRemoveFailed ${CURRENT_LANG} "Failed to remove Krita (32-bit)."
LangString MsgKrita2msi64bitRemoveFailed ${CURRENT_LANG} "Failed to remove Krita (64-bit)."
LangString MsgKrita2msi64bit ${CURRENT_LANG} "Krita (64-bit) 2.9 or below is installed.$\nIt must be removed before ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} can be installed.$\nDo you wish to remove it now?"
LangString MsgKrita2msi32bit ${CURRENT_LANG} "Krita (32-bit) 2.9 or below is installed.$\nIt must be removed before ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} can be installed.$\nDo you wish to remove it now?"
LangString MsgKritaSameVerReinstall ${CURRENT_LANG} "It appears that ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} is already installed.$\nThis setup will reinstall it."
LangString MsgKritaSameVer32bitReplaceWith64bit ${CURRENT_LANG} "It appears that Krita 32-bit ${KRITA_VERSION_DISPLAY} is currently installed. This setup will replace it with the 64-bit version."
LangString MsgKritaSameVer64bitReplaceWith32bit ${CURRENT_LANG} "It appears that Krita 64-bit ${KRITA_VERSION_DISPLAY} is currently installed. This setup will replace it with the 32-bit version."
LangString MsgKritaDiffVer32bitReplaceWith64bit ${CURRENT_LANG} "It appears that Krita 32-bit ($KritaNsisVersion) is currently installed. This setup will replace it with the 64-bit version of Krita ${KRITA_VERSION_DISPLAY}."
LangString MsgKritaDiffVer64bitReplaceWith32bit ${CURRENT_LANG} "It appears that Krita 64-bit ($KritaNsisVersion) is currently installed. This setup will replace it with the 32-bit version of Krita ${KRITA_VERSION_DISPLAY}."
LangString MsgKritaNewerAlreadyInstalled ${CURRENT_LANG} "It appears that a newer version of Krita $KritaNsisBitness-bit ($KritaNsisVersion) is currently installed. If you want to downgrade Krita to ${KRITA_VERSION_DISPLAY}, please uninstall the newer version manually before running this setup."
LangString MsgKritaRunning ${CURRENT_LANG} "Krita appears to be running. Please close Krita before running this installer."
LangString MsgKritaShellExOverwrite ${CURRENT_LANG} "Krita Shell Integration was installed separately. It will be uninstalled automatically when installing Krita.$\nDo you want to continue?"
LangString MsgUninstallKritaRunning ${CURRENT_LANG} "Krita appears to be running. Please close Krita before uninstalling."
!undef CURRENT_LANG
