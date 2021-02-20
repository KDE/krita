#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from .highpass import HighpassExtension

Scripter.addExtension(HighpassExtension(Krita.instance()))
