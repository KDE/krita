# A script that converts the palette with the given name to a gimp
# palette at the location asked for.

# By Wolthera(originally)

# SPDX-License-Identifier: CC0-1.0

# @package palette_docker


# Importing the relevant dependencies:
try:
    from PyQt6.QtWidgets import QMessageBox
except:
    from PyQt5.QtWidgets import QMessageBox
from krita import Palette, FileDialog
from builtins import Application, i18n


class gimpPaletteExporter:

    def __init__(self, name):
        # We want people to select a palette and a location to save to...
        self.fileName = FileDialog.getExistingDirectory()
        if not self.fileName: return
        allPalettes = Application.resources("palette")
        self.paletteName = name
        self.currentPalette = Palette(allPalettes[self.paletteName])
        self.export()
        done = QMessageBox()
        done.setWindowTitle(i18n("Export Successful"))
        done.setText(
            str(i18n("{input} has been exported to {output}.")).format(
                input=self.paletteName, output=self.fileName))
        done.exec()
        pass

    def export(self):
        # open the appropriate file...
        gplFile = open(self.fileName + "/" + self.paletteName + ".gpl", "w")
        gplFile.write("GIMP Palette\n")
        gplFile.write("Name: %s\n" % self.paletteName)
        gplFile.write("Columns: %s\n" % self.currentPalette.columnCount())
        gplFile.write("#%s\n" % self.currentPalette.comment())

        groupNames = self.currentPalette.groupNames()
        for groupName in groupNames:
            slotCount = self.currentPalette.slotCountGroup(groupName)
            for i in range(slotCount):
                entry = self.currentPalette.entryByIndexFromGroup(i, groupName)
                color = entry.color()
                # convert to sRGB
                color.setColorSpace("RGBA", "U8", "sRGB built-in")
                red = max(min(int(color.componentsOrdered()[0] * 255), 255), 0)
                green = max(min(int(color.componentsOrdered()[1] * 255), 255), 0)
                blue = max(min(int(color.componentsOrdered()[2] * 255), 255), 0)
                name = f"{entry.id()}-{entry.name()}" if entry.id() else entry.name()
                gplFile.write(
                    "{red:>3} {green:>3} {blue:>3}    {name}\n".format(
                        red=red, green=green, blue=blue, name=name))
        gplFile.close()
