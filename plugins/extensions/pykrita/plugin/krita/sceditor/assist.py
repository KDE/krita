#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtWidgets import (qApp, QListWidget, QListWidgetItem, QTextBrowser,
                             QVBoxLayout, QWidget)


class PopupWidget(QWidget):

    def __init__(self, textedit):
        flags = Qt.ToolTip
        flags = Qt.Window | Qt.FramelessWindowHint | \
            Qt.CustomizeWindowHint | Qt.X11BypassWindowManagerHint
        QWidget.__init__(self, None, flags)
        self.textedit = textedit
        self.vlayout = QVBoxLayout(self)
        self.vlayout.setContentsMargins(0, 0, 0, 0)
        self.init_popup()
        self.show()
        self.hide()
        self.active = False

    def show(self, timeout=0, above=False):
        self.cursor_start_col = self.textedit.textCursor().columnNumber()
        desktop = qApp.desktop()
        screen = desktop.screen(desktop.screenNumber(self))
        screen_width = screen.width()
        screen_height = screen.height()
        win_width = self.width()
        win_height = self.height()
        cursorRect = self.textedit.cursorRect()
        if above:
            pos = self.textedit.mapToGlobal(cursorRect.topLeft())
            pos.setY(pos.y() - win_height)
        else:
            pos = self.textedit.mapToGlobal(cursorRect.bottomLeft())
        if pos.y() < 0:
            pos = self.textedit.mapToGlobal(cursorRect.bottomLeft())
        if pos.y() + win_height > screen_height:
            pos = self.textedit.mapToGlobal(cursorRect.topLeft())
            pos.setY(pos.y() - win_height)
        if pos.x() + win_width > screen_width:
            pos.setX(screen_width - win_width)

        self.move(pos)
        QWidget.show(self)
        self.active = True
        if timeout:
            QTimer.singleShot(timeout * 1000, self.hide)

    def hide(self):
        self.active = False
        QWidget.hide(self)


class CallTip(PopupWidget):

    def init_popup(self):
        self.browser = QTextBrowser(self)
        self.layout().addWidget(self.browser)


class AutoCompleteItem(QListWidgetItem):

    def __init__(self, item):
        QListWidgetItem.__init__(self)
        value = item.name
        self.setText(value)
        self.value = value
        self.kind = item.kind


class AutoComplete(PopupWidget):

    def init_popup(self):
        self.list = QListWidget(self)
        self.list.itemClicked.connect(self.insertItem)
        self.layout().addWidget(self.list)
        self.items = []

    def insertItem(self, item):
        self.insert()

    def insert(self):
        completion = self.items[self.list.currentRow()].value
        cursor = self.textedit.textCursor()
        col = cursor.columnNumber()
        line = unicode(cursor.block().text())
        i = self.cursor_start_col
        while i > 0:
            # print(`line[i:col]`)
            if completion.startswith(line[i:col]):
                # print("break")
                break
            i -= 1
        # print(col,i)
        cursor.insertText(completion[col - i:])
        self.hide()

    def setItems(self, proposals):
        proposals = sorted(proposals, cmp=lambda p1, p2: cmp(p1.name, p2.name))
        del self.items[:]
        self.list.clear()
        for entry in proposals:
            i = AutoCompleteItem(entry)
            self.list.addItem(i)
            self.items.append(i)

    def keyPressEvent(self, event):
        self.list.keyPressEvent(event)
        key = event.key()
        text = event.text()
        if key in [Qt.Key_Right, Qt.Key_Enter, Qt.Key_Return]:
            text = ""
        cursor = self.textedit.textCursor()
        line = unicode(cursor.block().text())
        col = cursor.columnNumber()
        prefix = line[self.cursor_start_col:col] + unicode(text)

        found = False
        for row, item in enumerate(self.items):
            if item.value.startswith(prefix):
                current = self.items[self.list.currentRow()].value
                if not current.startswith(prefix):
                    self.list.setCurrentRow(row)
                found = True
                break
        if not found:
            self.hide()
            return

        if key in [Qt.Key_Up, Qt.Key_Down, Qt.Key_PageUp, Qt.Key_PageDown]:
            return True
        elif key in [Qt.Key_Tab, Qt.Key_Right, Qt.Key_Enter, Qt.Key_Return]:
            self.insert()
            return True
        elif not text:
            self.hide()
