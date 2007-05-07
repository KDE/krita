/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2002 Nicolas GOUTTE <goutte@kde.org>

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

#include "KoPictureFilePreview.h"
#include "KoPictureFilePreview.moc"
#include <kdialog.h>
#include <klocale.h>
#include <kurl.h>
#include <QBitmap>
#include <QLayout>
#include <QFileInfo>
#include <QPainter>
#include <q3scrollview.h>
#include <QPalette>
#include <QVBoxLayout>

#include <kdebug.h>

#include <KoPicture.h>

/**
 * This class implements the actual widget that shows the image.
 * It is a scrollview, to have scrollbars if the image is big,
 * and it supports both pixmaps and cliparts
 */
class KoPictureFilePreviewWidget : public Q3ScrollView
{
public:
    KoPictureFilePreviewWidget( QWidget *parent )
        : Q3ScrollView( parent ) {
        viewport()->setBackgroundRole( QPalette::Base );
    }

    bool setPicture( const KUrl& url )
    {
        KoPicture picture;
	if ( url.isLocalFile() )
	{
            if ( !picture.loadFromFile( url.path() ) )
	    {
	        return false;
	    }
	}
	else
	{
	    // ### TODO: find a way to avoid to download the file again later
            if ( !picture.setKeyAndDownloadPicture( url, this ) )
	    {
	        return false;
	    }
	}
        m_size = picture.getOriginalSize();
        m_picture = picture;
        resizeContents( m_size.width(), m_size.height() );
        repaintContents();
        return true;
    }

    void setNullPicture(void)
    {
        m_picture=KoPicture();
        m_size=QSize();
    }

    void drawContents( QPainter *p, int, int, int, int )
    {
        p->setBackground( QBrush( Qt::white ) );
        // Be sure that the background is white (for transparency)
        p->fillRect(0, 0, m_size.width(), m_size.height(), QBrush( Qt::white ));
        m_picture.draw( *p, 0 ,0, m_size.width(), m_size.height());
    }

private:
    KoPicture m_picture;
    QSize m_size;
};

KoPictureFilePreview::KoPictureFilePreview( QWidget *parent )
    : KPreviewWidgetBase( parent )
{
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin(KDialog::marginHint());
    m_widget = new KoPictureFilePreviewWidget( this );
    vb->addWidget( m_widget, 1 );
}

void KoPictureFilePreview::showPreview( const KUrl &u )
{
    m_widget->setPicture( u );
}

void KoPictureFilePreview::clearPreview()
{
    m_widget->setNullPicture();
}

QString KoPictureFilePreview::clipartPattern()
{
    return i18n( "*.svg *.wmf *.qpic|Clipart (*.svg *.wmf *.qpic)" );
}

QStringList KoPictureFilePreview::clipartMimeTypes()
{
    QStringList lst;
    lst << "image/svg+xml";
    lst << "image/x-wmf";
    lst << "image/x-vnd.trolltech.qpicture";
    return lst;
}
