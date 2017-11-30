import unittest
from krita import InfoObject
from PyQt5.QtCore import QObject


class TestInfoObject(unittest.TestCase):

    def setUp(self):
        self._instance = InfoObject()

    def testConstructor(self):
        self.assertTrue(InfoObject())

    def testConstructorWithParent(self):
        self.assertTrue(InfoObject(QObject()))

    def testConstructorInvalidParameter(self):
        with self.assertRaises(TypeError):
            InfoObject(str(''))

    def testEqualOperator(self):
        sameInfoObject = self._instance
        self.assertTrue(sameInfoObject == self._instance)

    def testInequalityOperator(self):
        newInfoObject = InfoObject()
        self.assertTrue(newInfoObject != self._instance)

    def testPropertiesAcessorsOneProperty(self):
        self._instance.setProperties({"test": "test"})
        self.assertEqual(self._instance.properties(), {"test": "test"})

    def testPropertiesAcessorsSetProperties(self):
        self._instance.setProperties({"test": "test", "test1": 1})
        self.assertEqual(self._instance.properties(), {"test": "test", "test1": 1})

    def testPropertySlotsString(self):
        self._instance.setProperty("key", "value")
        self.assertEqual(self._instance.property("key"), "value")

    def testPropertySlotsInvalidKey(self):
        self._instance.setProperty("key", "value")
        self.assertEqual(self._instance.property("keys"), None)
