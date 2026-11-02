# Windows build for the TQ native test

`android/build_windows.py` mirrors `android/build_linux.py` for a native Windows host. It uses the
same packaging, payload verification and signing checks, with Windows tool names (`.exe`, `.bat`) and
the NDK's `windows-x86_64` strip binary.

This full-source archive includes the pinned, already-patched SurrealEngine source and SDL3 source.
Do not run `apply_patches.sh` on the archive; it expects live submodule Git metadata.

## Toolchain used for the 2026-11-02 local build

| Tool | Version / path |
|---|---|
| JDK | Temurin 21.0.13+9, portable zip |
| Android SDK | command-line tools 13114758, platform 36, build-tools 36.0.0, platform-tools |
| NDK | 27.2.12479018 |
| CMake | 4.4.4 (portable zip) |
| Ninja | 1.13.2 |
| Python | 3.12.10 |

Set `JAVA_HOME` to the JDK directory when invoking the SDK manager. The build script sets it for its
own subprocesses.

## Build

Run from `native-runtime`, replacing the paths with absolute local paths:

```bat
python android\build_windows.py ^
  --sdk C:\path\android-sdk ^
  --ndk C:\path\android-sdk\ndk\27.2.12479018 ^
  --jdk C:\path\jdk21 ^
  --cmake C:\path\cmake\bin\cmake.exe ^
  --ninja C:\path\ninja\ninja.exe ^
  --keystore C:\private\release.jks ^
  --password-file C:\private\password.txt ^
  --previous-apk C:\path\Harry-Potter-TQ-0.4.2-Both-Games.apk ^
  --game-data C:\path\game-data\hp1 ^
  --hp2-data C:\path\game-data\hp2 ^
  --version-name 0.4.4D --version-code 44 ^
  --jobs 8
```

`--source-commit` overrides the `git rev-parse HEAD` lookup in `build-info.txt`; use it when building
from an archive without `.git` (for example `--source-commit snapshot-2026-11-02`).

The permanent keystore is required for an installable update. Without it, a test build can be signed
with a throwaway key, but it will **not** install over an existing release:

```bat
  --allow-certificate-change
```

The script records both certificates and `signingCertificateChanged` in
`android/dist/build-verification.json`.

## Output

`android\dist\Harry-Potter-TQ-<version>-Both-Games.apk` plus `build-verification.json`. The build
refuses to continue if either payload is missing, any embedded payload hash differs, or (unless
allowed) the signing certificate changes from `--previous-apk`.

Native libraries are stripped with the NDK's `windows-x86_64/llvm-strip.exe`. The APK is aligned with
`zipalign -P 16` and verified for AArch64 ELF and 16 KiB load alignment before the report is written.
