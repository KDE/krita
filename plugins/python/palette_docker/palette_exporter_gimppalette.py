'''
A script that converts the palette with the given name to a gimp palette at the location asked for.

By Wolthera(originally)

This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode

@package palette_docker
'''


# Importing the relevant dependencies:
import sys
from PyQt5.QtWidgets import QFileDialog, QMessageBox
import math
from krita import *


class gimpPaletteExporter:

    def __init__(self, name):
        # We want people to select a palette and a location to save to...
        self.fileName = QFileDialog.getExistingDirectory()
        allPalettes = Application.resources("palette")
        self.paletteName = name
        self.currentPalette = Palette(allPalettes[self.paletteName])
        self.export()
        done = QMessageBox()
        done.setWindowTitle(i18n("Export Successful"))
        done.setText(str(i18n("{input} has been exported to {output}.")).format(input=self.paletteName, output=self.fileName))
        done.exec_()
        pass

    def export(self):
        # open the appropriate file...
        gplFile = open(self.fileName + "/" + self.paletteName + ".gpl", "w")
        gplFile.write("GIMP Palette\n")
        gplFile.write("Name: " + self.paletteName + "\n")
        gplFile.write("Columns: " + str(self.currentPalette.columnCount()) + "\n")
        gplFile.write("#" + self.currentPalette.comment() + "\n")
        colorCount = self.currentPalette.colorsCountGroup("")

        for i in range(colorCount):
            entry = self.currentPalette.colorSetEntryFromGroup(i, "")
            color = self.currentPalette.colorForEntry(entry)
            # convert to sRGB
            color.setColorSpace("RGBA", "U8", "sRGB built-in")

            red = max(min(int(color.componentsOrdered()[0] * 255), 255), 0)
            green = max(min(int(color.componentsOrdered()[1] * 255), 255), 0)
            blue = max(min(int(color.componentsOrdered()[2] * 255), 255), 0)
            gplFile.write(str(red) + " " + str(green) + " " + str(blue) + "    " + entry.id() + "-" + entry.name() + "\n")
            groupNames = self.currentPalette.groupNames()
            for groupName in groupNames:
                colorCount = self.currentPalette.colorsCountGroup(groupName)
                for i in range(colorCount):
                    entry = self.currentPalette.colorSetEntryFromGroup(i, groupName)
                    color = self.currentPalette.colorForEntry(entry)
                    # convert to sRGB
                    color.setColorSpace("RGBA", "U8", "sRGB built-in")
                    red = max(min(int(color.componentsOrdered()[0] * 255), 255), 0)
                    green = max(min(int(color.componentsOrdered()[1] * 255), 255), 0)
                    blue = max(min(int(color.componentsOrdered()[2] * 255), 255), 0)
                    gplFile.write(str(red) + " " + str(green) + " " + str(blue) + "    " + entry.id() + "-" + entry.name() + "\n")
        gplFile.close()
