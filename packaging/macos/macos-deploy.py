#!/usr/bin/env python3

import argparse
import os
import shutil
import pathlib
import stat
import subprocess


def main():
    parser = argparse.ArgumentParser(prog='macos deploy',
                                     description='Utility to create the krita.app bundle',
                                     epilog="osxdeploy does not sign the resulting bundle")

    parser.add_argument('--buildroot', help="Directory where krita src and _install are located",
                        default=os.getenv("BUILDROOT", False))
    parser.add_argument('--install-dir', dest='install_dir', help="Path of install directory to deploy")
    parser.add_argument('--output-dir', dest='output_dir', help="Destination path to place the app")
    parser.add_argument('--krita-source', dest='source', help="source location of krita")
    args = parser.parse_args()

    # --- Locations
    if args.buildroot:
        if args.install_dir:
            print("WARNING: --install_dir ignored as --buildroot or env BUILDROOT is present")
        if args.source:
            print("WARNING: --krita_source ignored as --buildroot or env BUILDROOT is present")

        krita_root =pathlib.Path(args.buildroot).resolve()
        krita_install_dir = pathlib.Path(os.path.join(krita_root, "_install"))
        krita_source_dir = pathlib.Path(os.path.join(krita_root, "krita"))
        krita_dmg = pathlib.Path(os.path.join(krita_root, "_dmg"))

        if args.output_dir:
            krita_dmg = pathlib.Path(args.output_dir).resolve()

    else:
        if not args.install_dir or not args.output_dir or not args.source:
            print("ERROR: if --builroot or env BUILDROOT is missing all --install_dir, --output_dir and --krita_source must be present")
            exit(1)

        krita_install_dir = pathlib.Path(args.install_dir).resolve()
        krita_source_dir = pathlib.Path(args.source).resolve()
        krita_dmg = pathlib.Path(args.output_dir).resolve()


    kritaDeploy(krita_install_dir, krita_dmg, krita_source_dir)


# --- helpers
class DeployCmd:
    # achmod appends the mod to file
    achmod = lambda target, mode: os.lchmod(target, stat.S_IMODE(os.stat(target).st_mode) | mode)
    # xchmod removes the bit flags from file
    xchmod = lambda target, mode: os.lchmod(target, stat.S_IMODE(os.stat(target).st_mode) & ~mode)


def cmdLog(cmd: list):
    print(f'## RUNNING: {" ".join(map(lambda m: str(m), cmd))}')
    return


# cmdline wrapper for install_name_tool
def installNameTool(path: pathlib.Path, args: str, check=False):
    cmd = ['install_name_tool'] + args.split() + [path]
    cmdLog(cmd)
    subprocess.run(cmd,check=check)
    return


def copyDirSub(src: pathlib.Path, dst: pathlib.Path, extra_args: list=None
               , only_contents: bool=True
               , capture_output=True):
    if only_contents:
        src = str(src) + os.sep
    cmd = ['rsync','-priul']
    if extra_args:
        cmd.extend(extra_args)
    cmd.extend([src,dst])
    cmdLog(cmd)
    subprocess.run(cmd,text=True, check=True,capture_output=capture_output)


# avois using third party packages and rely on system utils
def isBinary(file: pathlib.Path) -> bool:
    # stderror is always empty
    # print(f"f:{file}")
    result = subprocess.run(['file', file], capture_output=True,text=True).stdout
    if "Mach-O" in result:
        return True
    return False


# get the base lib name to copy from
# return either the framework dir base name or lib name
def normalizeLibNames(lib: pathlib.Path) -> str:
    norm_name: str = next((f.name for f in lib.parents if f.name.endswith(".framework")), lib.name)
    return norm_name


def getLinkedLibs(lib: pathlib.Path, full_path: pathlib.Path=None) -> list[pathlib.Path]:
    libsUsed = []
    result = subprocess.run(['otool','-L',lib],capture_output=True,text=True)
    libList = result.stdout.split("\n")
    for entry in libList[1:]:
        # on fat-binaries we do not want to search twice
        if "architecture" in entry or not entry:
            break

        libEntry = entry.strip().split()[0]
        # we add rpath entries, or absolute path (if any)
        # absolute libpaths should have been fixed by fix-rpath script
        rpaths = ['@', str(full_path)]

        if libEntry.startswith(tuple(rpaths)):
            libsUsed.append(pathlib.Path(libEntry))

    return libsUsed


