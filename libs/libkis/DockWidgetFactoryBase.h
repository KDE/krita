/*
 *  SPDX-FileCopyrightText: 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    DockWidgetFactoryBase(const QString& _id, DockPosition _dockPosition);
    ~DockWidgetFactoryBase() override;
    QString id() const override;
    DockPosition defaultDockPosition() const override;
private:
    QString m_id;
    DockPosition m_dockPosition;
};

#endif
