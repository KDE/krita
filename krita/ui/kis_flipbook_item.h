/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_FLIPBOOK_ITEM_H
#define KIS_FLIPBOOK_ITEM_H

class QString;
class QSize;
class KisDoc2;
class KisPart2;

#include <QStandardItem>
#include <QImage>

#include <krita_export.h>

/**
 * The FlipbookItem class represents an item in the flipbook. It is lazy loading
 * when a non-kra image is referred. When the item gets shown for the first time in
 * the KisView2, then a KisDoc2 is created and cached.
 */
class KRITAUI_EXPORT KisFlipbookItem : public QStandardItem
{

public:
    KisFlipbookItem(const QString &filename);
    ~KisFlipbookItem();

    KisDoc2 *document();
    QString filename() const;
    QString name() const;

    int width() const;
    int height() const;
    const QSize &size() const;
private:

    QString m_filename;
    KisDoc2 *m_document;
    KisPart2 *m_part;
    QImage m_icon;
    QSize m_imageSize;
};

#endif // KIS_FLIPBOOK_ITEM_H
