/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageResolutionProxy.h"

#include <kis_image.h>
#include "kis_signal_auto_connection.h"

struct KisImageResolutionProxy::Private {
    Private(KisImageWSP image)
        : lastKnownXRes(1.0),
          lastKnownYRes(1.0)
    {
        setImage(image);
    }

    Private(const Private &rhs)
        : lastKnownXRes(rhs.lastKnownXRes),
          lastKnownYRes(rhs.lastKnownYRes)
    {
        setImage(rhs.image);
    }

    KisImageWSP image;
    qreal lastKnownXRes;
    qreal lastKnownYRes;
    QMetaObject::Connection imageConnection;

    void setImage(KisImageWSP image);
    void slotImageResolutionChanged(qreal xRes, qreal yRes);
};

KisImageResolutionProxy::KisImageResolutionProxy(KisImageWSP image)
    : m_d(new Private(image))
{
}

KisImageResolutionProxy::KisImageResolutionProxy(const KisImageResolutionProxy &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KisImageResolutionProxy::~KisImageResolutionProxy()
{
}

qreal KisImageResolutionProxy::xRes() const
{
    return m_d->image ? m_d->image->xRes() : m_d->lastKnownXRes;
}

qreal KisImageResolutionProxy::yRes() const
{
    return m_d->image ? m_d->image->yRes() : m_d->lastKnownYRes;
}

void KisImageResolutionProxy::detachFromImage()
{
    m_d->image = nullptr;
}

void KisImageResolutionProxy::Private::slotImageResolutionChanged(qreal xRes, qreal yRes)
{
    lastKnownXRes = xRes;
    lastKnownYRes = yRes;
}

void KisImageResolutionProxy::Private::setImage(KisImageWSP image)
{
    QObject::disconnect(imageConnection);

    if (image) {
        /**
         * NOTE: we cannot just use detached converter all the
         * time, because we cannot update its values in time
         * before the next update after image changed its
         * resolution (the signal is emitted at the very end
         * of the operation).
         */

        this->image = image;
        lastKnownXRes = image->xRes();
        lastKnownYRes = image->yRes();

        imageConnection = connect(image.data(), &KisImage::sigResolutionChanged,
                                    std::bind(&Private::slotImageResolutionChanged, this,
                                              std::placeholders::_1, std::placeholders::_2));
    } else {
        /**
         * The layer will keep "the lastly used resolution"
         * until being attached to the new image.
         */
        this->image = nullptr;
    }
}
