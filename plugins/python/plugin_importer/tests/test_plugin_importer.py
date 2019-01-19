import os
import pytest
from tempfile import TemporaryDirectory
from unittest import TestCase


from .. plugin_importer import PluginImporter, PluginReadError


class PluginImporterTestCase(TestCase):

    def setUp(self):
        self.resources_dir = TemporaryDirectory()
        self.plugin_dir = TemporaryDirectory()

    def tearDown(self):
        self.resources_dir.cleanup()
        self.plugin_dir.cleanup()

    @property
    def zip_filename(self):
        return os.path.join(
            self.resources_dir.name, 'plugin.zip')

    def test_zipfile_doesnt_exist(self):
        with pytest.raises(PluginReadError):
            PluginImporter(self.zip_filename,
                           self.resources_dir.name,
                           lambda x: True)

    def test_zipfile_not_a_zip(self):
        with open(self.zip_filename, 'w') as f:
            f.write('foo')
        with pytest.raises(PluginReadError):
            PluginImporter(self.zip_filename,
                           self.resources_dir.name,
                           lambda x: True)
