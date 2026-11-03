[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$PythonPath)
$ErrorActionPreference='Stop'
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$version='6.0.13'
$sdkCommit='35659964bc2762fffec0d257997fb61475f765b7'
$raylibCommit='02566453880e06bec97f5b8160fa6fe1e98397ca'
$downloads=Join-Path $root 'downloads'
New-Item -ItemType Directory -Path $downloads -Force | Out-Null
if(-not(Test-Path -LiteralPath $PythonPath -PathType Leaf)){throw 'Supply a working Python 3.8+ executable.'}
& $PythonPath --version
if($LASTEXITCODE -ne 0){throw 'Python could not start.'}

function Fetch-Source([string]$Repository,[string]$Commit,[string]$Destination){
    $repositoryName=$Repository.Split('/')[1]
    $expected=Join-Path $Destination "$repositoryName-$Commit"
    if(-not(Test-Path -LiteralPath $expected)){
        $archive=Join-Path $downloads "$repositoryName-$Commit.zip"
        & curl.exe --fail --location --silent --show-error --connect-timeout 10 --max-time 240 --output $archive "https://github.com/$Repository/archive/$Commit.zip"
        if($LASTEXITCODE -ne 0){throw "Source download failed: $Repository"}
        # The URL pins an immutable official commit over HTTPS. Record the local
        # archive digest; unlike the MSVC release, no upstream archive hash is supplied.
        (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash | Set-Content -LiteralPath ($archive+'.sha256')
        Expand-Archive -LiteralPath $archive -DestinationPath $Destination -Force
    }
    return $expected
}
$sdk=Fetch-Source 'emscripten-core/emsdk' $sdkCommit (Join-Path $root "third_party/emsdk-$version")
$raylib=Fetch-Source 'raysan5/raylib' $raylibCommit (Join-Path $root 'third_party/raylib-6.2-source')
$env:EMSDK_PYTHON=(Resolve-Path -LiteralPath $PythonPath).Path
$env:EM_CONFIG=Join-Path $sdk '.emscripten'
$env:EM_CACHE=Join-Path $root 'build/emscripten-cache'
$env:PYTHONDONTWRITEBYTECODE='1'
& $PythonPath (Join-Path $sdk 'emsdk.py') install $version
if($LASTEXITCODE -ne 0){throw 'Emscripten SDK installation failed.'}
& $PythonPath (Join-Path $sdk 'emsdk.py') activate $version
if($LASTEXITCODE -ne 0){throw 'Local SDK activation failed.'}
$metadata=[ordered]@{version=$version;sdk_commit=$sdkCommit;sdk=$sdk;raylib_source=$raylib;raylib_commit=$raylibCommit;python=$env:EMSDK_PYTHON;config=$env:EM_CONFIG;cache=$env:EM_CACHE}
$metadata | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $root 'third_party/web-toolchain.json') -Encoding UTF8
Write-Output "Prepared local WebAssembly toolchain $version. No global activation was requested."
