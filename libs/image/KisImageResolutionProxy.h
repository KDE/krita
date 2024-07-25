/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGERESOLUTIONPROXY_H
#define KISIMAGERESOLUTIONPROXY_H

#include <kis_types.h>
#include <kritaimage_export.h>

class KisImageResolutionProxy;
using KisImageResolutionProxySP = QSharedPointer<KisImageResolutionProxy>;


/**
 * KisImageResolutionProxy is a simple interface class that
 * keeps a link to the resolution of KisImage. When KisImage
 * is destroyed, the proxy still stores the "last known" image
 * resolution and returns that to the user.
 *
 * The proxy may also be in a detached state, that is, unconnected
 * to any image. Then is stored the "last known image resolution
 * value".
 */
class KRITAIMAGE_EXPORT KisImageResolutionProxy : public QObject
{
    Q_OBJECT
public:
    KisImageResolutionProxy();
    KisImageResolutionProxy(KisImageWSP image);
    KisImageResolutionProxy(const KisImageResolutionProxy &rhs);
    ~KisImageResolutionProxy();

    qreal xRes() const;
    qreal yRes() const;

    /**
     * Compare resolution of (*this) and \p rhs
     */
    bool compareResolution(const KisImageResolutionProxy &rhs) const;

    /**
     * Returns a copy of this proxy that stores the
     * same resolution, but is detached from the image.
     * That is, when the image changes its resolution
     * any time in the future, this proxy will no react
     * on that.
     */
    KisImageResolutionProxySP cloneDetached() const;

    /**
     * Helper function that checks if the passed image is
     * not null and creates a resolution proxy for that.
     * If the passed image is null, then a detached clone
     * of (*this) is returned.
     *
     * The function is used in layers, when they are detached
     * from the image (layer->setImage(nullptr)). In such a
     * case it is useful to keep the old resolution stored
     * in a detached proxy, other than resetting it to default.
     * It allows avoiding unnecessary updates.
     */
    KisImageResolutionProxySP createOrCloneDetached(KisImageWSP image) const;

    /**
     * Return an identity resolution proxy that returns 1.0 for both,
     * xRes() and yRes(). Used as a fallback proxy mostly.
     */
    static KisImageResolutionProxySP identity();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};



#endif // KISIMAGERESOLUTIONPROXY_H
