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
#include <KoProperties.h>

#include "KoRectangleShapeFactory.h"
#include "KoRectangleShape.h"

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

KoRectangleShapeFactory::KoRectangleShapeFactory(QObject *parent, const QStringList&)
: KoShapeFactory(parent, KoRectangleShape_SHAPEID, i18n("A simple square shape"))
{
    setToolTip(i18n("A simple square shape"));

    KoShapeTemplate t;
    t.name = "Red Square";
    t.toolTip = "Nicely colored square";
    KoProperties *props = new KoProperties();
    t.properties = props;
    t.icon = "redSquare";
    //t.pixmap = KGlobal::iconLoader()->loadIcon("redSquare", K3Icon::NoGroup);
    props->setProperty("fill", "red");
    addTemplate(t);

    t.name = "Blue Square";
    t.toolTip = "Coldly colored square";
    props = new KoProperties();
    t.properties = props;
    t.icon = "blueSquare";
    //t.pixmap = KGlobal::iconLoader()->loadIcon("blueSquare", K3Icon::NoGroup);
    props->setProperty("fill", "blue");
    addTemplate(t);
}

KoShape * KoRectangleShapeFactory::createDefaultShape() {
    KoRectangleShape *s = new KoRectangleShape();
    s->resize(QSizeF(100, 100));
    s->setBackground(QBrush(Qt::yellow));
    return s;
}

KoShape * KoRectangleShapeFactory::createShape(const KoProperties * params) const {
    KoRectangleShape *shape = new KoRectangleShape();
    if(params->getProperty("fill") == "red")
        shape->setBackground(QBrush(Qt::red));
    if(params->getProperty("fill") == "blue")
        shape->setBackground(QBrush(Qt::blue));
    return shape;
}
