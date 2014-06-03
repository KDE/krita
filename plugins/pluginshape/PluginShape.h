/* This file is part of the KDE project
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Vidhyapria  Arunkumar <vidhyapria.arunkumar@nokia.com>
 * Contact: Amit Aggarwal <amit.5.aggarwal@nokia.com>
 * Contact: Manikandaprasad N C <manikandaprasad.chandrasekar@nokia.com>
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

#ifndef PLUGINSHAPE_H
#define PLUGINSHAPE_H

#include <QMap>
#include <KoShape.h>
#include <KoFrameShape.h>

#define PLUGINSHAPEID "PluginShape"


class PluginShape : public KoShape, public KoFrameShape
{
public:
    PluginShape();
    virtual ~PluginShape();

    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);

private:
    //Note:- We assume that all the name of draw:param are unique.
    QMap<QString,QString> m_drawParams;
    QString m_mimetype;
    QString m_xlinkactuate;
    QString m_xlinkhref;
    QString m_xlinkshow;
    QString m_xlinktype;
};

#endif
