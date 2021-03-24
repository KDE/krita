/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOverlayPaintDeviceWrapperTest.h"

#include "KisOverlayPaintDeviceWrapper.h"
#include <KoColorSpaceRegistry.h>
#include <kis_paint_device.h>
#include "kistest.h"

#include <KoColor.h>

#include <kis_paint_device_debug_utils.h>


void KisOverlayPaintDeviceWrapperTest::test()
{
    QVector<const KoColorSpace*> colorSpaces;
    colorSpaces << KoColorSpaceRegistry::instance()->rgb16();
    colorSpaces << KoColorSpaceRegistry::instance()->graya8();

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisOverlayPaintDeviceWrapper wrapper(dev, 2, KisOverlayPaintDeviceWrapper::PreciseMode);

    KoColor colorR(Qt::red, dev->colorSpace());
    KoColor colorG(Qt::green, dev->colorSpace());
    KoColor colorB(Qt::blue, dev->colorSpace());
    KoColor colorB_rgb16(Qt::blue, wrapper.overlayColorSpace());

    dev->fill(QRect(0,0,100,100), colorR);
    dev->fill(QRect(100,0,100,100), colorG);
    dev->fill(QRect(0,100,200,100), colorB);

    KIS_DUMP_DEVICE_2(dev, QRect(0,0,200,200), "00_dev", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(0), QRect(0,0,200,200), "01_ov1", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(1), QRect(0,0,200,200), "02_ov2", "dd");

    wrapper.readRect(QRect(10,10,15,15));

    KIS_DUMP_DEVICE_2(dev, QRect(0,0,200,200), "03_dev", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(0), QRect(0,0,200,200), "04_ov1", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(1), QRect(0,0,200,200), "05_ov2", "dd");


    wrapper.readRect(QRect(70,10,15,15));

    KIS_DUMP_DEVICE_2(dev, QRect(0,0,200,200), "07_dev", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(0), QRect(0,0,200,200), "08_ov1", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(1), QRect(0,0,200,200), "09_ov2", "dd");

    wrapper.overlay(0)->fill(QRect(10,10,15,15), colorB_rgb16);
    wrapper.overlay(0)->fill(QRect(70,10,15,15), colorB_rgb16);

    wrapper.readRect(QRect(70,10,15,15));

    KIS_DUMP_DEVICE_2(dev, QRect(0,0,200,200), "10_dev", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(0), QRect(0,0,200,200), "11_ov1", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(1), QRect(0,0,200,200), "12_ov2", "dd");

    // todo: when writing, do not align
    wrapper.writeRect(QRect(10,10,15,15));

    KIS_DUMP_DEVICE_2(dev, QRect(0,0,200,200), "13_dev", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(0), QRect(0,0,200,200), "14_ov1", "dd");
    KIS_DUMP_DEVICE_2(wrapper.overlay(1), QRect(0,0,200,200), "15_ov2", "dd");

}

KISTEST_MAIN(KisOverlayPaintDeviceWrapperTest)
