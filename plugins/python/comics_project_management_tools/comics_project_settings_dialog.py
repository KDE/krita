"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
A dialog for editing the general project settings.
"""
import os
from PyQt5.QtWidgets import QWidget, QDialog, QDialogButtonBox, QHBoxLayout, QFormLayout, QPushButton, QLabel, QLineEdit, QToolButton, QFrame, QAction, QFileDialog, QComboBox, QSizePolicy
from PyQt5.QtCore import QDir, Qt, pyqtSignal
from krita import *

"""
A Widget that contains both a qlabel and a button for selecting a path.
"""


class path_select(QWidget):
    projectUrl = ""
    question = i18n("Which folder?")

    """
    emits when a new directory has been chosen.
    """
    locationChanged = pyqtSignal()
    """
    Initialise the widget.
    @param question is the question asked when selecting a directory.
    @param project url is the url to which the label is relative.
    """

    def __init__(self, parent=None, flags=None, question=str(), projectUrl=None):
        super(path_select, self).__init__(parent)
        self.setLayout(QHBoxLayout())
        self.location = QLabel()
        self.button = QToolButton()  # Until we have a proper icon
        self.layout().addWidget(self.location)
        self.layout().addWidget(self.button)
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Minimum)
        self.location.setFrameStyle(QFrame.StyledPanel | QFrame.Sunken)
        self.button.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        self.location.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Minimum)
        self.location.setAlignment(Qt.AlignRight)
        self.location.setLineWidth(1)
        if projectUrl is None:
            self.projectUrl = QDir.homePath()
        else:
            self.projectUrl = projectUrl
        self.question = question
        self.action_change_folder = QAction(i18n("Change Folder"), self)
        self.action_change_folder.setIcon(Application.icon("folder"))
        self.action_change_folder.triggered.connect(self.slot_change_location)
        self.button.setDefaultAction(self.action_change_folder)

    """
    pops up a directory chooser widget, and when a directory is chosen a locationChanged signal is emitted.
    """

    def slot_change_location(self):
        location = QFileDialog.getExistingDirectory(caption=self.question, directory=self.projectUrl)
        if location is not None and location.isspace() is False and len(location) > 0:
            location = os.path.relpath(location, self.projectUrl)
            self.location.setText(location)
            self.locationChanged.emit()
    """
    Set the location.
    @param path - the location relative to the projectUrl.
    """

    def setLocation(self, path=str()):
        self.location.setText(path)
    """
    Get the location.
    @returns a string with the location relative to the projectUrl.
    """

    def getLocation(self):
        return str(self.location.text())


"""
Dialog for editing basic proect details like the project name, default template,
template location, etc.
"""


class comics_project_details_editor(QDialog):
    configGroup = "ComicsProjectManagementTools"
    """
    Initialise the editor.
    @param projectUrl - The directory to which all paths are relative.
    """

    def __init__(self, projectUrl=str()):
        super().__init__()
        self.projectUrl = projectUrl
        layout = QFormLayout()
        self.setLayout(layout)
        self.setWindowTitle(i18n("Comic Project Settings"))
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        self.lnProjectName = QLineEdit()
        self.lnProjectConcept = QLineEdit()
        self.cmb_defaultTemplate = QComboBox()

        self.pagesLocation = path_select(question=i18n("Where should the pages go?"), projectUrl=self.projectUrl)
        self.exportLocation = path_select(question=i18n("Where should the export go?"), projectUrl=self.projectUrl)
        self.templateLocation = path_select(question=i18n("Where are the templates?"), projectUrl=self.projectUrl)
        self.translationLocation = path_select(question=i18n("Where are the translations?"), projectUrl=self.projectUrl)
        self.keyLocation = path_select(question=i18n("Where are the extra auto-completion keys located?"))
        self.keyLocation.setToolTip(i18n("The location for extra autocompletion keys in the metadata editor. Point this at a folder containing key_characters/key_format/key_genre/key_rating/key_author_roles/key_other with inside txt files (csv for rating) containing the extra auto-completion keys, each on a new line. This path is stored in the Krita configuration, and not the project configuration."))
        self.templateLocation.locationChanged.connect(self.refill_templates)

        layout.addRow(i18n("Project name:"), self.lnProjectName)
        layout.addRow(i18n("Project concept:"), self.lnProjectConcept)
        layout.addRow(i18n("Pages folder:"), self.pagesLocation)
        layout.addRow(i18n("Export folder:"), self.exportLocation)
        layout.addRow(i18n("Template folder:"), self.templateLocation)
        layout.addRow(i18n("Translation folder:"), self.translationLocation)
        layout.addRow(i18n("Default template:"), self.cmb_defaultTemplate)
        layout.addRow(i18n("Extra keys folder:"), self.keyLocation)

        self.layout().addWidget(buttons)

    """
    Fill the templates doc with the kra files found in the templates directory.
    Might want to extend this to other files as well, as they basically get resaved anyway...
    """

    def refill_templates(self):
        self.cmb_defaultTemplate.clear()
        templateLocation = os.path.join(self.projectUrl, self.templateLocation.getLocation())
        for entry in os.scandir(templateLocation):
            if entry.name.endswith('.kra') and entry.is_file():
                name = os.path.relpath(entry.path, templateLocation)
                self.cmb_defaultTemplate.addItem(name)

    """
    Load the UI values from the config dictionary given.
    """

    def setConfig(self, config, projectUrl):

        self.projectUrl = projectUrl
        if "projectName"in config.keys():
            self.lnProjectName.setText(config["projectName"])
        if "concept"in config.keys():
            self.lnProjectConcept.setText(config["concept"])
        if "pagesLocation" in config.keys():
            self.pagesLocation.setLocation(config.get("pagesLocation", "pages"))
            self.exportLocation.setLocation(config.get("exportLocation", "export"))
            self.templateLocation.setLocation(config.get("templateLocation", "templates"))
            self.translationLocation.setLocation(config.get("translationLocation", "translations"))
            self.refill_templates()
        self.keyLocation.setLocation(Application.readSetting(self.configGroup, "extraKeysLocation", str()))

    """
    Store the GUI values into the config dictionary given.
    
    @return the config diactionary filled with new values.
    """

    def getConfig(self, config):
        config["projectName"] = self.lnProjectName.text()
        config["concept"] = self.lnProjectConcept.text()
        config["pagesLocation"] = self.pagesLocation.getLocation()
        config["exportLocation"] = self.exportLocation.getLocation()
        config["templateLocation"] = self.templateLocation.getLocation()
        config["translationLocation"] = self.translationLocation.getLocation()
        config["singlePageTemplate"] = os.path.join(self.templateLocation.getLocation(), self.cmb_defaultTemplate.currentText())
        Application.writeSetting(self.configGroup, "extraKeysLocation", self.keyLocation.getLocation())
        return config
