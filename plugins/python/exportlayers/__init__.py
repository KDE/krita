#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import krita
from .exportlayers import ExportLayersExtension


Scripter.addExtension(ExportLayersExtension(krita.Krita.instance()))
