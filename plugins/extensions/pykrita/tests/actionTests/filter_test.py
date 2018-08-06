import unittest
from krita import Filter, InfoObject
from PyQt5.QtCore import QObject


class TestFilter(unittest.TestCase):

    def setUp(self):
        self._instance = Filter()

    def testConstructor(self):
        self.assertTrue(Filter())

    def testConstructorInvalidParameter(self):
        with self.assertRaises(TypeError):
            Filter(str(''))

    def testEqualOperator(self):
        sameFilter = self._instance
        self.assertTrue(sameFilter == self._instance)

    def testEmptyNameProperty(self):
        self.assertFalse(self._instance.name())

    # segmentation fault here, I need to verify that.
    def testConfigurationProperties(self):
        pass
        # infoObject = InfoObject()
        # f = Filter()
        # f.setConfiguration(infoObject)
        # self.assertEqual(f.configuration(), infoObject)
