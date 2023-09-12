import jmake
import shutil
from pathlib import Path

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
    workspace.libc = "mtd"

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

@jmake.postbuild(lib)
def copytonvim(project):
    path = Path.home()
    if host.os == jmake.Platform.WIN32:
        path /= "AppData/Local/nvim/bin"
    binary = Path("bin/Release/libfuzzy.dll").absolute()
    if not binary.is_file():
        print(f"could not find {str(binary)}, aborting...")
        return
    print(f"copying {str(binary)} to {path}")
    path.mkdir(exist_ok=True)
    path /= "libfuzzy.dll"
    path.touch()
    shutil.copy(binary, path)

jmake.generate(workspace)
