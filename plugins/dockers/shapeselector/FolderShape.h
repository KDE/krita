/*
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef FOLDERSHAPE_H
#define FOLDERSHAPE_H

#include <KoShapeContainer.h>

#include <QDomDocument>

#define FOLDERSHAPE_MIMETYPE "application/x-flake-shapeSelector-folder"

class FolderShape : public KoShapeContainer
{
public:
    FolderShape();

    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);

    virtual bool loadOdf(const KoXmlElement&, KoShapeLoadingContext&) { return true; }
    virtual void saveOdf(KoShapeSavingContext&) const {}
    virtual void setSize( const QSizeF &size );

    QDomDocument save();
    void load(const QDomDocument &document);
};

#endif
