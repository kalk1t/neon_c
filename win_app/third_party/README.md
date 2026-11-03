# Dependencies

raylib is pinned to release **6.2**, source commit `02566453880e06bec97f5b8160fa6fe1e98397ca`. Official release metadata and the unmodified license were retrieved through the GitHub connector on 2026-11-02. The official Windows x64 MSVC archive was subsequently downloaded through the installer, verified, and extracted into `raylib-6.2-msvc-x64/`.

`raylib.lock.json` records the Windows x64 MSVC archive URL, size, and the release's published SHA-256. `../scripts/Install-Raylib.ps1` downloads or accepts that archive, verifies it, checks extraction paths, and extracts inside this directory. It writes `raylib-installed.json` only after locating the header and static library. No system-wide installation or administrator privileges are needed.

From `win_app/`:

```powershell
.\scripts\Install-Raylib.ps1
# Or supply a local copy of the exact official archive:
.\scripts\Install-Raylib.ps1 -ArchivePath .\downloads\raylib-6.2_win64_msvc16.zip
# Verify a supplied archive without extracting it:
.\scripts\Install-Raylib.ps1 -ArchivePath .\downloads\raylib-6.2_win64_msvc16.zip -ValidateOnly
```

The prebuilt archive is an initial native setup aid. Browser/WebAssembly builds need the matching source/toolchain, not this Windows library. A future CMake source build will pin the same immutable revision. Do not mix MinGW libraries with the MSVC build.

The native C17 dependency check passed with this package. The static raylib library uses the dynamic MSVC C runtime (`/MD`); the check matches that setting. Static raylib linkage does not imply a fully standalone CRT-free executable. See `../docs/VALIDATION.md`.

Preserve the dependency's license and bundled third-party notices in distributions. The recorded raylib license does not establish licensing for game assets or any other dependency; audit those separately before distribution.

Official sources:

- https://github.com/raysan5/raylib/releases/tag/6.2
- https://api.github.com/repos/raysan5/raylib/releases/tags/6.2
- https://github.com/raysan5/raylib/blob/02566453880e06bec97f5b8160fa6fe1e98397ca/LICENSE

