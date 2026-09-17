param([Parameter(Mandatory=$true)][string]$Dll,[string]$OutputDirectory=(Split-Path -Parent $PSScriptRoot))
$ErrorActionPreference='Stop'
$OutputDirectory=[IO.Path]::GetFullPath($OutputDirectory)
$package=Join-Path $OutputDirectory 'LootAll'
New-Item -ItemType Directory -Force -Path $package | Out-Null
$required=@('LootAll.mod','LootAll.ini','RE_Kenshi.json','README.md','LICENSE','THIRD_PARTY_NOTICES.md','BUILD_REPORT.md')
foreach($file in $required) {
    $from=Join-Path $PSScriptRoot $file
    if(!(Test-Path -LiteralPath $from)) { throw "Missing release file $file" }
    Copy-Item -LiteralPath $from -Destination (Join-Path $package $file) -Force
}
Copy-Item -LiteralPath $Dll -Destination (Join-Path $package 'LootAll.dll') -Force
$manifest=Get-Content -LiteralPath (Join-Path $package 'RE_Kenshi.json') -Raw | ConvertFrom-Json
if($manifest.Plugins.Count -ne 1 -or $manifest.Plugins[0] -ne 'LootAll.dll') { throw 'Manifest invalid' }
$extra=Get-ChildItem -LiteralPath $package -File | Where-Object { $_.Name -notin ($required+@('LootAll.dll')) }
if($extra) { throw 'Unexpected files in release folder; refusing to package' }
Compress-Archive -LiteralPath $package -DestinationPath (Join-Path $OutputDirectory 'LootAll-1.0.1.zip') -Force
# Explicit source roots avoid accidentally packaging build products or Python caches.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$sourceZip=Join-Path $OutputDirectory 'LootAll-1.0.1-source.zip'
$stream=[IO.File]::Open($sourceZip,[IO.FileMode]::Create)
$archive=New-Object IO.Compression.ZipArchive($stream,[IO.Compression.ZipArchiveMode]::Create)
try {
    $sourceFiles=Get-ChildItem -LiteralPath $PSScriptRoot -Recurse -File | Where-Object {
        $_.FullName -notmatch '[\\/](Build|__pycache__)[\\/]'
    }
    foreach($file in $sourceFiles) {
        $relative=$file.FullName.Substring($PSScriptRoot.Length+1).Replace('\','/')
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive,$file.FullName,$relative) | Out-Null
    }
} finally { $archive.Dispose(); $stream.Dispose() }
$hashes=@('LootAll-1.0.1.zip','LootAll-1.0.1-source.zip') | ForEach-Object {
    $h=Get-FileHash -LiteralPath (Join-Path $OutputDirectory $_) -Algorithm SHA256
    "$($h.Hash.ToLower())  $_"
}
[IO.File]::WriteAllLines((Join-Path $OutputDirectory 'LootAll-1.0.1-SHA256SUMS.txt'),$hashes,[Text.Encoding]::ASCII)
Write-Output "PACKAGED: $OutputDirectory\LootAll-1.0.1.zip"
Write-Output "SOURCE: $OutputDirectory\LootAll-1.0.1-source.zip"
