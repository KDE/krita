import unittest
from krita import Action
from PyQt5.QtWidgets import QAction


class TestAction(unittest.TestCase):

    def setUp(self):
        self.instance = Action()
        self.instance.triggered.connect(self.slotTriggered)
        self.triggered = False

    def testConstructor(self):
        self.assertEqual(bool(self.instance), True)

    def testConstructorWithStringQAction(self):
        new_action = Action("test", QAction("test"))
        self.assertEqual(bool(new_action), True)

    def testConstructorInvalidParameter(self):
        with self.assertRaises(TypeError):
            Action(str(''))

    def testEqualOperator(self):
        new_action = self.instance
        self.assertEqual(new_action == self.instance, True)

    def testInequalityOperator(self):
        new_action = Action()
        self.assertEqual(new_action != self.instance, True)

    def testTextProperties(self):
        self.instance.setText("test")
        self.assertEqual(self.instance.text() == "test", True)

    def testNameProperties(self):
        self.instance.setName("test")
        self.assertEqual(self.instance.name() == "test", True)

    def testCheckableInitialState(self):
        self.assertEqual(self.instance.isCheckable(), False)

    def testCheckableToTrue(self):
        self.instance.setCheckable(True)
        self.assertEqual(self.instance.isCheckable(), True)

    def testCheckableToFalse(self):
        self.instance.setCheckable(False)
        self.assertEqual(self.instance.isCheckable(), False)

    def testCheckedInitialState(self):
        self.assertEqual(self.instance.isChecked(), False)

    def testCheckedToTrue(self):
        self.instance.setCheckable(True)
        self.instance.setChecked(True)
        self.assertEqual(self.instance.isChecked(), True)

    def testCheckedToFalse(self):
        self.instance.setChecked(False)
        self.assertEqual(self.instance.isChecked(), False)

    def testCheckedToFalseNotCheckable(self):
        self.instance.setChecked(True)
        self.assertEqual(self.instance.isChecked(), False)

    def testVisibleInitialState(self):
        self.assertEqual(self.instance.isVisible(), True)

    def testVisibleToTrue(self):
        self.instance.setVisible(True)
        self.assertEqual(self.instance.isVisible(), True)

    def testVisibleToFalse(self):
        self.instance.setVisible(False)
        self.assertEqual(self.instance.isVisible(), False)

    def testTrigger(self):
        self.instance.trigger()
        self.assertEqual(self.triggered, True)

    # helper method
    def slotTriggered(self):
        self.triggered = True
