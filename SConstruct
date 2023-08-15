cpppath = [
    '/System/Library/Frameworks/GLUT.framework/Headers',
    '/System/Library/Frameworks/OpenGL.framework/Headers',
    ]
libpath = []
libs = []
frameworks = ['GLUT', 'OpenGL', 'opencl']
cppflags = ['-Wall']
linkflags = []

#Sources.
cpps = Glob('*.cpp')
testCpps = Glob('test*.cpp')
for fileName in testCpps:
    cpps.remove(fileName)
srcs = [cpps]
testSrcs = [testCpps, 'particles.o', 'sph.o', 'spacialHash.o', 'gpuAccessor.o']


#Build variations.
if 'debug' in COMMAND_LINE_TARGETS:
    cppflags.append('-g')
else:
    cppflags.append('-O3')
    cppflags.append('-D NDEBUG')
if 'test' in COMMAND_LINE_TARGETS:
    libs.append('cppUnitLite')

mp = Program('main.out', srcs, CPPPATH=cpppath, FRAMEWORKS=frameworks, CPPFLAGS=cppflags, LINKFLAGS=linkflags, LIBPATH=libpath, LIBS=libs)
tp = Program('test.out', testSrcs, CPPPATH=cpppath, FRAMEWORKS=frameworks, CPPFLAGS=cppflags, LIBPATH=libpath, LIBS=libs)
run = Command(None, 'main.out', './main.out > result.txt')
test = Command(None, 'test.out', './test.out')

#doc = Command(None, [Glob('*.cpp'), Glob('*.h')], '/opt/local/bin/doxygen')

Alias('build', mp)
Alias('debug', mp)
#Alias('doc', doc)

Default(run)
