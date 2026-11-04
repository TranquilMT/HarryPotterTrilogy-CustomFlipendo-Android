# Android startup build: verified blockers

5 November 2026. Build target: the v0.4.5-based compatibility branch `hp3/0.4.5-package-loading`, inspected at `f2f49cd5f2c9c2f842363859c23fccdee17d97fb`.

## Checks performed

- The working directory contains the inspected/modified source subset, not the complete repository checkout. The remote tree contains 3,837 file blobs. A complete checkout is required before CMake can build the engine.
- No configured Android SDK or NDK was found. `cmake`, `ninja`, `adb` and `sdkmanager` are unavailable on PATH. Java and the host compiler used by regression tests are available.
- A direct request to Google's Android SDK repository failed with `Proxy CONNECT aborted due to timeout`. No SDK/NDK installation completed.
- Desktop Commander reports that no devices are connected. There is no authorized remote build computer or attached test phone accessible through that connection.
- The repository does not contain `native-runtime/android/deps/SDL/CMakeLists.txt`. Android CMake requires that directory. The existing build-apk.sh pins SDL to `release-3.2.20` and fetches it; the prepared Linux/Windows packagers assume dependencies already exist.
- No full Android compilation or HP3 startup was attempted. There is no new APK or real game crash/startup trace from this check. Existing 97-test host results are not substituted for those gates.

## Next executable build sequence

1. Connect a build computer and locate its project checkout, SDK/NDK, signing material, original game folders and previous APK. Inspect project/build paths rather than scanning unrelated personal files.
2. Fetch the compatibility branch and verify its revision. Prepare the pinned SDL dependency and the remaining documented engine/build dependencies.
3. Configure/build the complete Android ARM64 native target and compile Java against the real SDK/SDL APIs. Fix compiler/linker failures and record exact results.
4. Package and verify the test APK, preserving the chosen signing identity and existing HP1/HP2 payload requirements. Do not generate a replacement key or claim update compatibility without checking the certificate.
5. Connect the test phone, install the verified build, select the original PoA folder and capture logcat/application reports from an actual launch attempt. Keep an unsuccessful startup distinct from first-frame/playable acceptance.
6. Use that trace to prioritize the real UE2 metadata/default/native/resource blockers. Confirm HP1/HP2 launcher and input regressions on the same build.

G0 is blocked on build inputs/access. G1–G6 remain open for their existing runtime/device requirements. This is an environment audit, not another implemented compatibility feature or completed build milestone.
