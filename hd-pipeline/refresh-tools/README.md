# HP1 HD Refresh recovery build

Run from a directory containing `refresh-tools/` and `texture-base/`.
Reconstruct `texture-base/` by concatenating saved v0.1 `.001` and `.002`
parts into a ZIP and extracting it. Those parts remain separate saved inputs.

Install `Pillow==12.3.0`, `numpy==2.5.4`, and
`opencv-contrib-python-headless==5.0.0.93` into your Python environment.
Run `python refresh-tools/build_refresh.py`, followed by
`python refresh-tools/package_refresh.py`.

The model is FSRCNN x4 from
https://github.com/Saafke/FSRCNN_Tensorflow/tree/master/models .
Its license is bundled. The script uses the recovered compressed DDS backup,
downsampled to the original recorded dimensions; it does not imply pristine
source recovery. A bounded neural delta avoids invented atlas layouts.
Images are not stretched to screen resolution. Alpha is preserved before
recompression. Opaque DDS use BC1, remaining DDS use BC3, all with mipmaps.

Game code and runtime are separate from this texture-only deliverable.
These scripts do not build or repair an APK. Category assignment is heuristic
and some packages contain mixed object types; every replacement path is kept.
No original packages, mesh geometry, audio or animations are modified.
