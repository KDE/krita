#!/usr/bin/env python

"""
  This Python script is used to test the Kross scripting framework.
"""

#def testobjectCallback():
#    print "function testobjectCallback() called !"
#    return "this is the __main__.testobjectCallback() returnvalue!"
#def testobjectCallbackWithParams(argument):
#    print "testobjectCallbackWithParams() argument = %s" % str(argument)
#    return "this is the __main__.testobjectCallbackWithParams() returnvalue!"
#def testQtObject(self):
    ## Get the QtObject instance to access the QObject.
    ##testobject = get("TestObject")
    #testobject = self.get("TestObject")
    #if testobject == None: raise "Object 'TestObject' undefined !!!"
    #print "testobject = %s %s" % (str(testobject),dir(testobject))
    ##print "propertyNames = %s" % testobject.propertyNames()
    ##print "slotNames = %s" % testobject.slotNames()
    ##print "signalNames = %s" % testobject.signalNames()
    ## We could just call a slot or a signal.
    #print testobject.call("testSlot2()");
    #print testobject.call("testSignal()");
    ##print testobject.call() #KrossTest: List::item index=0 is out of bounds. Raising TypeException.
    ## Each slot a QObject spends is a object itself.
    #myslot = testobject.get("testSlot()")
    #print "myslotevent = %s" % str(myslot)
    #print myslot.call()
    #print "__name__ = %s" % __name__
    #print "__dir__ = %s" % dir()
    ##print "__builtin__ = %s" % __builtin__
    #print "self = %s %s" % (str(self),dir(self))
    ##print "TestCase = %s" % str(TestCase)
    #print "self.list = %s" % self.list()
    #print "self.dict = %s" % self.dict()
    #testobject = self.get("TestObject")
    #print "testobject = %s" % testobject
    #if not testobject.connect("testSignal()",testobject,"testSlot2()"):
        #raise "Failed to connect testSignal() with testSlot2() at object 'TestObject'."
    #testobject.signal("testSignal()")
    ##testobject.testSlot()
    #testobject.slot("testSlot()")
    #testobject.disconnect("testSignal()")
#def testActionEvent(self):
    ##action1 = get("Action1")
    #action1 = self.get("Action1")
    #if action1 == None:
        #raise "Object 'Action1' undefined !!!"
    #print "action1 = %s %s" % (str(action1),dir(action1))
    ##action1.call()
    #action1.activate()

import unittest

class TestPlugin(unittest.TestCase):
	""" Testcase to test the Kross python functionality for regressions. """

	def setUp(self):
		import krosstestpluginmodule
		self.pluginobject1 = krosstestpluginmodule.testpluginobject1()
		self.assert_( self.pluginobject1 )

		self.pluginobject2 = krosstestpluginmodule.testpluginobject2()
		self.assert_( self.pluginobject2 )

		self.testqobject1 = krosstestpluginmodule.testqobject1()
		self.assert_( self.testqobject1 )

	def testBasicDataTypes(self):
		self.assert_( self.pluginobject1.uintfunc(177321) == 177321 )
		self.assert_( self.pluginobject1.intfunc(93675) == 93675 )
		self.assert_( self.pluginobject1.intfunc(-73673) == -73673 )
		self.assert_( self.pluginobject1.boolfunc(True) == True )
		self.assert_( self.pluginobject1.boolfunc(False) == False )
		self.assert_( self.pluginobject1.doublefunc(4265.3723) == 4265.3723 )
		self.assert_( self.pluginobject1.doublefunc(-4265.68) == -4265.68 )
		#self.assert_( self.pluginobject1.cstringfunc(" This is a Test! ") == " This is a Test! " )
		self.assert_( self.pluginobject1.stringfunc(" Another \n\r Test! $%&\"") == " Another \n\r Test! $%&\"" )
		#self.assert_( self.pluginobject1.stringfunc(unicode(" Another Test! ")) == unicode(" Another Test! ") )
		self.assert_( self.pluginobject1.stringstringfunc("MyString1", "MyString2") == "MyString1" )
		self.assert_( self.pluginobject1.uintdoublestringfunc(8529,285.246,"String") == 8529 )
		self.assert_( self.pluginobject1.stringlistbooluintdouble(["s1","s2"],True,6,7.0,"String") == ["s1","s2"] )

	def testStringList(self):
		self.assert_( self.pluginobject1.stringlistfunc( [] ) == [] )
		self.assert_( self.pluginobject1.stringlistfunc( ["First Item"," Second Item "] ) == ["First Item"," Second Item "] )
		self.assert_( self.pluginobject1.stringlistfunc( ("Theird Item"," Forth Item ","Fifth Item") ) == ["Theird Item"," Forth Item ","Fifth Item"] )

	def testVariant(self):
		self.assert_( self.pluginobject1.variantfunc(True) == True )
		self.assert_( self.pluginobject1.variantfunc(False) == False )
		self.assert_( self.pluginobject1.variantfunc(187937) == 187937 )
		self.assert_( self.pluginobject1.variantfunc(-69825) == -69825 )
		self.assert_( self.pluginobject1.variantfunc(8632.274) == 8632.274 )
		self.assert_( self.pluginobject1.variantfunc(-8632.351) == -8632.351 )
		self.assert_( self.pluginobject1.variantfunc(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")

	def testObjects(self):
		print "-----------------1"
		newobjref = self.pluginobject1.objectfunc(self.pluginobject2)
		print "-----------------2"
		print str(newobjref)

		#self.assert_( newobjref.myuniqueid == self.pluginobject2.myuniqueid )
		#print "===========> %s" % self.pluginobject2.myName()

		print "testqobject1 properties=%s" % self.testqobject1.propertyNames()
		print "testqobject1 slots=%s" % self.testqobject1.slotNames()
		print "testqobject1 signals=%s" % self.testqobject1.signalNames()
		print "-----------------3"
		print "DIR=>%s" % dir(self.testqobject1)

		print "===================> slotcall-result: %s" % self.testqobject1.slot("self()")

		#testobject = newobjref.get("TestObject")
		#print testobject
		print "-----------------9"

	def testDefaultArguments(self):
		self.assert_( self.pluginobject1.uintfunc_defarg(98765) == 98765 )
		self.assert_( self.pluginobject1.uintfunc_defarg() == 12345 )
		self.assert_( self.pluginobject1.stringfunc_defarg("MyString") == "MyString" )
		self.assert_( self.pluginobject1.stringfunc_defarg() == "MyDefaultString" )
		self.assert_( self.pluginobject1.stringlistfunc_defarg(["s1","s2","s3"]) == ["s1","s2","s3"] )
		self.assert_( self.pluginobject1.stringlistfunc_defarg() == ["Default1","Default2"] )
		self.assert_( self.pluginobject1.variantfunc_defarg(822.75173) == 822.75173 )
		self.assert_( self.pluginobject1.variantfunc_defarg() == "MyDefaultVariantString" )

	#def testExpectedFailures(self):
		# to less arguments
		#self.assertRaises(ValueError, self.pluginobject1.uintfunc)
		#self.assert_( self.pluginobject1.uintfunc() != 8465 )

print "__name__ = %s" % __name__
#print "self = %s" % self
#print self.get("TestObject")

suite = unittest.makeSuite(TestPlugin)
unittest.TextTestRunner(verbosity=2).run(suite)