def findMissingLibs(kisLib: list[str|pathlib.Path], location: pathlib.Path, search_path: pathlib.Path=None) -> list[str]:
    libsFound = set()
    for lib in kisLib:
        if isBinary(lib):
            # find dep libs with oTool
            libsFound.update(getLinkedLibs(lib, search_path))

    # We only need the name of the lib or Framework
    libsFoundNorm = {normalizeLibNames(lib) for lib in libsFound}
    # missingLibs = set()

    # get all files from the dmg list missing files
    kritaDmgFiles = [file.name for file in location.rglob("*")]
    # findLibsNotInBundle
    missingLibs = [lib for lib in libsFoundNorm if lib not in kritaDmgFiles]

    return missingLibs


# TODO: (low) make dstApp a locations object
# src install_dir
# dst .app dir, we send either to frameworks or plugins
def copyMissingLibs(missingLib: list[str], src: pathlib.Path, dstApp: pathlib.Path):
    copyFiles = []
    for lib in missingLib:
        libPath = pathlib.Path(os.path.join(src, "lib", lib))
        if not libPath.exists():
            print(f'{lib} not found in {libPath} searching in {src}')
            libPath = next(src.rglob(lib), None)

        if not libPath:
            print(f'could not find {lib}')
            continue
        # we can't shutil the entire framework dir
        # BUG: https://github.com/python/cpython/issues/105919
        # if "framework" in libPath.name:
        #     copyFiles.extend(libPath.rglob("*"))
        #     continue

        copyFiles.append(libPath)
        while libPath.is_symlink():
            libPath = libPath.parent.joinpath(libPath.readlink())
            copyFiles.append(libPath)

    newlibs = list()
    locations = dict()
    locations['plugins'] = dstApp.joinpath('Contents', 'PlugIns')
    locations['frameworks'] = dstApp.joinpath('Contents', 'Frameworks')

    for file in copyFiles:
        print(f"Adding missing lib: {file}")
        loc = 'plugins' if "plugin" in str(file) else 'frameworks'
        if file.is_dir():
            try:
                # TODO: shutil error if symlink inside directory exist
                # BUG: https://github.com/python/cpython/issues/105919
                cmd = ['rsync','-prulq']
                # We should avoid copying debug libraries from frameworks
                # as both Lib_debug and Lib can't be signed at the same time.
                cmd.extend(['--exclude' ,f'**{file.stem}_debug', '--exclude', f'{file.stem}_debug.prl'])
                cmd.extend([file,locations[loc]])
                subprocess.run(cmd)
                newlibs.extend([f for f in file.rglob('*') if isBinary(f)])
                # shutil.copytree(file, locations[locations].joinpath(file.name), symlinks=True, dirs_exist_ok=True)
            except Exception as e:
                print(e)
        else:
            try:
                shutil.copy2(file, locations[loc], follow_symlinks=False)
                newlibs.append(file)
            except Exception as e:
                print(e)

    return newlibs


def kritaCreatePyKrita(src: pathlib.Path, dst: pathlib.Path, version: str):
    frame_name = "PyKrita"
    frame_version = version

    frame_loc = dict()
    frame_root = dst.joinpath(frame_name + ".framework") # PyKrita.framework
    frame_loc['root'] = frame_root
    frame_loc['versions'] = frame_root.joinpath('Versions') # PyKrita.framework/Versions
    frame_loc[frame_version] = frame_loc['versions'].joinpath(frame_version) # PyKrita.framework/Versions/x.y.z

    for key in frame_loc:
        frame_loc[key].mkdir(exist_ok=True)
    for name in ['Resources','lib']:
        frame_loc[frame_version].joinpath(name).mkdir(exist_ok=True)

    copyDirSub(src.joinpath('lib', 'krita-python-libs'),frame_loc[frame_version].joinpath('lib'))
    krita_so = pathlib.Path('lib', 'PyKrita', 'krita.so')
    shutil.move(frame_loc[frame_version].joinpath(krita_so),frame_loc[frame_version].joinpath(frame_name))

    # Create symlinks
    frame_loc[frame_version].joinpath(krita_so).symlink_to(pathlib.Path('..', '..', frame_name))
    frame_loc['versions'].joinpath('Current').symlink_to(frame_version)

    frame_loc['root'].joinpath(frame_name).symlink_to(pathlib.Path('Versions','Current',frame_name))
    frame_loc['root'].joinpath('Resources').symlink_to(pathlib.Path('Versions','Current','Resources'))

    krita_python_lib = dst.joinpath('krita-python-libs')
    krita_python_link = frame_loc['versions'].joinpath('Current', 'lib').relative_to(dst)
    krita_python_lib.symlink_to(krita_python_link)

    info_plist = frame_loc[frame_version].joinpath('Resources', 'Info.plist')

    plistbuddy_loc = pathlib.Path('/', 'usr', 'libexec', 'PlistBuddy')
    plistbuddy = lambda key, value: subprocess.run([plistbuddy_loc, info_plist, '-c', f'Add:{key} string {value}'])

    plistbuddy("CFBundleExecutable", frame_name)
    plistbuddy("CFBundleIdentifier", f'org.krita.{frame_name}')
    plistbuddy("CFBundlePackageType", "FMWK")
    plistbuddy("CFBundleShortVersionString", f'{frame_version}')
    plistbuddy("CFBundleVersion", f'{frame_version}')

    return


