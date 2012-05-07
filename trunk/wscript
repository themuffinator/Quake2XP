#! /usr/bin/env python
# encoding: utf-8

# Notes
# - host_speed reports almost always zero for anything else than ref,
#   so the renderer greatly dominates frame time
# - renderer frame seems to take less when using cl_maxfps, which makes
#   sense as some frames fit on a timeslice uninterrupted by the scheduler
#   because it uses usleep() often (and then is where it pauses)
# - there are some commands not in menu: {hi,medium,low}_spec
# - cl_maxfps works fine at 60, and uses usleep() to wait
#
# TODO list
#
# important
# - upload data in ZIP format or just PKX to sourceforge.net
# - get launchpad account, create Ubuntu package and promote in
#   forums (english and spanish)
# - add cache for GL_Resample...
# - write README_linux.txt with notes and compilation instructions
#
# other/maybe
# - add support for Rogue expansion pack (check Yamagi Q2 and QuDos)
# - add support for Zaero expansion pack (check Yamagi Q2 and QuDos)


VERSION = '1.0'
APPNAME = 'quake2xp'
top = '.'
out = 'build'

# Sources
sources_glob = {
    'game' : [
        'game/*.c'
        ],
    'xatrix' : [
        'xsrc/*.c'
        ],
    '3zb2' : [
        '3zb2src97/*.c'
        ],
    'client' : [
        'game/q_shared.c',
        'client/*.c',
        'qcommon/*.c',
        'server/*.c',
        'ref_gl/*.c',
        'linux/*.c',
        'win32/r_qglwin.c',
        'win32/vid_menu.c',
        'game/m_flash.c'
        ]
}

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    for lib in ['sdl', 'ogg', 'vorbis', 'vorbisfile', 'IL', 'ILU', 'ILUT', 'openal']:
        conf.check_cfg(package=lib, args=['--cflags', '--libs'])

def build(bld):
    src_dir = bld.srcnode
    #src_dir = bld.path.find_dir('src')

    bld.env.append_value('CFLAGS', ['-O3', '-march=native'])
    #bld.env.append_value('CFLAGS', ['-g', '-Wall'])
    #bld.env.append_value('LINKFLAGS', ['-pg'])
    #bld.env.append_value('LINKFLAGS', ['-pg'])

    # Expand source files
    sources = {}
    for k, v in sources_glob.items():
        sources[k] = []
        for pat in v:
            sources[k] += src_dir.ant_glob(pat)

    # Game shared library environment
    genv = bld.env.derive()
    genv.cshlib_PATTERN = genv.cshlib_PATTERN.replace('lib', '')

    bld.shlib(
        source = sources['game'],
        target = 'baseq2/gamexp',
        env = genv
        )

    bld.shlib(
        source = sources['xatrix'],
        target = 'xatrix/gamexp',
        env = genv
        )

    bld.shlib(
        source = sources['3zb2'],
        target = '3zb2/gamexp',
        env = genv
        )

    bld.program(
        source = sources['client'],
        target = 'quake2xp',
        lib = ['z', 'm', 'dl'],
        use = ['IL', 'ILU', 'ILUT', 'OPENAL', 'SDL', 'OGG', 'VORBIS', 'VORBISFILE']
    )
