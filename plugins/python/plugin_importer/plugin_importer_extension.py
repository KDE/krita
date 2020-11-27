# Copyright (c) 2019 Rebecca Breu <rebecca@rbreu.de>

# This file is part of Krita.

# SPDX-License-Identifier: GPL-3.0-or-later

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

    def confirm_activate(self, plugins):
        txt = [
            '<p>',
            i18n('The following plugins were imported:'),
            '</p>',
            '<ul>'
        ]
        for plugin in plugins:
            txt.append('<li>%s</li>' % plugin['ui_name'])

        txt.append('</ul>')
        txt.append('<p><strong>')
        txt.append(i18n(
            'Enable plugins now? (Requires restart)'))
        txt.append('</strong></p>')

        reply = QMessageBox.question(
            self.parent.activeWindow().qwindow(),
            i18n('Activate Plugins?'),
            ('\n').join(txt),
            QMessageBox.Yes | QMessageBox.No)
        return reply == QMessageBox.Yes

    def activate_plugins(self, plugins):
        for plugin in plugins:
            Application.writeSetting(
                'python',
                'enable_%s' % plugin['name'],
                'true')

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
            activate = self.confirm_activate(imported)
            if activate:
                self.activate_plugins(imported)
