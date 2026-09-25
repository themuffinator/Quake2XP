<h1 align="center">Quake2XP</h1>

<p align="center">
  <strong>A multi-platform graphics port of id Software's Quake II.</strong><br>
  Completely updated rendering that takes full advantage of modern graphics cards, while preserving the original style of the game.
</p>

<p align="center">
  <a href="https://sourceforge.net/projects/quake2xp/"><img src="https://img.shields.io/badge/upstream-SourceForge-FF6600?logo=sourceforge&logoColor=white" alt="SourceForge"></a>
  <img src="https://img.shields.io/badge/license-GPLv2-blue" alt="License: GPLv2">
  <img src="https://img.shields.io/badge/language-C-555555?logo=c&logoColor=white" alt="Language: C">
  <img src="https://img.shields.io/badge/renderer-OpenGL%20%2B%20GLSL-5586A4?logo=opengl&logoColor=white" alt="Renderer: OpenGL + GLSL">
  <img src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20FreeBSD-lightgrey" alt="Platforms: Windows, Linux, FreeBSD">
</p>

<p align="center">
  <img src="https://a.fsdn.com/con/app/proj/quake2xp/screenshots/q2xp0000.jpg/max/max/1" width="49%" alt="Quake2XP screenshot 1">
  <img src="https://a.fsdn.com/con/app/proj/quake2xp/screenshots/q2xp0001.jpg/max/max/1" width="49%" alt="Quake2XP screenshot 2">
</p>

<p align="center">
  <a href="https://www.youtube.com/watch?v=aTbXzVfUqcE">
    <img src="https://img.youtube.com/vi/aTbXzVfUqcE/hqdefault.jpg" width="480" alt="Watch: quake2xp 1.26.4 gameplay">
  </a><br>
  <sub>▶ <a href="https://www.youtube.com/watch?v=aTbXzVfUqcE">quake2xp 1.26.4 gameplay</a> on YouTube</sub>
</p>

---

## Features

### Lighting and materials
- Real-time per-pixel lighting and shadowing (in the style of Doom 3)
- High-quality parallax mapping (relief mapping), including self-shadowing parallax
- Oren-Nayar diffuse with GGX specular BRDF, plus Phong, Lambert and subsurface scattering lighting models
- Radiosity normal mapping with high-resolution lightmaps
- Cubemap and caustic light filters
- Light coronas
- 2D lighting for HUD digits, console background and menu tags
- In-game light editor

### Rendering
- Editable GLSL shaders
- sRGB render buffer
- Reflective (screen-space local reflections) and refractive surfaces
- Advanced decal and particle system with infinite decals and soft particles
- MD3 model support (without tags; Quake III player models are not supported)
- SLI friendly

### Post-processing
FXAA 3.11 · Bloom · Depth of field · Radial blur · Thermal vision · Film grain · Brightness / contrast / saturation · Motion blur · SSAO · Scanline and cinematic filters

### Audio
- OpenAL 3D audio engine with EFX high-quality environmental effects (Windows and Linux)
- OGG and WAV music playback from disk

### Input
- Raw mouse input *(Windows only)*
- Xbox 360 controller support *(Windows only)*

### Game support
- Native support for the mission packs **The Reckoning** (Xatrix) and **Ground Zero** (Rogue)
- All other add-ons and mods run in compatibility mode

## Platforms

| Platform | Status |
| --- | --- |
| Windows | Supported (Visual Studio solution) |
| Linux | Supported (waf build) |
| FreeBSD | Experimental |

## Repository layout

This repository mirrors the complete SourceForge SVN tree.

| Path | Contents |
| --- | --- |
| [`trunk/`](trunk) | Engine source and build files |
| &nbsp;&nbsp;↳ [`client/`](trunk/client), [`server/`](trunk/server), [`qcommon/`](trunk/qcommon), [`renderer/`](trunk/renderer) | Core engine |
| &nbsp;&nbsp;↳ [`win64/`](trunk/win64), [`linux/`](trunk/linux) | Platform layers |
| &nbsp;&nbsp;↳ [`game/`](trunk/game), [`xsrc/`](trunk/xsrc), [`roguesrc/`](trunk/roguesrc), [`zaero/`](trunk/zaero) | Game modules: Quake II, The Reckoning, Ground Zero, Zaero |
| &nbsp;&nbsp;↳ [`steamLauncher/`](trunk/steamLauncher) | Steam launcher (Windows) |
| [`glsl/`](glsl) | GLSL shader programs used by the renderer |
| [`maps/`](maps), [`mapsx/`](mapsx), [`mapsr/`](mapsr) | Relight light definitions (`.xplit`) for Quake II, The Reckoning and Ground Zero |
| [`tools/`](tools) | Map compilers (`qbsp3`, `qvis3`, `qrad3`, `bspinfo3`) |
| [`importmd2.ms`](importmd2.ms), [`md2tagExport.ms`](md2tagExport.ms) | 3ds Max scripts for MD2 import and tag export |
| [`branch/`](branch) | Older snapshot of the tree (last changed r243, June 2012) |

