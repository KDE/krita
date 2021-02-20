#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from krita import DockWidgetFactory, DockWidgetFactoryBase
from .quick_settings_docker import QuickSettingsDocker


Application.addDockWidgetFactory(
    DockWidgetFactory("quick_settings_docker",
                      DockWidgetFactoryBase.DockRight,
                      QuickSettingsDocker))
