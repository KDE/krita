from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *
from scripter import syntax

class ScripterViewExtension(ViewExtension):

  def __init__(self, parent):
      super().__init__(parent)

  def setup(self, viewManager):
      action = viewManager.createAction("Scripter")
      action.triggered.connect(self.showScripter)

      self.currentWindow = viewManager.mainWindow()

  def showScripter(self):
      dialog = QDialog(self.currentWindow)
      dialog.setWindowModality(Qt.NonModal)

      editor = QPlainTextEdit()
      f = QFont("monospace", 10, QFont.Normal)
      f.setFixedPitch(True)
      editor.document().setDefaultFont(f)
      highlight = syntax.PythonHighlighter(editor.document())

      vbox = QVBoxLayout(dialog)
      vbox.addWidget(editor)

      dialog.resize(350, 200)
      dialog.setWindowTitle("Scripter")
      dialog.setSizeGripEnabled(True)
      dialog.show()
      dialog.activateWindow()

Krita.instance().addViewExtension(ScripterViewExtension(Krita.instance()))

