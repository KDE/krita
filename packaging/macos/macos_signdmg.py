#!/usr/bin/env python3

import argparse
import pathlib
import shutil
import subprocess
import os
import stat
import concurrent.futures

from dataclasses import dataclass


def main_signdmg():
    parser = argparse.ArgumentParser(prog='macos signdmg',
                                     description='Utility that receives krita.dmg and signs it')
    parser.add_argument('krita_dmg', metavar='krita.dmg', type=dmgpath,
                        help='krita.dmg to sign')
    parser.add_argument('-s','--sign-identity',dest='sign_identity', metavar='<identity>',
                        help='Apple certificate name to use for signing', required=True)
    parser.add_argument('--store', action='store_true', help="Sign app for store submission")
    parser.add_argument('--bundle-version',dest='bundle_version', metavar='<bversion>'
                        , help="Store submission requires a unique bundle version (see appstoreconnect Testflight pane)")
    parser.add_argument('--entitlements-dir', dest='entitlements_dir', metavar='<entitlements-dir>'
                        , help="Location of entitlements.plist, macStore-entitlements.plist, sandboxdev_sub-entitlements.plist and kis-thumbnail.entitlements"
                        , type=pathlib.Path)
    args = parser.parse_args()

    if args.store:
        if not args.bundle_version:
            print("## ERROR: if --store is provided, bundle_version is mandatory")
            exit(1)
        print(f"## Signing {args.krita_dmg.stem} for store submission")

    krita_dmg: pathlib.Path = args.krita_dmg
    print(f'{krita_dmg} - {args.sign_identity}')

    #### Mount dmg to extract krita.app
    krita_app = pathlib.Path.cwd().joinpath('krita.app')
    if krita_app.exists():
        shutil.rmtree(krita_app)
    cmd = f"hdiutil attach {krita_dmg} -mountpoint {krita_dmg.stem}".split()
    subprocess.run(cmd)
    cmd = f"rsync -prult --delete {krita_dmg.stem + os.sep + krita_app.name + os.sep} krita.app".split()
    subprocess.run(cmd)
    subprocess.run(f"hdiutil detach {krita_dmg.stem}".split())

    #### FIX BUNDLE
    print("## Fixing bundle files permissions...")
    fix_bundle(krita_app, args.bundle_version, args.store)

    #### SIGN BUNDLE
    print("## Starting bundle signning...")
    # we do not check if the sign certificate identity is valid
    signer = Codesigner(args.sign_identity)
    if args.entitlements_dir:
        entitlements_path = args.entitlements_dir
    else:
        entitlements_path = pathlib.Path('bundle_root', '..', '..', 'krita', 'packaging', 'macos').resolve()

    sign_bundle(krita_app, signer, entitlements_path, args.store)

    #### VERIFY BUNDLE
    print("## Verifying all binary files signatures...")
    if not verify_bundle(krita_app):
        print('Codesign errors!')
        exit(1)

    print('## Could not detect codesign problems, app is ready for notarization')
    print(f"## {krita_app.name} signed")

    return



@dataclass
class Entitlements:
    """Structure to hold all entitlement paths"""
    general: pathlib.Path | str = ""
    subprocess: pathlib.Path | str = ""
    plugins: pathlib.Path | str = ""



class DeployCmd:
    # achmod appends the mod to file
    achmod = lambda target, mode: os.lchmod(target, stat.S_IMODE(os.stat(target).st_mode) | mode)
    # xchmod removes the bit flags from file
    xchmod = lambda target, mode: os.lchmod(target, stat.S_IMODE(os.stat(target).st_mode) & ~mode)



