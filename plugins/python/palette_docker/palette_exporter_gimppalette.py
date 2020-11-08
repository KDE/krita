# A script that converts the palette with the given name to a gimp
# palette at the location asked for.

# By Wolthera(originally)

# SPDX-License-Identifier: CC0-1.0

# @package palette_docker


# Importing the relevant dependencies:
from PyQt5.QtWidgets import QFileDialog, QMessageBox
from krita import Palette


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
        done.setText(
            str(i18n("{input} has been exported to {output}.")).format(
                input=self.paletteName, output=self.fileName))
        done.exec_()
        pass

    def export(self):
        # open the appropriate file...
        gplFile = open(self.fileName + "/" + self.paletteName + ".gpl", "w")
        gplFile.write("GIMP Palette\n")
        gplFile.write("Name: %s\n" % self.paletteName)
        gplFile.write("Columns: %s/n", self.currentPalette.columnCount())
        gplFile.write("#%s\n" % self.currentPalette.comment())
        colorCount = self.currentPalette.colorsCountGroup("")

        for i in range(colorCount):
            entry = self.currentPalette.colorSetEntryFromGroup(i, "")
            color = self.currentPalette.colorForEntry(entry)
            # convert to sRGB
            color.setColorSpace("RGBA", "U8", "sRGB built-in")

            red = max(min(int(color.componentsOrdered()[0] * 255), 255), 0)
            green = max(min(int(color.componentsOrdered()[1] * 255), 255), 0)
            blue = max(min(int(color.componentsOrdered()[2] * 255), 255), 0)
            gplFile.write(
                "{red} {green} {blue}    {id}-{name}\n".format(
                    red=red, green=green, blue=blue, id=entry.id(),
                    name=entry.name))
            groupNames = self.currentPalette.groupNames()
            for groupName in groupNames:
                colorCount = self.currentPalette.colorsCountGroup(groupName)
                for i in range(colorCount):
                    entry = self.currentPalette.colorSetEntryFromGroup(
                        i, groupName)
                    color = self.currentPalette.colorForEntry(entry)
                    # convert to sRGB
                    color.setColorSpace("RGBA", "U8", "sRGB built-in")
                    red = max(
                        min(int(color.componentsOrdered()[0] * 255), 255), 0)
                    green = max(
                        min(int(color.componentsOrdered()[1] * 255), 255), 0)
                    blue = max(
                        min(int(color.componentsOrdered()[2] * 255), 255), 0)
                    gplFile.write(
                        "{red} {green} {blue}    {id}-{name}\n".format(
                            red=red, green=green, blue=blue, id=entry.id(),
                            name=entry.name))
        gplFile.close()
