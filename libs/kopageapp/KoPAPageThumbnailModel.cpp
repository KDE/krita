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

#include "KoPAPageThumbnailModel.h"

#include <QtGui/QIcon>

#include <KLocale>

#include "KoPAPageBase.h"

KoPAPageThumbnailModel::KoPAPageThumbnailModel(QList<KoPAPageBase *> pages, QObject *parent)
    : QAbstractListModel(parent),
    m_pages(pages),
    m_iconSize(512, 512)
{
}

KoPAPageThumbnailModel::~KoPAPageThumbnailModel()
{
}

int KoPAPageThumbnailModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_pages.size();

    return 0;
}

QModelIndex KoPAPageThumbnailModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row >= 0 && row < m_pages.size())
            return createIndex(row, column, m_pages.at(row));
    }

    return QModelIndex();
}

QVariant KoPAPageThumbnailModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        QString name = m_pages.at( index.row() )->name();
        if ( name.isEmpty() ) {
            if(m_pages.at(index.row() )->pageType() == KoPageApp::Slide ) {
                name = i18n( "Slide %1", index.row() + 1 );
            } else {
                name = i18n( "Page %1", index.row() + 1 );
            }
        }
        return name;
    }
    else if (role == Qt::DecorationRole) {
        return QIcon( m_pages.at(index.row())->thumbnail( m_iconSize ) );
    }

    return QVariant();
}

void KoPAPageThumbnailModel::setIconSize(const QSize &size)
{
    m_iconSize = size;
}

#include <KoPAPageThumbnailModel.moc>

