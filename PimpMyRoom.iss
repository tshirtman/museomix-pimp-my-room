[Setup]
AppName=PimpMyRoom
AppVersion=1.0
OutputBaseFilename=PimpMyRoom-setup
DefaultDirName={pf}\PimpMyRoom
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\PimpMyRoom.exe
OutputDir=dist

[Files]
Source: "dist\PimpMyRoom\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{commonprograms}\PimpMyRoom"; Filename: "{app}\PimpMyRoome.exe"
Name: "{commondesktop}\PimpMyRoom"; Filename: "{app}\PimpMyRoom.exe"
