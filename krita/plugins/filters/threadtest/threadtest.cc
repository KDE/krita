/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QRect>
#include <QThread>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

// #include <kmessagebox.h>

#include "threadtest.h"

typedef KGenericFactory<KritaThreadTest> KritaThreadTestFactory;
K_EXPORT_COMPONENT_FACTORY( kritathreadtest, KritaThreadTestFactory( "krita" ) )

KritaThreadTest::KritaThreadTest(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaThreadTestFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * r = dynamic_cast<KisFilterRegistry *>(parent);
        r->add(new KisFilterInvert());
    }
}

KritaThreadTest::~KritaThreadTest()
{
}

class InversionThread : public QThread
{

public:

    InversionThread(const QString & name, KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect)
        : QThread()
        , m_name(name)
        , m_src(src)
        , m_dst(dst)
        , m_rect(rect)
        {
            kDebug() << "Thread created " << m_name << ", " << QThread::currentThread() << ", " << m_rect << "\n";
        };

    void run()
        {
            kDebug() << "Thread started:" << m_name  << ", " << QThread::currentThread() << "\n";

            KisRectIteratorPixel dstIt = m_dst->createRectIterator(m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height(), true );
            KisRectIteratorPixel srcIt = m_src->createRectIterator(m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height(), false);
            qint32 depth = m_src -> colorSpace() -> nColorChannels();

            kDebug() << "Thread " << m_name << " starts loop \n";

            while( ! srcIt.isDone() ) {
                if ( srcIt.isSelected() ) {
                    for( int i = 0; i < depth; i++) {
                        dstIt.rawData()[i] = quint8_MAX - srcIt.oldRawData()[i];
                   }
                }
                ++srcIt;
                ++dstIt;
            }
            kDebug() << "Thread " << m_name << " finished \n";
        };

private:
    QString m_name;
    KisPaintDeviceSP m_src;
    KisPaintDeviceSP m_dst;
    QRect m_rect;

};

KisFilterInvert::KisFilterInvert() : KisFilter(id(), "colors", i18n("Invert with &Threads"))
{
}

void KisFilterInvert::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    kDebug() << "Going to invert " << rect << "\n";
    int h2 = rect.height() / 2;
    int w2 = rect.width() / 2;

    setProgressTotalSteps(4);
    InversionThread t1("t1", src, dst, QRect(0, 0, w2, h2));
    InversionThread t2("t2", src, dst, QRect(w2, 0, w2, h2));
    InversionThread t3("t3", src, dst, QRect(0, h2, w2, h2));
    InversionThread t4("t4", src, dst, QRect(w2, h2, w2, h2));

    t1.start();
    t2.start();
    t3.start();
    t4.start();
    t1.wait();
    setProgress(1);
    t2.wait();
    setProgress(2);
    t3.wait();
    setProgress(3);
    t4.wait();
    setProgress(4);

    setProgressDone();
}
