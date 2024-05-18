

from typing import Type


class DockWidgetFactory(DockWidgetFactoryBase):

    def __init__(self, _id: str, _dockPosition: DockWidgetFactoryBase.DockPosition, _klass: Type[DockWidget]): ...


def qDebug(text: str): ...

