/* This file is part of the KDE project
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
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

#include "SpiralShapeFactory.h"
#include "SpiralShape.h"
#include "SpiralShapeConfigWidget.h"
#include <KoShapeStroke.h>
#include <KoShapeLoadingContext.h>

#include <KoIcon.h>
#include <klocalizedstring.h>

SpiralShapeFactory::SpiralShapeFactory()
    : KoShapeFactoryBase(SpiralShapeId, i18n("Spiral"))
{
    setToolTip(i18n("A spiral shape"));
    setIconName(koIconNameCStr("spiral-shape"));
    setFamily("geometric");
    setLoadingPriority(1);
}

KoShape *SpiralShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    SpiralShape *spiral = new SpiralShape();

    spiral->setStroke(new KoShapeStroke(1.0));
    spiral->setShapeId(KoPathShapeId);

    return spiral;
}

bool SpiralShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}

QList<KoShapeConfigWidgetBase *> SpiralShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase *> panels;
    panels.append(new SpiralShapeConfigWidget());
    return panels;
}
