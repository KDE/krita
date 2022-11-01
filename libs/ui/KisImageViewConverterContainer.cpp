/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageViewConverterContainer.h"
#include <kis_image_view_converter.h>
#include <KisDetachedShapesViewConverter.h>
#include <kis_signal_auto_connection.h>
#include <kis_image.h>

struct KisImageViewConverterContainer::Private
{
    QScopedPointer<KisClonableViewConverter> viewConverter;
    KisImageWSP image;
    qreal lastKnownXRes {1.0};
    qreal lastKnownYRes {1.0};
    KisSignalAutoConnectionsStore imageConnections;
};

KisImageViewConverterContainer::KisImageViewConverterContainer(KisImageWSP image)
    : m_d(new Private)
{
    setImage(image);
}

KisImageViewConverterContainer::KisImageViewConverterContainer(const KisImageViewConverterContainer &rhs)
    : m_d(new Private)
{
    m_d->lastKnownXRes = rhs.m_d->lastKnownXRes;
    m_d->lastKnownYRes = rhs.m_d->lastKnownYRes;
    setImage(rhs.m_d->image);
}

KisImageViewConverterContainer::~KisImageViewConverterContainer()
{
}

void KisImageViewConverterContainer::setImage(KisImageWSP image)
{
    m_d->imageConnections.clear();

    if (image) {
        /**
         * NOTE: we cannot just use detached converter all the
         * time, because we cannot update its values in time
         * before the next update after image changed its
         * resolution (the signal is emitted at the very end
         * of the operation).
         */

        m_d->image = image;
        m_d->lastKnownXRes = image->xRes();
        m_d->lastKnownYRes = image->yRes();
        m_d->viewConverter.reset(new KisImageViewConverter(image));

        m_d->imageConnections.addUniqueConnection(image.data(), SIGNAL(sigResolutionChanged(double, double)),
                                                  this, SLOT(slotImageResolutionChanged(qreal, qreal)));
    } else {
        /**
         * The layer will keep "the lastly used resolution"
         * until being attached to the new image.
         */
        m_d->viewConverter.reset(new KisDetachedShapesViewConverter(m_d->lastKnownXRes, m_d->lastKnownYRes));
        m_d->image = nullptr;
    }
}

void KisImageViewConverterContainer::detach()
{
    setImage(0);
}

KoViewConverter *KisImageViewConverterContainer::viewConverter() const
{
    return m_d->viewConverter.data();
}

void KisImageViewConverterContainer::slotImageResolutionChanged(qreal xRes, qreal yRes)
{
    m_d->lastKnownXRes = xRes;
    m_d->lastKnownYRes = yRes;
}
