!define KRITA_OLD_MSI_x86_UPGRADECODE "{3FC4A463-F967-47AE-858B-F5F451CD8322}"
#!define KRITA_OLD_MSI_x86_UPGRADECODE_REGISTRY "364A4CF3769FEA7458B85F4F15DC3822"
!define KRITA_OLD_MSI_x64_UPGRADECODE "{295D13BE-4593-43E8-81AE-10CDCA99E918}"
#!define KRITA_OLD_MSI_x64_UPGRADECODE_REGISTRY "EB31D59239548E3418EA01DCAC999E81"

!define KRITA_OLD_MSI_3alpha1_PRODUCTCODE "{350B584A-B4D2-497A-9932-D39CFE9BFB77}"
#!define KRITA_OLD_MSI_3alpha1_PRODUCTCODE_REGISTRY "A485B0532D4BA79499233DC9EFB9BF77"
!define KRITA_OLD_MSI_3alpha1_PRODUCTVERSION "2.99.89.0"

!macro DetectMsiUpgradeCode_Macro ProductCodeOut UpgradeCode
	push $0
	push $1
	# UINT MsiEnumRelatedProducts(
	#   _In_  LPCTSTR lpUpgradeCode,
	#   _In_  DWORD   dwReserved,
	#   _In_  DWORD   iProductIndex,
	#   _Out_ LPTSTR  lpProductBuf
	# );
	System::Call 'MSI::MsiEnumRelatedProducts(t "${UpgradeCode}", i 0, i 0, t.r0) i.r1'
	${If} $1 == 0
		push $0
	${Else}
		push ""
	${EndIf}
	exch 2
	pop $0
	pop $1
	pop ${ProductCodeOut}
!macroend
!define DetectMsiUpgradeCode '!insertmacro DetectMsiUpgradeCode_Macro'

!macro DetectKritaMsi32bit_Macro ProductCodeOut
	push $0
	${DetectMsiUpgradeCode} $0 ${KRITA_OLD_MSI_x86_UPGRADECODE}
	exch $0
	pop ${ProductCodeOut}
!macroend
!define DetectKritaMsi32bit '!insertmacro DetectKritaMsi32bit_Macro'

!macro DetectKritaMsi64bit_Macro ProductCodeOut
	push $0
	${DetectMsiUpgradeCode} $0 ${KRITA_OLD_MSI_x64_UPGRADECODE}
	exch $0
	pop ${ProductCodeOut}
!macroend
!define DetectKritaMsi64bit '!insertmacro DetectKritaMsi64bit_Macro'

!macro IfKritaMsi3Alpha_Macro ProductCode
	${If} ${ProductCode} == ${KRITA_OLD_MSI_3alpha1_PRODUCTCODE}
!macroend
!define IfKritaMsi3Alpha '!insertmacro IfKritaMsi3Alpha_Macro'

!macro MsiUninstall_Macro ProductCode ExitCodeOut
	ExecWait "msiexec.exe /uninstall ${ProductCode} /passive /norestart" ${ExitCodeOut}
!macroend
!define MsiUninstall '!insertmacro MsiUninstall_Macro'

!macro DetectKritaNsis_Macro VersionOut BitsOut InstallLocationOut
	push $0
	push $1
	StrCpy $1 0

	SetRegView 64

	ClearErrors
	ReadRegStr $0 HKLM "Software\Krita" "Version"
	${If} ${Errors}
		StrCpy $1 1
	${EndIF}
	push $0

	ReadRegDWORD $0 HKLM "Software\Krita" "x64"
	${If} $0 == 1
		push 64
	${Else}
		push 32
	${EndIf}

	ClearErrors
	ReadRegStr $0 HKLM "Software\Krita" "InstallLocation"
	${If} ${Errors}
		StrCpy $1 1
	${EndIF}
	push $0

	SetRegView lastused
	${If} $1 == 1
		# Has Error
		pop $1
		pop $1
		pop $1
		pop $1
		pop $0
		StrCpy ${VersionOut} ""
		StrCpy ${BitsOut} ""
		StrCpy ${InstallLocationOut} ""
	${Else}
		exch 3
		pop $1
		exch 3
		pop $0
		pop ${VersionOut}
		pop ${InstallLocationOut}
		pop ${BitsOut}
	${EndIf}
!macroEnd
!define DetectKritaNsis '!insertmacro DetectKritaNsis_Macro'
