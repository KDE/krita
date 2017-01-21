from PyQt5.QtWidgets import QTableWidget, QTableWidgetItem


class DebuggerTable(QTableWidget):

    def __init__(self, parent=None):
        super(DebuggerTable, self).__init__(parent)

        self.setRowCount(10)
        self.setColumnCount(4)

        tableHeader = ['Scope', 'Name', 'Value', 'Type']
        self.setHorizontalHeaderLabels(tableHeader)
        self.setEditTriggers(self.NoEditTriggers)

    def updateTable(self, data):
        self.clearContents()

        if data and not data.get('quit'):
            line = 0
            locals_list = data['frame']['locals']
            globals_list = data['frame']['globals']

            for item in locals_list:
                for key, value in item.items():
                    self.setItem(line, 0, QTableWidgetItem('locals'))
                    self.setItem(line, 1, QTableWidgetItem(key))
                    self.setItem(line, 2, QTableWidgetItem(str(value)))
                    self.setItem(line, 3, QTableWidgetItem(str(type(value))))
                    line += 1

            for item in globals_list:
                for key, value in item.items():
                    self.setItem(line, 0, QTableWidgetItem('globals'))
                    self.setItem(line, 1, QTableWidgetItem(key))
                    self.setItem(line, 2, QTableWidgetItem(str(value)))
                    self.setItem(line, 3, QTableWidgetItem(str(type(value))))
                    line += 1
