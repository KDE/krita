#  SPDX-FileCopyrightText: 2024 Grum 999 <grum999@grum.fr>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#
# Execute this script in scripter, and check output results are OK
# - check document guides settings


from krita import (Document, GuidesConfig)
from PyQt5.Qt import *

import tempfile
import os.path


class TestDocument():
    def __init__(self):
        self.newDoc = Krita.instance().createDocument(500, 500, "Test autosave", "RGBA", "U8", "", 300)

        with tempfile.TemporaryDirectory() as tmpDirname:
            tmpFileName = os.path.join(tmpDirname, 'testing_document.kra')

            qDebug("-- Test 01 --")
            self.setTestValues01(self.newDoc)
            self.checkTestValues01(self.newDoc)
            self.newDoc.saveAs(tmpFileName)
            self.newDoc.close()
            qDebug("-- Test 01 (loaded) --")
            self.newDoc = Krita.instance().openDocument(tmpFileName)
            self.checkTestValues01(self.newDoc)

            qDebug("-- Test 02 --")
            self.setTestValues02(self.newDoc)
            self.checkTestValues02(self.newDoc)
            self.newDoc.save()
            self.newDoc.close()
            qDebug("-- Test 02 (loaded) --")
            self.newDoc = Krita.instance().openDocument(tmpFileName)
            self.checkTestValues02(self.newDoc)

            qDebug("-- Test 03 --")
            self.setTestValues03(self.newDoc)
            self.checkTestValues03(self.newDoc)
            self.newDoc.save()
            self.newDoc.close()
            qDebug("-- Test 03 (loaded) --")
            self.newDoc = Krita.instance().openDocument(tmpFileName)
            self.checkTestValues03(self.newDoc)
            self.newDoc.close()

    def setTestValues01(self, doc):
        guides = GuidesConfig()
        guides.setColor(QColor("#ff0000"))
        guides.setLineType("solid")
        guides.setHorizontalGuides([50, 100, 150])
        guides.setVerticalGuides([25, 75, 125])
        guides.setVisible(True)
        guides.setLocked(True)
        guides.setSnap(True)
        doc.setGuidesConfig(guides)

    def checkTestValues01(self, doc):
        guides = doc.guidesConfig()
        self.checkResult('guide color', guides.color().name(), "#ff0000")
        self.checkResult('guide lineType', guides.lineType(), "solid")
        self.checkResult('guide hasGuides', guides.hasGuides(), True)
        self.checkResult('guide horizontal', guides.horizontalGuides(), [50, 100, 150])
        self.checkResult('guide vertical', guides.verticalGuides(), [25, 75, 125])
        self.checkResult('guide visible', guides.visible(), True)
        self.checkResult('guide locked', guides.locked(), True)
        self.checkResult('guide snap', guides.snap(), True)

    def setTestValues02(self, doc):
        guides = GuidesConfig()
        guides.setColor(QColor("#ff00ff"))
        guides.setLineType("dashed")
        guides.setHorizontalGuides([110, 120])
        guides.setVerticalGuides([220.22, 230.23])
        guides.setVisible(False)
        guides.setLocked(False)
        guides.setSnap(False)
        doc.setGuidesConfig(guides)

    def checkTestValues02(self, doc):
        guides = doc.guidesConfig()
        self.checkResult('guide color', guides.color().name(), "#ff00ff")
        self.checkResult('guide lineType', guides.lineType(), "dashed")
        self.checkResult('guide hasGuides', guides.hasGuides(), True)
        self.checkResult('guide horizontal', guides.horizontalGuides(), [110, 120])
        self.checkResult('guide vertical', guides.verticalGuides(), [220.22, 230.23])
        self.checkResult('guide visible', guides.visible(), False)
        self.checkResult('guide locked', guides.locked(), False)
        self.checkResult('guide snap', guides.snap(), False)

    def setTestValues03(self, doc):
        guides = GuidesConfig()
        guides.setColor(QColor("#008800"))
        guides.setLineType("dotted")
        guides.setHorizontalGuides([])
        guides.setVerticalGuides([])
        guides.setVisible(False)
        guides.setLocked(False)
        guides.setSnap(False)
        doc.setGuidesConfig(guides)

    def checkTestValues03(self, doc):
        guides = doc.guidesConfig()
        self.checkResult('guide color', guides.color().name(), "#008800")
        self.checkResult('guide lineType', guides.lineType(), "dotted")
        self.checkResult('guide hasGuides', guides.hasGuides(), False)
        self.checkResult('guide horizontal', guides.horizontalGuides(), [])
        self.checkResult('guide vertical', guides.verticalGuides(), [])
        self.checkResult('guide visible', guides.visible(), False)
        self.checkResult('guide locked', guides.locked(), False)
        self.checkResult('guide snap', guides.snap(), False)

    def checkResult(self, name, value, ref):
        returned = None

        if isinstance(value, float) and isinstance(ref, float):
            returned = qFuzzyCompare(value, ref)
        elif isinstance(value, list) and isinstance(ref, list) and len(value) == len(ref):
            returned = True
            for index in range(len(value)):
                if not qFuzzyCompare(value[index], ref[index]):
                    returned = False
                    break

        if returned or value == ref:
            returned = True
            isOk = "OK"
            qDebug(f"Check {name}: '{value}' == '{ref}'  -->  {isOk}")
        else:
            returned = False
            isOk = "INVALID"
            qWarning(f"Check {name}: '{value}' == '{ref}'  -->  {isOk}")

        return returned


test = TestDocument()
