#!/usr/bin/python3

stable_url = "svn://anonsvn.kde.org/home/kde/branches/stable/l10n-kf5/"
unstable_url = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
krita_location = "messages/krita"

# determine whether we're in the master branch or not
from git import Repo
repo = Repo(".")
print("Current git branch", repo.active_branch)

if repo.active_branch.name == "master":
    print("Getting unstable translations")
    url = unstable_url
else:
    print("Getting stable translations")
    url = stable_url

print (url);

# construct the url and get the subdirs file
svn_command = url + "subdirs"

import subprocess
subdirs = subprocess.run(["svn", "cat", svn_command], stdout=subprocess.PIPE)
for subdir in subdirs.stdout.decode('utf-8').split('\n'):
    po_url = url + '/' + subdir + '/' + krita_location + "/krita.po"
    
    res = subprocess.run(["svn", "cat", po_url], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    po_contents = res.stdout.decode('utf-8')
    if (len(po_contents) == 0):
        print ("empty pofile for", subdir, " -- continuing.")
        continue

    import os
    import shutil
    try:
        shutil.rmtree("po/" + subdir)
    except:
        print("Could not remove", "po/" + subdir)
        pass
    
    try:
        os.makedirs("po/" + subdir, exist_ok=True)
    except:
        print("Could not create", "po/" + subdir)
        pass
    
    pofile = "po/" + subdir + "/krita.po"
    
    print("writing", len(po_contents), "bytes to", pofile)
        
    f = open(pofile, 'w')
    f.write(po_contents)
    f.close()
    

