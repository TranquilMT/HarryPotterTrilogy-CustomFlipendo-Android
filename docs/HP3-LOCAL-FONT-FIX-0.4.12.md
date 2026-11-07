# HP3 local font startup fix — 0.4.12-test

Local test build, 7 November 2026. No GitHub upload. Version code 54.

The previous reader expected a page texture reference where HP3 EU package 129 stores the first glyph coordinate. That coordinate resolved to the Pawn class, causing the fatal UTexture cast. The native reader now reads the global glyph table, per-glyph atlas byte, and subsequent texture array for EU package versions 122/129. The bundled older fonts retain their page-first reader; versions 120/121 also preserve their extra 32-bit spacing field. Texture casts remain checked.

The font reader, writer and glyph lookup are tested against all 110 font exports in the supplied EU installation, plus synthetic remapping, multiple atlases, invalid atlas and truncated data cases, and a legacy page-first roundtrip. The compatibility suite passed 104 tests; Android packaging suite passed 28 tests. Native ARM64 engine rebuilt. APK verification independently checks signing continuity, all three game payloads, and 16 KiB native alignment.

Phone acceptance is pending: launch HP3, select language, and record the next screen or exported report. This is not a claim that HP3 reaches its menu or gameplay. Install over the existing app without uninstalling to retain saves. Keep further fixes local until the user confirms the 0.5.0 HP3 release is ready.

The full source archive includes the patched engine and updated patch representation. A Git-dependent repository audit cannot run on this extracted source tree; source archive integrity is recorded in its source lock.
