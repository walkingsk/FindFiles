Set WshShell = WScript.CreateObject("WScript.Shell")
Set WshFSO = WScript.CreateObject("Scripting.FileSystemObject")
Set RegFile = WshFSO.CreateTextFile("List All Files.reg", True, True)
StrCD = WshShell.CurrentDirectory

RegFile.WriteLine("REGEDIT4")
RegFile.WriteLine("")
RegFile.WriteLine("[HKEY_CLASSES_ROOT\Directory\Background\shell\List All Files]")
RegFile.WriteLine("@=""&List All Files""")
RegFile.WriteLine("""Icon""=""%SystemRoot%\\System32\\SHELL32.dll,36""")
RegFile.WriteLine("")
RegFile.WriteLine("[HKEY_CLASSES_ROOT\Directory\Background\shell\List All Files\Command]")

StrCD = Replace(StrCD, "\", "\\")
' WScript.Echo (StrCD)
RegFile.WriteLine("@=""" + StrCD + "\\FindFilesListView.exe %V""")
RegFile.Close