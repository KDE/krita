#!/usr/bin/env python3

import argparse
import pathlib
import subprocess


class NotarizationHandler:

    def __init__(self
                 , account: str=None
                 , password: str=None
                 , provider: str=None
                 , store_credentials_key: str="KritaNotarizeAccount"
                 ):
        self.account = account
        self.provider = provider
        self.password = password
        self.store_credentials_key = store_credentials_key
        self.__can_notarize = False
        self.__cmd_login = list()

    def can_notarize(self):
        return self.__can_notarize

    def notarize(self, target: pathlib.Path) -> bool:
        self.__check_notarization()

        if not self.__can_notarize:
            print("## ERROR: notarization credentials failed!")
            return False

        notarized_success = True
        target_intermediate = pathlib.Path(f'{target.stem}.zip')
        cmd = ['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', target, target_intermediate]
        subprocess.run(cmd)

        submit_id = 0
        cmd = ['xcrun', 'notarytool', 'submit', target_intermediate, '--wait']
        cmd.extend(self.__cmd_login)
        try:
            proc = subprocess.Popen(cmd, text=True, bufsize=1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            for line in proc.stdout:
                if 'id:' in line:
                    submit_id = line.split(':')[-1].strip()

                if 'status: Invalid' in line:
                    proc.returncode = 1;
                print(f'{line}', end='')

            proc.stdout.close()
            proc.wait()

            if proc.returncode != 0:
                raise subprocess.CalledProcessError(
                    returncode=proc.returncode,
                    cmd=cmd,
                    output=proc.stdout,
                    stderr=proc.stderr
                )
            # subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as ex:
            print(f"## ERROR: Notarization failed!\n{ex.stderr}")
            print(f'Fetching submit id {submit_id} log')
            cmd = ['xcrun', 'notarytool', 'log', submit_id]
            cmd.extend(self.__cmd_login)
            cmd_log = subprocess.run(cmd, capture_output=True, text=True).stdout
            print(cmd_log)

            notarized_success = False

        else:
            print('## Notarization success!')

        # clean up
        target_intermediate.unlink()


        return notarized_success


    def __check_notarization(self):
        cmd = ['xcrun', 'notarytool', 'history']
        if self.account and self.password and self.provider:
            login_ops = f"--apple-id {self.account} --password {self.password} --team-id {self.provider}".split()
            cmd.extend(login_ops)
        else:
            login_ops = ['--keychain-profile', self.store_credentials_key]
            cmd.extend(login_ops)

        print(f'## RUNNING: {' '.join(map(str,cmd))}')
        error_notarize = subprocess.run(cmd, capture_output=True, text=True).stderr

        if not error_notarize:
            self.__can_notarize = True
            self.__cmd_login.extend(login_ops)
            print("## Notarization checks complete, This build will be notarized")
        else:
            print("WARNING: Account information missing, Notarization will not be performed")



def main_notarize():
    parser = argparse.ArgumentParser(prog='macos notarize krita.app',
                                     description='Utility that receives krita.app and sends for notarization',
                                     )
    parser.add_argument('target', type=pathlib.Path
                        , help='signed krita.app for notarization')
    parser.add_argument('--apple-id', dest="appleid", help='apple id email')
    parser.add_argument('--password', dest='password', help='application password for appleid')
    parser.add_argument('--team-id', dest='teamid', help='apple developer team id')
    parser.add_argument('--stored-credentials', dest="cred_key"
                        , help='stored credentials name as set when notarytool --store-credentials was run')
    args = parser.parse_args()

    target: pathlib.Path = args.target

    #### NOTARIZE BUNDLE
    # NOTE: it is enough to notarize the outer container
    notarizer = NotarizationHandler()

    if args.appleid:
        print("### WARNING: it is prefered to store-credentials to notarytool!")
        print("""   xcrun notarytool store-credentials 
        --apple-id <apple-id> --password <app-pass> --team-id <team-id> <key>]""")
        print("   By default this script searches for 'KritaNotarizeAccount' key")
        notarizer.account = args.appleid

        if not (args.password and args.teamid):
            print('## ERROR: if apple-id is provided, --password and --team-id must be present')
            exit(1)

        if args.teamid:
            notarizer.provider = args.teamid
        if args.password:
            notarizer.password = args.password

    else:
        if args.cred_key:
            notarizer.account = args.cred_key


    if notarizer.notarize(target):
        # Notarization success
        stapler = ['xcrun', 'stapler']
        subprocess.run(stapler + ['staple',target])
        subprocess.run(stapler + ['validate', '-v',target])

    return



if __name__ == '__main__':
    main_notarize()
