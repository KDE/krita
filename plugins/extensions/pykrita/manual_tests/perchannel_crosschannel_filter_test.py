#
#  SPDX-FileCopyrightText: 2024 Grum 999 <grum999@grum.fr>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#
# This script:
# - Create documents with a filter node ('perchannel', 'crosschannel')
# - Check created document 
# - Modify document filter
# - Check modified document

import tempfile

from krita import *


def checkResult(name, value, ref):
    if value == ref:
        returned = True
        isOk = "OK"
    else: 
        returned = False
        isOk = "INVALID"
        
    print(f"Check {name}: '{value}' == '{ref}'  -->  {isOk}")
    return returned
    

def createDocument(fileName, filterName, curves):
    doc = Krita.instance().createDocument(500, 500, "TestFilter", "RGBA", "U8", "", 72.0)
    #Krita.instance().activeWindow().addView(doc)

    filter = Krita.instance().filter(filterName)
    selection = Selection()
    selection.selectAll(doc.rootNode(), 255)
    node = doc.createFilterLayer("TestFilter", filter, selection)
    doc.rootNode().addChildNode(node, None)
    # here need to retrieve filter from layer, using the one (filter 'filter' here) that has been created does nothing
    filter2 = node.filter()
    cfg = filter2.configuration()
    for curveId, curveValue in curves.items():
        cfg.setProperty(curveId, curveValue)
    doc.refreshProjection()
    doc.saveAs(fileName)
    doc.close()


def modifyDocument(fileName, curves):
    doc = Krita.instance().openDocument(fileName)
    #Krita.instance().activeWindow().addView(doc)

    nodes = doc.rootNode().childNodes()
    
    if nodes[1].name() == "TestFilter":
        returned = True
        
        filter = nodes[1].filter()
        # get filter configuration
        cfg = filter.configuration()
        for curveId, curveValue in curves.items():
            cfg.setProperty(curveId, curveValue)
                
        doc.refreshProjection()
        doc.save()
        doc.close()


def checkDocument(fileName, curves):
    # open testing document and check if filter properties are OK
    returned = False
    
    doc = Krita.instance().openDocument(fileName)
    #Krita.instance().activeWindow().addView(doc)
    
    nodes = doc.rootNode().childNodes()
    
    if nodes[1].name() == "TestFilter":
        returned = True
        
        filter = nodes[1].filter()
        # get filter configuration
        cfg = filter.configuration()
        for curveId, curveValue in curves.items():
            if not checkResult(curveId, cfg.property(curveId), curveValue):
                returned = False
        
        doc.close()
        
    return returned


if Krita.instance():
    curvesPCC = {'nTransfers': 8,
                 'curve0': '0,0;0.5,1;1,0;',
                 'curve1': '0,0.01;1,0.91;',
                 'curve2': '0,0.02;1,0.92;',
                 'curve3': '0,0.03;1,0.93;',
                 'curve4': '0,0.04;1,0.94;',
                 'curve5': '0,0.05;1,0.95;',
                 'curve6': '0,0.06;1,0.96;',
                 'curve7': '0,0.07;1,0.97;'
                 }
    curvesPCM = {'nTransfers': 8,
                 'curve0': '0,1;0.5,1;0,9;',
                 'curve1': '0,0.11;1,0.91;',
                 'curve2': '0,0.22;1,0.92;',
                 'curve3': '0,0.33;1,0.93;',
                 'curve4': '0,0.44;1,0.94;',
                 'curve5': '0,0.55;1,0.95;',
                 'curve6': '0,0.66;1,0.96;',
                 'curve7': '0,0.77;1,0.97;'
                 }

    curvesCCC = {'nTransfers': 8,
                 'curve0': '0,0;0.5,1;1,0;',
                 'curve1': '0,0.01;1,0.91;',
                 'curve2': '0,0.02;1,0.92;',
                 'curve3': '0,0.03;1,0.93;',
                 'curve4': '0,0.04;1,0.94;',
                 'curve5': '0,0.05;1,0.95;',
                 'curve6': '0,0.06;1,0.96;',
                 'curve7': '0,0.07;1,0.97;',
                 'driver0': 0,
                 'driver1': 1,
                 'driver2': 2,
                 'driver3': 3,
                 'driver4': 4,
                 'driver5': 5,
                 'driver6': 6,
                 'driver7': 7
                 }
    curvesCCM = {'nTransfers': 8,
                 'curve0': '0,1;0.5,1;0,9;',
                 'curve1': '0,0.11;1,0.91;',
                 'curve2': '0,0.22;1,0.92;',
                 'curve3': '0,0.33;1,0.93;',
                 'curve4': '0,0.44;1,0.94;',
                 'curve5': '0,0.55;1,0.95;',
                 'curve6': '0,0.66;1,0.96;',
                 'curve7': '0,0.77;1,0.97;',
                 'driver0': 7,
                 'driver1': 6,
                 'driver2': 5,
                 'driver3': 4,
                 'driver4': 3,
                 'driver5': 2,
                 'driver6': 1,
                 'driver7': 0
                 }


    with tempfile.TemporaryDirectory() as tmpDirname: 
        returned = True
        
        print("-- Testing 'perchannel' filter")
        tmpFileNamePC = os.path.join(tmpDirname, 'testing_filter_pc.kra')
        createDocument(tmpFileNamePC, "perchannel", curvesPCC)
        returned &= checkDocument(tmpFileNamePC, curvesPCC)
        modifyDocument(tmpFileNamePC, curvesPCM)
        returned &= checkDocument(tmpFileNamePC, curvesPCM)

        print("-- Testing 'crosschannel' filter")
        tmpFileNameCC = os.path.join(tmpDirname, 'testing_filter_cc.kra')
        createDocument(tmpFileNameCC, "crosschannel", curvesCCC)
        returned &= checkDocument(tmpFileNameCC, curvesCCC)
        modifyDocument(tmpFileNameCC, curvesCCM)
        returned &= checkDocument(tmpFileNameCC, curvesCCM)

        if returned:
            print('Global check: OK')
        else:
            print('Global check: INVALID')
