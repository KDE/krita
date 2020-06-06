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
Page Custom func_BeforeInstallPage_Init
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

# Uninstaller Pages
!define MUI_PAGE_CUSTOMFUNCTION_PRE un.func_UnintallFirstpage_Init
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "SimpChinese"

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
	#   InstallerLanguage:
	#     Language used by the installer (to be re-used for the uninstaller)
	WriteRegStr HKLM "Software\Krita" \
	                 "InstallerLanguage" "$LANGUAGE"
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

	${IfNot} ${Silent}
		# Language selection, seems that the order is predefined.
		Push "" # This value is for languages auto count
		Push ${LANG_ENGLISH}
		Push English
		Push ${LANG_TRADCHINESE}
		Push "繁體中文"
		Push ${LANG_SIMPCHINESE}
		Push "简体中文"
		Push A # = auto count languages
		LangDLL::LangDialog "$(^SetupCaption)" "$(SetupLangPrompt)"
		Pop $LANGUAGE
		${If} $LANGUAGE == "cancel"
			Abort
		${Endif}
	${EndIf}

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

	# Detect ancient Krita versions
	${DetectKritaMsi32bit} $KritaMsiProductX86
	${If} ${RunningX64}
		${DetectKritaMsi64bit} $KritaMsiProductX64
	${EndIf}
	${If} $KritaMsiProductX86 != ""
	${OrIf} $KritaMsiProductX64 != ""
		${IfNot} ${Silent}
			MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1 "$(MsgAncientVerMustBeRemoved)" \
						/SD IDYES \
						IDYES lbl_removeAncientVer
			Abort
		${EndIf}
		lbl_removeAncientVer:
		${If} $KritaMsiProductX64 != ""
			push $R0
			${MsiUninstall} $KritaMsiProductX64 $R0
			${If} $R0 != 0
				${IfNot} ${Silent}
					${IfKritaMsi3Alpha} $KritaMsiProductX64
						MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita3alpha1RemoveFailed)"
					${Else}
						MessageBox MB_OK|MB_ICONSTOP "$(MsgKrita2msi64bitRemoveFailed)"
					${EndIf}
				${EndIf}
				Abort
			${EndIf}
			pop $R0
			StrCpy $KritaMsiProductX64 ""
		${EndIf}
		${If} $KritaMsiProductX86 != ""
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
					MessageBox MB_OK|MB_ICONINFORMATION "$(MsgKrita3264bitSwap)"
!else
					MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgKrita3264bitSwap)"
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
					MessageBox MB_OK|MB_ICONINFORMATION "$(MsgKrita3264bitSwap)"
!else
					MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgKrita3264bitSwap)"
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
			SetErrorLevel 10
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

	# Get and use installer language:
	Push $0
	ReadRegStr $0 HKLM "Software\Krita" "InstallerLanguage"
	${If} $0 != ""
		StrCpy $LANGUAGE $0
	${EndIf}
	Pop $0

	ReadRegDWORD $UninstallShellExStandalone HKLM "Software\Krita\ShellExtension" "Standalone"
	${If} ${Silent}
		# Only check here if running in silent mode. It's otherwise checked in
		# un.func_UnintallFirstpage_Init in order to display a prompt in the
		# correct language.
		${If} ${IsFileinUse} "$INSTDIR\bin\krita.exe"
			SetErrorLevel 10
			Abort
		${EndIf}
	${EndIf}
FunctionEnd

Function un.func_UnintallFirstpage_Init
	${If} ${IsFileinUse} "$INSTDIR\bin\krita.exe"
		${IfNot} ${Silent}
			MessageBox MB_OK|MB_ICONEXCLAMATION "$(MsgUninstallKritaRunning)"
		${EndIf}
		SetErrorLevel 10
		Quit
	${EndIf}
FunctionEnd

Function func_ShellExLicensePage_Init
	${IfNot} ${SectionIsSelected} ${SEC_shellex}
		# Skip ShellEx license page if not selected
		Abort
	${EndIf}
FunctionEnd

Var hwndChkDesktopIcon

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

	${NSD_CreateLabel} 0u 40u 300u 140u "$(ConfirmInstallPageDesc2)"
	pop $R0

	# TODO: Add install option summary for review?

	nsDialogs::Show

	pop $R0
FunctionEnd


# Strings
!include "translations\English.nsh"
!include "translations\TradChinese.nsh"
!include "translations\SimpChinese.nsh"
