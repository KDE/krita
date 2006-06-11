/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoCreateShapesToolFactory.h"
#include "KoCreateShapesTool.h"
#include "KoRectangleShape.h"

#include <klocale.h>

#include <QColor>
#include <QRectF>
#include <QPixmap>

KoCreateShapesToolFactory::KoCreateShapesToolFactory() {
}

KoCreateShapesToolFactory::~KoCreateShapesToolFactory() {
}

KoTool* KoCreateShapesToolFactory::createTool(KoCanvasBase *canvas) {
    return new KoCreateShapesTool(canvas);
}

KoID KoCreateShapesToolFactory::id() {
    return KoID("createShapesTool", i18n("Create Shapes"));
}

quint32 KoCreateShapesToolFactory::priority() const {
    return 1;
}

const QString& KoCreateShapesToolFactory::toolType() const {
    return QString("main");
}

const QString& KoCreateShapesToolFactory::tooltipText() const {
    return i18n("Create object");
}

KoID KoCreateShapesToolFactory::activationShapeId() const {
    return KoID("createTool", i18n("Create object"));
}

const QPixmap& KoCreateShapesToolFactory::icon() const {
    return QPixmap();
}
