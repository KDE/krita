/* This file is part of the KDE project
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
#include "KoRectangleShapeFactory.h"
#include "KoRectangleShape.h"

#include <klocale.h>

KoRectangleShapeFactory::KoRectangleShapeFactory() {
    setId(KoID("rectangleShape", i18n("A simple square shape")));
    setToolTip(i18n("A simple square shape"));
    // XXX: Add a nice icon using the KIconLoader
}


KoShape * KoRectangleShapeFactory::createDefaultShape() {
    KoRectangleShape *s = new KoRectangleShape();
    s->resize(QSizeF(100, 100));
    s->setBackground(QBrush(Qt::yellow));
    return s;
}

KoShape * KoRectangleShapeFactory::createShape(KoProperties * params) const {
    return new KoRectangleShape();
}

KoShape * KoRectangleShapeFactory::createShapeFromTemplate(KoShapeTemplate * shapeTemplate) const {
    return new KoRectangleShape();
}
