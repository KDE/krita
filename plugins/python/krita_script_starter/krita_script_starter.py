#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

"""
BBD's Krita script starter

This script does the boring stuff involved in creating a script for Krita.
it creates
* a directory for the script in the correct Krita resources subdirectory,
* populates that directory with:
-- a __init__.py file,
-- a skeleton file for the script proper
-- a Manual.html file
* creates a .desktop file for the script
It also:
* correctly imports the script proper nto __init__.py, creates
* creates basic skeleton code depending on whether the script is intended
to be an extension or a docker
* creates skeleton code in the Manual.html file
* (optionally) automatically enables the script in the Krita menu

Script can be run from the command line. This can be used to
bootstrap the script into a Krita menu entry - create a new script
called Krita Script Starter, then copy the script (and the .ui file)
into the directory you have just created, overwriting the existing
files.

BBD
16 March 2018
"""

import os
import sys
from PyQt5.QtCore import QStandardPaths, QSettings
from PyQt5.QtWidgets import QApplication, QWidget, QMessageBox
import PyQt5.uic as uic

try:
    import krita
    CONTEXT_KRITA = True
    EXTENSION = krita.Extension

except ImportError:
    # script being run in testing environment without Krita
    CONTEXT_KRITA = False
    EXTENSION = QWidget

# TESTING = True
TESTING = False

MAIN_KRITA_ID = "Krita Script Starter"
MAIN_KRITA_MENU_ENTRY = "Krita Script Starter"

SCRIPT_NAME = "script_name"
SCRIPT_COMMENT = "script_comment"
KRITA_ID = "krita_id"
LIBRARY_NAME = "library_name"
MENU_ENTRY = "menu_entry"
SCRIPT_TYPE = "script_type"  # extension v docker
PYTHON_FILE_NAME = "python_file"
CLASS_NAME = "class_name"

# from LIBRARY_NAME get:
# the name of the directory
# the name of the main python file
# the name of the class

SCRIPT_EXTENSION = "Extension"
SCRIPT_DOCKER = "Docker`"

SCRIPT_SETTINGS = 'python'

UI_FILE = "bbdkss.ui"


def load_ui(ui_file):
    """If this script has been distributed with a ui file in the same
    directory, then find that directory (since it will likely be
    different from krita's current working directory) and use that to
    load the ui file.

    return the loaded ui
    """
    abs_path = os.path.dirname(os.path.realpath(__file__))
    ui_file = os.path.join(abs_path, UI_FILE)
    return uic.loadUi(ui_file)


DESKTOP_TEMPLATE = """[Desktop Entry]
Type=Service
ServiceTypes=Krita/PythonPlugin
X-KDE-Library={library_name}
X-Python-2-Compatible=false
X-Krita-Manual=Manual.html
Name={script_name}
Comment={script_comment}
"""

INIT_TEMPLATE_EXTENSION = """from .{library_name} import {class_name}

# And add the extension to Krita's list of extensions:
app = Krita.instance()
# Instantiate your class:
extension = {class_name}(parent = app)
app.addExtension(extension)
"""

INIT_TEMPLATE_DOCKER = """from .{library_name} import {class_name}
"""


EXTENSION_TEMPLATE = """# BBD's Krita Script Starter Feb 2018

from krita import Extension

EXTENSION_ID = '{krita_id}'
MENU_ENTRY = '{menu_entry}'


class {class_name}(Extension):

    def __init__(self, parent):
        # Always initialise the superclass.
        # This is necessary to create the underlying C++ object
        super().__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction(EXTENSION_ID, MENU_ENTRY, "tools/scripts")
        # parameter 1 = the name that Krita uses to identify the action
        # parameter 2 = the text to be added to the menu entry for this script
        # parameter 3 = location of menu entry
        action.triggered.connect(self.action_triggered)

    def action_triggered(self):
        pass  # your active code goes here.
"""

