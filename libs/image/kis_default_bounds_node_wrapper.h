/*
 *  Copyright (c) 2020 eoinoneill1991@gmail.com <eoinoneill1991@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H
#define KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H

#include "kis_default_bounds_base.h"
#include "kis_node.h"

class KisDefaultBoundsNodeWrapper;
typedef KisSharedPtr<KisDefaultBoundsNodeWrapper> KisDefaultBoundsNodeWrapperSP;

class KisDefaultBoundsNodeWrapper : public KisDefaultBoundsBase {
public:
    KisDefaultBoundsNodeWrapper(KisNodeWSP node = 0);
    KisDefaultBoundsNodeWrapper(KisDefaultBoundsNodeWrapper& rhs);
    ~KisDefaultBoundsNodeWrapper() override;

    QRect bounds() const override;
    QRect imageBorderRect() const override;
    bool wrapAroundMode() const override;
    int currentLevelOfDetail() const override;
    int currentTime() const override;
    bool externalFrameActive() const override;
    void * sourceCookie() const override;

    static const QRect infiniteRect;

private:
    struct Private;
    Private* m_d;
};

#endif // KIS_DEFAULT_BOUNDS_NODE_WRAPPER_H
