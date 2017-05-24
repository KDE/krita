import unittest
import os
import sys
from krita import Action
from PyQt5.QtWidgets import QAction


class TestAction(unittest.TestCase):

    def setUp(self):
        self.instance = Action()

    def testCreateNewAction(self):
        self.assertEqual(bool(self.instance), True)

    def testCreateNewActionWithQAction(self):
        new_action = Action("test", QAction("test"))
        self.assertEqual(bool(new_action), True)
