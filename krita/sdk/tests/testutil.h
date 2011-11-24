/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TEST_UTIL
#define TEST_UTIL

#include <QList>
#include <QTime>
#include <kundo2qstack.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoProgressUpdater.h>
#include <KoProgressProxy.h>
#include <kis_paint_device.h>
#include <kis_node.h>
#include <kis_undo_adapter.h>
#include "kis_node_graph_listener.h"

/**
 * Routines that are useful for writing efficient tests
 */

namespace TestUtil
{

void dumpNodeStack(KisNodeSP node, QString prefix = QString("\t"))
{
    qDebug() << node->name();
    KisNodeSP child = node->firstChild();

    while (child) {

        if (child->childCount() > 0) {
            dumpNodeStack(child, prefix + "\t");
        } else {
            qDebug() << prefix << child->name();
        }
        child = child->nextSibling();
    }
}

class TestProgressBar : public KoProgressProxy {
public:
    TestProgressBar()
        : m_min(0), m_max(0), m_value(0)
    {}

    int maximum() const {
        return m_max;
    }
    void setValue(int value) {
        m_value = value;
    }
    void setRange(int min, int max) {
        m_min = min;
        m_max = max;
    }
    void setFormat(const QString &format) {
        m_format = format;
    }

    int min() { return m_min; }
    int max() { return m_max; }
    int value() { return m_value; }
    QString format() { return m_format; }

private:
    int m_min;
    int m_max;
    int m_value;
    QString m_format;
};


bool compareQImages(QPoint & pt, const QImage & image1, const QImage & image2, int fuzzy = 0)
{
    //     QTime t;
    //     t.start();

    const int w1 = image1.width();
    const int h1 = image1.height();
    const int w2 = image2.width();
    const int h2 = image2.height();
    const int bytesPerLine = image1.bytesPerLine();

    if (w1 != w2 || h1 != h2) {
        pt.setX(-1);
        pt.setY(-1);
        qDebug() << "Images have different sizes" << image1.size() << image2.size();
        return false;
    }

    for (int y = 0; y < h1; ++y) {
        const QRgb * const firstLine = reinterpret_cast<const QRgb *>(image2.scanLine(y));
        const QRgb * const secondLine = reinterpret_cast<const QRgb *>(image1.scanLine(y));

        if (memcmp(firstLine, secondLine, bytesPerLine) != 0) {
            for (int x = 0; x < w1; ++x) {
                const QRgb a = firstLine[x];
                const QRgb b = secondLine[x];
                const bool same = qAbs(qRed(a) - qRed(b)) <= fuzzy
                                  && qAbs(qGreen(a) - qGreen(b)) <= fuzzy
                                  && qAbs(qBlue(a) - qBlue(b)) <= fuzzy;
                const bool sameAlpha = qAlpha(a) == qAlpha(b);
                const bool bothTransparent = sameAlpha && qAlpha(a)==0;

                if (!bothTransparent && (!same || !sameAlpha)) {
                    pt.setX(x);
                    pt.setY(y);
                    qDebug() << " Different at" << pt
                             << "source" << qRed(a) << qGreen(a) << qBlue(a) << qAlpha(a)
                             << "dest" << qRed(b) << qGreen(b) << qBlue(b) << qAlpha(b);
                    return false;
                }
            }
        }
    }
    //     qDebug() << "compareQImages time elapsed:" << t.elapsed();
    //    qDebug() << "Images are identical";
    return true;
}

bool comparePaintDevices(QPoint & pt, const KisPaintDeviceSP dev1, const KisPaintDeviceSP dev2)
{
    //     QTime t;
    //     t.start();

    QRect rc1 = dev1->exactBounds();
    QRect rc2 = dev2->exactBounds();

    if (rc1 != rc2) {
        pt.setX(-1);
        pt.setY(-1);
    }

    KisHLineConstIteratorPixel iter1 = dev1->createHLineConstIterator(0, 0, rc1.width());
    KisHLineConstIteratorPixel iter2 = dev2->createHLineConstIterator(0, 0, rc1.width());

    int pixelSize = dev1->pixelSize();

    for (int y = 0; y < rc1.height(); ++y) {

        while (!iter1.isDone()) {
            if (memcmp(iter1.rawData(), iter2.rawData(), pixelSize) != 0)
                return false;
            ++iter1;
            ++iter2;
        }

        iter1.nextRow();
        iter2.nextRow();
    }
    //     qDebug() << "comparePaintDevices time elapsed:" << t.elapsed();
    return true;
}

quint8 alphaDevicePixel(KisPaintDeviceSP dev, qint32 x, qint32 y)
{
    KisHLineConstIteratorPixel iter = dev->createHLineConstIterator(x, y, 1);
    const quint8 *pix = iter.rawData();
    return *pix;
}

void alphaDeviceSetPixel(KisPaintDeviceSP dev, qint32 x, qint32 y, quint8 s)
{
    KisHLineIteratorPixel iter = dev->createHLineIterator(x, y, 1);
    quint8 *pix = iter.rawData();
    *pix = s;
}


QList<const KoColorSpace*> allColorSpaces()
{
    return KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
}

class TestGraphListener : public KisNodeGraphListener
{
public:

    virtual void aboutToAddANode(KisNode *, int) {
        beforeInsertRow = true;
    }

    virtual void nodeHasBeenAdded(KisNode *, int) {
        afterInsertRow = true;
    }

    virtual void aboutToRemoveANode(KisNode *, int) {
        beforeRemoveRow  = true;
    }

    virtual void nodeHasBeenRemoved(KisNode *, int) {
        afterRemoveRow = true;
    }


    virtual void aboutToMoveNode(KisNode *, int, int) {
        beforeMove = true;
    }

    virtual void nodeHasBeenMoved(KisNode *, int, int) {
        afterMove = true;
    }

    virtual void nodeChanged(KisNode*) {

    }

    virtual void requestProjectionUpdate(KisNode *, const QRect& ) {

    }


    bool beforeInsertRow;
    bool afterInsertRow;
    bool beforeRemoveRow;
    bool afterRemoveRow;
    bool beforeMove;
    bool afterMove;

    void resetBools() {
        beforeRemoveRow = false;
        afterRemoveRow = false;
        beforeInsertRow = false;
        afterInsertRow = false;
        beforeMove = false;
        afterMove = false;
    }
};

}

#endif
