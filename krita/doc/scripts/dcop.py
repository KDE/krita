from pydcop import *

app = ""
for a in apps():
    if (a.startswith("krita")):
        app = anyAppCalled(a)
        
doc = app.KoApplicationIface.getDocuments()[0]
img=doc.currentImage()
dev=img.activeDevice()
dev.setName("A new name")
print dev.pixelSize()
print dev.nChannels()
print dev.readBytes(10, 10, 1, 1)