class Codesigner:
    def __init__(self, signature: str):
        self.cores = os.cpu_count()
        self.signature = signature
        self.__targets: list[tuple[pathlib.Path, str]] = []

    @property
    def targets(self):
        return self.__targets

    def add(self, targets: list[pathlib.Path] | pathlib.Path, entitlement: str):
        if type(targets) is list:
            self.__targets.extend([(target, entitlement) for target in targets])
        else:
            self.__targets.append((targets, entitlement))

    def codesign(self, target_data: tuple[pathlib.Path,str]):
        target, entitlements = target_data
        cmd = ['codesign', '--options', 'runtime', '--timestamp', '-f']
        cmd.extend(['-s', self.signature])
        cmd.extend(['--entitlements', entitlements])
        cmd.append(target)

        result = subprocess.run(cmd,capture_output=True, text=True).stderr
        return target, result

    def signtargets(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.cores) as executor:
            result = executor.map(lambda target: self.codesign(target), self.__targets)

        self.__targets.clear()
        return result



class PlistBuddyModifier:
    def __init__(self, plist: pathlib.Path):
        self.__plist = plist
        self.__plistbuddy = pathlib.Path(os.sep, 'usr', 'libexec', 'PlistBuddy')
        self.__cmd = [self.__plistbuddy, plist, '-c']

    @property
    def plist(self):
        return self.__plist

    @plist.setter
    def plist(self, plist: pathlib.Path):
        self.__plist = plist
        self.__cmd = [self.__plistbuddy, plist]

    def set(self, key: str, value: str):
        cmd = self.__cmd + [f'Set :{key} {value}']
        self.__execute(cmd)

    def delete(self, key: str):
        cmd = self.__cmd + [f'Delete :{key}']
        self.__execute(cmd)

    def add(self, key: str, value: str):
        cmd = self.__cmd + [f'Add :{key} {value}']
        self.__execute(cmd)

    @staticmethod
    def __execute(cmd: list[str]):
        print(f'- {' '.join(map(str, cmd))}')
        subprocess.run(cmd)



# security find-identity -v -p codesigning
# check if path points to dmg file
def dmgpath(path: [str]) -> pathlib.Path:
    dmgfile = pathlib.Path(path)
    if dmgfile.suffix != '.dmg' or not dmgfile.exists():
        raise argparse.ArgumentTypeError(f'{path} is not a dmg file or does not exist')

    return dmgfile


def fix_bundle(bundle_root: pathlib.Path, bundle_version: str="", for_store: bool=False):
    # All files in bundle must have read permissions
    bundle_files = [f for f in bundle_root.rglob('*') if f.is_file(follow_symlinks=False)]

    files_tofix = [f for f in bundle_files if (stat.S_IMODE(f.stat().st_mode) & 0o044) == 0]
    for file in files_tofix:
        DeployCmd.achmod(file, 0o444)

    # Fixing permissions of dynamic libraries
    files_tofix = [f for f in bundle_files if (f.suffix == '.dylib') and (stat.S_IMODE(f.stat().st_mode) & 0o111) == 0]
    for file in files_tofix:
        DeployCmd.achmod(file, 0o111)

    # any file owned by root is an ERROR
    files_tofix = [f for f in bundle_files if f.owner() == 'root']
    if len(files_tofix) > 0:
        print(f'files {files_tofix}\n cannot be read by non-root users!')
        print('submission will fail, please fix and relaunch script')
        exit(1)

    # remove Qt*debug as normal and debug versions can't be signed at the same time
    files_tofix = [f for f in bundle_files if f.name.endswith('_debug') and f.name.startswith('Qt')]
    for file in files_tofix:
        file.unlink()

    # NOT_PORTED: remove from Python.framework all *.so files

    if for_store:
        print("## Modifying bundle for store distribution...")
        # TODO: store move embedded.provisionprofile to krita.app/Contents/

        # get info from the packaged krita
        # krita_version_full = subprocess.run([bundle_root.joinpath('Contents', 'MacOS', 'krita_version'), 'v']
        #                                     , capture_output=True, text=True).stdout
        krita_version_full = subprocess.run([pathlib.Path(os.sep,'usr', 'libexec', 'PListBuddy')
                                                , bundle_root.joinpath('Contents','Info.plist')
                                                , '-c', f'Print :CFBundleLongVersionString']
                                                , capture_output=True, text=True).stdout
        krita_version = krita_version_full.replace("-", " ").split()[0]

        # modify InfoPlist (krita_version, macosx_deployment_target)
        fix_krita_plists(bundle_root, 'org.kde.krita', krita_version, bundle_version)

        # clean dmg: removes krita_version, kritarunner fom MacOS
        bundle_macos = bundle_root.joinpath('Contents', 'MacOS')
        bundle_macos.joinpath('krita_version').unlink()
        bundle_macos.joinpath('kritarunner').unlink()

    return


