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

#include <QSlider>
#include <QPoint>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QTextEdit>
#include <QDateTime>
#include <QColor>

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

#include "kis_meta_registry.h"
#include "kis_resourceserver.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_config.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view.h"
#include "kis_selection.h"
#include "KoColorSpaceFactoryRegistry.h"
#include "KoColorSpace.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "KoID.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "perftest.h"
#include "kis_filter_config_widget.h"
#include "kis_factory.h"

#include "dlg_perftest.h"

#define USE_CALLGRIND 0

#if USE_CALLGRIND
#include <valgrind/callgrind.h>
#endif


typedef KGenericFactory<PerfTest> PerfTestFactory;
K_EXPORT_COMPONENT_FACTORY( kritaperftest, PerfTestFactory( "krita" ) )

PerfTest::PerfTest(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(PerfTestFactory::instance());
        setXMLFile(locate("data","kritaplugins/perftest.rc"), true);

        KAction *action = new KAction(i18n("&Performance Test..."), actionCollection(), "perf_test");
        connect(action, SIGNAL(triggered()), this, SLOT(slotPerfTest()));
        m_view = (KisView*) parent;
    }
}

PerfTest::~PerfTest()
{
    m_view = 0;
}

void PerfTest::slotPerfTest()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgPerfTest * dlgPerfTest = new DlgPerfTest(m_view, "PerfTest");
    Q_CHECK_PTR(dlgPerfTest);

    dlgPerfTest->setCaption(i18n("Performance Test"));

    QString report = QString("");

    if (dlgPerfTest->exec() == QDialog::Accepted) {

        qint32 testCount = (qint32)qRound(dlgPerfTest->page()->intTestCount->value());

        if (dlgPerfTest->page()->chkBitBlt->isChecked()) {
            kDebug() << "bltTest:\n";
            QString s = bltTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkFill->isChecked()) {
            kDebug() << "Filltest\n";
            QString s= fillTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkGradient->isChecked()) {
            kDebug() << "Gradienttest\n";
            QString s = gradientTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPixel->isChecked()) {
            kDebug() << "Pixeltest\n";
            QString s = pixelTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkShape->isChecked()) {
            kDebug() << "Shapetest\n";
            QString s = shapeTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkLayer->isChecked()) {
            kDebug() << "LayerTest\n";
            QString s = layerTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkScale->isChecked()) {
            kDebug() << "Scaletest\n";
            QString s = scaleTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkRotate->isChecked()) {
            kDebug() << "Rotatetest\n";
            QString s = rotateTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkRender->isChecked()) {
            kDebug() << "Rendertest\n";
            QString s = renderTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkSelection->isChecked()) {
            kDebug() << "Selectiontest\n";
            QString s = selectionTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkColorConversion->isChecked()) {
            kDebug() << "Colorconversiontest\n";
            QString s = colorConversionTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkFilter-> isChecked()) {
            kDebug() << "filtertest\n";
            QString s = filterTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkReadBytes->isChecked()) {
            kDebug() << "Readbytes test\n";
            QString s = readBytesTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkWriteBytes-> isChecked()) {
            kDebug() << "Writebytes test\n";
            QString s = writeBytesTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkIterators->isChecked()) {
            kDebug() << "Iterators test\n";
            QString s = iteratorTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPaintView->isChecked()) {
            kDebug() << "paintview test\n";
            QString s = paintViewTest(testCount);
            report = report.append(s);
            kDebug() << s << "\n";
        }
        if (dlgPerfTest->page()->chkPaintViewFPS->isChecked()) {
            kDebug() << "paint current view (fps) test\n";
            QString s = paintViewFPSTest();
            report = report.append(s);
            kDebug() << s << "\n";
        }
        KDialog *d = new KDialog(m_view, i18n("Performance test results"), KDialogBase::Ok);
        Q_CHECK_PTR(d);

        QTextEdit * e = new QTextEdit(d);
        Q_CHECK_PTR(e);
        d->setMainWidget(e);
        e->setPlainText(report);
        e->setMinimumWidth(600);
        e->setMinimumHeight(600);
        d->exec();
        delete d;

    }
    delete dlgPerfTest;
}