DOCKER_TEMPLATE = """#BBD's Krita Script Starter Feb 2018
from krita import DockWidget, DockWidgetFactory, DockWidgetFactoryBase

DOCKER_NAME = '{script_name}'
DOCKER_ID = '{krita_id}'


class {class_name}(DockWidget):

    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)

    def canvasChanged(self, canvas):
        pass


instance = Krita.instance()
dock_widget_factory = DockWidgetFactory(DOCKER_ID,
                                        DockWidgetFactoryBase.DockRight,
                                        {class_name})

instance.addDockWidgetFactory(dock_widget_factory)
"""

MANUAL_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<!--BBD's Krita Script Starter, Feb 2018 -->
<head><title>{script_name}</title>
</head>
<body>
<h3>{script_name}</h3>
Tell people about what your script does here.
This is an html document so you can format it with html tags.
<h3>Usage</h3>
Tell people how to use your script here.

</body>
</html>"""


class KritaScriptStarter(EXTENSION):

    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        self.script_abs_path = os.path.dirname(os.path.realpath(__file__))
        self.ui_file = os.path.join(self.script_abs_path, UI_FILE)
        self.ui = load_ui(self.ui_file)

        self.ui.e_name_of_script.textChanged.connect(self.name_change)
        self.ui.cancel_button.clicked.connect(self.cancel)
        self.ui.create_button.clicked.connect(self.create)

        target_directory = Krita.instance().getAppDataLocation()
        if not CONTEXT_KRITA:
            target_directory = os.path.join(target_directory, "krita")
        target_directory = os.path.join(target_directory, "pykrita")
        self.target_directory = target_directory

    def createActions(self, window):
        """ Called by Krita to create actions."""
        action = window.createAction(
            MAIN_KRITA_ID, MAIN_KRITA_MENU_ENTRY, "tools/scripts")
        # parameter 1 = the name that Krita uses to identify the action
        # parameter 2 = the text to be added to the menu entry for this script
        # parameter 3 = location of menu entry
        action.triggered.connect(self.action_triggered)

    def action_triggered(self):
        self.ui.show()
        self.ui.activateWindow()

    def cancel(self):
        self.ui.close()

    def create(self):
        """Go ahead and create the relevant files. """

        if self.ui.e_name_of_script.text().strip() == "":
            # Don't create script with empty name
            return

        def full_dir(path):
            # convenience function
            return os.path.join(self.target_directory, path)

        # should already be done, but just in case:
        self.calculate_file_names(self.ui.e_name_of_script.text())

        menu_entry = self.ui.e_menu_entry.text()
        if menu_entry.strip() == "":
            menu_entry = self.ui.e_name_of_script.text()

        comment = self.ui.e_comment.text()
        if comment.strip() == "":
            comment = "Replace this text with your description"

        values = {
            SCRIPT_NAME: self.ui.e_name_of_script.text(),
            SCRIPT_COMMENT: comment,
            KRITA_ID: "pykrita_" + self.package_name,
            SCRIPT_TYPE: SCRIPT_DOCKER if self.ui.rb_docker.isChecked() else SCRIPT_EXTENSION,  # noqa: E501
            MENU_ENTRY: menu_entry,
            LIBRARY_NAME: self.package_name,
            CLASS_NAME: self.class_name
        }

        try:
            # create package directory
            package_directory = full_dir(self.package_name)
            # needs to be lowercase and no spaces
            os.mkdir(package_directory)
        except FileExistsError:
            # if package directory exists write into it, overwriting
            # existing files.
            pass

        # create desktop file
        fn = full_dir(self.desktop_fn)
        with open(fn, 'w+t') as f:
            f.write(DESKTOP_TEMPLATE.format(**values))

        fn = full_dir(self.init_name)
        with open(fn, 'w+t') as f:
            if self.ui.rb_docker.isChecked():
                f.write(INIT_TEMPLATE_DOCKER.format(**values))
            else:
                f.write(INIT_TEMPLATE_EXTENSION.format(**values))


        # create main package file
        fn = full_dir(self.package_file)

        if self.ui.rb_docker.isChecked():
            with open(fn, 'w+t') as f:
                f.write(DOCKER_TEMPLATE.format(**values))
        else:
            # Default to extension type
            with open(fn, 'w+t') as f:
                f.write(EXTENSION_TEMPLATE.format(**values))

        # create manual file.
        fn = full_dir(self.manual_file)
        with open(fn, 'w+t') as f:
            f.write(MANUAL_TEMPLATE.format(**values))
        # enable script in krita settings (!)

        if self.ui.cb_enable_script.isChecked():
            Application.writeSetting(SCRIPT_SETTINGS, 'enable_'+self.package_name, 'true')

        # notify success
        # Assemble message
        title = "Krita Script files created"
        message = []
        message.append("<h3>Directory</h3>")
        message.append("Project files were created in the directory<p>%s"
                       % self.target_directory)
        message.append(
            "<h3>Files Created</h3>The following files were created:<p>")
        for f in self.files:
            message.append("%s<p>" % f)
        message.append("%s<p>" % self.manual_file)
        message.append("<h3>Location of script</h3>")
        message.append("Open this file to edit your script:<p>")
        script_path = os.path.join(self.target_directory, self.package_file)
        message.append("%s<p>" % script_path)
        message.append("Open this file to edit your Manual:<p>")
        script_path = os.path.join(self.target_directory, self.manual_file)
        message.append("%s<p>" % script_path)
        message.append("<h3>Record these locations</h3>")
        message.append(
            "Make a note of these locations before you click ok.<p>")
        message = "\n".join(message)

        # Display message box
        if CONTEXT_KRITA:
            msgbox = QMessageBox()
        else:
            msgbox = QMessageBox(self)
        msgbox.setWindowTitle(title)
        msgbox.setText(message)
        msgbox.setStandardButtons(QMessageBox.Ok)
        msgbox.setDefaultButton(QMessageBox.Ok)
        msgbox.setIcon(QMessageBox.Information)
        msgbox.exec()

        self.ui.close()

    def name_change(self, text):
        """
        name of script has changed,
        * calculate consequential names
        * update the e_files_display edit
        """

        text = text.strip()
        if len(text) == 0:
            return

        self.calculate_file_names(text)
        edit_text = self.calculate_edit_text()
        self.ui.e_files_display.setText(edit_text)

    def calculate_file_names(self, text):
        # name of class

        spam = text.split(" ")
        for i, s in enumerate(spam):
            s = s.strip()
            s = s.lower()
            try:
                spam[i] = s[0].upper()+s[1:]
            except IndexError:
                continue
        self.class_name = "".join(spam)

        # Normalise library name
        if TESTING:
            self.package_name = "bbdsss_"+text.lower().replace(" ", "_")
        else:
            self.package_name = text.lower().replace(" ", "_")

        # create desktop file
        self.desktop_fn = self.package_name+'.desktop'

        self.init_name = os.path.join(self.package_name, "__init__.py")
        self.package_file = os.path.join(
            self.package_name, self.package_name + ".py")
        self.manual_file = os.path.join(self.package_name, "Manual.html")
        self.files = [self.desktop_fn, self.init_name,
                      self.package_name, self.package_file,
                      self.manual_file]

    def calculate_edit_text(self):
        """
       Determine if any of the intended files will collide with existing files.

        """
        conflict_template = "%s - FILE ALREADY EXISTS<p>"
        non_conflict_template = "%s<p>"
        file_conflict = False

        html = ["<h3>Will create the following files:</h3>"]
        for path in self.files:
            target_file = os.path.join(self.target_directory, path)
            if os.path.exists(path):
                html.append(conflict_template % target_file)
                file_conflict = True
            else:
                html.append(non_conflict_template % target_file)

        if file_conflict:
            html.append("""<h2><span style="color:red;font-weight:bold">
            Warning:</span></h2><p>
            <span style="color:red;font-weight:bold">
            One or more of the files to be created already exists.
            If you click "Create Script" those files will be deleted
            and new files created in their place.</span><p>""")

        return "\n".join(html)


if __name__ == "__main__":
    # this includes when the script is run from the command line or
    # from the Scripter plugin.
    if CONTEXT_KRITA:
        # scripter plugin
        # give up - has the wrong context to create widgets etc.
        # maybe in the future change this.
        pass
    else:
        app = QApplication([])

        extension = KritaScriptStarter(None)
        extension.setup()
        extension.action_triggered()
        sys.exit(app.exec_())