def fix_krita_plists(krita_app: pathlib.Path, bundle_id: str, version: str, build_version: str):

    macosx_deployment_target = '10.14' if version.startswith('5') else '12'
    appex_plugins_deployment_target = '10.15' # if user is in 10.14 the old thumbnailer will be used.

    kritaspotlight = pathlib.Path(krita_app, 'Contents', 'Library','Spotlight', 'kritaspotlight.mdimporter', 'Contents')
    kritaquicklook = pathlib.Path(krita_app, 'Contents', 'Library', 'Quicklook', 'kritaquicklook.qlgenerator', 'Contents')
    krita_thumbnailer = pathlib.Path(krita_app, 'Contents', 'Plugins', 'krita-thumbnailer.appex', 'Contents')
    krita_preview = pathlib.Path(krita_app, 'Contents', 'Plugins', 'krita-preview.appex', 'Contents')

    plists = dict()
    plists['krita'] = PlistBuddyModifier(pathlib.Path(krita_app, 'Contents', 'Info.plist'))
    plists['spotlight'] = PlistBuddyModifier(pathlib.Path(kritaspotlight, 'Info.plist'))
    plists['quicklook'] = PlistBuddyModifier(pathlib.Path(kritaquicklook, 'Info.plist'))
    plists['thumbnailer'] = PlistBuddyModifier(pathlib.Path(krita_thumbnailer, 'Info.plist'))
    plists['preview'] = PlistBuddyModifier(pathlib.Path(krita_preview, 'Info.plist'))

    # modify plist contents
    plists['krita'].set('CFBundleIdentifier', bundle_id)
    plists['krita'].set('CFBundleLongVersionString', version)
    plists['krita'].set('LSMinimumSystemVersion', macosx_deployment_target)

    plists['spotlight'].set('CFBundleIdentifier', bundle_id + '.spotlight')
    plists['quicklook'].set('CFBundleIdentifier', bundle_id + '.kritaquicklook')
    plists['thumbnailer'].set('CFBundleIdentifier', bundle_id + '.krita-thumbnailer')
    plists['preview'].set('CFBundleIdentifier', bundle_id + '.krita-preview')

    plists['spotlight'].set('LSMinimumSystemVersion', macosx_deployment_target)
    plists['quicklook'].set('LSMinimumSystemVersion', macosx_deployment_target)
    plists['thumbnailer'].set('LSMinimumSystemVersion', appex_plugins_deployment_target)
    plists['preview'].set('LSMinimumSystemVersion', appex_plugins_deployment_target)

    # common keys
    for plist in plists.values():
        plist.set('CFBundleVersion', build_version)
        plist.set('CFBundleShortVersionString', version)


