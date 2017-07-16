import krita
from documenttools import uidocumenttools


class DocumentToolsExtension(krita.Extension):

    def __init__(self, parent):
        super(DocumentToolsExtension, self).__init__(parent)

    def setup(self):
        action = krita.Krita.instance().createAction("document_tools", "Document Tools")
        action.setToolTip("Plugin to manipulate properties of selected documents")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uidocumenttools = uidocumenttools.UIDocumentTools()
        self.uidocumenttools.initialize()


Scripter.addExtension(DocumentToolsExtension(krita.Krita.instance()))
