env = Environment()


# output object files to 'bin' folder
env.binary_dir = 'bin'
env.source_dir = 'src'
env.executable_dir = "."
env.executable_name = "dwndiff"
env.VariantDir(env.binary_dir,env.source_dir,duplicate = 0)  # output to binary
# folder without copying files
env.output_executable = env.Program(env.executable_name,Glob(env.binary_dir +
                                                             '/*.c'))

# defines
env.dna_modes = ['ECOLI','SINGLE_START_CODON']
env.Append(CPPDEFINES = env.dna_modes)
# env.process_modes = ['CONCURRENT']
# env.Append(CPPDEFINES = env.process_modes)

# debug/release
debug = ARGUMENTS.get('debug',0) # defaults to debug configuration
release = ARGUMENTS.get('release',0)
if (int(debug) or not int(release)): # default to debug
    env.build_mode = 'DEBUG'
else:
    env.build_mode = 'RELEASE'
env.Append(CPPDEFINES = env.build_mode)

# extra warnings for cleanup before release
cleanup = ARGUMENTS.get('cleanup',0)
if int(cleanup):
    env.cleanup_mode = 'CLEAN'
else:
    env.cleanup_mode = 'UNCLEAN'
env.Append(CPPDEFINES = env.cleanup_mode)

# flags
env.Append(CCFLAGS = ['-std=c11'])
# optimization for debug vs release
if (env.build_mode == 'DEBUG'):
    env.Replace(CC = 'gcc')   # i KNOW it's a bad idea to change compilers for
    # debug and release, but clang is a LOT better at producing errors, and gcc
    # is better at producing faster code, which is important for this project
    env.Append(CCFLAGS = ['-O0','-ggdb','-g3','-Wall','-Wextra','-Werror'])
    if (env.cleanup_mode == 'CLEAN'):
        env.Replace(CC = 'gcc') # because most of these only work with gcc
        env.Append(CCFLAGS = ['-Wundef','-Wshadow','-Wformat=2',
                              '-Wpointer-arith','-Wcast-align',
                              '-Wstrict-prototypes','-Wswitch-default',
                              '-Wswitch-enum','-Wunused','-Wstrict-overflow=5',
                              '-Wmissing-format-attribute','-Wwrite-strings',
                              '-Wsuggest-attribute=pure','-Wc++-compat',
                              '-Wsuggest-attribute=const',
                              '-Wsuggest-attribute=noreturn','-Wtrampolines',
                              '-Wtype-limits','-Wbad-function-cast',
                              '-Wcast-qual','-Wcast-align','-Wconversion',
                              '-Wjump-misses-init','-Wlogical-op',
                              '-Wold-style-definition','-Wmissing-prototypes',
                              '-Wmissing-declarations','-Wdouble-promotion',
                              '-Wdeclaration-after-statement',
                              '-Wmissing-field-initializers','-Wpacked',
                              '-Wredundant-decls','-Wnested-externs',
                              '-Winline','-Wvector-operation-performance',
                              '-Wdisabled-optimization','-Wpointer-sign',
                              '-Wunused-parameter','-Wuninitialized'])
    else:
        env.Replace(CC = 'clang')
elif (env.build_mode == 'RELEASE'):
    env.Replace(CC = 'gcc')
    env.Append(CCFLAGS = ['-Ofast','-finline-functions',
                          '-funsafe-loop-optimizations',
                          '-Wunsafe-loop-optimizations',
                          '-funroll-loops']) # TODO: ADD MARCH OPTIONS
    env.Append(LINKFLAGS = ['-s'])           # remove symbol infos
    if (env.cleanup_mode == 'CLEAN'):
        env.Append(CCFLAGS = ['-Wdisabled-optimization'])

# add glib
# TODO: may want to explicitly use glib 2.4 later, possibly using 2.12 now
env.ParseConfig('pkg-config --cflags --libs glib-2.0')
# add gmp
env.Append(LIBS = ['gmp'])
