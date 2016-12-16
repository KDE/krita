#
# Tests the PyKrita API
#

import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

def __main__(args):
    print("Arguments:", args)
    Application.setBatchmode(True)
    print("Batchmode: ", Application.batchmode())
    document = Application.openDocument(args[0])
    print("Opened", document.fileName(), "WxH", document.width(), document.height(), "resolution", document.xRes(), document.yRes(), "in ppi", document.resolution())
    node = document.rootNode()
    print("Root", node.name(), "opacity", node.opacity())
    for child in node.childNodes():
        print("\tChild", child.name(), "opacity", node.opacity(), node.blendingMode())
        r = child.save(child.name() + ".png", document.xRes(), document.yRes());
        print("Saving result:", r)
        for channel in child.channels():
            print("Channel", channel.name(), "contents:", len(channel.pixelData(node.bounds())))
