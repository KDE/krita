#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from .krita_script_starter import KritaScriptStarter

Scripter.addExtension(KritaScriptStarter(Krita.instance()))
