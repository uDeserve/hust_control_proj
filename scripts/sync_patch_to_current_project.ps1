$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectRoot = Join-Path $repoRoot "firmware\current_project"
$patchRoot = Join-Path $repoRoot "patches\current_patch"

$mappings = @(
    @{ Source = "Src\mc_tasks.c"; Target = "Src\mc_tasks.c" },
    @{ Source = "Inc\mc_tasks.h"; Target = "Inc\mc_tasks.h" },
    @{ Source = "Src\main.c";     Target = "Src\main.c" }
)

foreach ($item in $mappings) {
    $src = Join-Path $patchRoot $item.Source
    $dst = Join-Path $projectRoot $item.Target

    if (!(Test-Path $src)) {
        throw "Patch file not found: $src"
    }

    $dstDir = Split-Path -Parent $dst
    if (!(Test-Path $dstDir)) {
        New-Item -ItemType Directory -Path $dstDir -Force | Out-Null
    }

    Copy-Item $src $dst -Force
    Write-Host "Synced patches/current_patch/$($item.Source) -> $($item.Target)"
}

Write-Host "Current project updated from patch snapshot."
