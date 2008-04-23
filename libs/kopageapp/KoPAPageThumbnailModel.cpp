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

#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <KoShapeManager.h>
#include <KoCanvasController.h>
#include <KoShapePainter.h>
#include <KoZoomHandler.h>
#include <KoPageLayout.h>
#include <KoSelection.h>
#include <KoPAPageBase.h>
#include <KoPAPage.h>
#include <KoPAMasterPage.h>
#include <KoShapeLayer.h>

#include "KoPAView.h"
#include "KoPACanvas.h"
#include "KoPADocument.h"

#include <KLocale>

KoPAPageThumbnailModel::KoPAPageThumbnailModel(QList<KoPAPageBase *> pages, bool master, QObject *parent)
    : QAbstractListModel(parent),
    m_pages(pages),
    m_master(master),
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
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_pages.at(index.row())->name();
    else if (role == Qt::DecorationRole)
        return QIcon(paintPage(m_pages.at(index.row())));

    return QVariant();
}

void KoPAPageThumbnailModel::setIconSize(const QSize &size)
{
    m_iconSize = size;
}

/* int KoPAPageThumbnailModel::selectedPageIndex()
{
    return currentRow();
}
*/

QPixmap KoPAPageThumbnailModel::paintPage(KoPAPageBase *page) const
{
    QSize size(m_iconSize);
    KoShapePainter shapePainter;

    QList<KoShape*> shapes;
    double zoom;

    KoPageLayout layout = page->pageLayout();
    double ratio = (double)layout.width / layout.height;
    if (ratio > 1) {
        zoom = (double) size.width() / layout.width;
        size.setWidth(size.width() * ratio);
    }
    else {
        zoom = (double) size.height() / layout.height;
        size.setHeight(size.height() / ratio);
    }

    shapes = page->iterator();
    // also draw shapes from master page if this page is not a master
    if (!m_master) {
        KoPAMasterPage *masterPage = dynamic_cast<KoPAPage *>(page)->masterPage();
        shapes += masterPage->iterator();
    }
    shapePainter.setShapes(shapes);

    QPixmap pixmap(size.width(), size.height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setClipRect(QRect(0, 0, size.width(), size.height()));
    QPen pen(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawRect(0, 0, size.width() - 1, size.height() - 1);
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom(zoom);
    shapePainter.paintShapes(painter, zoomHandler);

    return pixmap;
}

#include "KoPAPageThumbnailModel.moc"

