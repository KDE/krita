Set FSO = CreateObject("Scripting.FileSystemObject")
Set wshShell = CreateObject("WScript.Shell") 
Set oArgs=wscript.Arguments
Dim sArguments
Dim sCommand

sArguments=" "
For each item in oArgs
	sArguments = sArguments & item & " "
Next
sCommand = """" & FSO.GetFile(Wscript.ScriptFullName).ParentFolder & "\env.bat"" " & sArguments

WshShell.Run sCommand, 0, FALSE
Set wshShell = Nothing