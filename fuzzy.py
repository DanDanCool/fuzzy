import jmake

jmake.setupenv()

workspace = jmake.Workspace('fuzzy')
workspace.lang = "c17"

jollyc = jmake.package("jollyc", "https://github.com/DanDanCool/jollyc")

lib = jmake.Project("libfuzzy", jmake.Target.SHARED_LIBRARY)
files = jmake.glob("src", "*.h") + jmake.glob("src", "*.c")
files = [ file for file in files if file != "main.c" ]
lib.add(files)

lib.depend(jollyc)

debug = lib.filter("debug")
debug["debug"] = True

test = jmake.Project("test", jmake.Target.EXECUTABLE)
test.add("src/main.c")
test.depend([jollyc])

debug = test.filter("debug")
debug["debug"] = True

workspace.add(lib)
workspace.add(test)

jmake.generate(workspace)
