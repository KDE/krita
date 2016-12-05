class ActionLoader(object):

    @property
    def importPath(self):
        return 'scripter.ui_scripter.actions'

    def addComponents(self, components):
        for action in components:
            if action['parent']:
                action['parent'].addAction(action['component'])
