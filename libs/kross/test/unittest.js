#!/usr/bin/env kross

// Print something on the console.
println("Let's start the KjsEmbed Unittest :-)");

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

	this.assertArray = function(actual, expected) {
		if(actual.length != expected.length) {
			this.failed(actual, expected);
			println("Array-Length does not match");
		}
		else {
			var failed = 0;
			for(i=0;i<actual.length;i++) {
				if(actual[i] != expected[i]) {
					failed = 1;
					this.failed(actual, expected);
					println("Array-Item actual[i]=" + actual[i] + " expected[i]=" + expected[i]);
					break;
				}
			}
			if(failed == 0)
				this.passed(actual, expected);
		}
	}

	this.printResult = function() {
		println("Tests passed: " + numpassed);
		println("Tests failed: " + numfailed);
	}
}

tester = new UnitTest();

// We have 2 instances of TestObject which inherit QObject.
var testobj1 = TestObject1
var testobj2 = TestObject2

// object
tester.assert(testobj1, "TestObject");
tester.assert(testobj2, "TestObject");
tester.assert(testobj1.name(), "TestObject1");
tester.assert(testobj2.name(), "TestObject2");

// bool
tester.assert(testobj1.func_bool_bool(true), true);
tester.assert(testobj1.func_bool_bool(false), false);

// int
tester.assert(testobj1.func_int_int(0), 0);
tester.assert(testobj1.func_int_int(177321), 177321);
tester.assert(testobj1.func_int_int(-98765), -98765);

// uint
tester.assert(testobj1.func_uint_uint(0), 0);
tester.assert(testobj1.func_uint_uint(177321), 177321);

// double
tester.assert(testobj1.func_double_double(0.0), 0.0);
tester.assert(testobj1.func_double_double(1773.2177), 1773.2177);
tester.assert(testobj1.func_double_double(-548993.271993), -548993.271993);

// longlong
//TODO segfault
//tester.assert(testobj1.func_qlonglong_qlonglong(0), 0);
//tester.assert(testobj1.func_qlonglong_qlonglong(7379), 7379);
//tester.assert(testobj1.func_qlonglong_qlonglong(-6384673), -6384673);
//tester.assert(testobj1.func_qlonglong_qlonglong(678324787843223472165), 678324787843223472165);

// ulonglong
//TODO segfault
//tester.assert(testobj1.func_qulonglong_qulonglong(0), 0);
//tester.assert(testobj1.func_qulonglong_qulonglong(378972), 378972);

// bytearray
//TODO handle in kjsembed/qobject_binding.h QByteArray for KJS::StringType
//tester.assert(testobj1.func_qbytearray_qbytearray("  Some String as ByteArray  "), "  Some String as ByteArray  ");
//tester.assert(testobj1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test "), " \0\n\r\t\s\0 test ");

// string
tester.assert(testobj1.func_qstring_qstring(""), "");
tester.assert(testobj1.func_qstring_qstring(" "), " ");
tester.assert(testobj1.func_qstring_qstring(" Another \n\r Test!   $%&\" "), " Another \n\r Test!   $%&\" ");

// stringlist
tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array()), new Array());
tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array("s1","s2")), new Array("s1","s2"));
tester.assertArray(testobj1.func_qstringlist_qstringlist([]), []);
tester.assertArray(testobj1.func_qstringlist_qstringlist(["abc","def"]), ["abc","def"]);

// variantlist
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array()), new Array());
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array("s1","s2",17,-95)), new Array("s1","s2",17,-95));
tester.assertArray(testobj1.func_qvariantlist_qvariantlist([]), []);
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(["abc","def",426,-842,96.23,-275.637]), ["abc","def",426,-842,96.23,-275.637]);

//variantmap
//TODO
//var v = new Array;
//v["key1"] = "value1";
//v["key2"] = "value2";
//aa = testobj1.func_qvariantmap_qvariantmap( v )
//for(i in v) { println("1 =======> i=" + i + " aa[i]=" + aa[i]); }
//for(i in aa) { println("2 =======> i=" + i + " aa[i]=" + aa[i]); }
//aa = testobj1.func_qvariantmap_qvariantmap( ["key1":"value1","key2":"value"] )
//aa = testobj1.func_qvariantmap_qvariantmap( ["key1"="value1","key2"="value"] )
//aa = testobj1.func_qvariantmap_qvariantmap( ["key1"=>"value1","key2"=>"value"] )
//for(i in aa) { println("2 =======> i=" + i + " aa[i]=" + aa[i]); }

//TODO test following cases
//variant
//propertymembers
//propertymethods
//enumerators
//signals and slots

// print the test-results
tester.printResult();

// Create a dialog and show it.
//var frame = new Widget("QFrame", this );
//frame.frameShape = frame.StyledPanel;
//frame.frameShadow = frame.Sunken;
//frame.lineWidth = 4;
//frame.show();
//exec();
