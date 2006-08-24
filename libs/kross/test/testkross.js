var kross = Kross
var kjsaction = kross.action("MyKjsScript")

kjsaction.setInterpreter("python")
kjsaction.setCode("print \"Hello world from Python\"")

println("-----------------------> trigger");
kjsaction.trigger()
