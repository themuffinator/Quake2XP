==============================================================================
Quake2XP Linux Notes
==============================================================================

1. Building
2. Installing
3. Notes
4. Contact

==============================================================================
1. Building
==============================================================================

The following libraries are needed:
- DevIL
- OpenGL
- OpenAL
- SDL
- Vorbisfile (which requires Ogg and Vorbis)

In Ubuntu they can be installed with the following command:
$ sudo apt-get install build-essential libvorbis-dev libdevil-dev \
  libsdl1.2-dev libopenal-dev

As the project uses the Waf build system, Python is also needed. Once you have
the necessary tools, run the following commands:
$ python waf configure
$ python waf

By default the installation prefix is "/usr/local", but can be changed via:
$ python waf configure --prefix=/usr

==============================================================================
2. Installing
==============================================================================

After configuring and building, the following command installs the program:

$ sudo python waf install

Then you need to copy or symlink the original Quake II data (only
pak[0-2].pak) to "${PREFIX}/share/quake2xp".

You can also uninstall it with the "uninstall" target.

==============================================================================
3. Notes
==============================================================================

document important cvars (not in menu) here
document music subsystem
document how to install update from IdSoftware's FTP
add something about the cache
...

==============================================================================
4. Contact
==============================================================================

If you have any problems or suggestions regarding the Linux version, feel free
to mail me at "alepulver at gmail.com".

The Quake2XP author's address is "barnes at yandex.ru".

Website: http://quake2xp.sourceforge.net/

==============================================================================
