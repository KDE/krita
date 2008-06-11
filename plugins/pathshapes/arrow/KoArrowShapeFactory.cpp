/* This file is part of the KDE project
 * Copyright (C) 2006 Laurent Montel <montel@kde.org>
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

#include <KoShape.h>
#include <KoProperties.h>
#include <KoLineBorder.h>
#include <KoColorBackground.h>
#include "arrow/KoArrowShapeFactory.h"
#include "arrow/KoArrowShape.h"

#include <klocale.h>

KoArrowShapeFactory::KoArrowShapeFactory(QObject *parent)
: KoShapeFactory(parent, KoArrowShapeId , i18n("A arrow shape"))
{
    setToolTip(i18n("A arrow"));
    KoShapeTemplate t;
    t.name = i18n("Right Arrow");
    t.toolTip = i18n("A right arrow");
    t.icon = "arrow-right";
    KoProperties *props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowRight);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Left Arrow");
    t.toolTip = i18n("A left arrow");
    t.icon = "arrow-left";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowLeft);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Top Arrow");
    t.toolTip = i18n("A top arrow");
    t.icon = "arrow-up";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowTop);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Bottom Arrow");
    t.toolTip = i18n("A bottom arrow");
    t.icon = "arrow-down";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowBottom);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Left Down Arrow");
    t.toolTip = i18n("A left down arrow");
    t.icon = "arrow-left-down";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowLeftDown);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Left Top Arrow");
    t.toolTip = i18n("A left top arrow");
    t.icon = "arrow-left-up";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowLeftTop);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Right Down Arrow");
    t.toolTip = i18n("A right down arrow");
    t.icon = "arrow-right-down";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowRightDown);
    t.properties = props;
    addTemplate(t);

    t.name = i18n("Right Top Arrow");
    t.toolTip = i18n("A right top arrow");
    t.icon = "arrow-right-up";
    props = new KoProperties();
    props->setProperty("type", KoArrowShape::ArrowRightTop);
    t.properties = props;
    addTemplate(t);
}

KoShape *KoArrowShapeFactory::createDefaultShape() const
{
    KoArrowShape *s = new KoArrowShape();
    s->setSize(QSizeF(100,100));
    s->setBackground( new KoColorBackground( QColor(Qt::red) ) );
    s->setBorder( new KoLineBorder( 1.0 ) );
    return s;
}

KoShape *KoArrowShapeFactory::createShape(const KoProperties* props) const
{
    KoArrowShape *s = new KoArrowShape();
    s->setType(static_cast<KoArrowShape::KoArrowType> (props->intProperty("type", KoArrowShape::ArrowLeft)));
    s->setSize(QSizeF(100,100));
    s->setBackground( new KoColorBackground( QColor(Qt::red) ) );
    s->setBorder( new KoLineBorder( 1.0 ) );
    return s;
}

#include <KoArrowShapeFactory.moc>
