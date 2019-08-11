# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode


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
