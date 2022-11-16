/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageResolutionProxy.h"

#include <kis_image.h>
#include "kis_pointer_utils.h"
#include "kis_signal_auto_connection.h"

namespace {

struct IdentityResolutionProxyHolder
{
    IdentityResolutionProxyHolder()
        : identity(new KisImageResolutionProxy())
    {
    }

    KisImageResolutionProxySP identity;
};

Q_GLOBAL_STATIC(IdentityResolutionProxyHolder, s_holder)
}

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

    ~Private() {
        /**
         * Since we are not using QObject for the connection,
         * we should disconnect the connection automatically
         * on destruction.
         */
        if (imageConnection) {
            QObject::disconnect(imageConnection);
        }
    }

    KisImageWSP image;
    qreal lastKnownXRes;
    qreal lastKnownYRes;
    QMetaObject::Connection imageConnection;

    void setImage(KisImageWSP image);
    void slotImageResolutionChanged(qreal xRes, qreal yRes);
};

KisImageResolutionProxy::KisImageResolutionProxy()
    : KisImageResolutionProxy(nullptr)
{
}

KisImageResolutionProxy::KisImageResolutionProxy(KisImageWSP image)
    : m_d(new Private(image))
{
}

KisImageResolutionProxy::KisImageResolutionProxy(const KisImageResolutionProxy &rhs)
    : QObject(nullptr)
    , m_d(new Private(*rhs.m_d))
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

bool KisImageResolutionProxy::compareResolution(const KisImageResolutionProxy &rhs) const
{
    return qFuzzyCompare(xRes(), rhs.xRes()) &&
        qFuzzyCompare(yRes(), rhs.yRes());

}

KisImageResolutionProxySP KisImageResolutionProxy::cloneDetached() const
{
    KisImageResolutionProxySP proxy(new KisImageResolutionProxy(*this));
    proxy->m_d->setImage(nullptr);
    return proxy;
}

KisImageResolutionProxySP KisImageResolutionProxy::createOrCloneDetached(KisImageWSP image) const
{
    return image ? toQShared(new KisImageResolutionProxy(image)) : cloneDetached();
}

KisImageResolutionProxySP KisImageResolutionProxy::identity()
{
    return s_holder->identity;
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
