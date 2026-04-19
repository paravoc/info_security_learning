param(
    [switch]$Gui,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Args
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $repoRoot "build"
$guiExe = Join-Path $buildDir "Release\\WindowsSecurityWorkbenchGUI.exe"
$cliExe = Join-Path $buildDir "Release\\WindowsSecurityWorkbench.exe"
$cmakeCache = Join-Path $buildDir "CMakeCache.txt"

function Get-NewestSourceWriteTimeUtc {
    $patterns = @("*.cpp", "*.h", "*.hpp", "*.cxx", "*.cmake", "CMakeLists.txt", "*.ps1")
    $files = Get-ChildItem -Path $repoRoot -Recurse -File -Include $patterns | Where-Object {
        $_.FullName -notlike "$buildDir*"
    }

    if (-not $files) {
        return [datetime]::MinValue
    }

    return ($files | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1).LastWriteTimeUtc
}

function Invoke-CMakeBuildIfNeeded {
    param(
        [string]$TargetExe
    )

    $needConfigure = -not (Test-Path $cmakeCache)
    $needBuild = $needConfigure -or -not (Test-Path $TargetExe)

    if (-not $needBuild) {
        $latestSourceTime = Get-NewestSourceWriteTimeUtc
        $targetTime = (Get-Item $TargetExe).LastWriteTimeUtc
        if ($latestSourceTime -gt $targetTime) {
            $needBuild = $true
        }
    }

    if ($needConfigure) {
        Write-Host "Configuring CMake project..."
        & cmake -S $repoRoot -B $buildDir
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configure failed with exit code $LASTEXITCODE."
        }
    }

    if ($needBuild) {
        Write-Host "Building Release binaries..."
        & cmake --build $buildDir --config Release
        if ($LASTEXITCODE -ne 0) {
            throw "CMake build failed with exit code $LASTEXITCODE."
        }
    }
}

$launchGui = $Gui.IsPresent -or $Args.Count -eq 0
$targetExe = if ($launchGui) { $guiExe } else { $cliExe }

Invoke-CMakeBuildIfNeeded -TargetExe $targetExe

if ($launchGui) {
    Write-Host "Starting GUI..."
    Start-Process -FilePath $guiExe -WorkingDirectory $repoRoot | Out-Null
    return
}

Write-Host "Starting CLI..."
& $cliExe @Args
exit $LASTEXITCODE
