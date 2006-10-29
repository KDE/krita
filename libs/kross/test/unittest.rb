#!/usr/bin/env kross

require 'test/unit'

class TestKross < Test::Unit::TestCase

	def setup
		require "TestObject1"
		require "TestObject2"
	end

	def testBool
		assert_raises(TypeError) { TestObject1.func_bool_bool(nil) }
		assert( TestObject1.func_bool_bool(true) == true )
		assert( TestObject1.func_bool_bool(false) == false )
	end

	def testInt
		assert_raises(TypeError) { TestObject1.func_int_int(nil) }
		assert( TestObject1.func_int_int(0) == 0 )
		assert( TestObject1.func_int_int(177321) == 177321 )
		assert( TestObject1.func_int_int(-98765) == -98765 )
	end

	def testUInt
		assert_raises(TypeError) { TestObject1.func_uint_uint(nil) }
		assert( TestObject1.func_uint_uint(0) == 0 )
		assert( TestObject1.func_uint_uint(177321) == 177321 )
	end

	def testDouble
		assert_raises(TypeError) { TestObject1.func_double_double(nil) }
		assert( TestObject1.func_double_double(0.0) == 0.0 )
		assert( TestObject1.func_double_double(1773.2177) == 1773.2177 )
		assert( TestObject1.func_double_double(-548993.271993) == -548993.271993 )
	end

	def testLongLong
		assert_raises(TypeError) { TestObject1.func_qlonglong_qlonglong(nil) }
		assert( TestObject1.func_qlonglong_qlonglong(0) == 0 )
		assert( TestObject1.func_qlonglong_qlonglong(7379) == 7379 )
		assert( TestObject1.func_qlonglong_qlonglong(-6384673) == -6384673 )
		#assert( TestObject1.func_qlonglong_qlonglong(678324787843223472165) == 678324787843223472165 )
	end

	def testULongLong
		assert_raises(TypeError) { TestObject1.func_qulonglong_qulonglong(nil) }
		assert( TestObject1.func_qulonglong_qulonglong(0) == 0 )
		assert( TestObject1.func_qulonglong_qulonglong(378972) == 378972 )
		#assert( TestObject1.func_qulonglong_qulonglong(-8540276823902375665225676321823) == -8540276823902375665225676321823 )
	end

	def testByteArray
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(nil) }
		assert( TestObject1.func_qbytearray_qbytearray("") == "" )
		assert( TestObject1.func_qbytearray_qbytearray("  Some String as ByteArray  ") == "  Some String as ByteArray  " )
		assert( TestObject1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test ") == " \0\n\r\t\s\0 test " )
	end

	def testString
		assert_raises(TypeError) { TestObject1.func_qstring_qstring(nil) }
		assert( TestObject1.func_qstring_qstring("") == "" )
		assert( TestObject1.func_qstring_qstring(" ") == " " )
		assert( TestObject1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )
	end

	def testStringList
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist(nil) }
		assert( TestObject1.func_qstringlist_qstringlist( [] ) == [] )
		assert( TestObject1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		assert( TestObject1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )
	end
 
	def testVariantList
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(nil) }
		assert( TestObject1.func_qvariantlist_qvariantlist( [] ) == [] )
		assert( TestObject1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		assert( TestObject1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )
	end
 
	def testVariantMap
		assert_raises(TypeError) { TestObject1.func_qvariantmap_qvariantmap(nil) }
		#assert( {} )
		#assert( {"1":73682,"2":285} )
		#assert( {"a":-6892.957,"b":692.66} )
		#assert( {"key1":True,"key2":False} )
		#assert( {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} )
		#assert( {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} )
	end

	def testVariant
		#assert( TestObject1.func_qvariant_qvariant(0.0) == 0.0 )
		#assert( TestObject1.func_qvariant_qvariant(true) == true )
		#assert( TestObject1.func_qvariant_qvariant(false) == false )
		#assert( TestObject1.func_qvariant_qvariant(187937) == 187937 )
		#assert( TestObject1.func_qvariant_qvariant(-69825) == -69825 )
		#assert( TestObject1.func_qvariant_qvariant(8632.274) == 8632.274 )
		#assert( TestObject1.func_qvariant_qvariant(-8632.351) == -8632.351 )
		#assert( TestObject1.func_qvariant_qvariant(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")
	end

	def testObject
		assert( TestObject1.name() == "TestObject1" )
		assert( TestObject2.name() == "TestObject2" )

		assert( TestObject1.func_bool_bool(true) == TestObject2.func_bool_bool(true) )
		assert( TestObject2.func_bool_bool(false) == TestObject2.func_bool_bool(false) )
		assert( TestObject1.func_int_int(82396) == TestObject2.func_int_int(82396) )
		assert( TestObject1.func_int_int(-672) == TestObject2.func_int_int(-672) )
		assert( TestObject1.func_qstringlist_qstringlist( ["s1","s2"] ) == TestObject2.func_qstringlist_qstringlist( ["s1","s2"] ) )
	end

	def testProperties
		TestObject1.boolProperty = true
		assert( TestObject1.boolProperty == true )
		TestObject1.boolProperty = false
		assert( TestObject1.boolProperty == false )

		TestObject1.intProperty = 20
		assert( TestObject1.intProperty == 20 )

		TestObject1.doubleProperty = 7436.671
		assert( TestObject1.doubleProperty == 7436.671 )

		TestObject1.stringProperty = " SoMe StRiNg "
		assert( TestObject1.stringProperty == " SoMe StRiNg " )

		TestObject1.stringListProperty = [ "TestString", " Other String " ]
		assert( TestObject1.stringListProperty == [ "TestString", " Other String " ] )

		TestObject1.listProperty = [ true, [2464, -8295], -572.07516, "test", [] ]
		assert( TestObject1.listProperty == [ true, [2464, -8295], -572.07516, "test", [] ] )
	end

	def testEnumerations
		assert( TestObject1.TESTENUM1 == 1 )
		assert( TestObject1.TESTENUM2 == 2 )
		assert( TestObject1.TESTENUM3 == 4 )
		assert( TestObject1.TESTENUM4 == 8 )
		#self.assert_( self.object1.testEnum( self.object1.TESTENUM3 ) == 4 )
	end

end

require 'test/unit/ui/console/testrunner'
Test::Unit::UI::Console::TestRunner.run(TestKross)
