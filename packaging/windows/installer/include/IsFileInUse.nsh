;
;  SPDX-License-Identifier: GPL-3.0-or-later
;
; Checks whether a file is in use by trying to open it in Append mode

!macro _IsFileInUse _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	StrCpy $_LOGICLIB_TEMP "0"
	IfFileExists `${_b}` `0` +4 ; If file not exist then not in use
	FileOpen $_LOGICLIB_TEMP ${_b} a ; If file open failed then file is in use
	IfErrors +2
	FileClose $_LOGICLIB_TEMP
	StrCmp $_LOGICLIB_TEMP `` `${_t}` `${_f}` ; Var is empty when file in use
!macroend
!define IsFileInUse `"" IsFileInUse`
