# SPDX-License-Identifier: CC0-1.0


class FilterManagerTreeItem(object):

    def __init__(self, data, parent=None):
        self.itemData = data
        self.parentItem = parent
        self.childItems = []

    def appendChild(self, child):
        self.childItems.append(child)

    def appenChildren(self, children):
        self.childItems.extend(children)

    def child(self, row):
        return self.childItems[row]

    def childCount(self):
        return len(self.childItems)

    def columnCount(self):
        return len(self.itemData)

    def data(self, column):
        try:
            return self.itemData[column]
        except IndexError:
            return None

    def row(self):
        if self.parentItem:
            return self.parentItem.childItems.index(self)
        return 0

    def parent(self):
        return self.parentItem