QString PerfTest::bltTest(quint32 testCount)
{
    QString report = QString("* bitBlt test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        kDebug() << "Image->" << (*it).name() << "\n";

        report = report.append( "  Testing blitting on " + (*it).name() + '\n');

         KisImageSP img = doc->newImage("blt-" + (*it).name(), 1000, 1000,
                KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE, testCount, img));
        report = report.append( "\n");
        report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE / 2, testCount, img));
        report = report.append( "\n");
    }

    return report;


}


QString PerfTest::doBlit(const KoCompositeOp& op,
                         KoID cspace,
                         quint8 opacity,
                         quint32 testCount,
                         KisImageSP img)
{

    QTime t;
    QString report;

    // ------------------------------------------------------------------------------
    // Small

    KisPaintDeviceSP small = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "small blit"));
    Q_CHECK_PTR(small);

    KisFillPainter pf(small) ;
    pf.fillRect(0, 0, 32, 32, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    KisPainter p(img->activeDevice());
    for (quint32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small, 0, 0, 32, 32);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles < tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Medium
    KisPaintDeviceSP medium = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "medium blit"));
    Q_CHECK_PTR(medium);

    pf.begin(medium);
    pf.fillRect(0, 0, 64 * 3, 64 * 3, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice());
    for (quint32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, medium, 0, 0, 96, 96);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles 3 * tilesize with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Big
    KisPaintDeviceSP big = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "big blit"));
    Q_CHECK_PTR(big);

    pf.begin(big) ;
    pf.fillRect(0, 0, 800, 800, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice());
    for (quint32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, big, 0, 0, 800, 800);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 800 x 800 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));


    // ------------------------------------------------------------------------------
    // Outside

    KisPaintDeviceSP outside = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "outside blit"));
    Q_CHECK_PTR(outside);
    pf.begin(outside) ;
    pf.fillRect(0, 0, 500, 500, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    pf.end();

    t.restart();
    p.begin(img->activeDevice());
    for (quint32 i = 0; i < testCount; ++i) {
        p.bitBlt(600, 600, op, outside, 0, 0, 500, 500);

    }
    p.end();
    report = report.append(QString("   %1 blits of rectangles 500 x 500 at 600,600 with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    // ------------------------------------------------------------------------------
    // Small with varied source opacity

    KisPaintDeviceSP small_with_alpha = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace,""), "small blit with alpha"));
    Q_CHECK_PTR(small_with_alpha);

    pf.begin(small_with_alpha) ;
    pf.fillRect(0, 0, 32, 32, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_TRANSPARENT);
    pf.fillRect(4, 4, 24, 24, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
    pf.fillRect(8, 8, 16, 16, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE);
    pf.end();

    t.restart();
    p.begin(img->activeDevice());
    for (quint32 i = 0; i < testCount; ++i) {
        p.bitBlt(0, 0, op, small_with_alpha, 0, 0, 32, 32);
    }
    p.end();

    report = report.append(QString("   %1 blits of rectangles < tilesize with source alpha, with opacity %2 and composite op %3: %4ms\n")
                   .arg(testCount)
                   .arg(opacity)
                   .arg(op.id().name())
                   .arg(t.elapsed()));

    return report;

}

