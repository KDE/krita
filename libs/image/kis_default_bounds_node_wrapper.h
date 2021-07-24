/*
 *  SPDX-FileCopyrightText: 2020 eoinoneill 1991@gmail.com <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H
#define KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H

#include "kis_default_bounds_base.h"
#include "kis_node.h"
#include "kritaimage_export.h"

class KisDefaultBoundsNodeWrapper;
typedef KisSharedPtr<KisDefaultBoundsNodeWrapper> KisDefaultBoundsNodeWrapperSP;

class KRITAIMAGE_EXPORT KisDefaultBoundsNodeWrapper : public KisDefaultBoundsBase {
public:
    KisDefaultBoundsNodeWrapper(KisBaseNodeWSP node = 0);
    KisDefaultBoundsNodeWrapper(KisDefaultBoundsNodeWrapper& rhs);
    ~KisDefaultBoundsNodeWrapper() override;

    QRect bounds() const override;
    QRect imageBorderRect() const override;
    bool wrapAroundMode() const override;
    int currentLevelOfDetail() const override;
    int currentTime() const override;
    bool externalFrameActive() const override;
    void *sourceCookie() const override;

    static const QRect infiniteRect;

private:
    struct Private;
    Private* m_d;
};

#endif // KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H
