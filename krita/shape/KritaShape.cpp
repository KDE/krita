/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "KritaShape.h"

#include <QPainter>
#include <QFrame>
#include <QVBoxLayout>

#include <kdebug.h>

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include "kis_image.h"
#include "kis_doc2.h"

class KritaShape::Private
{
public:
    KUrl url;
    KoColorProfile * displayProfile;
    KisDoc2 * doc;
};

KritaShape::KritaShape(const KUrl& url, const QString & profileName)
    : QObject()
    , KoShape()
{
    m_d = new Private();
    m_d->url = url;
    m_d->doc = 0;
    if ( !url.isEmpty() ) {
        importImage( url );
    }
    m_d->displayProfile = KoColorSpaceRegistry::instance()->profileByName(profileName);

}

KritaShape::~KritaShape()
{
    delete m_d;
}

void KritaShape::importImage(const KUrl & url )
{
    delete m_d->doc;
    m_d->doc = new KisDoc2(0, 0, true, true);
    m_d->doc->openURL(url);
    if ( !m_d->doc->isLoading() ) {
        slotLoadingFinished();
    }
    else {
        connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    }

}

void KritaShape::slotLoadingFinished()
{
    if ( m_d && m_d->doc && m_d->doc->currentImage() ) {
        repaint();
    }

}

void KritaShape::paint( QPainter& painter, const KoViewConverter& converter )
{
    if ( m_d && m_d->doc && m_d->doc->currentImage() ) {
        // XXX: Only convert the bit the painter needs for painting?
        //      Or should we keep a converted qimage in readiness,
        //      just as with KisCanvas2?
        KisImageSP kimage= m_d->doc->currentImage();

        QImage qimg = kimage->convertToQImage(0, 0, kimage->width(), kimage->height(),
                                              m_d->displayProfile); // XXX: How about exposure?

        const QRectF paintRect = QRectF( QPointF( 0.0, 0.0 ), size() );
        applyConversion( painter, converter );
        painter.drawImage(paintRect.toRect(), qimg);

    }

}

void KritaShape::setDisplayProfile( const QString & profileName ) {
    m_d->displayProfile = KoColorSpaceRegistry::instance()->profileByName( profileName );
    repaint();
}

#include "KritaShape.moc"
