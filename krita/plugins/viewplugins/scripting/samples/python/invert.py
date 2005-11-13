class Inverter:
    def __init__(self):
        try:
            print "Hello World ! 1\n"
            import krosskritacore
            print "Hello World ! 2\n"
            self.doc = krosskritacore.get("KritaDocument")
            print "Hello World ! 3\n"
            self.image = self.doc.getImage();
        except:
            raise "Import of the KritaCore module failed."

Inverter()
