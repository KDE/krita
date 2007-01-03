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
#include <KoTextShapeFactory.h>

#include <klocale.h>

#include <KoProperties.h>
#include <KoShapeGeometry.h>
#include <KoShape.h>

#include "KoTextShapeData.h"
#include "KoTextShape.h"
#include "KoTextToolFactory.h"

KoTextShapeFactory::KoTextShapeFactory(QObject *parent)
    : KoShapeFactory(parent, KoTextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("A Shape That Shows Text"));

    KoShapeTemplate t;
    t.name = i18n("Simple text");
    t.toolTip = i18n("Text Shape With Some Text");
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("text", "<b>Koffie</b>, koffie... Querelanten\ndrinken geen KOffice maar groene thee.<br>Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");
    addTemplate(t);
}

KoShape *KoTextShapeFactory::createDefaultShape() const {
    KoTextShape *text = new KoTextShape();
    return text;
}

KoShape *KoTextShapeFactory::createShape(const KoProperties * params) const {
    KoTextShape *shape = new KoTextShape();
    KoTextShapeData *data = static_cast<KoTextShapeData*> (shape->userData());
    data->document()->setHtml( params->getProperty("text").toString() );
    return shape;
}

#if 0
QList<KoShapeConfigWidgetBase*> KoTextShapeFactory::createShapeOptionPanels() {
    QList<KoShapeConfigWidgetBase*> answer;
    answer.append(new KoShapeGeometry());
    return answer;
}
#endif

#include "KoTextShapeFactory.moc"
