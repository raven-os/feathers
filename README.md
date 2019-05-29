# Feathers

Feathers is a compositor for raven-os.

## Build instructions

```bash
mkdir -p build/<build_type>
cd build/<build_type>
cmake ../.. -DCMAKE_BUILD_TYPE=<BuildType> # cmake wants a CamelCase value, so "Debug" for debug, "Release" for release etc.
make
```

## Run instructions

Feathers can be run in TTY or inside another window manager. Simply run the executable.
If you're a frenchman like us, you may need to change the env to have the right keyboard:
```
XKB_DEFAULT_LAYOUT=fr ./build/<build_type>/feathers
```

## Shortcuts

These are the current shortcuts:
- Alt+Return: Open terminal
- Alt+F2: Toggle fullscreen
- Alt+E: Change parent container tilling direction
- Alt+Space: Toggle floating mode
- Alt+Tab: Switch window (will be removed when movement key shortcuts are added)
- Alt+Escape: Quit feathers 

## Albinos integration

To work, the albinos service must be started **before** the compositor. This may be improved in the future.
Feathers will create to files "configKey.txt" and "readonlyConfigKey.txt" which contain the keys to both configurations.

You can edit settings using the albinos editor. See the albinos project for more details (well, if they gave them, that is).
