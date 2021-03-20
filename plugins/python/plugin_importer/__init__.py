#
# SPDX-FileCopyrightText: 2019 Rebecca Breu <rebecca@rbreu.de>
#
# This file is part of Krita.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import krita

from .plugin_importer_extension import PluginImporterExtension


krita_instance = krita.Krita.instance()
krita_instance.addExtension(PluginImporterExtension(krita_instance))