# TODO: test if frame_python_lib is actually pointing to 'pythonx.y'
def kritaStripPythonFramework(frameworkPath: pathlib.Path):
    print("Removing unnecessary files from Python.Framework to be packaged...")
    frame_python_lib = next(frameworkPath.joinpath('Versions','Current', 'lib').glob('python*'),
                            frameworkPath.joinpath('Versions','Current', 'lib','python3.10'))
    print(f'found frame_python_lib: {frame_python_lib}')

    files_for_rm = list()
    files_for_rm.extend([file for file in frameworkPath.rglob("test*") if file.is_dir() and file.name in ['test', 'tests']])
    files_for_rm.extend([file for file in frameworkPath.joinpath('Versions','Current', 'bin').rglob("*")
                         if not file.name.startswith('python')])

    files_for_rm.append(frame_python_lib.joinpath('bin', 'python3-intel64'))

    for name in "tkinter ensurepip distutils lib2to3 turtledemo idlelib".split():
        files_for_rm.append(frame_python_lib.joinpath(name))
    for pattern in "pip* PyQt_builder* setuptools* sip* easy-install.pth".split():
        files_for_rm.extend(frame_python_lib.joinpath('site-packages').glob(pattern))

    # removal of Python.app
    files_for_rm.append(frameworkPath.joinpath('Versions','Current', 'Resources','Python.app'))

    for file in files_for_rm:
        if file.exists(follow_symlinks=False):
            if file.is_dir():
                shutil.rmtree(file, ignore_errors=True)
            else:
                file.unlink(missing_ok=True)
        else:
            print(f'path does not exist: {file}')

    return


# Fix Python.Framework RPATH and links
def kritaFixPython(pyframe: pathlib.Path):
    # fix permissions
    pyframe_current = pyframe.joinpath('Versions','Current')
    pyframe_version = pyframe_current.readlink().name
    filesTofix = list()
    filesTofix.extend(pyframe.rglob("*.so"))
    filesTofix.append(pyframe_current.joinpath('lib',f'python{pyframe_version}','pydoc.py'))

    for file in filesTofix :
        DeployCmd.achmod(file,0o111)

    # fix rpath
    pythonLib = pyframe.joinpath('Python')
    installNameTool(pythonLib, f'-id {pythonLib.name}')
    installNameTool(pythonLib, '-add_rpath @loader_path/../../../')
    installNameTool(pythonLib, '-change @loader_path/../../../../libintl.9.dylib @loader_path/../../../libintl.9.dylib', check=True)

    # Python.app fix
    # pyframe_pyapp_python = pyframe_current.joinpath('Resources', 'Python.app','Contents','MacOS','Python')
    # installNameTool(pyframe_pyapp_python,'-add_rpath @executable_path/../../../../../../../')
    # installNameTool(pyframe_pyapp_python,f'-change "{krita_install_dir}/lib/Python.framework/Versions/{pyframe_version}/Python" @executable_path/../../../../../../Python')

    installNameTool(pyframe_current.joinpath('bin', f'python{pyframe_version}'), '-add_rpath @executable_path/../../../../')

    # this step is probably already achieved by deleteMissingRpath
    # which is more general and cover all the cases here
    # delete_install_rpath = lambda lib: subprocess.run([
    #     'install_name_tool', '-delete_rpath', krita_install_dir.joinpath('lib'), lib])
    #
    # filesTofix.clear()
    # filesTofix.append(pythonLib)
    # filesTofix.extend([f for f in pyframe.rglob('*') if f.is_file() and stat.S_IMODE(f.stat().st_mode) == 0o755])
    #
    # pyframe_site_pyqt5 = pyframe_current.joinpath('bin',f'python{pyframe_version}','site-packages','PyQt5')
    # filesTofix.extend([f for f in pyframe_site_pyqt5.glob('*.so') if f.is_file()])
    #
    # for file in filesTofix:
    #     delete_install_rpath(file)

    return


