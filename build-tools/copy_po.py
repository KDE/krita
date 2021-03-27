#!/usr/bin/python3
import subprocess

stable_url = "svn://anonsvn.kde.org/home/kde/branches/stable/l10n-kf5/"
unstable_url = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
krita_location = "messages/krita"

# determine whether we're in the master branch or not
res = subprocess.run(["git", "status"], stdout=subprocess.PIPE)
if "master" in res.stdout.decode('utf-8'):
    url = unstable_url
else:
    url = stable_url
        
print (url);

# construct the url and get the subdirs file
svn_command = url + "subdirs"

import os
import shutil
import subprocess

subdirs = subprocess.run(["svn", "cat", svn_command], stdout=subprocess.PIPE)
for subdir in subdirs.stdout.decode('utf-8').split('\n'):
    po_url = url + '/' + subdir + '/' + krita_location + "/krita.po"
    
    res = subprocess.run(["svn", "cat", po_url], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    po_contents = res.stdout.decode('utf-8')
    if (len(po_contents) == 0):
        print ("empty pofile for", subdir, " -- continuing.")
        continue

    po_subdir = "po/{}".format(subdir)

    try:
        if os.path.isdir(po_subdir):
            shutil.rmtree(po_subdir)
    except:
        print("Could not remove {}".format(po_subdir))
        pass
    
    try:
        os.makedirs(po_subdir, exist_ok=True)
    except:
        print("Could not create {}".format(po_subdir))
        pass

    pofile = "{}/krita.po".format(po_subdir)

    with open(pofile, "w", encoding="utf-8") as f:
        print("writing {} bytes to {}".format(len(po_contents), pofile))
        f.write(po_contents)
