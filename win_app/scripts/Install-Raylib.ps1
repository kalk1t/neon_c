[CmdletBinding()]
param(
    [string]$ArchivePath,
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$dependencyRoot = Join-Path $projectRoot 'third_party'
$lock = Get-Content -LiteralPath (Join-Path $dependencyRoot 'raylib.lock.json') -Raw | ConvertFrom-Json
$package = $lock.windows_msvc_x64
$downloadRoot = Join-Path $projectRoot 'downloads'
$installRoot = [System.IO.Path]::GetFullPath((Join-Path $dependencyRoot "raylib-$($lock.version)-msvc-x64"))
$projectPrefix = $projectRoot.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
if (-not $installRoot.StartsWith($projectPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'Dependency output must stay inside win_app.'
}

if ($ArchivePath) {
    $resolvedArchive = (Resolve-Path -LiteralPath $ArchivePath).Path
} else {
    $resolvedArchive = Join-Path $downloadRoot $package.archive
    if (-not (Test-Path -LiteralPath $resolvedArchive -PathType Leaf)) {
        if ($ValidateOnly) { throw "No cached archive exists at $resolvedArchive. Supply -ArchivePath or run without -ValidateOnly." }
        New-Item -ItemType Directory -Path $downloadRoot -Force | Out-Null
        $partialArchive = Join-Path $downloadRoot ($package.archive + '.' + [guid]::NewGuid().ToString('N') + '.partial')
        try {
            & curl.exe --fail --location --silent --show-error --connect-timeout 10 --max-time 120 --output $partialArchive $package.url
            if ($LASTEXITCODE -ne 0) { throw "curl exited with code $LASTEXITCODE" }
            $resolvedArchive = $partialArchive
        } catch {
            throw "raylib download failed: $($_.Exception.Message). Supply the official archive with -ArchivePath. No library was installed."
        }
    }
}

$archiveFile = Get-Item -LiteralPath $resolvedArchive
if ($archiveFile.Length -ne $package.size_bytes) { throw "Archive size mismatch. Expected $($package.size_bytes) bytes; got $($archiveFile.Length). No extraction performed." }
$actualHash = (Get-FileHash -LiteralPath $resolvedArchive -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualHash -ne $package.sha256) { throw 'raylib SHA-256 mismatch. No extraction performed.' }
if ($ValidateOnly) {
    Write-Output "Verified raylib $($lock.version) Windows x64 MSVC archive (SHA-256 $actualHash)."
    return
}

if (-not $ArchivePath -and $resolvedArchive.EndsWith('.partial', [System.StringComparison]::OrdinalIgnoreCase)) {
    $cachedArchive = Join-Path $downloadRoot $package.archive
    Move-Item -LiteralPath $resolvedArchive -Destination $cachedArchive
    $resolvedArchive = $cachedArchive
}

# Validate every archive path before Expand-Archive can write any content.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead($resolvedArchive)
try {
    $installPrefix = $installRoot.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    foreach ($entry in $archive.Entries) {
        $entryPath = $entry.FullName.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
        if ([System.IO.Path]::IsPathRooted($entryPath) -or $entryPath.Contains(':')) { throw 'Archive contains a rooted path.' }
        $entryTarget = [System.IO.Path]::GetFullPath((Join-Path $installRoot $entryPath))
        if (-not $entryTarget.StartsWith($installPrefix, [System.StringComparison]::OrdinalIgnoreCase)) { throw 'Archive entry escapes dependency directory.' }
    }
} finally {
    $archive.Dispose()
}

New-Item -ItemType Directory -Path $installRoot -Force | Out-Null
Expand-Archive -LiteralPath $resolvedArchive -DestinationPath $installRoot -Force
$headers = @(Get-ChildItem -LiteralPath $installRoot -Recurse -File -Filter raylib.h)
$libraries = @(Get-ChildItem -LiteralPath $installRoot -Recurse -File -Filter raylib.lib)
if ($headers.Count -ne 1 -or $libraries.Count -ne 1) {
    throw 'Archive extracted, but expected one raylib.h and one raylib.lib. Installation is not marked ready.'
}
$installed = [ordered]@{
    version = $lock.version
    source_commit = $lock.source_commit
    archive_sha256 = $actualHash
    include_directory = $headers[0].DirectoryName
    static_library = $libraries[0].FullName
}
$installed | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $installRoot 'raylib-installed.json') -Encoding UTF8
Write-Output "Prepared raylib $($lock.version) at $installRoot"
Write-Output 'The library is ready for build integration. The game has not been compiled.'