# Looks for RPATHS containing the rpath string,path and remove them
def cleanMissingRpath(rpath: [str|pathlib.Path], libs:list[pathlib.Path]=None):
    otool = lambda libin: subprocess.run(['otool','-l',libin],capture_output=True,text=True)
    if libs is None:
        libs = []
    for lib in libs:
        output = otool(lib)
        paths = [line.strip().split()[1] for line in output.stdout.split('\n') if f"path {rpath}" in line.strip()]
        for path in paths:
            # print(f'{lib}:{path}')
            installNameTool(lib, f'-delete_rpath {path}')
    return



def kritaDeploy(from_install: pathlib.Path, dst: pathlib.Path, source: pathlib.Path):

    krita_dmg = dst
    krita_install_dir = from_install
    krita_source_dir = source

    krita_app = dict()
    krita_app['root'] = pathlib.Path(os.path.join(krita_dmg, "krita.app"))
    krita_app['contents'] = pathlib.Path(os.path.join(krita_dmg, "krita.app", "Contents"))
    krita_app['plugins'] = pathlib.Path(os.path.join(krita_app['contents'], 'PlugIns'))
    krita_app['frameworks'] = pathlib.Path(os.path.join(krita_app['contents'], 'Frameworks'))
    krita_app['macos'] = pathlib.Path(os.path.join(krita_app['contents'], 'MacOS'))
    krita_app['resources'] = pathlib.Path(os.path.join(krita_app['contents'], 'Resources'))

    # --- path for subprocess
    kisenv = os.environ.copy()
    kisenv['PATH'] = f"{os.path.join(krita_install_dir, 'bin')}:{kisenv['PATH']}"

    # --- Qt version adjustments
    # TODO: probably better to rely on qtdiag
    try:
        qt_version = subprocess.run(["qtpaths", "--qt-version"],
                                    capture_output=True, text=True, env=kisenv).stdout
    except FileNotFoundError:
        print("Command not found, assuming Qt5!")
        qt_version = 5

    print(f"Found qt version: {qt_version}")
    osx_deployment_target = "12" if qt_version[0] == 6 else "10.14"
    kisenv['MACOSX_DEPLOYMENT_TARGET'] = osx_deployment_target
    kisenv['QMAKE_MACOSX_DEPLOYMENT_TARGET'] = osx_deployment_target

    # --- Krita version adjustments
    # os.environ['KRITACI_RELEASE_PACKAGE_NAMING'] = "ON"
    kis_version_full = subprocess.run(['krita_version', '-v'],
                                      capture_output=True, text=True, env=kisenv).stdout
    kis_version = kis_version_full.replace("-", " ").split()


    if krita_dmg.exists():
        print(f"Deleting previous krita.app run in {krita_dmg}")
        shutil.rmtree(krita_dmg)

    print(f"Preparing {krita_install_dir} for deployment")
    krita_dmg.mkdir(exist_ok=True)

    for key in krita_app:
        krita_app[key].mkdir(exist_ok=True, parents=True)

    print("copying krita.app...")
    copyDirSub(krita_install_dir.joinpath('bin', 'krita.app'), krita_dmg, only_contents=False)
    copyDirSub(krita_install_dir.joinpath('bin', 'kritarunner'), krita_app['macos'], only_contents=False)
    copyDirSub(krita_install_dir.joinpath('bin', 'krita_version'), krita_app['macos'], only_contents=False)

    print("Copying share...")
    extra_args = [     '--delete'
                       ,'--exclude', 'krita.icns'
                       ,'--exclude', 'krita-krz.icns'
                       ,'--exclude', 'krita-kra.icns'
                       ,'--exclude', 'Assets.car'
                       ,'--exclude', 'aclocal'
                       ,'--exclude', 'doc'
                       ,'--exclude', 'ECM'
                       ,'--exclude', 'eigen3'
                       ,'--exclude', 'emacs'
                       ,'--exclude', 'gettext'
                       ,'--exclude', 'gettext-0.19.8'
                       ,'--exclude', 'info'
                       ,'--exclude', 'kf5'
                       ,'--exclude', 'kservices5'
                       ,'--exclude', 'man'
                       ,'--exclude', 'ocio'
                       ,'--exclude', 'pkgconfig'
                       ,'--exclude', 'mime'
                       ,'--exclude', 'translations'
                       ,'--exclude', 'qml'
                        ]
    copyDirSub(krita_install_dir.joinpath('share'), krita_app['resources'], extra_args=extra_args)

    print("Copying Qt translations...")
    copyDirSub(krita_install_dir.joinpath('translations'), krita_app['contents'], only_contents=False)

    symlinks = [('share','Resources'),('lib','Frameworks'),('Resources/translations','translations')]
    for src,dst in symlinks:
        linkPath = krita_app['contents'].joinpath(src)
        if linkPath.is_symlink():
            linkPath.unlink()
        linkPath.symlink_to(dst)

    print("Copying mandatory libs...")
    pattern = ['libKF5*', 'libkrita*']
    mandatoryLibs = list()
    for pat in pattern:
        mandatoryLibs.extend(krita_install_dir.joinpath('lib').glob(pat))
    for file in mandatoryLibs:
        shutil.copy2(file,krita_app['frameworks'], follow_symlinks=False)

    print("Copying plugins...")
    extra_args = '--delete --delete-excluded --exclude kritaquicklook.qlgenerator --exclude kritaspotlight.mdimporter'
    copyDirSub(krita_install_dir.joinpath('plugins'), krita_app['plugins'], extra_args=extra_args.split())

    plugins = []
    krita_app_qlook = krita_app['contents'].joinpath('Library','QuickLook')
    krita_app_spotlight = krita_app['contents'].joinpath('Library', 'Spotlight')

    krita_app_qlook.mkdir(parents=True)
    krita_app_spotlight.mkdir(parents=True)

    plugins.append('QuickLook')
    copyDirSub(krita_install_dir.joinpath('plugins','kritaquicklook.qlgenerator'),krita_app_qlook,
               only_contents=False)
    plugins.append('Spotlight')
    copyDirSub(krita_install_dir.joinpath('plugins', 'kritaspotlight.mdimporter'), krita_app_spotlight,
               only_contents=False)

    plugins.append('krita-thumbnailer')
    copyDirSub(krita_install_dir.joinpath('plugins', 'krita-thumbnailer.appex'),krita_app['plugins'],
               only_contents=False)
    plugins.append('krita-preview')
    copyDirSub(krita_install_dir.joinpath('plugins', 'krita-preview.appex'), krita_app['plugins'],
               only_contents=False)
    print(f'Copied plugins: {",".join(plugins)}')


    print("Copying kritaplugins...")
    copyDirSub(krita_install_dir.joinpath('lib', 'kritaplugins'), krita_app['plugins'])
    copyDirSub(krita_install_dir.joinpath('lib', 'mlt'), krita_app['plugins'], only_contents=False)

    for name in ['ffmpeg', 'ffprobe']:
        shutil.copy2(krita_install_dir.joinpath('bin', name), krita_app['macos'])
        subprocess.run(f"install_name_tool -add_rpath @executable_path/../Frameworks/ "
                       f"{krita_app['macos'].joinpath(name)}".split())

    print("Copying python...")
    copyDirSub(krita_install_dir.joinpath('lib', 'Python.framework'),krita_app['frameworks'],only_contents=False)
    kritaCreatePyKrita(krita_install_dir, krita_app['frameworks'], kis_version[0])

    DeployCmd.achmod(krita_app['frameworks'].joinpath('Python.framework','Python'), stat.S_IWRITE)

    kritaStripPythonFramework(krita_app['frameworks'].joinpath('Python.framework'))
    kritaFixPython(krita_app['frameworks'].joinpath('Python.framework'))
    print("precompiling all python files")
    cmd = f"python -m compileall {krita_app['contents']}".split()
    cmdLog(cmd)
    subprocess.run(cmd,env=kisenv)

    # Remove unnecessary rpaths
    installNameTool(krita_app['macos'].joinpath('krita_version'), "-delete_rpath @executable_path/../lib")
    installNameTool(krita_app['macos'].joinpath('kritarunner'), "-delete_rpath @executable_path/../lib")
    installNameTool(krita_app['macos'].joinpath('krita'), "-delete_rpath @loader_path/../../../../lib")

    fileToRemove = krita_app['plugins'].joinpath('kf5', 'org.kde.kwindowsystem.platforms')
    if fileToRemove.exists():
        shutil.rmtree(fileToRemove)

    # Fix file permissions
    filesToFix = list()
    filesToFix.extend(krita_app['contents'].rglob('*.dylib'))
    filesToFix.extend(krita_app['contents'].rglob('*.so'))
    filesToFix.extend(krita_app['macos'].rglob('*'))
    for f in filesToFix:
        DeployCmd.achmod(f,0o111)
    for f in krita_app['resources'].joinpath('applications').rglob('*.desktop'):
        DeployCmd.xchmod(f,0o111)


    # Repair krita bundle
    print("Searching for missing libraries...")
    # Find binary files with execution flags
    # or files name finishing in 'dylib' or 'so'
    libs = [f for f in krita_app['contents'].rglob('*') if
            (f.is_file() and (stat.S_IMODE(f.stat().st_mode) & 0o111) and f.suffix != '.py')
            or f.suffix == '.dylib'
            or f.suffix == '.so'
            ]
    libs = [f for f in libs if isBinary(f)]


    missinglibs = findMissingLibs(libs, krita_app['contents'],krita_install_dir)
    while len(missinglibs) != 0:
        added_libs = copyMissingLibs(missinglibs, krita_install_dir, krita_app['root'])
        missinglibs = findMissingLibs(added_libs, krita_app['contents'],krita_install_dir)


    # Start run macdeployqt
    # We call this last as it does not copy links but duplicates many libs
    print("Looking for macdeployqt...\t", end="")
    exec_path = shutil.which("macdeployqt", path=kisenv['PATH'])
    if exec_path is not None:
        print("Found!")
        cmd = [exec_path
            ,krita_app['root']
            , '-verbose=0'
            , f'-executable={krita_app["macos"].joinpath("krita")}'
            , f'-libpath={krita_install_dir.joinpath("lib")}'
            , f"-qmldir={krita_source_dir.joinpath('plugins', 'dockers', 'textproperties')}"
            , '-appstore-compliant'
               ]
        cmdLog(cmd)
        proc = subprocess.Popen(cmd, text=True, bufsize=1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        # TODO: remove filter after patching macdeploytqt or upgrading to qt6
        for line in proc.stdout:
            if not 'ERROR: Could not parse otool' in line:
                print(f'{line}', end='')

        proc.stdout.close()
        proc.wait()
    else:
        print("Not Found!")
        print("WARNING: continuing without running macdeployqt may result in an invalid app")
    print("macdeployqt Done!")


    # TODO: remove after move to Qt6, those plugins are not compatible with macOS>=12
    # fixes kritaspotlight and kritaquicklook binaries
    filesToFix = [f for f in krita_app['contents'].joinpath('Library').rglob('*/Contents/MacOS/*') if
                  (f.is_file() and stat.S_IMODE(f.stat().st_mode) & 0o111)]
    for f in filesToFix:
        installNameTool(f, '-add_rpath @loader_path/../../../../../Frameworks')

    # Remove broken symlinks if any
    filesToFix = [f for f in krita_app['contents'].rglob('*') if f.is_symlink() and not f.exists()]
    for f in filesToFix:
        f.unlink()

    # Be extra paranoid about left over absolute paths
    # this may not be needed as macos-fix-rpaths.sh should deliver clean binaries
    filesToFix =[f for f in krita_app['contents'].rglob('*') if
                  (f.is_file() and (stat.S_IMODE(f.stat().st_mode) & 0o111) and f.suffix != '.py')
                  or f.suffix == '.dylib'
                  or f.suffix == '.so'
                  ]
    cleanMissingRpath(krita_install_dir,filesToFix)

    # remove debug version as both versions can't be signed.
    # krita_app['frameworks'].joinpath('QtScript.framework', 'Versions', 'Current', 'QtScript_debug').unlink(missing_ok=True)

    # delete .DS_Store if any
    for f in krita_app['contents'].rglob('*.DS_Store'):
        f.unlink()

    print("## Finished preparing krita.app bundle!")

    return



if __name__ == '__main__':
    main()
