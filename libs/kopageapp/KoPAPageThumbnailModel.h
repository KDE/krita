/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOPAPAGETHUMBNAILMODEL_H
#define KOPAPAGETHUMBNAILMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QSize>

#include "kopageapp_export.h"

class KoPAView;
class KoPAPageBase;

/**
 * Model class for the page thumbnails widget. This class is intented as a simple model to
 * create a list view of available pages. Example usage is widget for choosing master page
 * and slide/page sorter widget.
 *
 * XXX: Isn't that duplicating the model for the document section box? (boud)
 */
class KOPAGEAPP_EXPORT KoPAPageThumbnailModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit KoPAPageThumbnailModel(QList<KoPAPageBase *> pages, QObject *parent = 0);
    ~KoPAPageThumbnailModel();

    // from QAbstractItemModel
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setIconSize(const QSize &size);

private:
    KoPAView *m_view;
    QList<KoPAPageBase *> m_pages;

    int m_iconWidth;
    QSize m_iconSize;
};

#endif
