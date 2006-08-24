#!/usr/bin/env python

"""
  This Python script demonstrates how Kross could be used from
  within python scripts.
"""

print "__name__ = %s" % __name__
#print "__main__ = %s %s" % (__main__,dir(__main__))

#import TestObject1, TestObject2
#self.object1 = TestObject1
#self.object2 = TestObject2

import Kross
#print dir(Kross)
kjsaction = Kross.action("MyKjsScript")
#print dir(kjsaction)
kjsaction.setInterpreter("javascript")
kjsaction.setCode( "println(\"Hello world from Kjs\");" )
print "-----------------------> trigger"
kjsaction.trigger()
