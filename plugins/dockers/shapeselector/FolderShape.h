/*
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
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

/**
 * The shapeselector allows multiple 'folders' to show a selection of shape templates.
 * In the startup phase there is only one folder, which has no border. If the user adds
 * more folders multiple FolderShape items get created.
 * A FolderShape represents one 'book', which is essentially a collection of items the
 * user can drag to his koffice document to insert.  This class inherits KoShapeContainer
 * and the items to insert are the IconShape or GroupShape or ClipboardProxyShape.
 */
class FolderShape : public KoShapeContainer
{
public:
    FolderShape();

    /// reimplemented from KoShapeContainer
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented from KoShapeContainer
    virtual bool loadOdf(const KoXmlElement&, KoShapeLoadingContext&) { return true; }
    /// reimplemented from KoShapeContainer
    virtual void saveOdf(KoShapeSavingContext&) const {}
    /// reimplemented from KoShapeContainer
    virtual void setSize(const QSizeF &size);

    /// save the contents of the folder to an XML based QDomDocument
    QDomDocument save() const;
     /// load the QDomDocument based XML as saved in save()
    void load(const QDomDocument &document);
};

#endif
