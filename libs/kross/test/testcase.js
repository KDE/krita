// Print something on the console.
println("Hello world from kjs :-)");

// We have 2 instances of TestObject which is inherits QObject.
var testobj1 = TestObject1
var testobj2 = TestObject2
println("TestObject1: " + testobj1);
println("TestObject2: " + testobj1);

// Create a dialog and show it.
var frame = new Widget("QFrame", this );
frame.frameShape = frame.StyledPanel;
frame.frameShadow = frame.Sunken;
frame.lineWidth = 4;
frame.show();
exec();
