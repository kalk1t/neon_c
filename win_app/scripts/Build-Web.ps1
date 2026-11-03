[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$metadataPath=Join-Path $root 'third_party/web-toolchain.json'
if(-not(Test-Path -LiteralPath $metadataPath)){throw 'Run scripts/Install-WebToolchain.ps1 first.'}
$toolchain=Get-Content -LiteralPath $metadataPath -Raw | ConvertFrom-Json
$env:EMSDK_PYTHON=$toolchain.python
$env:EM_CONFIG=$toolchain.config
$env:EM_CACHE=$toolchain.cache
$env:PYTHONDONTWRITEBYTECODE='1'
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsInstall=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$ninja=Join-Path $vsInstall 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
if(-not(Test-Path -LiteralPath $ninja)){throw 'Visual Studio bundled Ninja not found.'}
$build=Join-Path $root 'build/web'
$emcmake=Join-Path $toolchain.sdk 'upstream/emscripten/emcmake.py'
& $toolchain.python $emcmake cmake -S $root -B $build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" -DCMAKE_BUILD_TYPE=Release -DPLATFORM=Web '-DOPENGL_VERSION=ES 3.0' "-DRR_RAYLIB_SOURCE_DIR=$($toolchain.raylib_source)"
if($LASTEXITCODE -ne 0){throw 'Web configuration failed.'}
& cmake --build $build --parallel 4
if($LASTEXITCODE -ne 0){throw 'Web compilation failed.'}
& ctest --test-dir $build --output-on-failure
if($LASTEXITCODE -ne 0){throw 'Web core reference tests failed.'}
Write-Output "Browser artifact: $build/bin/RoadstersRace.html"