QString PerfTest::fillTest(quint32 testCount)
{
    QString report = QString("* Fill test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        kDebug() << "Filltest on " << (*it).name() + '\n';

        report = report.append( "  Testing blitting on " + (*it).name() + '\n');

        KisImageSP img = doc->newImage("fill-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
        KisPaintDeviceSP l = img->activeDevice();

        // Rect fill
        KisFillPainter p(l);
        QTime t;
        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
        }
        report = report.append(QString("    Erased 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.eraseRect(50, 50, 500, 500);
        }
        report = report.append(QString("    Erased 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.eraseRect(-50, -50, 1100, 1100);
        }
        report = report.append(QString("    Erased rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        // Opaque Rect fill
        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        // Transparent rect fill

        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(50, 50, 500, 500, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.fillRect(-50, -50, 1100, 1100, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()), OPACITY_OPAQUE / 2);
        }
        report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        // Colour fill

        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillColor(0,0);
        }
        report = report.append(QString("    Opaque floodfill of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));


        // Pattern fill
        t.restart();
        for (quint32 i = 0; i < testCount; ++i) {
            p.eraseRect(0, 0, 1000, 1000);
//             p.paintEllipse(500, 1000, 100, 0, 0);
            p.setPaintColor(KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
            KisResourceServerBase* r = KisResourceServerRegistry::instance()->get("PatternServer");
            Q_CHECK_PTR(r);
            p.setPattern((KisPattern*)r->resources().first());
            p.setFillThreshold(15);
            p.setCompositeOp(COMPOSITE_OVER);
            p.fillPattern(0,0);
        }

        report = report.append(QString("    Opaque patternfill  of whole circle (incl. erase and painting of circle) %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    }

    return report;
}

QString PerfTest::gradientTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Gradient test\n");
}

QString PerfTest::pixelTest(quint32 testCount)
{
    QString report = QString("* pixel/setpixel test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();


    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing pixel/setpixel on " + (*it).name() + '\n');

         KisImageSP img = doc->newImage("fill-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        KisPaintDeviceSP l = img->activeDevice();

         QTime t;
         t.restart();

        QColor c = Qt::black;
        quint8 opacity = OPACITY_OPAQUE;
         for (quint32 i = 0; i < testCount; ++i) {
            for (quint32 x = 0; x < 1000; ++x) {
                for (quint32 y = 0; y < 1000; ++y) {
                    l->pixel(x, y, &c, &opacity);
                }
            }
         }
        report = report.append(QString("    read 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        c= Qt::black;
         t.restart();
         for (quint32 i = 0; i < testCount; ++i) {
            for (quint32 x = 0; x < 1000; ++x) {
                for (quint32 y = 0; y < 1000; ++y) {
                    l->setPixel(x, y, c, 128);
                }
            }
         }
        report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

    }

    return report;
}

QString PerfTest::shapeTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Shape test\n");
}

QString PerfTest::layerTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Layer test\n");
}

QString PerfTest::scaleTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Scale test\n");
}

QString PerfTest::rotateTest(quint32 testCount)
{
    QString report = QString("* Rotate test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();
    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        doc->undoAdapter()->setUndo( false );
        QTime t;

        for (uint i = 0; i < testCount; ++i) {
            for (double angle = 0; angle < 360; ++angle) {
                kDebug() << "Rotating " << (*it).name() << " at " << angle << " degrees\n";
                KisImageSP img = doc->newImage("cs-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
                img->rotate(angle, m_view->canvasSubject()->progressDisplay());
                kDebug() << "Size: " << img->projection()->extent() << endl;
            }
        }
        report = report.append(QString("    rotated  1000 x 1000 pixels over 360 degrees, degree by degree, %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    }
    return report;
}

QString PerfTest::renderTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Render test\n");
}

QString PerfTest::selectionTest(quint32 testCount)
{
    Q_UNUSED(testCount);
    return QString("Selection test\n");
}

QString PerfTest::colorConversionTest(quint32 testCount)
{
    QString report = QString("* Colorspace conversion test\n");

    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();
    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {

        KisImageSP img = doc->newImage("cs-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));

        QTime t;

        KoIDList l2 = KisMetaRegistry::instance()->csRegistry()->listKeys();
        for (KoIDList::Iterator it2 = l2.begin(); it2 != l2.end(); ++it2) {
            kDebug() << "test conversion from " << (*it).name() << " to " << (*it2).name() << endl;

            t.restart();
            for (uint i = 0; i < testCount; ++i) {
                KisImage * img2 = new KisImage(*img);
                img2->convertTo(KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it2,""));
                delete img2;
            }
            report = report.append(QString("    converted from " + (*it).name() + " to " + (*it2).name() + " 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        }
    }
    return report;

}

QString PerfTest::filterTest(quint32 testCount)
{

    QString report = QString("* Filter test\n");

    KoIDList filters = KisFilterRegistry::instance()->listKeys();
    KisDoc * doc = m_view->canvasSubject()->document();
    KoIDList l = KisMetaRegistry::instance()->csRegistry()->listKeys();

    for (KoIDList::Iterator it = l.begin(); it != l.end(); ++it) {
        report = report.append( "  Testing filtering on " + (*it).name() + '\n');

        KisImageSP img = doc->newImage("filter-" + (*it).name(), 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(*it,""));
        KisPaintDeviceSP l = img->activeDevice();

        QTime t;

        for (KoIDList::Iterator it = filters.begin(); it != filters.end(); ++it) {

            KisFilterSP f = KisFilterRegistry::instance()->get(*it);
            t.restart();
            kDebug() << "test filter " << f->id().name() << " on " << img->colorSpace()->id().name() << endl;
            for (quint32 i = 0; i < testCount; ++i) {
                f->enableProgress();
                f->process(l, l, f->configuration(f->createConfigurationWidget(m_view, l)), QRect(0, 0, 1000, 1000));
                f->disableProgress();
            }
            report = report.append(QString("    filtered " + (*it).name() + "1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));

        }

    }
    return report;

}

QString PerfTest::readBytesTest(quint32 testCount)
{
    QString report = QString("* Read bytes test\n\n");

    // On default tiles
    KisDoc * doc = m_view->canvasSubject()->document();
    KisImageSP img = doc->newImage("Readbytes ", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA",""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        quint8 * newData = new quint8[1000 * 1000 * l->pixelSize()];
        Q_CHECK_PTR(newData);
        l->readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from empty image: %2\n").arg(testCount).arg(t.elapsed()));

    // On tiles with data

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        quint8 * newData = new quint8[1000 * 1000 * l->pixelSize()];
        Q_CHECK_PTR(newData);
        l->readBytes(newData, 0, 0, 1000, 1000);
        delete[] newData;
    }

    report = report.append(QString("    read 1000 x 1000 pixels %1 times from filled image: %2\n").arg(testCount).arg(t.elapsed()));

    return report;
}


QString PerfTest::writeBytesTest(quint32 testCount)
{
    QString report = QString("* Write bytes test");

    // On default tiles
    KisDoc * doc = m_view->canvasSubject()->document();
    KisImageSP img = doc->newImage("Writebytes ", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();
    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    quint8 * data = new quint8[1000 * 1000 * l->pixelSize()];
    Q_CHECK_PTR(data);
    l->readBytes(data, 0, 0, 1000, 1000);

    QTime t;
    t.restart();
    for (quint32 i = 0; i < testCount; ++i) {
        l->writeBytes(data, 0, 0, 1000, 1000);
    }
    delete[] data;
    report = report.append(QString("    written 1000 x 1000 pixels %1 times: %2\n").arg(testCount).arg(t.elapsed()));
    return report;


}

/////// Iterator tests


QString hlineRODefault(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();


    for (quint32 i = 0; i < testCount; ++i) {
         int adv;

        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());


}

QString hlineRO(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
         int adv;

        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, false);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWRDefault(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
         int adv;

        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString hlineWR(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
         int adv;
        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(0, y2, 1000, true);
            while(! hiter.isDone())
            {
                 adv = hiter.nConseqHPixels();
                 hiter += adv;
            }
        }

    }

    return QString("    hline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}


QString vlineRODefault(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineRO(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString vlineWRDefault(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {

        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisVLineIterator hiter = l->createVLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());
}

QString vlineWR(KisDoc * doc, quint32 testCount)
{

    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        for(qint32 y2 = 0; y2 < 0 + 1000; y2++)
        {
            KisHLineIterator hiter = l->createHLineIterator(y2, 0, 1000, true);
            while(! hiter.isDone())
            {
                 ++hiter;
            }
        }

    }

    return QString("    vline iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectRODefault(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();
;
    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());


}

QString rectRO(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, false);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated read-only 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWRDefault(KisDoc * doc, quint32 testCount)
{


    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    QTime t;
    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }

    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over default tile: %2\n").arg(testCount).arg(t.elapsed());

}

QString rectWR(KisDoc * doc, quint32 testCount)
{
    KisImageSP img = doc->newImage("", 1000, 1000, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("RGBA", ""),""));
    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 1000, 1000, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();


    QTime t;
    t.restart();


    for (quint32 i = 0; i < testCount; ++i) {
        KisRectIterator r = l->createRectIterator(0, 0, 1000, 1000, true);
        while(! r.isDone())
        {
            ++r;
        }
    }


    return QString("    rect iterated writable 1000 x 1000 pixels %1 times over existing tile: %2\n").arg(testCount).arg(t.elapsed());


}
QString PerfTest::iteratorTest(quint32 testCount)
{
    QString report = "Iterator test";

    KisDoc * doc = m_view->canvasSubject()->document();

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

QString PerfTest::paintViewTest(quint32 testCount)
{
    QString report = QString("* paintView test\n\n");

    KisDoc * doc = m_view->canvasSubject()->document();

    KisImageSP img = doc->currentImage();
    img->resize(512,512);


    KisPaintDeviceSP l = img->activeDevice();

    KisFillPainter p(l);
    p.fillRect(0, 0, 512, 512, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    QTime t;
    t.restart();

#if USE_CALLGRIND
    CALLGRIND_ZERO_STATS();
#endif

    for (quint32 i = 0; i < testCount; ++i) {
        m_view->getCanvasController()->updateCanvas(QRect(0, 0, 512, 512));
    }

#if USE_CALLGRIND
    CALLGRIND_DUMP_STATS();
#endif

    report = report.append(QString("    painted a 512 x 512 image %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));

    img->newLayer("layer 2", OPACITY_OPAQUE);
    l = img->activeDevice();

    p.begin(l);
    p.fillRect(0, 0, 512, 512, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    img->newLayer("layer 3", OPACITY_OPAQUE);
    l = img->activeDevice();

    p.begin(l);
    p.fillRect(0, 0, 512, 512, KoColor(Qt::black, KisMetaRegistry::instance()->csRegistry()->getRGB8()));
    p.end();

    t.restart();

    for (quint32 i = 0; i < testCount; ++i) {
        m_view->getCanvasController()->updateCanvas(QRect(0, 0, 512, 512));
    }

    report = report.append(QString("    painted a 512 x 512 image with 3 layers %1 times: %2 ms\n").arg(testCount).arg(t.elapsed()));

    return report;
}

QString PerfTest::paintViewFPSTest()
{
    QString report = QString("* paintView (fps) test\n\n");

    QTime t;
    t.restart();

#if USE_CALLGRIND
    CALLGRIND_ZERO_STATS();
#endif

    int numViewsPainted = 0;
    const int millisecondsPerSecond = 1000;

    while (t.elapsed() < millisecondsPerSecond) {
        m_view->getCanvasController()->updateCanvas();
        numViewsPainted++;
    }

#if USE_CALLGRIND
    CALLGRIND_DUMP_STATS();
#endif

    report = report.append(QString("    painted current view at %1 frames per second\n").arg(numViewsPainted));

    return report;
}

#include "perftest.moc"
