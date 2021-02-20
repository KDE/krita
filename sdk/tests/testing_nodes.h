/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTING_NODES_H
#define TESTING_NODES_H

#include "kis_node.h"
#include "kis_image.h"

namespace TestUtil {

struct DefaultNode : public KisNode {
    DefaultNode() : KisNode(nullptr)
    {
    }

    KisPaintDeviceSP paintDevice() const override {
        return KisPaintDeviceSP();
    }

    KisPaintDeviceSP original() const override {
        return KisPaintDeviceSP();
    }

    KisPaintDeviceSP projection() const override {
        return KisPaintDeviceSP();
    }

    bool allowAsChild(KisNodeSP) const override {
        return true;
    }
    const KoColorSpace * colorSpace() const override {
        return 0;
    }
    const KoCompositeOp * compositeOp() const override {
        return 0;
    }
};

}

#endif // TESTING_NODES_H

