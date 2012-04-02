#! /usr/bin/env python
# encoding: utf-8

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
    'server' : [
        'qcommon/*.c',
        'server/*.c',
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
        'client/*.c',
        'qcommon/*.c',
        'server/*.c',
        ]
}

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    #conf.env['cshlib_PATTERN'] = '%s.so'
    for lib in ['sdl', 'ogg', 'vorbis', 'vorbisfile', 'x11', 'xxf86vm', 'IL', 'ILU']:
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
        source = sources['xatrix'],
        target = 'xatrix/game',
        )
    a.env['cshlib_PATTERN'] = '%s.so'

    a = bld.shlib(
        source = sources['3zb2'],
        target = '3zb2/game',
        )
    a.env['cshlib_PATTERN'] = '%s.so'

    if False:
        a = bld.shlib(
            source = sources['refresh'],
            target = 'ref_gl',
            use = ['SDL', 'X11','XXF86VM']
            #env = bld.env
        )
        a.env['cshlib_PATTERN'] = '%s.so'

        bld.program(
            source = sources['server'],
            target = 'q2ded',
            cflags = ['-DDEDICATED_ONLY'],
            lib = ['z', 'm', 'dl']
        )

    bld.program(
        source = sources['client'],
        target = 'quake2',
        lib = ['z', 'm', 'dl'],
        use = ['IL', 'ILU']
    )
