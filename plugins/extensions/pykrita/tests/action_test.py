import unittest
import os
import sys
import test_utils


# I will write a class decorator to avoid code repetition
# at this point of the module
test_utils.setPykritaInstPath()


class TestAction(unittest.TestCase):

    def setUp(self):
        from krita import Action
        self.instance = Action()

    def testCreateNewAction(self):
        self.assertEqual(bool(self.instance), True)