## Building

### Windows

Open [`trunk/quake2xp.sln`](trunk/quake2xp.sln) in **Visual Studio 2022** (platform toolset v143, Windows 10 SDK) and build. The solution contains the engine (`quake2xp`), the game modules (`game`, `xatrix`, `zaero`) and `steamLauncher`.

### Linux

Install the dependencies: OpenGL, OpenAL (≥ 1.14), SDL 1.2, Vorbisfile (Ogg/Vorbis) and libcurl.

```sh
# Ubuntu / Debian
sudo apt-get install build-essential libvorbis-dev libsdl1.2-dev libopenal-dev libcurl4-openssl-dev

# Fedora
sudo dnf install -y SDL-devel libvorbis-devel openal-soft-devel
```

Then build and install from `trunk/`:

```sh
cd trunk
./waf configure            # optionally: --prefix=$HOME/local  (default: /usr/local)
./waf
sudo ./waf install         # uninstall with: ./waf uninstall
```

See [`trunk/Readme_Linux.txt`](trunk/Readme_Linux.txt) for data layout, expansion packs, mods, music modes and troubleshooting.

## Game data

Quake2XP requires the original Quake II game data, which is **not** included in this repository. You need:

1. `baseq2/pak0.pak` from the original Quake II CD
2. `baseq2/pak1.pak` and `baseq2/pak2.pak` from the official 3.20 point release (`q2-3.20-x86-full.exe`)
3. The Quake2XP data packs (`.pkx`) from the [Quake2XP download](https://disk.yandex.ru/d/HqIZgWA9I_T5JQ), placed in `baseq2/`, `xatrix/` or `rogue/` as appropriate
4. The shaders: copy [`glsl/`](glsl) into `baseq2/`

The relight files in [`maps/`](maps), [`mapsx/`](mapsx) and [`mapsr/`](mapsr) are also installed manually.

To play the mission packs, copy their `pak*.pak` files and `video/` folder into sibling `xatrix/` and `rogue/` folders next to `baseq2/`, then launch with:

```sh
quake2xp +set game xatrix
quake2xp +set game rogue
```

## About this repository

This is a full-history git port of the upstream Subversion repository at [`svn.code.sf.net/p/quake2xp/code`](https://sourceforge.net/p/quake2xp/code/HEAD/tree/). Every SVN revision from r1 (April 2011) to r1542 (July 2025) is a separate commit with its original author and date. Each commit message ends with a `git-svn-id` line giving its SVN revision number.

In a checkout that has the original `git svn` import configured, newer upstream revisions can be pulled in with:

```sh
git svn fetch
git merge svn/git-svn
```

## Links

- **Project page:** [sourceforge.net/projects/quake2xp](https://sourceforge.net/projects/quake2xp/)
- **Downloads:** [sourceforge.net/projects/quake2xp/files](https://sourceforge.net/projects/quake2xp/files/)
- **ModDB:** [moddb.com/mods/quake-2-xp](https://www.moddb.com/mods/quake-2-xp)
- **Bug tracker:** [sourceforge.net/p/quake2xp/bugs](https://sourceforge.net/p/quake2xp/bugs/)
- **Discord:** [English](https://discord.gg/ncq3C2r) · [Russian](https://discord.gg/q9CeNH6)

## Credits

Quake2XP is developed by **Kirk Barnes**. Other SVN contributors: alepulver, krigssvin, danfe and przenziu.

Quake II is © id Software.

## License

Quake2XP is released under the **GNU General Public License, version 2** (GPLv2), as is the Quake II source code it is based on. The Ground Zero game source in [`trunk/roguesrc/`](trunk/roguesrc) is under id Software's *Limited Program Source Code License*; see [`trunk/roguesrc/LICENSE`](trunk/roguesrc/LICENSE).
