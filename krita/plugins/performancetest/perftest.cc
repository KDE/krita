/*
 * perftest.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <qcolor.h>

#include <kis_resourceserver.h>
#include "kis_cursor.h"
#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_colorspace_registry.h>
#include <kis_abstract_colorspace.h>
#include <kis_painter.h>
#include <kis_fill_painter.h>
#include <kis_id.h>
#include <kis_paint_device_impl.h>
#include <kis_iterators_pixel.h>
#include "kis_factory.h"
#include "perftest.h"

#include "dlg_perftest.h"

#define USE_CALLGRIND 0

#if USE_CALLGRIND
#include <valgrind/callgrind.h>
#endif


typedef KGenericFactory<PerfTest> PerfTestFactory;
K_EXPORT_COMPONENT_FACTORY( kritaperftest, PerfTestFactory( "krita" ) )

PerfTest::PerfTest(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(PerfTestFactory::instance());

     kdDebug(DBG_AREA_PLUGINS) << "PerfTest plugin. Class: " 
           << className() 
           << ", Parent: " 
           << parent -> className()
           << "\n";

    (void) new KAction(i18n("&Performance Test..."), 0, 0, this, SLOT(slotPerfTest()), actionCollection(), "perf_test");

    
    if ( !parent->inherits("KisView") )
    {
        m_view = 0;
    } else {
        m_view = (KisView*) parent;
    }
}

PerfTest::~PerfTest()
{
    m_view = 0;
}

void PerfTest::slotPerfTest()
{
    KisImageSP image = m_view -> currentImg();

    if (!image) return;

    DlgPerfTest * dlgPerfTest = new DlgPerfTest(m_view, "PerfTest");
    Q_CHECK_PTR(dlgPerfTest);

    dlgPerfTest -> setCaption(i18n("Performance Test"));
    
    QString report = QString("");

        if (dlgPerfTest -> exec() == QDialog::Accepted) {

        Q_INT32 testCount = (Q_INT32)qRound(dlgPerfTest -> page() -> intTestCount -> value());    
        if (dlgPerfTest -> page() -> chkBitBlt -> isChecked()) {
            report = report.append(bltTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkFill -> isChecked()) {
            report = report.append(fillTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkGradient -> isChecked()) {
            report = report.append(gradientTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkPixel -> isChecked()) {
            report = report.append(pixelTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkShape -> isChecked()) {
            report = report.append(shapeTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkLayer -> isChecked()) {
            report = report.append(layerTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkScale -> isChecked()) {
            report = report.append(scaleTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkRotate -> isChecked()) {
            report = report.append(rotateTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkRender -> isChecked()) {
            report = report.append(renderTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkSelection -> isChecked()) {
            report = report.append(selectionTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkColorConversion -> isChecked()) {
            report = report.append(colorConversionTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkFilter-> isChecked()) {
            report = report.append(filterTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkReadBytes -> isChecked()) {
            report = report.append(readBytesTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkWriteBytes-> isChecked()) {
            report = report.append(writeBytesTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkIterators -> isChecked()) {
            report = report.append(iteratorTest(testCount));
        }
        if (dlgPerfTest -> page() -> chkPaintView -> isChecked()) {
            report = report.append(paintViewTest(testCount));
        }
        KDialogBase * d = new KDialogBase(m_view, "", true, "", KDialogBase::Ok);
        Q_CHECK_PTR(d);

        d -> setCaption("Performance test results");
        QTextEdit * e = new QTextEdit(d);
        Q_CHECK_PTR(e);
        d -> setMainWidget(e);
        e -> setText(report);
        e -> setMinimumWidth(600);
        e -> setMinimumHeight(600);
        d -> exec();
        delete d;
        
    }
        delete dlgPerfTest;
}

QString PerfTest::bltTest(Q_UINT32 testCount)
{
    QString report = QString("* bitBlt test\n");

    KisDoc * doc = m_view -> getDocument();
    KisIDList l = KisColorSpaceRegistry::instance() -> listKeys();

    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing blitting on " + (*it).name() + "\n");

         KisImage * img = doc -> newImage("blt-" + (*it).name(), 1000, 1000,
                KisColorSpaceRegistry::instance() -> get(*it));

        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");

        delete img;

    }

    return report;
    

}


QString PerfTest::doBlit(const KisCompositeOp& op, 
             KisID cspace,
             QUANTUM opacity,
             Q_UINT32 testCount,
             KisImageSP img)
{
    
    QTime t;
    QString report;

    // ------------------------------------------------------------------------------
    // Small

    KisLayerSP small = new KisLayer(KisColorSpaceRegistry::instance() -> get(cspace), "small blit");
    Q_CHECK_PTR(small);

    KisFillPainter pf(small.data()) ;
    pf.fillRect(0, 0, 64/2, 64/2, Qt::black);
    pf.end();

    t.restart();
    KisPainter p(img -> activeLayer());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small.data(),0,0,32, 32);
    }
    p.end();
    
    report = report.append(QString("   %1 blits of rectangles < tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Medium
    KisLayerSP medium = new KisLayer(KisColorSpaceRegistry::instance() -> get(cspace), "medium blit");
    Q_CHECK_PTR(medium);

    pf.begin(medium.data()) ;
    pf.fillRect(0, 0, 64 * 3, 64 * 3, Qt::black);
    pf.end();

    t.restart();
    p.begin(img -> activeLayer().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, medium.data(),0,0,96, 96);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles 3 * tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Big
    KisLayerSP big = new KisLayer(KisColorSpaceRegistry::instance() -> get(cspace), "big blit");
    Q_CHECK_PTR(big);

    pf.begin(big.data()) ;
    pf.fillRect(0, 0, 800, 800, Qt::black);
    pf.end();

    t.restart();
    p.begin(img -> activeLayer().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, big.data(),0,0,800,800);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 800 x 800 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Outside

    KisLayerSP outside = new KisLayer(KisColorSpaceRegistry::instance() -> get(cspace), "outside blit");
    Q_CHECK_PTR(outside);
    pf.begin(outside.data()) ;
    pf.fillRect(0, 0, 500, 500, Qt::lightGray);
    pf.end();

    t.restart();
    p.begin(img -> activeLayer().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(600, 600, op, outside.data(),0,0,500,500);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 500 x 500 at 600,600 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    // ------------------------------------------------------------------------------
    // Small with varied source opacity

    KisLayerSP small_with_alpha = new KisLayer(KisColorSpaceRegistry::instance() -> get(cspace), "small blit with alpha");
    Q_CHECK_PTR(small_with_alpha);

    pf.begin(small_with_alpha.data()) ;
    pf.fillRect(0, 0, 32, 32, Qt::black, OPACITY_TRANSPARENT);
    pf.fillRect(4, 4, 24, 24, Qt::black, OPACITY_OPAQUE / 2);
    pf.fillRect(8, 8, 16, 16, Qt::black, OPACITY_OPAQUE);
    pf.end();

    t.restart();
    p.begin(img -> activeLayer().data());
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small_with_alpha.data(), 0, 0, 32, 32);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles < tilesize with source alpha, with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    return report;

}

QString PerfTest::fillTest(Q_UINT32 testCount)
{
    QString report = QString("* Fill test\n");

    KisDoc * doc = m_view -> getDocument();
    KisIDList l = KisColorSpaceRegistry::instance() -> listKeys();


    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing blitting on " + (*it).name() + "\n");

         KisImage * img = doc -> newImage("fill-" + (*it).name(), 1000, 1000, KisColorSpaceRegistry::instance() -> get(*it));
        KisLayerSP l = img -> activeLayer();

        // Rect fill
        KisFillPainter p(l.data());
        QTime t;
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
        }
        report = report.append(QString("    Erased 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(50, 50, 500, 500);
        }
        report = report.append(QString("    Erased 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(-50, -50, 1100, 1100);
        }
        report = report.append(QString("    Erased rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));
                                  
                       
        // Opaque Rect fill
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, Qt::red);
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, Qt::green);
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, Qt::lightGray);
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));
                                       
        // Transparent rect fill
        
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, Qt::red, OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, Qt::green, OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, Qt::blue, OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));
                    
        // Colour fill

        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(Qt::yellow);
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillColor(0,0);
        }
        report = report.append(QString("    Opaque floodfill of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));
                    

        // Pattern fill
        t.restart();
        for (Q_UINT32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(Qt::yellow);
            KisResourceServerBase* r = KisFactory::rServerRegistry() -> get("PatternServer");
            Q_CHECK_PTR(r);
            p.setPattern((KisPattern*)r -> resources().first());
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillPattern(0,0);
        }
        report = report.append(QString("    Opaque patternfill  of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));
        

        delete img;

    }



    return report;
    
}

QString PerfTest::gradientTest(Q_UINT32 testCount)
{
    return QString("Gradient test\n");
}

QString PerfTest::pixelTest(Q_UINT32 testCount)
{
    QString report = QString("* pixel/setpixel test\n");

    KisDoc * doc = m_view -> getDocument();
    KisIDList l = KisColorSpaceRegistry::instance() -> listKeys();


    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing pixel/setpixel on " + (*it).name() + "\n");

         KisImage * img = doc -> newImage("fill-" + (*it).name(), 1000, 1000, KisColorSpaceRegistry::instance() -> get(*it));
        
        KisLayerSP l = img -> activeLayer();

         QTime t;
         t.restart();

        QColor c = Qt::red;
        QUANTUM opacity = OPACITY_OPAQUE;
         for (Q_UINT32 i = 0; i < testCount; ++i) {
            for (Q_UINT32 x = 0; x < 1000; ++x) {
                for (Q_UINT32 y = 0; y < 1000; ++y) {
                    l -> pixel(x, y, &c, &opacity);
                }
            }
         }
        report = report.append(QString("    read 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));
        
        c= Qt::blue;
         t.restart();
         for (Q_UINT32 i = 0; i < testCount; ++i) {
            for (Q_UINT32 x = 0; x < 1000; ++x) {
                for (Q_UINT32 y = 0; y < 1000; ++y) {
                    l -> setPixel(x, y, c, 128);
                }
            }
         }
        report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        delete img;

    
        
    }

    


    return report;

}

QString PerfTest::shapeTest(Q_UINT32 testCount)
{
    return QString("Shape test\n");
}

QString PerfTest::layerTest(Q_UINT32 testCount)
{
    return QString("Layer test\n");
}

QString PerfTest::scaleTest(Q_UINT32 testCount)
{
    return QString("Scale test\n");
}

QString PerfTest::rotateTest(Q_UINT32 testCount)
{
    return QString("Rotate test\n");
}

QString PerfTest::renderTest(Q_UINT32 restCount)
{
    return QString("Render test\n");
}

QString PerfTest::selectionTest(Q_UINT32 testCount)
{
    return QString("Selection test\n");
}

QString PerfTest::colorConversionTest(Q_UINT32 testCount)
{
    return QString("Color conversion test");
}

QString PerfTest::filterTest(Q_UINT32 testCount)
{

    QString report = QString("* Filter test\n");

    KisIDList filters = KisFilterRegistry::instance()->listKeys();
    KisDoc * doc = m_view -> getDocument();
    KisIDList l = KisColorSpaceRegistry::instance()->listKeys();


    for (KisIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing filtering on " + (*it).name() + "\n");

         KisImage * img = doc -> newImage("filter-" + (*it).name(), 1000, 1000, KisColorSpaceRegistry::instance() -> get(*it));
        KisLayerSP l = img -> activeLayer();

        QTime t;

        for (KisIDList::Iterator it = filters.begin(); it != filters.end(); ++it) {
            KisFilterSP f = KisFilterRegistry::instance()->get(*it);
            t.restart();
            f -> enableProgress();
            f -> process(l.data(), l.data(), f -> configuration(f -> createConfigurationWidget(m_view, l.data()), l.data()), QRect(0, 0, 1000, 1000));
            f -> disableProgress();
            report = report.append(QString("    filtered " + (*it).name() + "1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        }
         
        delete img;

    }
    return report;
    
}

QString PerfTest::readBytesTest(Q_UINT32 testCount)
{
    QString report = QString("* Read bytes test\n\n");

    // On default tiles
    KisDoc * doc = m_view -> getDocument();
    KisImage * img = doc -> newImage("Readbytes ", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA","")));
    KisLayerSP l = img -> activeLayer();

    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        Q_UINT8 * newData = new Q_UINT8[1000 * 1000 * l -> pixelSize()];
        Q_CHECK_PTR(newData);
        l -> readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from empty image: %2\n").arg(testCount).arg(t.elapsed()));

    // On tiles with data

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();

    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        Q_UINT8 * newData = new Q_UINT8[1000 * 1000 * l -> pixelSize()];
        Q_CHECK_PTR(newData);
        l -> readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from filled image: %2\n").arg(testCount).arg(t.elapsed()));
    
    delete img;
    return report;
}


QString PerfTest::writeBytesTest(Q_UINT32 testCount)
{
    QString report = QString("* Write bytes test");
    
    // On default tiles
    KisDoc * doc = m_view -> getDocument();
    KisImage * img = doc -> newImage("Writebytes ", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();

    
    Q_UINT8 * data = new Q_UINT8[1000 * 1000 * l -> pixelSize()];
    Q_CHECK_PTR(data);
    l -> readBytes(data, 0, 0, 1000, 1000);
    
    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        l -> writeBytes(data, 0, 0, 1000, 1000);
    }
    delete[] data;
    delete img;
    report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    return report;


}

/////// Iterator tests


QString hlineRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();
;
    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();


    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l -> createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }
    
    delete img;
    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

    
}

QString hlineRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();
    
    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l -> createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }
    
    delete img;
    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWRDefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l -> createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    delete img;
    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWR(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();
    
    
    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
         int adv;
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l -> createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }
    
    delete img;
    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}


QString vlineRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l -> createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    delete img;
    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();
    

    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l -> createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }
    
    delete img;
    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineWRDefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {

        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l -> createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    delete img;
    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());
}

QString vlineWR(KisDoc * doc, Q_UINT32 testCount)
{

    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();

    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        for(Q_INT32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l -> createHLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }
    
    delete img;
    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectRODefault(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();
;
    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l -> createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }

    delete img;
    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

    
}

QString rectRO(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();
    
    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l -> createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }
    
    delete img;    
    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWRDefault(KisDoc * doc, Q_UINT32 testCount)
{


    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();

    int pixelSize = l -> pixelSize();
    
    QTime t;
    t.restart();

    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l -> createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }
    
    delete img;
    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWR(KisDoc * doc, Q_UINT32 testCount)
{
    KisImage * img = doc -> newImage("", 1000, 1000, KisColorSpaceRegistry::instance() -> get(KisID("RGBA", "")));
    KisLayerSP l = img -> activeLayer();
 ;
    int pixelSize = l -> pixelSize();
    
    KisFillPainter p(l.data());
    p.fillRect(0, 0, 1000, 1000, Qt::blue);
    p.end();
    

    QTime t;
    t.restart();


    for (Q_UINT32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l -> createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }

    
    delete img;
    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());


}
QString PerfTest::iteratorTest(Q_UINT32 testCount)
{
    QString report = "Iterator test";

    KisDoc * doc = m_view -> getDocument();
    
    report = report.append(hlineRODefault(doc, testCount));
    report = report.append(hlineRO(doc, testCount));
    report = report.append(hlineWRDefault(doc, testCount));
    report = report.append(hlineWR(doc, testCount));

    report = report.append(vlineRODefault(doc, testCount));
    report = report.append(vlineRO(doc, testCount));
    report = report.append(vlineWRDefault(doc, testCount));
    report = report.append(vlineWR(doc, testCount));

    report = report.append(rectRODefault(doc, testCount));
    report = report.append(rectRO(doc, testCount));
    report = report.append(rectWRDefault(doc, testCount));
    report = report.append(rectWR(doc, testCount));
    
    return report;

    
}

QString PerfTest::paintViewTest(Q_UINT32 testCount)
{
    QString report = QString("* paintView test\n\n");

    KisDoc * doc = m_view -> getDocument();
    KisImage * img = doc -> newImage("paintView ", 512, 512, KisColorSpaceRegistry::instance() -> get(KisID("RGBA","")));

    KisLayerSP l = img -> activeLayer();

    KisFillPainter p(l.data());
    p.fillRect(0, 0, 512, 512, Qt::blue);
    p.end();

    QTime t;
    t.restart();

#if USE_CALLGRIND
    CALLGRIND_ZERO_STATS();
#endif
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        m_view ->updateCanvas(QRect(0, 0, 512, 512));
    }
#if USE_CALLGRIND
    CALLGRIND_DUMP_STATS();
#endif

    report = report.append(QString("    painted a 512 x 512 image %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));

    img->layerAdd("layer 2", OPACITY_OPAQUE);
    l = img -> activeLayer();

    p.begin(l.data());
    p.fillRect(0, 0, 512, 512, Qt::blue);
    p.end();

    img->layerAdd("layer 3", OPACITY_OPAQUE);
    l = img -> activeLayer();

    p.begin(l.data());
    p.fillRect(0, 0, 512, 512, Qt::blue);
    p.end();

    t.restart();
    
    for (Q_UINT32 i = 0; i < testCount; ++i) {
        m_view ->updateCanvas(QRect(0, 0, 512, 512));
    }

    report = report.append(QString("    painted a 512 x 512 image with 3 layers %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));
    
    delete img;
    return report;
}

#include "perftest.moc"
