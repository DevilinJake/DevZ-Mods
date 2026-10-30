<#
Writes <outDir>\meta.cpp for a built mod, with its Workshop ID from workshop_ids.txt.
Used by build.ps1 and build_pack.ps1. A mod with no ID listed gets no meta.cpp.

The publishedid in meta.cpp is what a server sends to players' launchers so they can find and
download the mod from the Workshop. Publisher writes 0 on a first upload, which breaks that.
#>
param(
    [Parameter(Mandatory)] [string] $ModName,
    [Parameter(Mandatory)] [string] $OutDir
)
$ErrorActionPreference = 'Stop'
$idsFile = Join-Path (Split-Path $PSScriptRoot) 'workshop_ids.txt'
$id = $null
foreach ($line in Get-Content $idsFile) {
    $parts = -split $line.Trim()
    if ($parts.Count -ge 2 -and $parts[0] -eq $ModName) { $id = $parts[1] }
}
if (-not $id) {
    Write-Host "No Workshop ID for $ModName in workshop_ids.txt: no meta.cpp (add it after the first upload)"
    return
}
$modCpp = Get-Content (Join-Path $OutDir 'mod.cpp') -Raw
$name = if ($modCpp -match 'name\s*=\s*"([^"]*)"') { $Matches[1] } else { "DevZ_$ModName" }
@"
protocol = 1;
publishedid = $id;
name = "$name";
"@ | Set-Content (Join-Path $OutDir 'meta.cpp') -Encoding ASCII
Write-Host "meta.cpp: publishedid = $id"
