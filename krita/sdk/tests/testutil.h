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
#include <QUndoStack>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoProgressUpdater.h>
#include <KoProgressProxy.h>
#include <kis_paint_device.h>
#include <kis_node.h>
#include <kis_undo_adapter.h>
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

struct TestProgressBar : public KoProgressProxy {
    int maximum() const {
        return 0;
    }
    void setValue(int value) {
        Q_UNUSED(value);
        //qDebug() << "Progress (" << this << "): " << value ;
    }
    void setRange(int, int) {}
    void setFormat(const QString &) {}
};


bool compareQImages(QPoint & pt, const QImage & img1, const QImage & img2, int fuzzy = 0)
{
    //     QTime t;
    //     t.start();

    const int w1 = img1.width();
    const int h1 = img1.height();
    const int w2 = img2.width();
    const int h2 = img2.height();
    const int bytesPerLine = img1.bytesPerLine();

    if (w1 != w2 || h1 != h2) {
        pt.setX(-1);
        pt.setY(-1);
        qDebug() << "Images have different sizes";
        return false;
    }

    for (int y = 0; y < h1; ++y) {
        const QRgb * const firstLine = reinterpret_cast<const QRgb *>(img2.scanLine(y));
        const QRgb * const secondLine = reinterpret_cast<const QRgb *>(img1.scanLine(y));

        if (memcmp(firstLine, secondLine, bytesPerLine) != 0) {
            for (int x = 0; x < w1; ++x) {
                const QRgb a = firstLine[x];
                const QRgb b = secondLine[x];
                const bool same = qAbs(qRed(a) - qRed(b)) <= fuzzy
                                  && qAbs(qGreen(a) - qGreen(b)) <= fuzzy
                                  && qAbs(qBlue(a) - qBlue(b)) <= fuzzy;
                if (!same && (qAlpha(a) != 0 || qAlpha(b) != 0)) {
                    qDebug() << " Different! source" << qRed(a) << qGreen(a) << qBlue(a) << "dest" << qRed(b) << qGreen(b) << qBlue(b);
                    pt.setX(x);
                    pt.setY(y);
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



QList<const KoColorSpace*> allColorSpaces()
{

    QList<const KoColorSpace*> colorSpaces;

    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();

    foreach(QString csId, csIds) {
        QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(csId);
        if (profiles.size() == 0) {
            const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(csId, 0);
            colorSpaces.append(cs);
        } else {

            const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(csId, profiles[0]);
            colorSpaces.append(cs);

        }
    }
    return colorSpaces;
}

class KisUndoAdapterDummy : public KisUndoAdapter
{
public:
    KisUndoAdapterDummy() : KisUndoAdapter(0) {}
    ~KisUndoAdapterDummy() {}

public:
    void addCommand(QUndoCommand *cmd) {
        qDebug() << cmd;
        undostack.push(cmd);
    }

    void beginMacro(const QString& s = "Test") {
        undostack.beginMacro(s);
    }

    void endMacro() {
        undostack.endMacro();
    }

    void doUndo() {
        undostack.undo();
    }

private:
    QUndoStack undostack;
};


}

#endif
