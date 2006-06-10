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

#include "KoInteractionToolFactory.h"
#include "KoInteractionTool.h"

#include <klocale.h>

#include <QPixmap>

KoInteractionToolFactory::KoInteractionToolFactory() {
}

KoInteractionToolFactory::~KoInteractionToolFactory() {
}

KoTool* KoInteractionToolFactory::createTool(KoCanvasBase *canvas) {
    return new KoInteractionTool(canvas);
}

KoID KoInteractionToolFactory::id() {
    return KoID("Default", i18n("Default"));
}

quint32 KoInteractionToolFactory::priority() const {
    return 0;
}

const QString& KoInteractionToolFactory::toolType() const {
    return QString("main");
}

const QString& KoInteractionToolFactory::tooltipText() const {
    return i18n("Default tool");
}

KoID KoInteractionToolFactory::activationShapeId() const {
    return KoID("defaultTool", i18n("Default Tool"));
}

const QPixmap& KoInteractionToolFactory::icon() const {
    return QPixmap();
}