def sign_bundle(bundle_root: pathlib.Path, signer: Codesigner, entitlements_path: pathlib.Path, for_store: bool=False):

    entitlements = Entitlements()
    if for_store:
        entitlements.general = entitlements_path.joinpath('macStore-entitlements.plist')
    else:
        entitlements.general = entitlements_path.joinpath('entitlements.plist')

    entitlements.subprocess = entitlements_path.joinpath('sandboxdev_sub-entitlements.plist')
    entitlements.plugins = entitlements_path.joinpath('kis-thumbnail.entitlements')

    content_root = bundle_root.joinpath('Contents')

    # Start signing process, as codesigner signs in parallel, to ensure
    #   some order, we must process them in chunks.
    python_targets = [f for f in content_root.joinpath('Frameworks','Python.framework').rglob('*') if
         f.is_file() and f.suffix.endswith('o')
         or (stat.S_IMODE(f.stat().st_mode) & 0o111 > 0)
         and not (f.is_symlink() or f.is_dir())
    ]
    signer.add(python_targets, entitlements.general)
    signer.signtargets()

    # Sign every lib and framework in toplevel Frameworks
    targets = [f for f in content_root.joinpath('Frameworks').glob('*') if not f.is_symlink()]
    # explicitly sign framework/Version/Current
    # TODO: check if this condition is necessary
    #       testing showed first find skipped frameworks on the previous script
    targets.extend([f'{f}/Versions/Current' for f in content_root.joinpath('Frameworks').glob('*.framework')])

    # sign krita-python-libs
    # TODO: test,this should not be necessary as they are signed in the frameworks step
    # targets.extend([f for f in content_root.joinpath('Frameworks').glob('krita-python-libs')

    plugins_root = content_root.joinpath('PlugIns')
    targets.extend([f for f in plugins_root.rglob('*') if f.is_file()])

    resources_root = content_root.joinpath('Resources')
    targets.extend([f for f in resources_root.rglob('*') if f.is_file() and (stat.S_IMODE(f.stat().st_mode) & 0o111 > 0)])

    targets.append(content_root.joinpath('Library', 'QuickLook', 'kritaquicklook.qlgenerator'))
    targets.append(content_root.joinpath('Library', 'Spotlight', 'kritaspotlight.mdimporter'))

    signer.add(targets, entitlements.general)
    signer.signtargets()

    plugin_targets = [
        plugins_root.joinpath('krita-thumbnailer.appex')
        ,plugins_root.joinpath('krita-preview.appex')
    ]
    signer.add(plugin_targets, entitlements.plugins)

    bundle_macos_root = content_root.joinpath('MacOS')
    binary_helper_targets = [
        bundle_macos_root.joinpath('ffmpeg')
        ,bundle_macos_root.joinpath('ffprobe')
    ]
    signer.add(binary_helper_targets, entitlements.subprocess)

    if not for_store:
        binary_targets = [
            bundle_macos_root.joinpath('kritarunner')
            , bundle_macos_root.joinpath('krita_version')
        ]
        signer.add(binary_targets, entitlements.general)

    signer.signtargets()

    # Finally sign krita and krita.app
    krita_targets = [
        bundle_macos_root.joinpath('krita')
        , bundle_root
    ]
    signer.add(krita_targets, entitlements.general)
    signer.signtargets()

    return


def verify_bundle(bundle: pathlib.Path) -> bool:
    is_binary = lambda fpath: b'Mach-O' in subprocess.run(['file',fpath], capture_output=True).stdout
    is_signed = lambda fpath: 'is not signed at all' not in subprocess.run(['codesign', '-vvv', '--strict', fpath], capture_output=True, text=True).stderr

    # missing_sign = [f for f in bundle.rglob('*') if is_binary(f) and not is_signed(f)]
    with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        results = executor.map(lambda f: f if (f.is_file() and is_binary(f) and not is_signed(f)) else None, bundle.rglob('*'))

    missing_signature = [f for f in results if f is not None]
    for element in missing_signature:
        print(f'missing signature: {element}')

    check_pkg = [f for f in bundle.rglob('*.framework')]
    check_pkg.append(bundle.joinpath('Contents', 'Library', 'Quicklook', 'kritaquicklook.qlgenerator'))
    check_pkg.append(bundle.joinpath('Contents', 'Library', 'Spotlight', 'kritaspotlight.mdimporter'))
    check_pkg.append(bundle.joinpath('Contents', 'PlugIns', 'krita-thumbnailer.appex'))
    check_pkg.append(bundle.joinpath('Contents', 'PlugIns', 'krita-preview.appex'))
    check_pkg.append(bundle)

    pkg_error = list()
    for pkg in check_pkg:
        error = subprocess.run(['pkgutil', '--check-signature', pkg], capture_output=True).returncode
        if error:
            pkg_error.append(pkg)

    for pkg in pkg_error:
        print(f'pkgutil error: {pkg}')

    success = len(missing_signature) + len(pkg_error) == 0

    return success



if __name__ == '__main__':
    main_signdmg()
