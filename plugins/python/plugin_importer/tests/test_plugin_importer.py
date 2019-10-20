"""Unit tests for the plugin_importer module.

Unit tests can either be toplevel functions whose names start with
`test_`, or they can be class methods on classes derived from
`unittest.TestCase`; again the method names need to start with `test_`.
"""


import os
import pytest
from tempfile import TemporaryDirectory
from unittest import mock, TestCase
from zipfile import ZipFile

from .. plugin_importer import (
    NoPluginsFoundException,
    PluginImporter,
    PluginReadError,
)


class PluginImporterTestCase(TestCase):
    """Collection of unit tests for the plugin_importer module.

    Since the tests need a bit of setup for file system operations, we gather
    them together in a TestCase class.
    """

    def setUp(self):
        """This method will be run before each test in this class.

        We create temporary directories for creating and extracting
        zip files: `resources_dir` will be the destination (a stand-in
        for Krita's resources dir), `plugin_dir` the source where our
        plugin zip is located.
        """

        self.resources_dir = TemporaryDirectory()
        self.plugin_dir = TemporaryDirectory()

    def tearDown(self):
        """This method will be run after each test in this class.

        We remove the temporary directories created in `setUp`.
        """

        self.resources_dir.cleanup()
        self.plugin_dir.cleanup()

    @property
    def zip_filename(self):
        """Helper method for easy access to the full path of the zipped
        plugin.
        """
        return os.path.join(self.plugin_dir.name, 'plugin.zip')

    def zip_plugin(self, dirname):
        """Helper method to zip a plugin from the subdirectory `dirname` in
        the `fixtures` folder and store it in the temporary directory
        `self.plugin_dir`.

        The `fixtures` directory contains the non-zipped plugins for easier
        maintenance.
        """

        # Get the full name of the folder this file resides in:
        testroot = os.path.dirname(os.path.realpath(__file__))

        # From that, we can get the full name of the plugin fixture folder:
        src = os.path.join(testroot, 'fixtures', dirname)

        # Zip it:
        with ZipFile(self.zip_filename, 'w') as plugin_zip:
            for root, dirs, files in os.walk(src):
                dirname = root.replace(src, '')
                for filename in files + dirs:
                    plugin_zip.write(
                        filename=os.path.join(root, filename),
                        arcname=os.path.join(dirname, filename))

    def assert_in_resources_dir(self, *path):
        """Helper method to check whether a directory or file exists inside
        `self.resources_dir`.
        """

        assert os.path.exists(os.path.join(self.resources_dir.name, *path))

    def assert_not_in_resources_dir(self, *path):
        """Helper method to check whether a directory or file doesn't exist
        inside `self.resources_dir`.
        """

        assert not os.path.exists(os.path.join(self.resources_dir.name, *path))

    ############################################################
    # The actual tests start below

    def test_zipfile_doesnt_exist(self):
        """Test: Import a filename that doesn't exist."""

        # Create nothing here...

        with pytest.raises(PluginReadError):
            # We expect an exception
            PluginImporter(self.zip_filename,
                           self.resources_dir.name,
                           lambda x: True)

    def test_zipfile_not_a_zip(self):
        """Test: Import a file that isn't a zip file."""

        # Create a text file:
        with open(self.zip_filename, 'w') as f:
            f.write('foo')

        with pytest.raises(PluginReadError):
            # We expect an exception
            PluginImporter(self.zip_filename,
                           self.resources_dir.name,
                           lambda x: True)

    def test_simple_plugin_success(self):
        """Test: Import a basic plugin."""

        self.zip_plugin('success_simple')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        imported = importer.import_all()
        assert len(imported) == 1
        self.assert_in_resources_dir('pykrita', 'foo')
        self.assert_in_resources_dir('pykrita', 'foo', '__init__.py')
        self.assert_in_resources_dir('pykrita', 'foo', 'foo.py')
        self.assert_in_resources_dir('pykrita', 'foo.desktop')
        self.assert_in_resources_dir('actions', 'foo.action')

    def test_toplevel_plugin_success(self):
        """Test: Import a plugin with everything at toplevel."""

        self.zip_plugin('success_toplevel')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        imported = importer.import_all()
        assert len(imported) == 1
        self.assert_in_resources_dir('pykrita', 'foo')
        self.assert_in_resources_dir('pykrita', 'foo', '__init__.py')
        self.assert_in_resources_dir('pykrita', 'foo', 'foo.py')
        self.assert_in_resources_dir('pykrita', 'foo.desktop')
        self.assert_in_resources_dir('actions', 'foo.action')

    def test_nested_plugin_success(self):
        """Test: Import a plugin with nested directories."""

        self.zip_plugin('success_nested')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        imported = importer.import_all()
        assert len(imported) == 1
        self.assert_in_resources_dir('pykrita', 'foo')
        self.assert_in_resources_dir('pykrita', 'foo', '__init__.py')
        self.assert_in_resources_dir('pykrita', 'foo', 'foo.py')
        self.assert_in_resources_dir('pykrita', 'foo.desktop')
        self.assert_in_resources_dir('actions', 'foo.action')

    def test_no_action_success(self):
        """Test: Import a plugin without action file.

        Should import fine without creating an action file."""

        self.zip_plugin('success_no_action')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        imported = importer.import_all()
        assert len(imported) == 1
        self.assert_in_resources_dir('pykrita', 'foo')
        self.assert_in_resources_dir('pykrita', 'foo', '__init__.py')
        self.assert_in_resources_dir('pykrita', 'foo', 'foo.py')
        self.assert_in_resources_dir('pykrita', 'foo.desktop')
        self.assert_not_in_resources_dir('actions', 'foo.action')

    def test_overwrite_existing(self):
        """Test: Overwrite an existing plugin when overwrite confirmed."""

        self.zip_plugin('success_simple')

        # Create an existing python module in the resources directory:
        plugin_dir = os.path.join(self.resources_dir.name, 'pykrita', 'foo')
        os.makedirs(plugin_dir)
        init_file = os.path.join(plugin_dir, '__init__.py')
        with open(init_file, 'w') as f:
            f.write('# existing module')

        # Create a mock callback on which we can test that it has been called:
        confirm_callback = mock.MagicMock(return_value=True)
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  confirm_callback)
        imported = importer.import_all()
        assert len(imported) == 1
        assert confirm_callback.called

        # Existing plugin should be overwritten:
        with open(init_file, 'r') as f:
            assert f.read().strip() == "print('hello')"

    def test_dont_overwrite_existing(self):
        """Test: Don't overwrite an existing plugin when overwrite not
        confirmed."""

        self.zip_plugin('success_simple')

        # Create an existing python module in the resources directory:
        plugin_dir = os.path.join(self.resources_dir.name, 'pykrita', 'foo')
        os.makedirs(plugin_dir)
        init_file = os.path.join(plugin_dir, '__init__.py')
        with open(init_file, 'w') as f:
            f.write('# existing module')

        # Create a mock callback on which we can test that it has been called:
        confirm_callback = mock.MagicMock(return_value=False)
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  confirm_callback)
        imported = importer.import_all()
        assert len(imported) == 0
        assert confirm_callback.called

        # Existing plugin should not be overwritten:
        with open(init_file, 'r') as f:
            assert f.read().strip() == '# existing module'

    def test_missing_desktop_file(self):
        """Test: Import plugin without a desktop file."""

        self.zip_plugin('fail_no_desktop_file')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(NoPluginsFoundException):
            # We expect an exception
            importer.import_all()

    def test_unparsable_desktop_file(self):
        """Test: Import plugin whose desktop file is not parsable."""

        self.zip_plugin('fail_unparsable_desktop_file')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(PluginReadError):
            # We expect an exception
            importer.import_all()

    def test_missing_keys_in_desktop_file(self):
        """Test: Import plugin whose destkop file is missing needed keys."""

        self.zip_plugin('fail_missing_keys_desktop_file')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(PluginReadError):
            # We expect an exception
            importer.import_all()

    def test_no_matching_plugindir(self):
        """Test: Import plugin whose destkop file is missing needed keys."""

        self.zip_plugin('fail_no_matching_plugindir')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(NoPluginsFoundException):
            # We expect an exception
            importer.import_all()

    def test_no_init_file(self):
        """Test: Import plugin whose python module is missing the __init__.py
        file."""

        self.zip_plugin('fail_no_init_file')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(NoPluginsFoundException):
            # We expect an exception
            importer.import_all()

    def test_unparsable_action_file(self):
        """Test: Import plugin whose action file isn't parsable."""

        self.zip_plugin('fail_unparsable_action_file')
        importer = PluginImporter(self.zip_filename,
                                  self.resources_dir.name,
                                  lambda x: True)
        with pytest.raises(PluginReadError):
            # We expect an exception
            importer.import_all()
