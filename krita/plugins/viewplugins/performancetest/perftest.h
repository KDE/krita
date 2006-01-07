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
class KisID;

class PerfTest : public KParts::Plugin
{
    Q_OBJECT
public:
    PerfTest(QObject *parent, const char *name, const QStringList &);
    virtual ~PerfTest();
    
private slots:

    void slotPerfTest();

private:

    QString bltTest(Q_UINT32 testCount);
    QString fillTest(Q_UINT32 testCount);
    QString gradientTest(Q_UINT32 testCount);
    QString pixelTest(Q_UINT32 testCount);
    QString shapeTest(Q_UINT32 testCount);
    QString layerTest(Q_UINT32 testCount);
    QString scaleTest(Q_UINT32 testCount);
    QString rotateTest(Q_UINT32 testCount);
    QString renderTest(Q_UINT32 restCount);
    QString selectionTest(Q_UINT32 testCount);
    QString colorConversionTest(Q_UINT32 testCount);
    QString filterTest(Q_UINT32 testCount);
    QString readBytesTest(Q_UINT32 testCount);
    QString writeBytesTest(Q_UINT32 testCount);
    QString iteratorTest(Q_UINT32 testCount);
    QString paintViewTest(Q_UINT32 testCount);
    QString paintViewFPSTest();

    QString doBlit(const KisCompositeOp& op, 
               KisID cspace,
               Q_UINT8 opacity,
               Q_UINT32 testCount,
               KisImageSP img);

private:

    KisView * m_view;
    KisPainter * m_painter;

};

#endif // PERFTEST_H_
