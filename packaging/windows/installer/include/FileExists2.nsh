;
;  SPDX-License-Identifier: GPL-3.0-or-later
;

;FileExists is already part of LogicLib, but returns true for directories as well as files
!macro _FileExists2 _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	StrCpy $_LOGICLIB_TEMP "0"
	StrCmp `${_b}` `` +4 0 ;if path is not blank, continue to next check
	IfFileExists `${_b}` `0` +3 ;if path exists, continue to next check (IfFileExists returns true if this is a directory)
	IfFileExists `${_b}\*.*` +2 0 ;if path is not a directory, continue to confirm exists
	StrCpy $_LOGICLIB_TEMP "1" ;file exists
	;now we have a definitive value - the file exists or it does not
	StrCmp $_LOGICLIB_TEMP "1" `${_t}` `${_f}`
!macroend
!undef FileExists
!define FileExists `"" FileExists2`
!macro _DirExists _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	StrCpy $_LOGICLIB_TEMP "0"
	StrCmp `${_b}` `` +3 0 ;if path is not blank, continue to next check
	IfFileExists `${_b}\*.*` 0 +2 ;if directory exists, continue to confirm exists
	StrCpy $_LOGICLIB_TEMP "1"
	StrCmp $_LOGICLIB_TEMP "1" `${_t}` `${_f}`
!macroend
!define DirExists `"" DirExists`
