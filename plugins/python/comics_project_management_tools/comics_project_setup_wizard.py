"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
This is a wizard that helps you set up a comics project in Krita.
"""

import json  # For writing to json.
import os  # For finding the script location.
from pathlib import Path  # For reading all the files in a directory.
import random  # For selecting two random words from a list.
from PyQt5.QtWidgets import QWidget, QWizard, QWizardPage, QHBoxLayout, QFormLayout, QFileDialog, QLineEdit, QPushButton, QCheckBox, QLabel, QDialog
from PyQt5.QtCore import QDate, QLocale, QUuid
from krita import *
from . import comics_metadata_dialog

"""
The actual wizard.
"""


class ComicsProjectSetupWizard():
    setupDictionary = {}
    projectDirectory = ""

    def __init__(self):
        # super().__init__(parent)
        # Search the location of the script for the two lists that are used with the projectname generator.
        mainP = Path(__file__).parent
        self.generateListA = []
        self.generateListB = []
        if Path(mainP / "projectGenLists" / "listA.txt").exists():
            for l in open(str(mainP / "projectGenLists" / "listA.txt"), "r"):
                if l.isspace() == False:
                    self.generateListA.append(l.strip("\n"))
        if Path(mainP / "projectGenLists" / "listB.txt").exists():
            for l in open(str(mainP / "projectGenLists" / "listB.txt"), "r"):
                if l.isspace() == False:
                    self.generateListB.append(l.strip("\n"))

    def showDialog(self):
        # Initialise the setup directory empty toavoid exceptions.
        self.setupDictionary = {}

        # ask for a project directory.
        self.projectDirectory = None
        
        while self.projectDirectory == None:
            self.projectDirectory = QFileDialog.getExistingDirectory(caption=i18n("Where should the comic project go?"), options=QFileDialog.ShowDirsOnly)
            if os.path.exists(self.projectDirectory) is False:
                return
            if os.access(self.projectDirectory, os.W_OK) is False:
                QMessageBox.warning(None, i18n("Folder cannot be used"), i18n("Krita doesn't have write access to this folder, so files cannot be made. Please choose a different folder."), QMessageBox.Ok)
                self.projectDirectory = None
        self.pagesDirectory = os.path.relpath(self.projectDirectory, self.projectDirectory)
        self.exportDirectory = os.path.relpath(self.projectDirectory, self.projectDirectory)

        wizard = QWizard()
        wizard.setWindowTitle(i18n("Comic Project Setup"))
        wizard.setOption(QWizard.IndependentPages, True)
        wizard.setWizardStyle(QWizard.ClassicStyle)

        # Set up the UI for the wizard
        basicsPage = QWizardPage()
        basicsPage.setTitle(i18n("Basic Comic Project Settings"))
        formLayout = QFormLayout()
        basicsPage.setLayout(formLayout)
        projectLayout = QHBoxLayout()
        self.lnProjectName = QLineEdit()
        basicsPage.registerField("Project Name*", self.lnProjectName)
        self.lnProjectName.setToolTip(i18n("A Project name. This can be different from the eventual title"))
        btnRandom = QPushButton()
        btnRandom.setText(i18n("Generate"))
        btnRandom.setToolTip(i18n("If you cannot come up with a project name, our highly sophisticated project name generator will serve to give a classy yet down to earth name."))
        btnRandom.clicked.connect(self.slot_generate)
        projectLayout.addWidget(self.lnProjectName)
        projectLayout.addWidget(btnRandom)
        lnConcept = QLineEdit()
        lnConcept.setToolTip(i18n("What is your comic about? This is mostly for your own convenience so do not worry about what it says too much."))
        self.cmbLanguage = comics_metadata_dialog.language_combo_box()
        self.cmbLanguage.setToolTip(i18n("The main language the comic is in"))
        self.cmbLanguage.setEntryToCode(str(QLocale.system().name()).split("_")[0])
        self.cmbCountry = comics_metadata_dialog.country_combo_box()
        if QLocale.system() != QLocale.c():
            self.slot_update_countries()
            self.cmbCountry.setEntryToCode(str(QLocale.system().name()).split("_")[-1])
        else:
            self.cmbLanguage.setEntryToCode("en")
            self.slot_update_countries()
            self.cmbCountry.setEntryToCode("US")
        self.cmbLanguage.currentIndexChanged.connect(self.slot_update_countries)
        self.lnProjectDirectory = QLabel(self.projectDirectory)
        self.chkMakeProjectDirectory = QCheckBox()
        labelDirectory = QLabel(i18n("Make a new directory with the project name."))
        labelDirectory.setWordWrap(True)
        stringDirectoryTooltip = i18n("This allows you to select a generic comics project directory, in which a new folder will be made for the project using the given project name.")
        self.chkMakeProjectDirectory.setToolTip(stringDirectoryTooltip)
        labelDirectory.setToolTip(stringDirectoryTooltip)
        self.chkMakeProjectDirectory.setChecked(True)
        self.lnPagesDirectory = QLineEdit()
        self.lnPagesDirectory.setText(i18n("pages"))
        self.lnPagesDirectory.setToolTip(i18n("The name for the folder where the pages are contained. If it does not exist, it will be created."))
        self.lnExportDirectory = QLineEdit()
        self.lnExportDirectory.setText(i18n("export"))
        self.lnExportDirectory.setToolTip(i18n("The name for the folder where the export is put. If it does not exist, it will be created."))
        self.lnTemplateLocation = QLineEdit()
        self.lnTemplateLocation.setText(i18n("templates"))
        self.lnTemplateLocation.setToolTip(i18n("The name for the folder where the page templates are sought in."))

        self.lnTranslationLocation = QLineEdit()
        self.lnTranslationLocation.setText(i18n("translations"))
        self.lnTranslationLocation.setToolTip("This is the location that POT files will be stored to and PO files will be read from.")
        formLayout.addRow(i18n("Comic concept:"), lnConcept)
        formLayout.addRow(i18n("Project name:"), projectLayout)
        formLayout.addRow(i18n("Main language:"), self.cmbLanguage)
        formLayout.addRow("", self.cmbCountry)

        buttonMetaData = QPushButton(i18n("Meta Data"))
        buttonMetaData.clicked.connect(self.slot_edit_meta_data)

        wizard.addPage(basicsPage)

        foldersPage = QWizardPage()
        foldersPage.setTitle(i18n("Folder names and other."))
        folderFormLayout = QFormLayout()
        foldersPage.setLayout(folderFormLayout)
        folderFormLayout.addRow(i18n("Project directory:"), self.lnProjectDirectory)
        folderFormLayout.addRow(self.chkMakeProjectDirectory, labelDirectory)
        folderFormLayout.addRow(i18n("Pages directory"), self.lnPagesDirectory)
        folderFormLayout.addRow(i18n("Export directory"), self.lnExportDirectory)
        folderFormLayout.addRow(i18n("Template directory"), self.lnTemplateLocation)
        folderFormLayout.addRow(i18n("Translation directory"), self.lnTranslationLocation)
        folderFormLayout.addRow("", buttonMetaData)
        wizard.addPage(foldersPage)

        # Execute the wizard, and after wards...
        if (wizard.exec_()):

            # First get the directories, check if the directories exist, and otherwise make them.
            self.pagesDirectory = self.lnPagesDirectory.text()
            self.exportDirectory = self.lnExportDirectory.text()
            self.templateLocation = self.lnTemplateLocation.text()
            self.translationLocation = self.lnTranslationLocation.text()
            projectPath = Path(self.projectDirectory)
            
            # Only make a project directory if the checkbox for that has been checked.
            if self.chkMakeProjectDirectory.isChecked():
                projectPath = projectPath / self.lnProjectName.text()
                if projectPath.exists() is False:
                    projectPath.mkdir()
                self.projectDirectory = str(projectPath)
            if Path(projectPath / self.pagesDirectory).exists() is False:
                Path(projectPath / self.pagesDirectory).mkdir()
            if Path(projectPath / self.exportDirectory).exists() is False:
                Path(projectPath / self.exportDirectory).mkdir()
            if Path(projectPath / self.templateLocation).exists() is False:
                Path(projectPath / self.templateLocation).mkdir()
            if Path(projectPath / self.translationLocation).exists() is False:
                Path(projectPath / self.translationLocation).mkdir()

            # Then store the information into the setup diactionary.
            self.setupDictionary["projectName"] = self.lnProjectName.text()
            self.setupDictionary["concept"] = lnConcept.text()
            self.setupDictionary["language"] = str(self.cmbLanguage.codeForCurrentEntry())
            self.setupDictionary["pagesLocation"] = self.pagesDirectory
            self.setupDictionary["exportLocation"] = self.exportDirectory
            self.setupDictionary["templateLocation"] = self.templateLocation
            self.setupDictionary["translationLocation"] = self.translationLocation
            self.setupDictionary["uuid"] = QUuid.createUuid().toString()

            # Finally, write the dictionary into the json file.
            self.writeConfig()
    """
    This calls up the metadata dialog, for if people already have information they want to type in
    at the setup stage. Not super likely, but the organisation and management aspect of the comic
    manager means we should give the option to organise as smoothly as possible.
    """

    def slot_edit_meta_data(self):
        dialog = comics_metadata_dialog.comic_meta_data_editor()
        self.setupDictionary["language"] = str(self.cmbLanguage.codeForCurrentEntry())
        dialog.setConfig(self.setupDictionary)
        dialog.setConfig(self.setupDictionary)
        if (dialog.exec_() == QDialog.Accepted):
            self.setupDictionary = dialog.getConfig(self.setupDictionary)
            self.cmbLanguage.setEntryToCode(self.setupDictionary["language"])
            
    """
    Update the country list when the language list changes.
    """
    
    def slot_update_countries(self):
        code = self.cmbLanguage.codeForCurrentEntry()
        self.cmbCountry.set_country_for_locale(code)
        
    """
    Write the actual config to the chosen project directory.
    """

    def writeConfig(self):
        print("CPMT: writing comic configuration...")
        print(self.projectDirectory)
        configFile = open(os.path.join(self.projectDirectory, "comicConfig.json"), "w", newline="", encoding="utf-16")
        json.dump(self.setupDictionary, configFile, indent=4, sort_keys=True, ensure_ascii=False)
        configFile.close()
        print("CPMT: done")
    """
    As you may be able to tell, the random projectname generator is hardly sophisticated.
    It picks a word from a list of names of figures from Greek Mythology, and a name from a list
    of vegetables and fruits and combines the two camelcased.
    It makes for good codenames at the least.
    """

    def slot_generate(self):
        if len(self.generateListA) > 0 and len(self.generateListB) > 0:
            nameA = self.generateListA[random.randint(0, len(self.generateListA) - 1)]
            nameB = self.generateListB[random.randint(0, len(self.generateListB) - 1)]
            self.lnProjectName.setText(str(nameA.title() + nameB.title()))
