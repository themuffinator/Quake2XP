==============================================================================
Quake2XP Linux Notes
==============================================================================

1. Building and installing
2. Copying data
3. Notes
4. Contact
5. TODO

==============================================================================
1. Building and installing
==============================================================================

The following libraries are needed to compile Quake2XP.
- DevIL
- OpenGL
- OpenAL
- SDL
- Vorbisfile (which requires Ogg and Vorbis)

In Ubuntu they can be installed with the following command.
$ sudo apt-get install build-essential libvorbis-dev libdevil-dev \
  libsdl1.2-dev libopenal-dev

As the project uses the Waf build system, Python must also be present. Once
you have the mentioned packages, build and install with:

$ python waf configure
$ python waf
$ sudo python waf install

By default the installation prefix is "/usr/local", but can be changed via
arguments. In fact, Quake2XP will run from any directory because the data path
is added to the executable, and libraries are loaded at run-time. For example,
you can install it in "$HOME/local" as follows.

$ python waf configure --prefix=$HOME/local
$ python waf
$ python install

If you have the required libraries but still get an error, see below for
contact information.

You can also uninstall it with "python waf uninstall".

==============================================================================
2. Copying data
==============================================================================

Before running the program, you need to copy the following data to
"$PREFIX/share/quake2xp" (under baseq2/). Note that all EXEs are
self-extracting archives (i.e. can be extracted without Wine).

- baseq2/pak0.pak from the original Quake II CD

- baseq2 (without DLLs) from q2-3.20-x86-full.exe
  Available at ftp://ftp.idsoftware.com/idstuff/quake2/ or any mirror.

- baseq2 (without EXEs and DLLs) from the Quake2XP installer
  Available at http://sourceforge.net/projects/quake2xp/files/release/

- (optional) original CD music in Ogg format
  Available at http://forums.steampowered.com/forums/showthread.php?t=1756937
  Quake2XP expects tracks as "baseq2/music/trackNN.ogg", so renaming is
  needed (i.e. 02.ogg -> track02.ogg). You also need to select that music
  source in the options menu.

==============================================================================
3. Notes
==============================================================================

The framerate is unlimited by default, but you can adjust it with
"cl_maxfps 60" or similar. You may want to do it for smooth playing when
running background processes, for saving battery or just to avoid hearing the
GPU cooling fan.

The music system has three modes (accessible through the options menu or the
"s_musicsrc" cvar with integers from 0 to 3):
- disabled: do not play anything.
- CD-ROM: plays the appropiate tracks from an inserted CD.
- soundtrack files: play ogg/wav files with the name
  "baseq2/music/trackXX.EXT" (where XX is 02, 03, etc). They will be used as
  the original CD tracks (different for each level).
- any files: plays any ogg/wav files found in "baseq2/music".

If random playing is enabled, it should do what's expected. The command
"music" can switch tracks if playing random or any files.

==============================================================================
4. Contact
==============================================================================

If you have any problems or suggestions regarding the Linux version, feel free
to mail me at "alepulver at gmail.com".

The Quake2XP author's address is "barnes at yandex.ru".

Website: http://quake2xp.sourceforge.net/

==============================================================================
5. TODO
==============================================================================

- document important cvars (which are not in menu)
- add note about xatrix, after testing
- add note about 3zb2, after testing
- add support for Rogue expansion pack (check Yamagi Q2 and QuDos)
- add support for Zaero expansion pack (check Yamagi Q2 and QuDos)
- upload data in ZIP format or just PKX to sourceforge.net
- get launchpad account, create Ubuntu package and promote in
  forums (english and spanish)

==============================================================================
