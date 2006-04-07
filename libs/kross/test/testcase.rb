require 'test/unit'

class TestKross < Test::Unit::TestCase
	def setup:
		require "krosstestpluginmodule"
		testpluginobject1 = Krosstestpluginmodule::get("testpluginobject1")
		#Krosskritacore::get("KritaDocument")
		#testpluginobject1.func1()
	#def test_primitive
	#	print "---------------- 1\n"

	end
end

print "3----------------\n"
