#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import krita
from .scriptdocker import ScriptDocker

Application.addDockWidgetFactory(
    krita.DockWidgetFactory("scriptdocker",
                            krita.DockWidgetFactoryBase.DockRight,
                            ScriptDocker)
)
