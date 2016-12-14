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
    print(Application.batchmode())
    document = Application.openDocument(args[0])
    print(document, document.batchmode(), document.fileName(), document.height(), document.width())
    node = document.rootNode()
    print(node, node.name(), "opacity", node.opacity())
    children = node.childNodes()
    print(children)
