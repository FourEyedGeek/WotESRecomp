$ErrorActionPreference = 'Stop'
function Run-Git([string[]]$GitArgs) {
    & git @GitArgs
    if ($LASTEXITCODE) { throw "Git failed: $($GitArgs -join ' ')" }
}
$dependencyDir = Join-Path $PSScriptRoot 'deps/segagenesisrecomp'
$engineCommit = '9cedc86d96c4886919083e4ab2321aa22ffa4078'
if (-not (Test-Path -LiteralPath $dependencyDir)) {
    Run-Git -GitArgs @('clone','https://github.com/mstan/segagenesisrecomp.git',$dependencyDir)
    Run-Git -GitArgs @('-C',$dependencyDir,'checkout','--detach',$engineCommit)
}
$actualCommit = & git -C $dependencyDir rev-parse HEAD
if ($LASTEXITCODE -or $actualCommit -ne $engineCommit) {
    throw 'Dependency checkout is not the pinned version. Preserve your work and use a fresh deps folder.'
}
Run-Git -GitArgs @('-C',$dependencyDir,'submodule','update','--init',
    'external/m68k-recomp-core','external/z80-recomp-core','external/rbengine')
foreach ($entry in @(@($dependencyDir,'engine.patch'),
    @((Join-Path $dependencyDir 'external/m68k-recomp-core'),'m68k-core.patch'))) {
    $patchFile = Join-Path $PSScriptRoot ('patches/'+$entry[1])
    & git -C $entry[0] apply --reverse --check $patchFile 2>$null
    if ($LASTEXITCODE -eq 0) { continue }
    Run-Git -GitArgs @('-C',$entry[0],'apply','--check',$patchFile)
    Run-Git -GitArgs @('-C',$entry[0],'apply',$patchFile)
}
Write-Host 'Pinned dependencies are ready.'
