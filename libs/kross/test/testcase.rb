require 'test/unit'

class TestKross < Test::Unit::TestCase

	def setup
		require "TestObject1"
		require "TestObject2"
	end

	def testBool
		assert( TestObject1.func_bool_bool(true) == true )
		assert( TestObject1.func_bool_bool(false) == false )
	end

	def testInt
		assert( TestObject1.func_int_int(0) == 0 )
		assert( TestObject1.func_int_int(177321) == 177321 )
		assert( TestObject1.func_int_int(-98765) == -98765 )
	end

	def testUInt
		assert( TestObject1.func_uint_uint(0) == 0 )
		assert( TestObject1.func_uint_uint(177321) == 177321 )
	end

	def testDouble
		assert( TestObject1.func_double_double(0.0) == 0.0 )
		assert( TestObject1.func_double_double(1773.2177) == 1773.2177 )
		assert( TestObject1.func_double_double(-548993.271993) == -548993.271993 )
	end

	def testLongLong
		assert( TestObject1.func_qlonglong_qlonglong(0) == 0 )
		assert( TestObject1.func_qlonglong_qlonglong(7379) == 7379 )
		assert( TestObject1.func_qlonglong_qlonglong(-6384673) == -6384673 )
		#assert( TestObject1.func_qlonglong_qlonglong(678324787843223472165) == 678324787843223472165 )
	end

	def testULongLong
		assert( TestObject1.func_qulonglong_qulonglong(0) == 0 )
		assert( TestObject1.func_qulonglong_qulonglong(378972) == 378972 )
		#assert( TestObject1.func_qulonglong_qulonglong(-8540276823902375665225676321823) == -8540276823902375665225676321823 )
	end
 
	def testByteArray
		#assert( TestObject1.func_qbytearray_qbytearray("  Some String as ByteArray  ") == "  Some String as ByteArray  " )
		#assert( TestObject1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test ") == " \0\n\r\t\s\0 test " )
	end

	def testString
		assert( TestObject1.func_qstring_qstring("") == "" )
		assert( TestObject1.func_qstring_qstring(" ") == " " )
		assert( TestObject1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )
	end

	def testStringList
		assert( TestObject1.func_qstringlist_qstringlist( [] ) == [] )
		assert( TestObject1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		assert( TestObject1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )
	end
 
	def testVariantList
		assert( TestObject1.func_qvariantlist_qvariantlist( [] ) == [] )
		assert( TestObject1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		assert( TestObject1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )
	end
 
	def testVariantMap
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
		assert( TestObject1.name() == "TestObject1" && TestObject2.name() == "TestObject2" )
	end

end

require 'test/unit/ui/console/testrunner'
Test::Unit::UI::Console::TestRunner.run(TestKross)
