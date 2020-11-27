#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import krita
from .lastdocumentsdocker import LastDocumentsDocker


Application.addDockWidgetFactory(
    krita.DockWidgetFactory("lastdocumentsdocker",
                            krita.DockWidgetFactoryBase.DockRight,
                            LastDocumentsDocker))
