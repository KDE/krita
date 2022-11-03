/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGERESOLUTIONPROXY_H
#define KISIMAGERESOLUTIONPROXY_H

#include <kis_types.h>
#include <kritaimage_export.h>

class KRITAIMAGE_EXPORT KisImageResolutionProxy : public QObject
{
    Q_OBJECT
public:
    KisImageResolutionProxy(KisImageWSP image);
    KisImageResolutionProxy(const KisImageResolutionProxy &rhs);
    ~KisImageResolutionProxy();

    qreal xRes() const;
    qreal yRes() const;

    void detachFromImage();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

using KisImageResolutionProxySP = QSharedPointer<KisImageResolutionProxy>;

#endif // KISIMAGERESOLUTIONPROXY_H
