/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoResourceItem.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QFileInfo>
#include <QPainter>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "KoResourceChooser.h"
#include "KoResource.h"

#define THUMB_SIZE 30

KoResourceItem::KoResourceItem(KoResource *resource)
{
    m_resource = resource;
//     updatePixmaps();
    setIcon( QIcon( QPixmap::fromImage( thumbnail( QSize( THUMB_SIZE, THUMB_SIZE ) ) ) ) );
}

KoResourceItem::~KoResourceItem()
{
}

QImage KoResourceItem::thumbnail( const QSize &thumbSize ) const
{
    QSize imgSize = m_resource->img().size();

    if(imgSize.height() > thumbSize.height() || imgSize.width() > thumbSize.width()) {
        qreal scaleW = static_cast<qreal>( thumbSize.width() ) / static_cast<qreal>( imgSize.width() );
        qreal scaleH = static_cast<qreal>( thumbSize.height() ) / static_cast<qreal>( imgSize.height() );

        qreal scale = qMin( scaleW, scaleH );

        int thumbW = static_cast<int>( imgSize.width() * scale );
        int thumbH = static_cast<int>( imgSize.height() * scale );

        return m_resource->img().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio );
    }
    else
        return m_resource->img();
}

KoResource *KoResourceItem::resource() const
{
    return m_resource;
}

int KoResourceItem::compare(const QTableWidgetItem *o) const
{
    const KoResourceItem *other = dynamic_cast<const KoResourceItem *>(o);

    if (other != 0) {
        return m_resource->name().localeAwareCompare(other->m_resource->name());
    } else {
        return 0;
    }
}

QVariant KoResourceItem::data ( int role ) const
{
    if( role == KoResourceChooser::LargeThumbnailRole )
        return thumbnail( QSize( 100, 100 ));

    return QTableWidgetItem::data( role );
}

