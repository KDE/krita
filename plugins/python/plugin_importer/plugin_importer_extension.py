# SPDX-FileCopyrightText: 2019 Rebecca Breu <rebecca@rbreu.de>

# This file is part of Krita.

# SPDX-License-Identifier: GPL-3.0-or-later

import html
import os
import tempfile

import krita

from PyQt5.QtCore import QStandardPaths
from PyQt5.QtWidgets import QFileDialog, QMessageBox, QInputDialog

from .plugin_importer import PluginImporter, PluginImportError
from .plugin_downloader import download_plugin, PluginDownloadError


class PluginImporterExtension(krita.Extension):

    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction(
            'plugin_importer_file',
            i18n('Import Python Plugin from File...'),
            'tools/scripts')
        action.triggered.connect(self.import_plugin_from_file)
        action = window.createAction(
            'plugin_importer_web',
            i18n('Import Python Plugin from Web...'),
            'tools/scripts')
        action.triggered.connect(self.import_plugin_from_web)

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

    def display_errors(self, error):
        msg = '<p>%s</p><pre>%s</pre>' % (
            i18n('Error during import:'),
            html.escape(str(error)))
        QMessageBox.warning(
            self.parent.activeWindow().qwindow(),
            i18n('Error'),
            msg)

    def activate_plugins(self, plugins):
        for plugin in plugins:
            Application.writeSetting(
                'python',
                'enable_%s' % plugin['name'],
                'true')

    def get_resources_dir(self):
        return Krita.instance().getAppDataLocation()

    def import_plugin_from_web(self):
        infotext = i18n(
            '<p><strong>Enter download URL</strong></p>'
            '<p>For example:'
            '<ul>'
            '<li>Zip download link (http://example.com/plugin.zip)</li>'
            '<li>Github repository (https://github.com/test/plugin)</li>'
        )
        url = QInputDialog.getText(
            self.parent.activeWindow().qwindow(),
            i18n('Import Plugin'),
            infotext)[0]
        if url:
            with tempfile.TemporaryDirectory() as tmpdir:
                try:
                    zipfile = download_plugin(url=url, dest_dir=tmpdir)
                except PluginDownloadError as e:
                    self.display_errors(e)
                    return
                self.do_import(zipfile)

    def import_plugin_from_file(self):
        zipfile = QFileDialog.getOpenFileName(
            self.parent.activeWindow().qwindow(),
            i18n('Import Plugin'),
            os.path.expanduser('~'),
            '%s (*.zip)' % i18n('Zip Archives'),
        )[0]

        if not zipfile:
            return

        self.do_import(zipfile)

    def do_import(self, zipfile):
        try:
            imported = PluginImporter(
                zipfile,
                self.get_resources_dir(),
                self.confirm_overwrite
            ).import_all()
        except PluginImportError as e:
            self.display_errors(e)
            return

        if imported:
            activate = self.confirm_activate(imported)
            if activate:
                self.activate_plugins(imported)
