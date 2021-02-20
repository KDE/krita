#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from krita import Krita, DockWidgetFactory, DockWidgetFactoryBase
from .hello import HelloExtension, HelloDocker


# Initialize and add the extension
Scripter.addExtension(HelloExtension(Krita.instance()))


# Register the docker so Krita can use it!
Application.addDockWidgetFactory(
    DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
