/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KODOCUMENTMODEL_H
#define KODOCUMENTMODEL_H

#include <KoDocumentSectionModel.h>

class KoPADocument;
class KoShape;
class KoShapeContainer;
class QAbstractItemModel;
class KoViewConverter;

class KoPADocumentModel : public KoDocumentSectionModel
{
public:
    /// Constructs a new document section model using the specified documents data
    KoPADocumentModel( KoPADocument *document = 0 );
    /// Triggers an update of the complete model
    void update();
    
    /// Set the document used in the model
    void setDocument(KoPADocument* document);

    // from QAbstractItemModel
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData * mimeData( const QModelIndexList & indexes ) const;
    virtual bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
private:
    /// Returns properties of the given shape
    PropertyList properties( KoShape* shape ) const;
    /// Sets the properties on the given shape
    void setProperties( KoShape* shape, const PropertyList &properties );
    /// Creates a thumbnail image with the specified size from the given shape
    QImage createThumbnail( KoShape* shape, const QSize &thumbSize ) const;
    /// Returns the child shape with the given index from the parent shape
    KoShape * childFromIndex( KoShapeContainer *parent, int row ) const;
    /// Returns the zero based index of a child shape within its parent shape
    int indexFromChild( KoShapeContainer *parent, KoShape *child ) const;
    /// Returns the parent model index from the given child shape
    QModelIndex parentIndexFromShape( const KoShape * child );

    KoPADocument *m_document; ///< the undelying data structure
    KoShape *m_shape;
    mutable QList<KoShape*> m_childs;
    mutable KoShapeContainer *m_lastContainer;
};

#endif // KODOCUMENTMODEL_H
