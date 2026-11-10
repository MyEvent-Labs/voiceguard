; MyEvent Labs VoiceGuard v0.0.1 Alpha - Windows x64 installer
; Build with:  .\.tools\InnoSetup\ISCC.exe installer\VoiceGuard.iss
;
; Installs:
;   Standalone  -> {autopf}\MyEvent Labs\VoiceGuard\
;   VST3 bundle -> {commoncf64}\VST3\MyEvent Labs VoiceGuard.vst3
;
; The Alpha artifacts are unsigned. Do not claim otherwise.

#define AppName        "MyEvent Labs VoiceGuard"
#define AppVersion     "0.0.1-alpha"
#define AppVerName     "MyEvent Labs VoiceGuard 0.0.1 Alpha"
#define AppPublisher   "MyEvent Labs"
#define AppURL         "https://github.com/MyEvent-Labs/voiceguard"
#define BuildDir       "..\build\VoiceGuard_artefacts\Release"
#define StandaloneExe  "MyEvent Labs VoiceGuard.exe"
#define Vst3Bundle     "MyEvent Labs VoiceGuard.vst3"

[Setup]
AppId={{0E7B4C2A-5D61-4F8B-9A3C-2D1E6B7F8A90}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppVerName}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\MyEvent Labs\VoiceGuard
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir=..\release
OutputBaseFilename=VoiceGuard-0.0.1-alpha-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
LicenseFile=..\LICENSE
SetupLogging=yes
UninstallDisplayIcon={app}\{#StandaloneExe}
UninstallDisplayName={#AppVerName}
VersionInfoVersion=0.0.1.0
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} installer (ALPHA)
VersionInfoProductName={#AppName}
VersionInfoProductVersion=0.0.1.0
VersionInfoCopyright=MyEvent Labs. AGPL-3.0-or-later.
ChangesEnvironment=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
Source: "{#BuildDir}\Standalone\{#StandaloneExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\VST3\{#Vst3Bundle}\*"; DestDir: "{commoncf64}\VST3\{#Vst3Bundle}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "INSTALL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\THIRD-PARTY-NOTICES.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#StandaloneExe}"
Name: "{group}\Install notes"; Filename: "{app}\INSTALL.txt"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#StandaloneExe}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#StandaloneExe}"; Description: "Launch VoiceGuard"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove the VST3 bundle folder itself once its installed files are gone.
Type: filesandordirs; Name: "{commoncf64}\VST3\{#Vst3Bundle}"
