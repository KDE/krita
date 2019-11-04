"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"""
from PyQt5.QtWidgets import QTableWidget, QTableWidgetItem


class DebuggerTable(QTableWidget):

    def __init__(self, parent=None):
        super(DebuggerTable, self).__init__(parent)

        self.setColumnCount(4)

        tableHeader = [i18n('Scope'), i18n('Name'), i18n('Value'), i18n('Type')]
        self.setHorizontalHeaderLabels(tableHeader)
        self.setEditTriggers(self.NoEditTriggers)

    def updateTable(self, data):
        self.clearContents()
        self.setRowCount(0)

        if data and not data.get('quit') and not data.get('exception'):
            locals_list = data['frame']['locals']
            globals_list = data['frame']['globals']

            all_variables = {'locals': locals_list, 'globals': globals_list}

            for scope_key in all_variables:
                for item in all_variables[scope_key]:
                    for key, value in item.items():
                        row = self.rowCount()
                        self.insertRow(row)
                        self.setItem(row, 0, QTableWidgetItem(str(scope_key)))
                        self.setItem(row, 1, QTableWidgetItem(key))
                        self.setItem(row, 2, QTableWidgetItem(value['value']))
                        self.setItem(row, 3, QTableWidgetItem(value['type']))
