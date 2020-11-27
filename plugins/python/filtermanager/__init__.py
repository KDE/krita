#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import krita
from .filtermanager import FilterManagerExtension


Scripter.addExtension(FilterManagerExtension(krita.Krita.instance()))
