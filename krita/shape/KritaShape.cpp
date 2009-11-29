/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>
   Copyright 2007 Thomas Zander <zander@kde.org>

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
#include <QCoreApplication>

#include <klocale.h>
#include <kis_debug.h>

#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoImageData.h>

#include "kis_image.h"
#include "kis_doc2.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"

class KritaShape::Private
{
public:
    KUrl url;
    const KoColorProfile * displayProfile;
    KisDoc2 * doc;
};

KritaShape::KritaShape(const KUrl& url, const QString & profileName)
        : m_d(new Private())
{
    m_d->url = url;
    m_d->doc = 0;
    if (!url.isEmpty()) {
        importImage(url);
    }
    m_d->displayProfile = KoColorSpaceRegistry::instance()->profileByName(profileName);
    setKeepAspectRatio(true);
    moveToThread(QCoreApplication::instance()->thread()); // it's a QObject; lets make sure it always has a proper thread.
}

KritaShape::~KritaShape()
{
    delete m_d;
}

void KritaShape::importImage(const KUrl & url)
{
    delete m_d->doc;
    m_d->doc = new KisDoc2(0, 0, false);
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    m_d->doc->openUrl(url);
}

void KritaShape::slotLoadingFinished()
{
    m_mutex.lock();
    if (m_d && m_d->doc && m_d->doc->image()) {
        // XXX: Resize image to correct aspect ratio
        m_waiter.wakeAll();
        update();
    }
    m_mutex.unlock();

}

void KritaShape::paint(QPainter& painter, const KoViewConverter& converter)
{
    if (m_d && m_d->doc && m_d->doc->image()) {
        // XXX: Only convert the bit the painter needs for painting?
        //      Or should we keep a converted qimage in readiness,
        //      just as with KisCanvas2?
        KisImageWSP kimage = m_d->doc->image();

        QImage qimage = kimage->convertToQImage(0, 0, kimage->width(), kimage->height(),
                                              m_d->displayProfile); // XXX: How about exposure?

        const QRectF paintRect = QRectF(QPointF(0.0, 0.0), size());
        applyConversion(painter, converter);
        painter.drawImage(paintRect.toRect(), qimage);

    } else if (m_d->doc == 0)
        tryLoadFromImageData(qobject_cast<KoImageData*>(KoShape::userData()));
}

void KritaShape::setDisplayProfile(const QString & profileName)
{
    m_d->displayProfile = KoColorSpaceRegistry::instance()->profileByName(profileName);
    update();
}

void KritaShape::saveOdf(KoShapeSavingContext & context) const
{
    // TODO
    Q_UNUSED(context);
}
bool KritaShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return false; // TODO
}

void KritaShape::waitUntilReady(const KoViewConverter &, bool) const
{
    if (m_d && m_d->doc && m_d->doc->image())   // all done
        return;

    KoImageData *data = qobject_cast<KoImageData*>(KoShape::userData());
    if (data == 0 || data->image().isNull())
        return; // no data available at all, so don't try to wait later on.

    KritaShape *me = const_cast<KritaShape*>(this);

    m_mutex.lock();
    me->tryLoadFromImageData(data);
    m_waiter.wait(&m_mutex);
    m_mutex.unlock();
}

void KritaShape::tryLoadFromImageData(KoImageData *data)
{

    if (data == 0)
        return;

    // TODO maybe we want to use the image rawData for that
    QImage qimage = data->image();

    if (qimage.isNull())
        return;

    delete m_d->doc;

    m_d->doc = new KisDoc2(0, 0, false);
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));

    // Create an empty image
    KisImageWSP image = m_d->doc->newImage(i18n("Converted from KoImageData"), qimage.width(), qimage.height(), 0);

    // Convert the QImage to a paint device
    KisPaintLayer * layer = dynamic_cast<KisPaintLayer*>(image->root()->firstChild().data());
    if (layer)
        layer->paintDevice()->convertFromQImage(qimage, "", 0, 0);

    // emits sigLoadingFinished
    m_d->doc->setCurrentImage(image);
}

QImage KritaShape::convertToQImage()
{
    if (m_d->doc && m_d->doc->image()) {
        KisImageWSP image = m_d->doc->image();
        return image->convertToQImage(0, 0, image->width(), image->height(), m_d->displayProfile);
    }
    return QImage();
}

#include "KritaShape.moc"
