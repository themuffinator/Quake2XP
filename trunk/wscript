#! /usr/bin/env python
# encoding: utf-8

# TODO list
# mandatory
# - respect filesystem: f* -> FS_*, one write directory, etc
# - compare system.c and related with Yamagi Q2
# - add ogg/vorbis support (not the complete QuDos player, only for original tracks)
# performance
# - optimize Mod_LoadFaces with vectorization and/or OpenMP


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
        'client/cl_*.c',
        'client/keys.c',
        'client/menu.c',
        'client/qmenu.c',
        'client/snd_mem.c',
        'client/snd_openal.c',
        'client/snd_context.c',
        'client/music.c',
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
    for lib in ['sdl', 'ogg', 'vorbis', 'vorbisfile', 'x11', 'xxf86vm', 'IL', 'ILU', 'ILUT', 'openal']:
        conf.check_cfg(package=lib, args=['--cflags', '--libs'])

def build(bld):
    src_dir = bld.srcnode
    #src_dir = bld.path.find_dir('src')

    #bld.env.append_value('CFLAGS', ['-flax-vector-conversions'])
    #bld.env.append_value('CFLAGS', ['-O3', '-march=native'])
    #bld.env.append_value('CFLAGS', ['-O3', '-march=native', '-ftree-vectorize', '-ftree-vectorizer-verbose=2'])
    #bld.env.append_value('CFLAGS', ['-g', '-Wall'])
    bld.env.append_value('CFLAGS', ['-g'])
    #bld.env.append_value('LINKFLAGS', ['-pg'])
    #bld.env.append_value('CFLAGS', ['-fopenmp', '-g'])
    #bld.env.append_value('LINKFLAGS', ['-fopenmp'])

    # Expand source files
    sources = {}
    for k, v in sources_glob.items():
        sources[k] = []
        for pat in v:
            sources[k] += src_dir.ant_glob(pat)

    # Game shared library environment
    genv = bld.env.derive()
    genv.append_value('CFLAGS', ['-Wno-pointer-to-int-cast'])
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
        use = ['IL', 'ILU', 'ILUT', 'OPENAL', 'SDL']#, 'X11', 'XXF86VM']
    )
