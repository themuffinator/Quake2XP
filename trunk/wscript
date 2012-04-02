#! /usr/bin/env python
# encoding: utf-8

VERSION = '1.0'
APPNAME = 'test'
top = '.'
out = 'build'

# Sources
sources_glob = {
    'game' : [
        'game/*.c'
        ],
    'xsrc' : [
        'xsrc/*.c'
        ],
    '3zb2' : [
        '3zb2src97/*.c'
        ],
    'server' : [
        'common/**/*.c',
        'server/**/*.c',
        'unix/glob.c',
        'unix/hunk.c',
        'unix/misc.c',
        'unix/network.c',
        'unix/signalhandler.c',
        'unix/system.c'
        ],
    'refresh' : [
        'refresh/**/*.c',
        'sdl/input.c',
        'sdl/refresh.c',
        'common/shared/flash.c',
        'common/shared/shared.c',
        'unix/glob.c',
        'unix/hunk.c',
        'unix/misc.c',
        'unix/qgl.c'
        ],
    'client' : [
        'common/**/*.c',
        'client/**/*.c',
        'sdl/cd.c',
        'sdl/sound.c',
        'server/sv_*.c',
        'unix/glob.c',
        'unix/hunk.c',
        'unix/misc.c',
        'unix/network.c',
        'unix/signalhandler.c',
        'unix/system.c',
        'unix/vid.c'
        ]
}

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    #conf.env['cshlib_PATTERN'] = '%s.so'
    for lib in ['sdl', 'ogg', 'vorbis', 'vorbisfile', 'x11', 'xxf86vm']:
        conf.check_cfg(package=lib, args=['--cflags', '--libs'])

def build(bld):
    src_dir = bld.srcnode
    #src_dir = bld.path.find_dir('src')

    # Expand source files
    sources = {}
    for k, v in sources_glob.items():
        sources[k] = []
        for pat in v:
            sources[k] += src_dir.ant_glob(pat)

    a = bld.shlib(
        source = sources['game'],
        target = 'baseq2/game',
        )
    a.env['cshlib_PATTERN'] = '%s.so'
    # TODO: hacer copia de env con esa modificación y pasarla como parámetro en
    # ambos casos

    a = bld.shlib(
        source = sources['xsrc'],
        target = 'baseq2/xgame',
        )
    a.env['cshlib_PATTERN'] = '%s.so'

    a = bld.shlib(
        source = sources['3zb2'],
        target = '3zb2/game',
        )
    a.env['cshlib_PATTERN'] = '%s.so'

    if False:
        bld.program(
            source = sources['server'],
            target = 'q2ded',
            cflags = ['-DDEDICATED_ONLY'],
            lib = ['z', 'm', 'dl']
        )

        a = bld.shlib(
            source = sources['refresh'],
            target = 'ref_gl',
            use = ['SDL', 'X11','XXF86VM']
            #env = bld.env
        )
        a.env['cshlib_PATTERN'] = '%s.so'

        bld.program(
            source = sources['client'],
            target = 'quake2',
            lib = ['z', 'm', 'dl'],
            use = ['SDL', 'OGG', 'VORBIS', 'VORBISFILE']
        )
