# Optional mod-transmog bridge for City Life

This small patch adds the provider API used by City Life's temporary Legendary cosmetics. City Life
keeps the real level-80 BiS items, their stats, enchants and gems; the provider displays the configured
T6 armor and Legendary weapon appearances over them.

The patch changes only three upstream source files. It is not a replacement fork of mod-transmog.

## Verified base

- Upstream: [azerothcore/mod-transmog](https://github.com/azerothcore/mod-transmog).
- Verified patch baseline:
  [`327753a`](https://github.com/azerothcore/mod-transmog/tree/327753a05b9024f2a90738bb08e9b8afc2fc9a21),
  dated 2026-10-10.
- Patch: [patches/mod-transmog-city-life.patch](../patches/mod-transmog-city-life.patch).
- SHA-256: `da2983d43b9ef2388e91368375a2d8ea0a3bf3cd3def6f424b4ed271be7c92e7`.

All three unpatched source files were verified byte-for-byte against that revision. In an isolated
checkout, `git apply --check`, patch application, reverse application checking and whitespace checking
passed. Applying the patch reproduced the installed provider source byte-for-byte.
[Verification data](../patches/mod-transmog-city-life.verification.json) records the individual source hashes.

Other mod-transmog revisions have not been tested. Always run the application check on your own
checkout; keep your existing local changes backed up and resolve any conflicting hunks before applying.

## Apply

Use the matching City Life update that contains `CityLifeLegendaryTransmog.cpp` and `late.cmake`.
From the root of your existing mod-transmog checkout:

```sh
cd /path/to/AzerothCore/modules/mod-transmog
git apply --check ../mod-playerbots-city-life/patches/mod-transmog-city-life.patch
git apply ../mod-playerbots-city-life/patches/mod-transmog-city-life.patch
```

These commands assume the two module checkouts are siblings inside `modules/`. If you downloaded the
patch elsewhere, use its absolute path instead. The commands also work in PowerShell with Windows
paths; quote paths containing spaces.

Regenerate your usual AzerothCore CMake project and rebuild after applying the source patch. The
optional bridge requires City Life and mod-transmog to use static linkage, for example the standard
`MODULES=static` setup. Keep the regular mod-transmog database installation; this patch adds no SQL
migration or schema.

Normal City Life, questing, PvP, fishing and Beggar features do not require this patch or mod-transmog.
If the provider is missing, disabled, unpatched or dynamically linked, City Life still uses the real
BiS equipment and leaves its optional cosmetics inactive. Turning off the server's global transmog
setting also leaves actual BiS appearances visible.

To remove the patch from an otherwise unchanged checkout:

```sh
git apply --reverse --check ../mod-playerbots-city-life/patches/mod-transmog-city-life.patch
git apply --reverse ../mod-playerbots-city-life/patches/mod-transmog-city-life.patch
```

Then regenerate and rebuild normally.

## Scope

The bridge accepts only the owner's exact temporary item GUIDs already present in City Life's
equipment recovery journal. Registration happens before equipment hooks run. Temporary items are
excluded from automatic and retroactive appearance collection; existing earned items and unlocked
collections are preserved.

Cosmetics exist only in the provider's memory cache. The bridge does not write fake-entry database
rows, create source appearance items, charge money, change item stats or relax normal player transmog
restrictions. Temporary cache entries are removed during equipment restoration and logout. The cached
recovery journal also protects temporary items encountered during login recovery.

City Life integration created by iCore. Original mod-transmog code remains credited to its upstream
authors and retains its existing license.
