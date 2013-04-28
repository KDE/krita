Set fso = CreateObject("Scripting.FileSystemObject")
Set wshShell = CreateObject("WScript.Shell") 

Dim envvar, filedir

Sub CheckForApp(filename, varname)
	envvar = "set BUILD" & varname
	filedir = wshShell.ExpandEnvironmentStrings( "%C2WINSTALL_INPUT%\bin\" )
	
	If fso.FileExists( filedir & filename ) Then
		WScript.Echo envvar & "=on"
	Else
		Wscript.Echo envvar & "=off"
	End If
End Sub

' Check whether a file exists
' If so, then set an environment variable BUILD<app> to on or off, as appropriate
CheckForApp "calligrawords.exe", "words"
CheckForApp "calligrastage.exe", "stage"
CheckForApp "calligrasheets.exe", "sheets"

CheckForApp "calligraflow.exe", "flow"
CheckForApp "calligraplan.exe", "plan"

CheckForApp "karbon.exe", "karbon"
CheckForApp "krita.exe", "krita"

CheckForApp "kexi.exe", "kexi"

Set fso = Nothing
Set wshSystemEnv = Nothing
Set wshShell = Nothing