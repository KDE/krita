// Print something on the console.
println("Hello world from kjs :-)");

function UnitTest()
{
	var numpassed = 0;
	var numfailed = 0;

	this.passed = function(actual, expected) {
		numpassed = numpassed + 1;
	}

	this.failed = function(actual, expected) {
		println("FAILED actual=" + actual + " expected=" + expected);
		numfailed = numfailed + 1;
	}

	this.assert = function(actual, expected) {
		if(actual == expected)
			this.passed(actual, expected);
		else
			this.failed(actual, expected);
	}

	this.printResult = function() {
		println("Tests passed: " + numpassed);
		println("Tests failed: " + numfailed);
	}
}

myUnitTest = new UnitTest();
myUnitTest.assert(1,2);
myUnitTest.printResult();

// We have 2 instances of TestObject which is inherits QObject.
//var testobj1 = TestObject1
//var testobj2 = TestObject2
//println("TestObject1: " + testobj1);
//println("TestObject2: " + testobj1);

// Create a dialog and show it.
//var frame = new Widget("QFrame", this );
//frame.frameShape = frame.StyledPanel;
//frame.frameShadow = frame.Sunken;
//frame.lineWidth = 4;
//frame.show();
//exec();
