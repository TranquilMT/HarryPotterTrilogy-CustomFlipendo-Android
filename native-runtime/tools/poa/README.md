# PoA EU compatibility tools

The active identity reference is `reference-profile/hp3-pc-eu.json`. No alternate regional profile or fallback is included.

The profile describes the verified EU baseline, not a complete playable port. Its missing KWGame script dependency must be recovered from the supplied EU original before packaging a complete test build.

Use inspect_packages.py for bounded package tables, startup_profile.py/build_profile.py for identity and configuration, and the production tests for the supported readers. The shared historical dependency graph has been withdrawn; generate a fresh graph from the actual EU installation.

Current fixes are recorded in docs/HP3-EU-STARTUP-IMPLEMENTATION.md at the project root.
