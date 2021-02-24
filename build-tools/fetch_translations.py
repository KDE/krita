#!/usr/bin/python3



stable_url = "svn://anonsvn.kde.org/home/kde/branches/stable/l10n-kf5/"
unstable_url = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
krita_location = "messages/krita"

# switch these between the stable and unstable krita branch
url = unstable_url
#url = stable_url

print (url);

# construct the url and get the subdirs file
svn_command = url + "subdirs"

import os
os.makedirs("po", 0o777, True)
os.chdir("po")

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
        shutil.rmtree(subdir)
    except:
        pass
    
    try:
        os.mkdir(subdir)
    except:
        pass
    
    pofile = subdir + "/krita.po"
    
    print("writing", len(po_contents), "bytes to", pofile)
        
    f = open(pofile, 'w')
    f.write(po_contents)
    f.close()
    
