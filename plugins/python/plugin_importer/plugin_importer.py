# SPDX-FileCopyrightText: 2019 Rebecca Breu <rebecca@rbreu.de>

# This file is part of Krita.

# SPDX-License-Identifier: GPL-3.0-or-later

"""This module provides the actual importing logic. See
`:class:PluginImporter` for more info.

For easy command line testing, call this module like this:
  > python plugin_importer.py foo.zip /output/path
"""

from configparser import ConfigParser, Error as ConfigParserError
import os
import shutil
import sys
from tempfile import TemporaryDirectory
import zipfile


class PluginImportError(Exception):
    """Base class for all exceptions of this module."""
    pass


class NoPluginsFoundException(PluginImportError):
    """No valid plugins can be found in the zip file."""
    pass


class PluginReadError(PluginImportError):
    """Zip file can't be read or its content can't be parsed."""
    pass


class PluginImporter:
    """Import a Krita Python Plugin from a zip file into the given
    directory.

    The Importer makes barely any assumptions about the file structure
    in the zip file. It will find one or more plugins with the
    following strategy:

    1. Find files with the ending `.desktop` and read the Python
       module name from them
    2. Find directories that correspond to the Python module names
       and that contain an `__init__.py` file
    3. Find files with ending `.action` that have matching
       `<Action name=...>` tags (these files are optional)
    4. Extract the desktop- and action-files and the Python module
       directories into the corresponding pykrita and actions folders

    Usage:

    >>> importer = PluginImporter(
            '/path/to/plugin.zip',
            '/path/to/krita/resources/',
            confirm_overwrite_callback)
    >>> imported = importer.import_all()

    """

    def __init__(self, zip_filename, resources_dir,
                 confirm_overwrite_callback):

        """Initialise the importer.

        :param zip_filename: Filename of the zip archive containing the
          plugin(s)
        :param resources_dir: The Krita resources directory into which
          to extract the plugin(s)
        :param confirm_overwrite_callback: A function that gets called
          if a plugin already exists in the resources directory. It gets
          called with a dictionary of information about the plugin and
          should return whether the user wants to overwrite the plugin
          (True) or not (False).
        """

        self.resources_dir = resources_dir
        self.confirm_overwrite_callback = confirm_overwrite_callback
        try:
            self.archive = zipfile.ZipFile(zip_filename)
        except(zipfile.BadZipFile, zipfile.LargeZipFile, OSError) as e:
            raise PluginReadError(str(e))

        self.desktop_filenames = []
        self.action_filenames = []
        for filename in self.archive.namelist():
            if filename.endswith('.desktop'):
                self.desktop_filenames.append(filename)
            if filename.endswith('.action'):
                self.action_filenames.append(filename)

    @property
    def destination_pykrita(self):
        dest = os.path.join(self.resources_dir, 'pykrita')
        if not os.path.exists(dest):
            os.mkdir(dest)
        return dest

    @property
    def destination_actions(self):
        dest = os.path.join(self.resources_dir, 'actions')
        if not os.path.exists(dest):
            os.mkdir(dest)
        return dest

    def get_destination_module(self, plugin):
        return os.path.join(self.destination_pykrita, plugin['name'])

    def get_destination_desktop(self, plugin):
        return os.path.join(
            self.destination_pykrita, '%s.desktop' % plugin['name'])

    def get_destination_actionfile(self, plugin):
        return os.path.join(
            self.destination_actions, '%s.action' % plugin['name'])

    def get_source_module(self, name):
        namelist = self.archive.namelist()
        for filename in namelist:
            if (filename.endswith('/%s/' % name)
                    or filename == '%s/' % name):
                # Sanity check: There should be an __init__.py inside
                if ('%s__init__.py' % filename) in namelist:
                    return filename

    def get_source_actionfile(self, name):
        for filename in self.action_filenames:
            _, actionfilename = os.path.split(filename)
            if actionfilename == '%s.action' % name:
                return filename

    def read_desktop_config(self, desktop_filename):
        config = ConfigParser()
        try:
            config.read_string(
                self.archive.read(desktop_filename).decode('utf-8'))
        except ConfigParserError as e:
            raise PluginReadError(
                '%s: %s' % (i18n('Desktop file'), str(e)))
        return config

    def get_plugin_info(self):
        names = []
        for filename in self.desktop_filenames:
            config = self.read_desktop_config(filename)
            try:
                name = config['Desktop Entry']['X-KDE-Library']
                ui_name = config['Desktop Entry']['Name']
            except KeyError as e:
                raise PluginReadError(
                    'Desktop file: Key %s not found' % str(e))
            module = self.get_source_module(name)
            if module:
                names.append({
                    'name': name,
                    'ui_name': ui_name,
                    'desktop': filename,
                    'module': module,
                    'action': self.get_source_actionfile(name)
                })
        return names

    def extract_desktop(self, plugin):
        with open(self.get_destination_desktop(plugin), 'wb') as f:
            f.write(self.archive.read(plugin['desktop']))

    def extract_module(self, plugin):
        with TemporaryDirectory() as tmp_dir:
            for name in self.archive.namelist():
                if name.startswith(plugin['module']):
                    self.archive.extract(name, tmp_dir)
            module_dirname = os.path.join(
                tmp_dir, *plugin['module'].split('/'))
            try:
                shutil.rmtree(self.get_destination_module(plugin))
            except FileNotFoundError:
                pass
            shutil.copytree(module_dirname,
                            self.get_destination_module(plugin))

    def extract_actionfile(self, plugin):
        with open(self.get_destination_actionfile(plugin), 'wb') as f:
            f.write(self.archive.read(plugin['action']))

    def extract_plugin(self, plugin):
        # Check if the plugin already exists in the source directory:
        if (os.path.exists(self.get_destination_desktop(plugin))
                or os.path.exists(self.get_destination_module(plugin))):
            confirmed = self.confirm_overwrite_callback(plugin)
            if not confirmed:
                return False

        self.extract_desktop(plugin)
        self.extract_module(plugin)
        if plugin['action']:
            self.extract_actionfile(plugin)
        return True

    def import_all(self):
        """Imports all plugins from the zip archive.

        Returns a list of imported plugins.
        """

        plugins = self.get_plugin_info()
        if not plugins:
            raise NoPluginsFoundException(i18n('No plugins found in archive'))

        imported = []
        for plugin in plugins:
            success = self.extract_plugin(plugin)
            if success:
                imported.append(plugin)

        return imported


if __name__ == '__main__':
    def callback(plugin):
        print('Overwriting plugin:', plugin['ui_name'])
        return True

    imported = PluginImporter(
        sys.argv[1], sys.argv[2], callback).import_all()
    for plugin in imported:
        print('Imported plugin:', plugin['ui_name'])
