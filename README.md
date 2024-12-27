
# Simple Dreams Engine

# Requirements

## Linux

### Bazel

TODO

### Deb

```
sudo apt install           \
    libopenal-dev          \
    libaudio-dev           \
    libglfw3-dev           \
    libfreetype6-dev
```

# Todo

## App
- [ ] Add controller support

## Common
- [ ] Cleanup core utility targets
- [ ] Replace unordered_map with flat_map where possible
- [x] Replace float times with `Time` / `Duration` nanosecond time objects

## 2D rendering

- [ ] Fix test assets
- [x] Tile sets
- [x] Tile map rendering feature
- [x] Rendering to frame buffer target
- [x] Static VAO? (selectable VAO)
- [x] Rendering text
- [ ] Debug render layer (wire-frame boxes; times)
- [x] Stream-style logging
- [ ] Add static text entity with pre-computed bounds
- [ ] Camera zoom to focus
- [x] Cleanup graphics build targets
- [ ] Add drawable base and common on-draw errors
+ [ ] Add draw-layer argument
+ [ ] Add debug draw-layer

## Audio
- [x] Add basic audio facilities
- [ ] Add audio effects chains

## Systems

- [ ] "Script" interface
- [ ] Character script

---

## Running Todo

- [ ] Update script and comonent version computation to be based on IO fields
- [ ] Remove entities from registry when removed from cache
- [ ] Labels for asset browsers (tilesets, sounds, etc.)