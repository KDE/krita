
import argparse
import pathlib
import subprocess
import os

def main_app2pkg():
    parser = argparse.ArgumentParser(prog='macos app to pkg'
                                     ,description="create a pkg from krita.app to store submission")
    parser.add_argument('krita_app', metavar="krita.app", type=pathlib.Path
                        , help="krita.app to make pkg")
    parser.add_argument('-s','--sign-identity', dest='sign_identity', metavar='<identity>'
                        , required=True
                        , help='3rd Party Mac Developer Installer identity')
    args = parser.parse_args()


    krita_app: pathlib.Path = args.krita_app
    krita_infoplist: pathlib.Path = krita_app.joinpath('Contents', 'Info.plist')

    plistbuddy = pathlib.Path(os.sep, 'usr', 'libexec', 'PListBuddy')
    kis_bundle_version = subprocess.run([plistbuddy, krita_infoplist, '-c', 'Print :CFBundleVersion']
                                          , capture_output=True, text=True).stdout.strip()
    kis_version_full = subprocess.run([plistbuddy, krita_infoplist, '-c', 'Print :CFBundleLongVersionString']
                                        , capture_output=True, text=True).stdout.strip()
    kis_bundle_id = subprocess.run([plistbuddy, krita_infoplist, '-c', 'Print :CFBundleIdentifier']
                                      , capture_output=True, text=True).stdout.strip()


    krita_pkg: pathlib.Path = pathlib.Path(f'{krita_app.stem}-{kis_version_full}_bv-{kis_bundle_version}.pkg')
    print(f'creating {krita_pkg}')

    cmd = ['productbuild', '--component', krita_app, '/Applications', krita_pkg
           , '--sign', args.sign_identity]
    subprocess.run(cmd)


    print(f"""
this script does not submit {krita_pkg} yo the store, to do so you may run altool as:
    xcrun altool --upload-package {krita_pkg} --bundle-id {kis_bundle_id} -t macos -u <appleid-account> --bundle-version {kis_bundle_version} --bundle-short-version-string {kis_version_full} --apple-id <app-id from store>
    
    You may also need to add a teamID if you belong to more than one team
        --asc-provider <apple-teamid>
    
    Alternative the resulting pkg can be submitted using Transporter.app"""
)

if __name__ == '__main__':
    main_app2pkg()
