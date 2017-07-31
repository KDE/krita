import krita
from exportlayers import uiexportlayers


class ExportLayersExtension(krita.Extension):

    def __init__(self, parent):
        super(ExportLayersExtension, self).__init__(parent)

    def setup(self):
        action = krita.Krita.instance().createAction("export_layers", "Export Layers")
        action.setToolTip("Plugin to export layers from a document")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uiexportlayers = uiexportlayers.UIExportLayers()
        self.uiexportlayers.initialize()


Scripter.addExtension(ExportLayersExtension(krita.Krita.instance()))
