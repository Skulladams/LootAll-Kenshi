param([string]$VCRoot=$env:LOOTALL_VC100,[string]$SDKRoot=$env:LOOTALL_SDK71,[string]$BuildDirectory="$PSScriptRoot\Build")
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force -Path $BuildDirectory | Out-Null
$BuildDirectory=(Resolve-Path -LiteralPath $BuildDirectory).Path
$env:PATH="$VCRoot\bin\amd64;$env:PATH"
$env:INCLUDE="$VCRoot\include;$SDKRoot\Include"
$env:LIB="$VCRoot\lib\amd64;$SDKRoot\Lib\x64"
Push-Location $BuildDirectory
try {
    & "$VCRoot\bin\amd64\cl.exe" /nologo /MD /EHsc /O2 /D_ITERATOR_DEBUG_LEVEL=0 "$PSScriptRoot\Tests\ConfigTests.cpp" "$PSScriptRoot\Source\Config.cpp" /FeConfigTests.exe
    if($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
    & .\ConfigTests.exe
    if($LASTEXITCODE -ne 0) { throw 'Tests failed' }
} finally { Pop-Location }
