#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import krita
from .documenttools import DocumentToolsExtension

Scripter.addExtension(DocumentToolsExtension(krita.Krita.instance()))
