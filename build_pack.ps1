<#
Builds a bundle: several client mods from this repo in one mod folder, build\@DevZ_<Pack>.

  .\build_pack.ps1 Essentials -Key C:\keys\MyServer.biprivatekey
  .\build_pack.ps1 Essentials -Key C:\keys\MyServer.biprivatekey -Install C:\DayZServer

The mods in a bundle are listed in <Pack>\pack.txt, one folder name per line. Each is built
with build.ps1 (so its table is turned into config.cpp and signed as usual), then its PBO and
signature are copied into the bundle. The repo keeps every mod separate; only the build is
combined.

Don't load a bundle and the same single mods together: they're the same files.
#>
param(
    [Parameter(Mandatory)] [string] $Pack,
    [string] $Key = $env:DAYZ_SIGNING_KEY,
    [string] $Install
)

$ErrorActionPreference = 'Stop'
$packSrc = Join-Path $PSScriptRoot $Pack
$list    = Join-Path $packSrc 'pack.txt'
if (-not (Test-Path $list)) { throw "No bundle called '$Pack' (needs $list)" }
if (-not $Key) { throw 'Pass -Key <your .biprivatekey>: a bundle of client mods has to be signed' }

$name   = "DevZ_$Pack"
$outDir = Join-Path $PSScriptRoot "build\@$name"
$mods   = Get-Content $list | ForEach-Object { $_.Trim() } | Where-Object { $_ -and -not $_.StartsWith('#') }

# Build every mod first, so a failure leaves the old bundle in place
foreach ($mod in $mods) {
    & (Join-Path $PSScriptRoot 'build.ps1') $mod -Key $Key
}

if (Test-Path $outDir) { Remove-Item $outDir -Recurse -Force }
$addons  = New-Item -ItemType Directory -Force (Join-Path $outDir 'addons')
$keysDir = New-Item -ItemType Directory -Force (Join-Path $outDir 'keys')

foreach ($mod in $mods) {
    $modBuild = Join-Path $PSScriptRoot "build\@DevZ_$mod"
    Copy-Item (Join-Path $modBuild 'addons\*') $addons
    Copy-Item (Join-Path $modBuild 'keys\*.bikey') $keysDir -Force
}
Copy-Item (Join-Path $packSrc 'mod.cpp') $outDir

if ($Install) {
    $target = Join-Path $Install "@$name"
    if (Test-Path $target) { Remove-Item $target -Recurse -Force }
    Copy-Item $outDir $target -Recurse
    Get-ChildItem $keysDir -Filter *.bikey | ForEach-Object { Copy-Item $_.FullName (Join-Path $Install 'keys') -Force }
    Write-Host "Installed to $target"
}

Write-Host "Built $outDir with $($mods.Count) mods: $($mods -join ', ')"
