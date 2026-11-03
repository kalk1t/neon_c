[CmdletBinding()]
param([switch]$Web)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$source=Join-Path $root 'tools/race-rules-check'
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsInstall=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if($LASTEXITCODE -ne 0 -or -not $vsInstall){throw 'Visual Studio C tools not found.'}
if($Web){
    $toolchain=Get-Content -LiteralPath (Join-Path $root 'third_party/web-toolchain.json') -Raw | ConvertFrom-Json
    $env:EMSDK_PYTHON=$toolchain.python
    $env:EM_CONFIG=$toolchain.config
    $env:EM_CACHE=$toolchain.cache
    $env:EMCC_TEMP_DIR=Join-Path $root 'build/headless-web-temp'
    New-Item -ItemType Directory -Path $env:EMCC_TEMP_DIR -Force | Out-Null
    $env:PYTHONDONTWRITEBYTECODE='1'
    $ninja=Join-Path $vsInstall 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
    $build=Join-Path $root 'build/parity-headless-web'
    & $toolchain.python (Join-Path $toolchain.sdk 'upstream/emscripten/emcmake.py') cmake -S $source -B $build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" -DCMAKE_BUILD_TYPE=Release
    if($LASTEXITCODE -ne 0){throw 'Headless WebAssembly configure failed.'}
    & cmake --build $build --parallel 2
    if($LASTEXITCODE -ne 0){throw 'Headless WebAssembly build failed.'}
    & ctest --test-dir $build --output-on-failure
}else{
    $devCommand=Join-Path $vsInstall 'Common7/Tools/VsDevCmd.bat'
    $build=Join-Path $root 'build/parity-headless-native'
    foreach($value in @($source,$build,$devCommand)){
        if($value -match '[&|<>^%!"\r\n]'){throw 'Unsupported build path metacharacter.'}
    }
    # Embedded debug info avoids the sandbox's unavailable PDB manager during
    # CMake's Debug compiler probe; this does not change system configuration.
    $command='call "{0}" -no_logo -arch=x64 -host_arch=x64 && cmake -S "{1}" -B "{2}" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_FLAGS_DEBUG=/Z7 /Od /RTC1" && cmake --build "{2}" && ctest --test-dir "{2}" --output-on-failure' -f $devCommand,$source,$build
    & cmd.exe /d /c $command
}
if($LASTEXITCODE -ne 0){throw 'Headless validation failed.'}
Write-Output 'Headless checks only. No native game, raylib/OpenGL window, or graphics browser artifact was launched.'
