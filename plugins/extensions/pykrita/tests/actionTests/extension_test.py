import unittest
from krita import Extension
from PyQt5.QtCore import QObject


class TestExtension(unittest.TestCase):

    def testConstructorSubClass(self):
        self.assertEqual(bool(SubClassExtension()), True)

    def testConstructorSubClassWithParent(self):
        self.assertEqual(bool(SubClassExtension(QObject())), True)

    def testConstructorInvalidParameter(self):
        with self.assertRaises(TypeError):
            SubClassExtension(str(''))

    def testConstructorAbstractClass(self):
        with self.assertRaises(TypeError):
            Extension()

    def testExtensionHasMethodsetup(self):
        setup = getattr(SubClassExtension(), "setup", None)
        self.assertEqual(bool(callable(setup)), True)


class SubClassExtension(Extension):

    def setup(self):
        pass
