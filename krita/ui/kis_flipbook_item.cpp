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
#include "kis_flipbook_item.h"

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_part2.h>

#include <QImage>
#include <QFileInfo>
#include <QSize>
#include <QPixmap>
#include <QPainter>

KisFlipbookItem::KisFlipbookItem(const QString &filename)
    : m_filename(filename)
    , m_document(0)
    , m_part(0)
{
    if (m_icon.isNull()) {
        m_icon.load(m_filename);
        if (m_icon.isNull()) {
            // This is an image that Qt cannot load quickly, so we load it ourselves
            KisDoc2 *doc = document();
            if (doc) {
                doc->image()->refreshGraph();
                m_icon = doc->image()->convertToQImage(doc->image()->bounds(), 0);
            }
        }

        m_imageSize = m_icon.size();
        m_icon = m_icon.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap pm(128, 128);
    pm.fill(Qt::darkGray);
    QPainter gc(&pm);
    int x = (128 - m_icon.width()) / 2;
    int y = (128 - m_icon.height()) / 2;
    gc.drawImage(x, y, m_icon);
    gc.end();

    QIcon icon;
    icon.addPixmap(pm);

    setIcon(icon);
    setText(name());
    setToolTip(filename);
}

KisFlipbookItem::~KisFlipbookItem()
{
    delete m_document;
}

KisDoc2 *KisFlipbookItem::document()
{
    if (!m_document) {
        m_document = new KisDoc2();
        m_document->openUrl(KUrl(m_filename));

        if (!m_document->image().isValid()) {
            delete m_document;
            m_document = 0;
            return 0;
        }
    }
    return m_document;
}


QString KisFlipbookItem::filename() const
{
    return m_filename;
}

QString KisFlipbookItem::name() const
{
    QFileInfo info(m_filename);
    return info.fileName();
}

int KisFlipbookItem::width() const
{
    return m_imageSize.width();
}

int KisFlipbookItem::height() const
{
    return m_imageSize.height();
}

const QSize &KisFlipbookItem::size() const
{
    return m_imageSize;
}

