#!/usr/bin/env python

"""
  This Python script is used as performance-tester and profiler
  for the Kross scripting framework.
"""

def runner():
	import MyTestObject1
	testobject1 = MyTestObject1

	def test1():
		""" Ok. It's a stupid test. But well... :) """
		print "test1"
		for i in range(300000):
			testobject1.func_uint_uint(233)
			testobject1.func_qstring_qstring("Some string")

	test1()

import profile
__main__.runner=runner
profile.run("runner()")
