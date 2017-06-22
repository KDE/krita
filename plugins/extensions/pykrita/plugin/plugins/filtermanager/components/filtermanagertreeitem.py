class FilterManagerTreeItem(object):

    def __init__(self, data, parent=None):
        self.itemData = data
        self.parent = parent
        self.childItems = []

    def appendChild(self, child):
        self.childItems.append(child)

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
        if self.parent:
            return self.parent.childItems.index(self)
        return 0

    def parentItem(self):
        return self.parent
