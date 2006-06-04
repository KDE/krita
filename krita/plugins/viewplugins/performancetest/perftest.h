/*
 * perftest.h -- Part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef PERFTEST_H_
#define PERFTEST_H_

#include <kparts/plugin.h>
#include <kis_types.h>
#include <kis_global.h>

class KisView;
class KoID;

class PerfTest : public KParts::Plugin
{
    Q_OBJECT
public:
    PerfTest(QObject *parent, const QStringList &);
    virtual ~PerfTest();

private slots:

    void slotPerfTest();

private:

    QString bltTest(quint32 testCount);
    QString fillTest(quint32 testCount);
    QString gradientTest(quint32 testCount);
    QString pixelTest(quint32 testCount);
    QString shapeTest(quint32 testCount);
    QString layerTest(quint32 testCount);
    QString scaleTest(quint32 testCount);
    QString rotateTest(quint32 testCount);
    QString renderTest(quint32 restCount);
    QString selectionTest(quint32 testCount);
    QString colorConversionTest(quint32 testCount);
    QString filterTest(quint32 testCount);
    QString readBytesTest(quint32 testCount);
    QString writeBytesTest(quint32 testCount);
    QString iteratorTest(quint32 testCount);
    QString paintViewTest(quint32 testCount);
    QString paintViewFPSTest();

    QString doBlit(const KoCompositeOp& op,
               KoID cspace,
               quint8 opacity,
               quint32 testCount,
               KisImageSP img);

private:

    KisView * m_view;
    KisPainter * m_painter;

};

#endif // PERFTEST_H_
