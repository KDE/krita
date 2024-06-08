#  SPDX-FileCopyrightText: 2024 Grum 999 <grum999@grum.fr>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#
# Execute this script in scripter, and check output results are OK
# - check document grid settings


from krita import (Document, GridConfig)
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
        grid = GridConfig()
        grid.setType("rectangular")
        grid.setVisible(True)
        grid.setSnap(True)
        grid.setOffset(QPoint(5, 10))
        grid.setSpacing(QPoint(15, 20))
        grid.setSpacingActiveHorizontal(True)
        grid.setSpacingActiveVertical(True)
        grid.setSubdivision(2)
        grid.setAngleLeft(0)
        grid.setAngleRight(0)
        grid.setAngleLeftActive(True)
        grid.setAngleRightActive(True)
        grid.setCellSpacing(10)
        grid.setCellSize(10)
        grid.setOffsetAspectLocked(False)
        grid.setSpacingAspectLocked(False)
        grid.setAngleAspectLocked(False)
        grid.setLineTypeMain("solid")
        grid.setLineTypeSubdivision("dashed")
        grid.setLineTypeVertical("dotted")
        grid.setColorMain(QColor("#ff0000"))
        grid.setColorSubdivision(QColor("#00ff00"))
        grid.setColorVertical(QColor("#0000ff"))
        doc.setGridConfig(grid)

    def checkTestValues01(self, doc):
        grid = doc.gridConfig()
        self.checkResult('grid type', grid.type(), "rectangular")
        self.checkResult('grid visible', grid.visible(), True)
        self.checkResult('grid snap', grid.snap(), True)
        self.checkResult('grid offset', grid.offset(), QPoint(5, 10))
        self.checkResult('grid spacing', grid.spacing(), QPoint(15, 20))
        self.checkResult('grid spacing active H', grid.spacingActiveHorizontal(), True)
        self.checkResult('grid spacing active V', grid.spacingActiveVertical(), True)
        self.checkResult('grid subdivision', grid.subdivision(), 2)
        self.checkResult('grid angle left', grid.angleLeft(), 0)
        self.checkResult('grid angle right', grid.angleRight(), 0)
        self.checkResult('grid angle left active', grid.angleLeftActive(), True)
        self.checkResult('grid angle right active', grid.angleRightActive(), True)
        self.checkResult('grid cell spacing', grid.cellSpacing(), 10)
        self.checkResult('grid cell size', grid.cellSize(), 10)
        self.checkResult('grid offset aspect locked', grid.offsetAspectLocked(), False)
        self.checkResult('grid spacing aspect locked', grid.spacingAspectLocked(), False)
        self.checkResult('grid angle aspect locked', grid.angleAspectLocked(), False)
        self.checkResult('grid line type main', grid.lineTypeMain(), "solid")
        self.checkResult('grid line type subdivision', grid.lineTypeSubdivision(), "dashed")
        self.checkResult('grid line type verical', grid.lineTypeVertical(), "dotted")
        self.checkResult('grid color main', grid.colorMain().name(), "#ff0000")
        self.checkResult('grid color subdivision', grid.colorSubdivision().name(), "#00ff00")
        self.checkResult('grid color vertical', grid.colorVertical().name(), "#0000ff")

    def setTestValues02(self, doc):
        grid = GridConfig()
        grid.setType("isometric_legacy")
        grid.setVisible(False)
        grid.setSnap(False)
        grid.setOffset(QPoint(10, 5))
        grid.setSpacing(QPoint(20, 15))
        grid.setSpacingActiveHorizontal(False)
        grid.setSpacingActiveVertical(False)
        grid.setSubdivision(3)
        grid.setAngleLeft(15)
        grid.setAngleRight(30)
        grid.setAngleLeftActive(False)
        grid.setAngleRightActive(False)
        grid.setCellSpacing(10)
        grid.setCellSize(15)
        grid.setOffsetAspectLocked(True)
        grid.setSpacingAspectLocked(True)
        grid.setAngleAspectLocked(True)
        grid.setLineTypeMain("dashed")
        grid.setLineTypeSubdivision("dotted")
        grid.setLineTypeVertical("none")
        grid.setColorMain(QColor("#00ff00"))
        grid.setColorSubdivision(QColor("#0000ff"))
        grid.setColorVertical(QColor("#ff0000"))
        doc.setGridConfig(grid)

    def checkTestValues02(self, doc):
        grid = doc.gridConfig()
        self.checkResult('grid type', grid.type(), "isometric_legacy")
        self.checkResult('grid visible', grid.visible(), False)
        self.checkResult('grid snap', grid.snap(), False)
        self.checkResult('grid offset', grid.offset(), QPoint(10, 5))
        self.checkResult('grid spacing', grid.spacing(), QPoint(20, 15))
        self.checkResult('grid spacing active H', grid.spacingActiveHorizontal(), False)
        self.checkResult('grid spacing active V', grid.spacingActiveVertical(), False)
        self.checkResult('grid subdivision', grid.subdivision(), 3)
        self.checkResult('grid angle left', grid.angleLeft(), 15)
        self.checkResult('grid angle right', grid.angleRight(), 30)
        self.checkResult('grid angle left active', grid.angleLeftActive(), False)
        self.checkResult('grid angle right active', grid.angleRightActive(), False)
        self.checkResult('grid cell spacing', grid.cellSpacing(), 10)
        self.checkResult('grid cell size', grid.cellSize(), 15)
        self.checkResult('grid offset aspect locked', grid.offsetAspectLocked(), True)
        self.checkResult('grid spacing aspect locked', grid.spacingAspectLocked(), True)
        self.checkResult('grid angle aspect locked', grid.angleAspectLocked(), True)
        self.checkResult('grid line type main', grid.lineTypeMain(), "dashed")
        self.checkResult('grid line type subdivision', grid.lineTypeSubdivision(), "dotted")
        self.checkResult('grid line type verical', grid.lineTypeVertical(), "none")
        self.checkResult('grid color main', grid.colorMain().name(), "#00ff00")
        self.checkResult('grid color subdivision', grid.colorSubdivision().name(), "#0000ff")
        self.checkResult('grid color vertical', grid.colorVertical().name(), "#ff0000")

    def setTestValues03(self, doc):
        grid = GridConfig()
        grid.setType("isometric")
        grid.setVisible(False)
        grid.setSnap(False)
        grid.setOffset(QPoint(-10, -10))
        grid.setSpacing(QPoint(-20, -20))
        grid.setSpacingActiveHorizontal(False)
        grid.setSpacingActiveVertical(False)
        grid.setSubdivision(-3)
        grid.setAngleLeft(-15)
        grid.setAngleRight(-30)
        grid.setAngleLeftActive(False)
        grid.setAngleRightActive(False)
        grid.setCellSpacing(-10)
        grid.setCellSize(-15)
        grid.setOffsetAspectLocked(True)
        grid.setSpacingAspectLocked(True)
        grid.setAngleAspectLocked(True)
        grid.setLineTypeMain("none")
        grid.setLineTypeSubdivision("none")
        grid.setLineTypeVertical("none")
        grid.setColorMain(QColor("#00ff00"))
        grid.setColorSubdivision(QColor("#0000ff"))
        grid.setColorVertical(QColor("#ff0000"))
        doc.setGridConfig(grid)

    def checkTestValues03(self, doc):
        grid = doc.gridConfig()
        self.checkResult('grid type', grid.type(), "isometric")
        self.checkResult('grid visible', grid.visible(), False)
        self.checkResult('grid snap', grid.snap(), False)
        self.checkResult('grid offset', grid.offset(), QPoint())
        self.checkResult('grid spacing', grid.spacing(), QPoint(1, 1))
        self.checkResult('grid spacing active H', grid.spacingActiveHorizontal(), False)
        self.checkResult('grid spacing active V', grid.spacingActiveVertical(), False)
        self.checkResult('grid subdivision', grid.subdivision(), 1)
        self.checkResult('grid angle left', grid.angleLeft(), 0.0)
        self.checkResult('grid angle right', grid.angleRight(), 0.0)
        self.checkResult('grid angle left active', grid.angleLeftActive(), False)
        self.checkResult('grid angle right active', grid.angleRightActive(), False)
        self.checkResult('grid cell spacing', grid.cellSpacing(), 10)
        self.checkResult('grid cell size', grid.cellSize(), 10)
        self.checkResult('grid offset aspect locked', grid.offsetAspectLocked(), True)
        self.checkResult('grid spacing aspect locked', grid.spacingAspectLocked(), True)
        self.checkResult('grid angle aspect locked', grid.angleAspectLocked(), True)
        self.checkResult('grid line type main', grid.lineTypeMain(), "solid")
        self.checkResult('grid line type subdivision', grid.lineTypeSubdivision(), "solid")
        self.checkResult('grid line type verical', grid.lineTypeVertical(), "none")
        self.checkResult('grid color main', grid.colorMain().name(), "#00ff00")
        self.checkResult('grid color subdivision', grid.colorSubdivision().name(), "#0000ff")
        self.checkResult('grid color vertical', grid.colorVertical().name(), "#ff0000")

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
