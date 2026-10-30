<#
Builds one mod from this repo into build\@<ModName>, ready to copy onto a server.

  .\build.ps1 InventorySizes
  .\build.ps1 InventorySizes -Key C:\keys\MyServer.biprivatekey -Install C:\DayZServer
  .\build.ps1 InventorySizes -NoGenerate    (keeps a hand-edited src\config.cpp)

Steps:
  1. If the mod has generate_config.py, runs it (e.g. sizes.csv -> src\config.cpp)
  2. Packs src\ into build\@DevZ_<ModName>\addons\DevZ_<ModName>.pbo
  3. Signs the PBO with your private key and copies the matching .bikey into keys\
  4. Writes meta.cpp with the mod's Workshop ID from workshop_ids.txt (if it has one)
  5. With -Install, copies the finished mod folder into your server folder and its key into keys\

Needs Python 3 and, for signing, DayZ Tools (free on Steam).
#>
param(
    [Parameter(Mandatory)] [string] $ModName,
    [string] $Key = $env:DAYZ_SIGNING_KEY,
    [string] $Install,
    [switch] $NoGenerate,
    [string] $DayZTools = 'C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools'
)

$ErrorActionPreference = 'Stop'
$modSrc  = Join-Path $PSScriptRoot $ModName
if (-not (Test-Path (Join-Path $modSrc 'src'))) { throw "No mod folder with src\ called '$ModName'" }

$name    = "DevZ_$ModName"
$outDir  = Join-Path $PSScriptRoot "build\@$name"
$addons  = Join-Path $outDir 'addons'
$keysDir = Join-Path $outDir 'keys'
$pbo     = Join-Path $addons "$name.pbo"

$generator = Join-Path $modSrc 'generate_config.py'
if ((Test-Path $generator) -and -not $NoGenerate) {
    python -I $generator
    if ($LASTEXITCODE -ne 0) { throw 'generate_config.py failed' }
}

if (Test-Path $outDir) { Remove-Item $outDir -Recurse -Force }
New-Item -ItemType Directory -Force $addons, $keysDir | Out-Null

# The prefix is the mod's name, unless src\$PBOPREFIX$ says otherwise (BarrelStorage keeps the
# original mod's prefix, because its models point at their textures and proxies by that path)
$prefix = $name
$prefixFile = Join-Path $modSrc 'src\$PBOPREFIX$'
if (Test-Path -LiteralPath $prefixFile) { $prefix = (Get-Content -LiteralPath $prefixFile -Raw).Trim() }
python -I (Join-Path $PSScriptRoot 'tools\pbo_tool.py') pack (Join-Path $modSrc 'src') $pbo --prefix $prefix
if ($LASTEXITCODE -ne 0) { throw 'Packing failed' }
Copy-Item (Join-Path $modSrc 'mod.cpp') $outDir
& (Join-Path $PSScriptRoot 'tools\write_meta.ps1') $ModName $outDir

if ($Key) {
    # DSSignFile needs a full path
    $Key = (Resolve-Path $Key).Path
    $signer = Join-Path $DayZTools 'Bin\DsUtils\DSSignFile.exe'
    & $signer $Key $pbo
    if ($LASTEXITCODE -ne 0) { throw 'Signing failed' }
    $bikey = [IO.Path]::ChangeExtension($Key, '.bikey')
    if (Test-Path $bikey) { Copy-Item $bikey $keysDir }
    Write-Host "Signed with $Key"
} else {
    Write-Warning 'Not signed: pass -Key <your .biprivatekey> (servers with verifySignatures=2 need it)'
}

if ($Install) {
    $target = Join-Path $Install "@$name"
    if (Test-Path $target) { Remove-Item $target -Recurse -Force }
    Copy-Item $outDir $target -Recurse
    Get-ChildItem $keysDir -Filter *.bikey | ForEach-Object { Copy-Item $_.FullName (Join-Path $Install 'keys') -Force }
    Write-Host "Installed to $target"
}

Write-Host "Built $outDir"
