# FM26 AI Manager V0.6
Portable Windows diagnostic foundation for the FM26 AI Manager project.

## Build
GitHub Actions builds a native x64 Windows executable with the static MSVC runtime and packages it as `FM26_AI_MANAGER_V0.6_PORTABLE.zip`.

Run **Actions → Build FM26 AI Manager Portable → Run workflow**, then download the artifact.

## Current safety boundary
V0.6 is read-only: FM26 window detection, screen capture, basic screen training/classification, decision dry-run and diagnostic export. No mouse/keyboard automation and no save modification.
