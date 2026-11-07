$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Install Visual Studio Desktop development with C++.' }
$vsVersion = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
$generator = if ($vsVersion.StartsWith('18.')) { 'Visual Studio 18 2026' } else { 'Visual Studio 17 2022' }
$cmake = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
if (-not (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'deps/segagenesisrecomp'))) {
    throw 'Run setup-deps.ps1 first.'
}
& $cmake -S $PSScriptRoot -B "$PSScriptRoot/build" -G $generator -A x64 -DWOTES_TEST_PARTY_STATS=OFF
if ($LASTEXITCODE) { throw 'Configuration failed.' }
& $cmake --build "$PSScriptRoot/build" --config Release --target WotESRecomp -- /m:1 /nr:false
if ($LASTEXITCODE) { throw 'Build failed.' }
