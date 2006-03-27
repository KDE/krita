print "---------------- 1\n"
require "krosstestpluginmodule"
print "---------------- 2\n"
testpluginobject1 = Krosstestpluginmodule.get("testpluginobject1")
print "---------------- 3\n"
testpluginobject1.func1()
print "---------------- 4\n"
