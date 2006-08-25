#require 'test/unit'
#class TestKross < Test::Unit::TestCase
#	def setup:
		#require "krosstestpluginmodule"
		#testpluginobject1 = Krosstestpluginmodule::get("testpluginobject1")
		#Krosskritacore::get("KritaDocument")
		#testpluginobject1.func1()
	#def test_primitive
	#	print "---------------- 1\n"
#	end
#end

require 'TestObject1'
print "\n1------------------------------------\n"
print TestObject1
print "\n2------------------------------------\n"
#TestObject1.func_void()
print TestObject1.func_qstring_qstring( "string" )
print "\n3------------------------------------\n"

#require 'kross/TestObject1'
#require 'kross/TestObject2'
#print "3----------------\n"
#print TestObject1::get("testpluginobject1")
