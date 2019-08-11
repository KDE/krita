# Copyright (c) 2019 Rebecca Breu <rebecca@rbreu.de>

# This file is part of Krita.

# Krita is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Krita is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Krita.  If not, see <https://www.gnu.org/licenses/>.

import os

import krita

from PyQt5.QtCore import QStandardPaths
from PyQt5.QtWidgets import QFileDialog, QMessageBox

from .plugin_importer import PluginImporter, PluginImportError


class PluginImporterExtension(krita.Extension):

    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction(
            'plugin_importer',
            i18n('Import Python Plugin...'),
            'tools/scripts')
        action.triggered.connect(self.import_plugin)

    def confirm_overwrite(self, plugin):
        reply = QMessageBox.question(
            self.parent.activeWindow().qwindow(),
            i18n('Overwrite Plugin'),
            i18n('The plugin "%s" already exists. Overwrite it?') % (
                plugin['ui_name']),
            QMessageBox.Yes | QMessageBox.No)
        return reply == QMessageBox.Yes

    def get_success_text(self, plugins):
        txt = [
            '<p>',
            i18n('The following plugins were imported:'),
            '</p>',
            '<ul>'
        ]
        for plugin in plugins:
            txt.append('<li>%s</li>' % plugin['ui_name'])

        txt.append('</ul>')
        txt.append('<p>')
        txt.append(i18n(
            'Please restart Krita and activate the plugins in '
            '<em>Settings -> Configure Krita -> '
            'Python Plugin Manager</em>.'))
        txt.append('</p>')
        return ('\n').join(txt)

    def get_resources_dir(self):
        return QStandardPaths.writableLocation(
            QStandardPaths.AppDataLocation)

    def import_plugin(self):
        zipfile = QFileDialog.getOpenFileName(
            self.parent.activeWindow().qwindow(),
            i18n('Import Plugin'),
            os.path.expanduser('~'),
            '%s (*.zip)' % i18n('Zip Archives'),
        )[0]

        if not zipfile:
            return

        try:
            imported = PluginImporter(
                zipfile,
                self.get_resources_dir(),
                self.confirm_overwrite
            ).import_all()
        except PluginImportError as e:
            msg = '<p>%s</p><pre>%s</pre>' % (
                i18n('Error during import:'), str(e))
            QMessageBox.warning(
                self.parent.activeWindow().qwindow(),
                i18n('Error'),
                msg)
            return

        if imported:
            QMessageBox.information(
                self.parent.activeWindow().qwindow(),
                i18n('Import successful'),
                self.get_success_text(imported))
