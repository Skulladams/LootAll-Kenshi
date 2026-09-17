param(
    [string]$VCRoot = $env:LOOTALL_VC100,
    [string]$SDKRoot = $env:LOOTALL_SDK71,
    [string]$Dependencies = $env:LOOTALL_DEPS,
    [string]$BuildDirectory = "$PSScriptRoot\Build",
    [string]$Python = 'python',
    [string]$BinaryToolsPath = $env:LOOTALL_BINARY_TOOLS,
    [string]$KenshiDirectory = $env:LOOTALL_KENSHI
)
$ErrorActionPreference='Stop'
if (!$VCRoot -or !$SDKRoot -or !$Dependencies) { throw 'Supply -VCRoot (VC2010), -SDKRoot (SDK7.1), and -Dependencies (KenshiLib_Examples_deps).' }
if (!$KenshiDirectory) { throw 'Supply -KenshiDirectory (matching installed game) for mandatory post-build verification.' }
if (!(Test-Path -LiteralPath (Join-Path $KenshiDirectory 'KenshiLib.dll'))) { throw 'KenshiLib.dll missing from verification directory.' }
$cl=Join-Path $VCRoot 'bin\amd64\cl.exe'
$link=Join-Path $VCRoot 'bin\amd64\link.exe'
if (!(Test-Path -LiteralPath $cl)) { throw "Missing x64 v100 compiler: $cl" }
New-Item -ItemType Directory -Force -Path $BuildDirectory | Out-Null
$BuildDirectory=(Resolve-Path -LiteralPath $BuildDirectory).Path
$env:PATH="$(Join-Path $VCRoot 'bin\amd64');$env:PATH"
$env:INCLUDE="$VCRoot\include;$SDKRoot\Include"
$env:LIB="$VCRoot\lib\amd64;$SDKRoot\Lib\x64"
$includes=@("/I$Dependencies\KenshiLib\Include", "/I$Dependencies\KenshiLib\Include\ogre", "/I$Dependencies\boost_1_60_0")
$sources=Get-ChildItem -LiteralPath "$PSScriptRoot\Source" -Filter '*.cpp'
$objects=@()
foreach ($source in $sources) {
    $object=Join-Path $BuildDirectory ($source.BaseName+'.obj')
    & $cl /nologo /c /O2 /GL /MD /EHsc /W3 /DNDEBUG /DWIN32 /D_WINDOWS /D_USRDLL /DUNICODE /D_UNICODE /D_ITERATOR_DEBUG_LEVEL=0 /DBOOST_ALL_NO_LIB /DBOOST_SYSTEM_NO_LIB /DBOOST_SYSTEM_NO_DEPRECATED /DBOOST_ERROR_CODE_HEADER_ONLY @includes "/Fo$object" $source.FullName
    if($LASTEXITCODE -ne 0) { throw "Compilation failed: $($source.Name)" }
    $objects+=$object
}
# Required by KenshiLib 0.5.0: /GL + /LTCG resolve addresses of SDK members
# through their IAT entries instead of producing this plugin's local import thunks.
& $link /NOLOGO /DLL /LTCG /MACHINE:X64 /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /INCREMENTAL:NO "/MAP:$BuildDirectory\LootAll.map" "/OUT:$BuildDirectory\LootAll.dll" @objects "/LIBPATH:$Dependencies\KenshiLib\Libraries" KenshiLib.lib OgreMain_x64.lib MyGUIEngine_x64.lib kernel32.lib user32.lib
if($LASTEXITCODE -ne 0) { throw 'DLL link failed' }
$verifyArgs=@('--dll',"$BuildDirectory\LootAll.dll",'--kenshilib',(Join-Path $KenshiDirectory 'KenshiLib.dll'),'--report',"$BuildDirectory\HookAddressVerification.txt")
if($BinaryToolsPath) { $verifyArgs+=@('--tools-path',$BinaryToolsPath) }
& $Python "$PSScriptRoot\VerifyHookAddresses.py" @verifyArgs
if($LASTEXITCODE -ne 0) { throw 'Post-build hook-address verification failed; do not distribute this DLL.' }
& "$PSScriptRoot\VerifyBinary.ps1" -Dll "$BuildDirectory\LootAll.dll" -KenshiDirectory $KenshiDirectory
Write-Output "BUILD SUCCEEDED: $BuildDirectory\LootAll.dll"
