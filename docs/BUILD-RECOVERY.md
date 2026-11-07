# Build inputs and recovery

Verify files and access before reporting them as available. Record observations and
unverified items separately. A temporary workspace is a working copy. Keep recovery
locations and checksums with each release.

## Current release

Version 0.4.11 uses Android version code 53 and package `com.tranquil.hp1native`.

- Source commit `c9abecba39e2e5eb46e468ab964af0579c63d059`.
- The APK rebuilds the Android launcher and uses the native libraries compiled from `395b3117e8b49e63bd29dd5e1049678a4fdb4aed`.
- The GitHub release includes the APK, full source ZIP, build verification JSON and SHA-256 file.
- The APK bundles 339 HP1 files, 901 HP2 files and 766 EU HP3 files. Check every file hash after packaging.
- The full source ZIP includes SDL and a regenerated source lock. Game files, SDK packages and private signing files are separate build inputs.

## Private recovery locations

The owner's Remaster Vault stores the uploaded original game images and the verified
0.4.9 three-game APK. Release backups use their full versioned filenames. The vault's
authenticated `/api/files` route lists completed files and `/api/download/{id}`
returns a full file, with range support. Use owner-authorized access and verify
the downloaded bytes. A listed filename or size alone does not verify a backup.

The private signing archive is named
`Harry-Potter-TQ-Native-Private-Signing-Recovery.zip` and is retained in the owner's
file collection. Recover it privately. The expected certificate SHA-256 is
`25aa34c1db19a5007e934df03f5927e4b71a990856fe439a57e3e73ead8eae40`.

Never include signing keys, passwords, access tokens or original game files in a
source archive.

## Toolchain

Use Android platform 36 and Build Tools 36.0.0. Native rebuilding uses NDK r27c
(`27.2.12479018`) and CMake 3.22.1. Obtain packages from Google and verify their
published checksums. Verify the actual installation paths before using them.
A previous successful download does not prove that its temporary working copy
is still present.

## Completion checks

- Verify package/version metadata, signing continuity and native alignment.
- Verify source archive contents against the source revision and source lock.
- Match GitHub asset hashes against local release files after publication.
- Verify private recovery downloads against release checksums when backing up.
- Keep phone execution and gameplay acceptance open until supported by a phone result.
