/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paintop_test.h"

#include <QTest>
#include "kis_paintop.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_spacing_information.h"

class TestPaintOp : public KisPaintOp
{
public:

    TestPaintOp(KisPainter * gc)
            : KisPaintOp(gc) {
    }

protected:

    KisSpacingInformation paintAt(const KisPaintInformation&) override {
        return KisSpacingInformation(0.0);
    }

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation&) const override
    {
        return KisSpacingInformation(0.0);
    }

};

void KisPaintopTest::testCreation()
{
    KisPainter p;
    TestPaintOp test(&p);
}


QTEST_MAIN(KisPaintopTest)
