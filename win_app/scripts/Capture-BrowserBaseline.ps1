[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$browserRoot = [System.IO.Path]::GetFullPath((Join-Path $projectRoot '../game'))
$referenceRoot = Join-Path $projectRoot 'reference'

function Read-BrowserGit([string[]]$GitArguments) {
    $result = & git -c "safe.directory=$browserRoot" -C $browserRoot @GitArguments
    if ($LASTEXITCODE -ne 0) { throw "Cannot read browser Git metadata: $GitArguments" }
    return ($result -join "`n")
}

$commitBefore = Read-BrowserGit @('rev-parse', 'HEAD')
$statusBefore = Read-BrowserGit @('status', '--porcelain=v1')
$paths = @(
    'package.json', 'package-lock.json', 'index.html', 'vite.config.js',
    'firestore.rules', 'functions/index.js', 'functions/rules.js',
    'server/live.js', 'README.md', 'docs/feature-inventory.md'
)
foreach ($directory in @('src', 'public')) {
    $paths += Get-ChildItem -LiteralPath (Join-Path $browserRoot $directory) -Recurse -File | ForEach-Object {
        $_.FullName.Substring($browserRoot.Length + 1).Replace('\', '/')
    }
}
$files = @($paths | Sort-Object -Unique | ForEach-Object {
    $absolutePath = Join-Path $browserRoot $_
    $info = Get-Item -LiteralPath $absolutePath
    [ordered]@{ path = $_; bytes = $info.Length; sha256 = (Get-FileHash -LiteralPath $absolutePath -Algorithm SHA256).Hash.ToLowerInvariant() }
})
# Dirty files may change without their Git status text changing. Check content twice.
foreach ($file in $files) {
    $checkedHash = (Get-FileHash -LiteralPath (Join-Path $browserRoot $file.path) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($checkedHash -ne $file.sha256) { throw "Browser content changed during capture: $($file.path). No reference written." }
}
$commitAfter = Read-BrowserGit @('rev-parse', 'HEAD')
$statusAfter = Read-BrowserGit @('status', '--porcelain=v1')
if ($commitBefore -ne $commitAfter -or $statusBefore -ne $statusAfter) {
    throw 'Browser revision or working-tree status changed during capture. No reference written; retry at a stable point.'
}
$package = Get-Content -LiteralPath (Join-Path $browserRoot 'package.json') -Raw | ConvertFrom-Json
$baseline = [ordered]@{
    schema_version = 1
    captured_utc = [DateTime]::UtcNow.ToString('o')
    source_directory = '../game'
    git_commit = $commitBefore
    working_tree_clean = [string]::IsNullOrEmpty($statusBefore)
    working_tree_status = $statusBefore
    content_hashes_verified_twice = $true
    package_version = $package.version
    user_stated_baseline = '0.2.1'
    purpose = 'Observed source reference for planning and initial parity work; not confirmation of the deployed version or a frozen product release.'
    excludes = @('.env*', 'credentials', 'node_modules', 'dist', 'Git internal files', 'live deployment')
    files = $files
}
New-Item -ItemType Directory -Path $referenceRoot -Force | Out-Null
$outputPath = Join-Path $referenceRoot 'browser-baseline.json'
$baseline | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $outputPath -Encoding UTF8
Write-Output "Captured $($files.Count) source/content hashes from browser $($package.version), commit $commitBefore."
Write-Output "Working tree clean: $($baseline.working_tree_clean). Reference: $outputPath"
