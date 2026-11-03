[CmdletBinding()]
param(
    [ValidateSet('Release','RelWithDebInfo','Debug')][string]$Configuration='RelWithDebInfo',
    [switch]$SmokeTest,
    [switch]$Run,
    [switch]$Package
)
$ErrorActionPreference='Stop'
$projectRoot=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if(-not(Test-Path -LiteralPath $vswhere)){throw 'Visual Studio Installer/vswhere.exe is missing.'}
$vsInstall=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if($LASTEXITCODE -ne 0 -or -not $vsInstall){throw 'Visual Studio C/C++ tools were not found.'}
$devCommand=Join-Path $vsInstall 'Common7/Tools/VsDevCmd.bat'
$buildRoot=Join-Path $projectRoot "build/windows-$Configuration"
# All paths are local fixed project/toolchain paths; reject cmd metacharacters.
foreach($value in @($projectRoot,$buildRoot,$devCommand)){
    if($value -match '[&|<>^%!"\r\n]'){throw 'Build paths contain unsupported cmd metacharacters.'}
}
$command='call "{0}" -no_logo -arch=x64 -host_arch=x64 && cmake -S "{1}" -B "{2}" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE={3} && cmake --build "{2}" && ctest --test-dir "{2}" --output-on-failure' -f $devCommand,$projectRoot,$buildRoot,$Configuration
& cmd.exe /d /c $command
if($LASTEXITCODE -ne 0){throw "Build or tests failed (exit $LASTEXITCODE)."}
$app=Join-Path $buildRoot 'bin/RoadstersRace.exe'
Write-Output "Application: $app"
if($Package){
    $packageRoot=Join-Path $projectRoot 'dist/first-flight'
    & cmake --install $buildRoot --prefix $packageRoot
    if($LASTEXITCODE -ne 0){throw 'Packaging failed.'}
    Write-Output "Runnable folder: $packageRoot"
}
if($SmokeTest){ & $app --smoke-test; if($LASTEXITCODE -ne 0){throw 'Graphics smoke test failed.'} }
elseif($Run){ & $app }
