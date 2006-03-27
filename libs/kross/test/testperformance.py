#!/usr/bin/env python

"""
  This Python script is used as performance-tester and profiler
  for the Kross scripting framework.
"""

def runner():
	import krosstestpluginmodule
	testobject1 = krosstestpluginmodule.testpluginobject1()

	def testKexiDB(kexidbfile,drivername,sqlstring):
		print "test kexidb"
		import krosskexidb
		drivermanager = krosskexidb.DriverManager()
		connectiondata = drivermanager.createConnectionData()
		connectiondata.setFileName(kexidbfile)
		driver = drivermanager.driver(drivername)
		connection = driver.createConnection(connectiondata)
		if not connection.connect(): raise "Connect failed"
		if not connection.useDatabase(kexidbfile): raise "Use database failed"
		for i in range(20000):
			cursor = connection.executeQueryString(sqlstring)
			if not cursor: raise "Failed to create cursor"
			cursor.moveFirst()
			while(not cursor.eof()):
				for i in range( cursor.fieldCount() ):
					(item,field,value) = (cursor.at(), i, cursor.value(i))
					cursor.moveNext()

	def test1():
		print "test1"
		for i in range(100000):
			testobject1.func1()
			testobject1.func1()
			testobject1.func1()
			
			testobject1.func2("f2s1","f2s2")
			testobject1.func2("f2s3","f2s4")
			testobject1.func2("f2s5","f2s6")
			
			testobject1.func3("f3s1","f3s2")
			testobject1.func3("f3s3","f3s4")
			testobject1.func3("f3s5","f3s6")

			testobject1.func4("f4s1","f4s2")
			testobject1.func4("f4s3","f4s4")
			testobject1.func4("f4s5","f4s6")

			testobject1.func5("f5s1","f5s2")
			testobject1.func5("f5s3","f5s4")
			testobject1.func5("f5s5","f5s6")

			testobject1.func6( ("One1","Two2"), "haha" )
			testobject1.func6( ("One3","Two4"), 123456789 )
			testobject1.func6( ("One5","Two6"), 12345.67890 )

			testobject1.func7("f5s1",123)
			testobject1.func7("f5s3",456)
			testobject1.func7("f5s5",789)

			testobject1.func8(123,456)
			testobject1.func8(12,34)
			testobject1.func8(1,2)
			
			testobject1.func9(2.0,1.0)
			testobject1.func9(4.0,3.0)
			testobject1.func9(6.0,5.0)

	#test1()
	testKexiDB("/home/snoopy/test.kexi", "SQLite3", "SELECT * FROM dept")

import profile
__main__.runner=runner
profile.run("runner()")
