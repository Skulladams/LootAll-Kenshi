param([Parameter(Mandatory=$true)][string]$Dll,[Parameter(Mandatory=$true)][string]$KenshiDirectory)
$ErrorActionPreference='Stop'
function ReadPE([string]$path) {
    $b=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $path).Path)
    $pe=[BitConverter]::ToInt32($b,0x3c)
    if([BitConverter]::ToUInt32($b,$pe) -ne 0x4550) { throw 'Invalid PE signature' }
    $machine=[BitConverter]::ToUInt16($b,$pe+4)
    $sections=[BitConverter]::ToUInt16($b,$pe+6)
    $opt=$pe+24
    $optionalSize=[BitConverter]::ToUInt16($b,$pe+20)
    if([BitConverter]::ToUInt16($b,$opt) -ne 0x20b) { throw "Not PE32+ x64: $path" }
    $map=@()
    for($i=0;$i -lt $sections;$i++) {
        $s=$opt+$optionalSize+40*$i
        $map+=,@([BitConverter]::ToUInt32($b,$s+12),[Math]::Max([BitConverter]::ToUInt32($b,$s+8),[BitConverter]::ToUInt32($b,$s+16)),[BitConverter]::ToUInt32($b,$s+20))
    }
    return @{Bytes=$b;Machine=$machine;Optional=$opt;Map=$map}
}
function Offset($p,[uint32]$rva) {
    foreach($s in $p.Map) { if($rva -ge $s[0] -and $rva -lt $s[0]+$s[1]) { return [int]($s[2]+$rva-$s[0]) } }
    if($rva -lt 4096) { return [int]$rva }
    throw "Unmapped RVA $rva"
}
function CString($p,[uint32]$rva) {
    $o=Offset $p $rva; $end=$o
    while($p.Bytes[$end] -ne 0) { $end++ }
    return [Text.Encoding]::ASCII.GetString($p.Bytes,$o,$end-$o)
}
function ExportNames($p) {
    $rva=[BitConverter]::ToUInt32($p.Bytes,$p.Optional+112)
    $names=@{}
    if(!$rva) { return $names }
    $o=Offset $p $rva
    $count=[BitConverter]::ToUInt32($p.Bytes,$o+24)
    $table=Offset $p ([BitConverter]::ToUInt32($p.Bytes,$o+32))
    for($i=0;$i -lt $count;$i++) { $names[(CString $p ([BitConverter]::ToUInt32($p.Bytes,$table+4*$i))) ]=$true }
    return $names
}
$p=ReadPE $Dll
if($p.Machine -ne 0x8664) { throw 'Wrong machine architecture' }
$exports=ExportNames $p
if(!$exports.ContainsKey('?startPlugin@@YAXXZ')) { throw 'Missing RE_Kenshi C++ entry point' }
Write-Output 'PASS: PE32+ AMD64 DLL, correct startPlugin export'
$dir=Offset $p ([BitConverter]::ToUInt32($p.Bytes,$p.Optional+120))
$total=0
while([BitConverter]::ToUInt32($p.Bytes,$dir+12)) {
    $module=CString $p ([BitConverter]::ToUInt32($p.Bytes,$dir+12))
    $path=Join-Path $KenshiDirectory $module
    if(!(Test-Path -LiteralPath $path)) { $path=Join-Path "$env:WINDIR\System32" $module }
    if(!(Test-Path -LiteralPath $path)) { throw "Dependency missing: $module" }
    $available=ExportNames (ReadPE $path)
    $lookup=[BitConverter]::ToUInt32($p.Bytes,$dir)
    if(!$lookup) { $lookup=[BitConverter]::ToUInt32($p.Bytes,$dir+16) }
    $table=Offset $p $lookup; $count=0
    while($thunk=[BitConverter]::ToUInt64($p.Bytes,$table)) {
        if($thunk -ge [uint64]::Parse('9223372036854775808')) { throw 'Ordinal import needs manual verification' }
        $name=CString $p ([uint32]$thunk+2)
        if(!$available.ContainsKey($name)) { throw "Missing import in ${module}: $name" }
        $count++; $table+=8
    }
    $total+=$count
    Write-Output "PASS: $module, $count named imports present"
    $dir+=20
}
Write-Output "PASS: all $total direct imports resolve statically. This is not an in-game load test."
