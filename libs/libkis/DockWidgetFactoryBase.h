/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef LIBKIS_DOCKWIDGETFACTORY_H
#define LIBKIS_DOCKWIDGETFACTORY_H

#include <QString>
#include <KoDockFactoryBase.h>

#include "kritalibkis_export.h"

/**
 * @brief The DockWidgetFactoryBase class is the base class for plugins that want
 * to add a dock widget to every window. You do not need to implement this class
 * yourself, but create a DockWidget implementation and then add the DockWidgetFactory
 * to the Krita instance like this:
 *
 * @code
 * class HelloDocker(DockWidget):
 *   def __init__(self):
 *       super().__init__()
 *       label = QLabel("Hello", self)
 *       self.setWidget(label)
 *       self.label = label
 *
 * def canvasChanged(self, canvas):
 *       self.label.setText("Hellodocker: canvas changed");
 *
 * Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
 *
 * @endcode
 */
class KRITALIBKIS_EXPORT DockWidgetFactoryBase : public KoDockFactoryBase
{
public:
    DockWidgetFactoryBase(const QString& _id, DockPosition _dockPosition, bool _isCollapsable = true, bool _defaultCollapsed = false);
    ~DockWidgetFactoryBase() override;
    QString id() const override;
    DockPosition defaultDockPosition() const override;
    bool isCollapsable() const override;
    bool defaultCollapsed() const override;
private:
    QString m_id;
    DockPosition m_dockPosition;
    bool m_isCollapsable, m_defaultCollapsed;
};

#endif
