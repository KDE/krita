#!/usr/bin/python3

# SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime

parser = argparse.ArgumentParser()

parser.add_argument("-u", "--unstable", action='store_true',
                    help="Download unstable translations")

args = parser.parse_args()

stable_url = "svn://anonsvn.kde.org/home/kde/branches/stable/l10n-kf5"
unstable_url = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5"
krita_location = "messages/krita"

if args.unstable:
    url = unstable_url
else:
    url = stable_url

# construct the url and get the subdirs file
svn_command = "{}/subdirs".format(url)

# construct the revision regex
rev = re.compile(r'Last Changed Rev: ([0-9]+)')

# construct the system-dependent neutral locale
env = os.environ
if os.name == 'nt':
    env["LANG"] = "English"
else:
    env["LANG"] = "en_US.UTF-8"
    env["LANGUAGE"] = "en_US.UTF-8"


last_update_date_filename = "po/last_update_date.txt"
should_update = True
update_time_format = "%Y/%m/%d %H:%M"
if os.path.exists(last_update_date_filename):
    with open(last_update_date_filename, "r", encoding="utf-8") as f:
        last_update_string = f.read()
        try:
            last_update_time = datetime.strptime(last_update_string, update_time_format)
        except ValueError:
            last_update_time = datetime.min
        current_time = datetime.utcnow()
        time_delta = current_time - last_update_time
        should_update = time_delta.days >= 1

if not should_update:
    sys.exit()

subdirs = subprocess.run(["svn", "cat", svn_command], stdout=subprocess.PIPE)
for subdir in subdirs.stdout.decode('utf-8').strip().split('\n'):
    po_url = "{}/{}/{}/krita.po".format(url, subdir, krita_location)

    status = subprocess.run(["svn", "info", po_url], env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    status_metadata = status.stdout.decode('latin-1')
    if(len(status_metadata) == 0):
        print("empty pofile for", subdir, " -- continuing.")
        continue

    po_subdir = "po/{}".format(subdir)

    try:
        os.makedirs(po_subdir, exist_ok=True)
    except:
        print("Could not create {}".format(po_subdir))
        pass

    revfile = "{}/revision".format(po_subdir)
    pofile = "{}/krita.po".format(po_subdir)

    current = rev.search(status_metadata)

    try:
        with open(revfile, 'r', encoding='utf-8') as r:
            existing = r.readline()

            if existing == current.groups()[0]:
                print("Translation {} is up to date, skipping.".format(subdir))
                continue
    except:
        pass

    res = subprocess.run(["svn", "cat", po_url],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    po_contents = res.stdout.decode('utf-8')

    with open(pofile, "w", encoding="utf-8") as f:
        with open(revfile, "w", encoding="utf-8") as r:
            print("{}: rev {}, {} bytes".format(
                pofile, current.groups()[0], len(po_contents)))
            r.write(current.groups()[0])
        f.write(po_contents)

with open(last_update_date_filename, "w", encoding="utf-8") as f:
    f.write(datetime.utcnow().strftime(update_time_format))
