#!/usr/bin/env python

"""
  This Python script is used to test the Kross scripting framework.
"""

import unittest

class TestPlugin(unittest.TestCase):
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
		self.assert_( self.object1.func_qbytearray_qbytearray("\0") == "" )
		self.assert_( self.object1.func_qbytearray_qbytearray("  \0  ") == "  " )
		self.assert_( self.object1.func_qbytearray_qbytearray("\n\r\t\s \0 test \0\0\0 test") == "\n\r\t\s " )

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
	        #TODO we can't just compare the dicts like at the other cases cause at least QMap changes the sorting order!!!
		self.assert_( self.object1.func_qvariantmap_qvariantmap( {} ) == {} )
		#self.assert_( self.object1.func_qvariantmap_qvariantmap( {"1":73682,"2":285} ) == {"1":73682,"2":285} )
		#self.assert_( self.object1.func_qvariantmap_qvariantmap( {"a":-6892.957,"b":692.66} ) == {"a":-6892.957,"b":692.66} )
		#self.assert_( self.object1.func_qvariantmap_qvariantmap( {"key1":True,"key2":False} ) == {"key1":True,"key2":False} )
		#self.assert_( self.object1.func_qvariantmap_qvariantmap( {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} ) == {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} )
		#self.assert_( self.object1.func_qvariantmap_qvariantmap( {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} ) == {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} )

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
		#self.assert_( self.object1.func_testobject_testobject(self.object1).name() == self.object1.name() )
		#self.assert_( self.object1.func_testobject_testobject(self.object2).name() == self.object2.name() )
		#self.assert_( self.object2.func_testobject_testobject(self.object1).name() == self.object1.name() )
		#self.assert_( self.object2.func_testobject_testobject(self.object2).name() == self.object2.name() )
		pass

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
#print "========> %s" % TestObject3.name()

suite = unittest.makeSuite(TestPlugin)
unittest.TextTestRunner(verbosity=2).run(suite)

#print "1==============> %s" % TestObject3.func_testobject_testobject(TestObject3)
#print "2==============> %s" % TestObject3.func_qobject_qobject( TestObject3 )
#print "3==============> %s" % TestObject3.func_qstringlist_qstringlist( [" string1","string2 "] )
#print "4==============> %s" % TestObject3.func_void_qstringlist( [" string1","string2 "] )
#print "5==============> %s" % TestObject3.func_void()
