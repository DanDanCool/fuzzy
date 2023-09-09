import jmake

jmake.setupenv()

workspace = jmake.Workspace('fuzzy')
workspace.lang = "c17"

jollyc = jmake.package("jollyc", "https://github.com/DanDanCool/jollyc")

lib = jmake.Project("libfuzzy", jmake.Target.SHARED_LIBRARY)
files = jmake.glob("src", "*.h") + jmake.glob("src", "*.c")
files = [ file for file in files if "main.c" not in file ]
lib.add(files)

lib.depend(jollyc)

lib.define("JOLLY_EXPORT", 1)

host = jmake.Host()
if host.os == jmake.Platform.WIN32:
    lib.define("JOLLY_WIN32", 1)
    lib.compile("/experimental:c11atomics")

debug = lib.filter("debug")
debug["debug"] = True

test = jmake.Project("test", jmake.Target.EXECUTABLE)
test.add("src/main.c")
test.depend(lib)
test.depend(jollyc)

debug = test.filter("debug")
debug["debug"] = True

workspace.add(lib)
workspace.add(test)

jmake.generate(workspace)
