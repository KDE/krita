/*
 * Copyright (c) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTOOLBOXFACTORY_H
#define KOTOOLBOXFACTORY_H

#include <KoDockFactoryBase.h>
#include "kritawidgets_export.h"

#include <QString>
#include <QDockWidget>


/**
 * Factory class to create a new KoToolBox that contains the buttons
 * to activate tools.
 */
class KRITAWIDGETS_EXPORT KoToolBoxFactory : public KoDockFactoryBase
{
public:
    explicit KoToolBoxFactory();
    ~KoToolBoxFactory() override;

    QString id() const override;
    KoDockFactoryBase::DockPosition defaultDockPosition() const override;
    QDockWidget* createDockWidget() override;
    bool isCollapsable() const override { return false; }
};

#endif
