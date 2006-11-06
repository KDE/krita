#!/usr/bin/env kross

"""
  This Python script is used to test the Kross scripting framework.
"""

import unittest

class TestKross(unittest.TestCase):
	""" Testcases to test the Kross python functionality for regressions. """

	def setUp(self):
		import TestObject1, TestObject2
		self.object1 = TestObject1
		self.object2 = TestObject2

	def testBool(self):
		self.assert_( self.object1.func_bool_bool(True) == True )
		self.assert_( self.object1.func_bool_bool(False) == False )

	def testInt(self):
		self.assert_( self.object1.func_int_int(0) == 0 )
		self.assert_( self.object1.func_int_int(177321) == 177321 )
		self.assert_( self.object1.func_int_int(-98765) == -98765 )

	def testUInt(self):
		self.assert_( self.object1.func_uint_uint(0) == 0 )
		self.assert_( self.object1.func_uint_uint(177321) == 177321 )
		#self.assertRaises(OverflowError, self.object1.func_uint_uint, -1)

	def testDouble(self):
		self.assert_( self.object1.func_double_double(0.0) == 0.0 )
		self.assert_( self.object1.func_double_double(1773.2177) == 1773.2177 )
		self.assert_( self.object1.func_double_double(-548993.271993) == -548993.271993 )

	def testLongLong(self):
		self.assert_( self.object1.func_qlonglong_qlonglong(0) == 0 )
		self.assert_( self.object1.func_qlonglong_qlonglong(7379) == 7379 )
		self.assert_( self.object1.func_qlonglong_qlonglong(-6384673) == -6384673 )
		#self.assert_( self.object1.func_qlonglong_qlonglong(678324787843223472165) == 678324787843223472165 )

	def testULongLong(self):
		self.assert_( self.object1.func_qulonglong_qulonglong(0) == 0 )
		self.assert_( self.object1.func_qulonglong_qulonglong(378972) == 378972 )
		#self.assert_( self.object1.func_qulonglong_qulonglong(-8540276823902375665225676321823) == -8540276823902375665225676321823 )

	def testByteArray(self):
		self.assert_( self.object1.func_qbytearray_qbytearray("  Some String as ByteArray  ") == "  Some String as ByteArray  " )
		self.assert_( self.object1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test ") == " \0\n\r\t\s\0 test " )

	def testString(self):
		self.assert_( self.object1.func_qstring_qstring("") == "" )
		self.assert_( self.object1.func_qstring_qstring(" ") == " " )
		self.assert_( self.object1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )

	def testStringList(self):
		self.assert_( self.object1.func_qstringlist_qstringlist( [] ) == [] )
		self.assert_( self.object1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		self.assert_( self.object1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )

	def testVariantList(self):
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [] ) == [] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[True,[],False,"Other String"],"test"] ) == [[True,[],False,"Other String"],"test"] )

	def testVariantMap(self):

		def doTestVariantMap(vmap):
			rmap = self.object1.func_qvariantmap_qvariantmap( vmap )
			self.assert_( len(rmap) == len(vmap) )
			for k in vmap:
				self.assert_( rmap[k] == vmap[k] )

		doTestVariantMap( {} )
		doTestVariantMap( {"1":73682,"2":285} )
		doTestVariantMap( {"a":-6892.957,"b":692.66} )
		doTestVariantMap( {"key1":True,"key2":False} )
		doTestVariantMap( {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} )
		doTestVariantMap( {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} )

	def testVariant(self):
		#self.assert_( self.object1.func_qvariant_qvariant(0.0) == 0.0 )
		#self.assert_( self.object1.func_qvariant_qvariant(True) == True )
		#self.assert_( self.object1.func_qvariant_qvariant(False) == False )
		#self.assert_( self.object1.func_qvariant_qvariant(187937) == 187937 )
		#self.assert_( self.object1.func_qvariant_qvariant(-69825) == -69825 )
		#self.assert_( self.object1.func_qvariant_qvariant(8632.274) == 8632.274 )
		#self.assert_( self.object1.func_qvariant_qvariant(-8632.351) == -8632.351 )
		#self.assert_( self.object1.func_qvariant_qvariant(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")
		pass

	def testObject(self):
		self.assert_( self.object1.name() == "TestObject1" and self.object2.name() == "TestObject2" )
		self.assert_( self.object1.objectName == "TestObject1" and self.object2.objectName == "TestObject2" )
		self.assert_( self.object1.className() == "TestObject" and self.object2.className() == "TestObject" )

	def testPropertyMembers(self):
		self.object1.boolProperty = True
		self.assert_( self.object1.boolProperty == True )
		self.object1.boolProperty = False
		self.assert_( self.object1.boolProperty == False )

		self.object1.intProperty = 20
		self.assert_( self.object1.intProperty == 20 )

		self.object1.doubleProperty = 7436.671
		self.assert_( self.object1.doubleProperty == 7436.671 )

		self.object1.stringProperty = " SoMe StRiNg "
		self.assert_( self.object1.stringProperty == " SoMe StRiNg " )

		self.object1.stringListProperty = [ "TestString", " Other String " ]
		self.assert_( self.object1.stringListProperty == [ "TestString", " Other String " ] )

		self.object1.listProperty = [ True, [2464, -8295], -572.07516, "test", [] ]
		self.assert_( self.object1.listProperty == [ True, [2464, -8295], -572.07516, "test", [] ] )

	def testPropertyMethods(self):
		self.object1.setProperty("boolProperty", False)
		self.assert_( self.object1.property("boolProperty") == False )
		self.object1.setProperty("boolProperty", True)
		self.assert_( self.object1.property("boolProperty") == True )

		self.object1.setProperty("intProperty", -75825)
		self.assert_( self.object1.property("intProperty") == -75825 )

		self.object1.setProperty("doubleProperty", -9373.8721)
		self.assert_( self.object1.property("doubleProperty") == -9373.8721 )

		self.object1.setProperty("stringProperty", " AnOtHeR sTrInG ")
		self.assert_( self.object1.property("stringProperty") == " AnOtHeR sTrInG " )

		self.object1.setProperty("stringListProperty", ["s1","s2",""])
		self.assert_( self.object1.property("stringListProperty") == ["s1","s2",""] )

		self.object1.setProperty("listProperty", [[True,False],"abc",[846,-573.02],[]])
		self.assert_( self.object1.property("listProperty") == [[True,False],"abc",[846,-573.02],[]] )

	def testEnumerationMembers(self):
		self.assert_( self.object1.TESTENUM1 == 1 )
		self.assert_( self.object1.TESTENUM2 == 2 )
		self.assert_( self.object1.TESTENUM3 == 4 )
		self.assert_( self.object1.TESTENUM4 == 8 )
		#self.assert_( self.object1.testEnum( self.object1.TESTENUM3 ) == 4 )

	def testSignalsSlots(self):
		self.assert_( self.object1.connect("signalBool(bool)", "func_bool_bool(bool)") )
		self.assert_( self.object1.connect("signalInt(int)", self.object2, "func_int_int(int)") )

		def callback(s):
			self.assert_(s == " The Argument String ")
		self.assert_( self.object1.connect("signalString(const QString&)", callback) )
		self.object1.signalString(" The Argument String ")

	#def testExpectedFailures(self):
		# to less arguments
		#self.assertRaises(ValueError, self.pluginobject1.uintfunc)
		#self.assert_( self.pluginobject1.uintfunc() != 8465 )

	#def testPyQt(self):
		#pyqtextension = TestObject3.toPyQt()
		#print "pyqtextension=%s" % pyqtextension
		#import PyQt4, sip
		#qobj = pyqtextension.getQObject()
		#qo = sip.wrapinstance (qobj, QObject)
		#print ">>>>>>>>>>>>>>>>>>> %s" % qo

print "__name__ = %s" % __name__
#print "__main__ = %s %s" % (__main__,dir(__main__))
#print "TestObject3.name = %s" % TestObject3.name()

suite = unittest.makeSuite(TestKross)
unittest.TextTestRunner(verbosity=2).run(suite)
