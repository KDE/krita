/*
 * perftest.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#include <koColor.h>

#include "kis_cursor.h"
#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators_quantum.h>
#include <kis_selection.h>
#include <kis_colorspace_registry.h>
#include <kis_strategy_colorspace.h>
#include <kis_painter.h>
#include <kis_fill_painter.h>

#include "perftest.h"

#include "dlg_perftest.h"

typedef KGenericFactory<PerfTest> PerfTestFactory;
K_EXPORT_COMPONENT_FACTORY( kritaperftest, PerfTestFactory( "krita" ) )

PerfTest::PerfTest(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(PerfTestFactory::instance());

 	kdDebug() << "PerfTest plugin. Class: " 
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

		KDialogBase * d = new KDialogBase(m_view, "", true, "", KDialogBase::Ok);
		d -> setCaption("Performance test results");
		QTextEdit * e = new QTextEdit(d);
		d -> setMainWidget(e);
		e -> setText(report);
		d -> exec();
		delete d;
		
	}
        delete dlgPerfTest;
}

QString PerfTest::bltTest(Q_UINT32 testCount)
{
	QString report = QString("* bitBlt test\n");

	KisDoc * doc = m_view -> getDocument();
	QStringList l = KisColorSpaceRegistry::singleton() -> listColorSpaceNames();


	for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		report = report.append( "  Testing blitting on " + *it + "\n");

 		KisImage * img = doc -> newImage("blt-" + *it, 1000, 1000, KisColorSpaceRegistry::singleton() -> get(*it));
		doc -> addImage(img);

		report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE, testCount, img));
		report = report.append(doBlit(COMPOSITE_OVER, *it, OPACITY_OPAQUE / 2, testCount, img));

		report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE, testCount, img));
		report = report.append(doBlit(COMPOSITE_COPY, *it, OPACITY_OPAQUE / 2, testCount, img));

	}

	return report;
	

}


QString PerfTest::doBlit(CompositeOp op, 
			 QString cspace,
			 QUANTUM opacity,
			 Q_UINT32 testCount,
			 KisImageSP img)
{
	
	QTime t;
	QString report;

	// ------------------------------------------------------------------------------
	// Small

	KisLayerSP small = new KisLayer(TILE_WIDTH / 2, TILE_HEIGHT /2, 
					KisColorSpaceRegistry::singleton() -> get(cspace),
					"small blit");


	KisFillPainter pf(small.data()) ;
	pf.fillRect(0, 0, TILE_WIDTH/2, TILE_HEIGHT/2, KoColor::black());
	pf.end();

	t.restart();
	KisPainter p(img -> activeLayer().data());
	for (Q_UINT32 i = 0; i < testCount; ++i) {
		p.bitBlt(0, 0, op, small.data());
	}
	p.end();
	
	report = report.append(QString("   %1 blits of rectangles < tilesize with opacity %2 and composite op %3: %4ms\n")
			       .arg(testCount)
			       .arg(opacity)
			       .arg(op)
			       .arg(t.elapsed()));


	// ------------------------------------------------------------------------------
	// Medium
	KisLayerSP medium = new KisLayer(TILE_WIDTH * 3, TILE_HEIGHT * 3,
					 KisColorSpaceRegistry::singleton() -> get(cspace),
					 "medium blit");
		
	pf.begin(medium.data()) ;
	pf.fillRect(0, 0, TILE_WIDTH * 3, TILE_HEIGHT * 3, KoColor::black());
	pf.end();

	t.restart();
	p.begin(img -> activeLayer().data());
	for (Q_UINT32 i = 0; i < testCount; ++i) {
		p.bitBlt(0, 0, op, medium.data());
	}
	p.end();

	report = report.append(QString("   %1 blits of rectangles 3 * tilesize with opacity %2 and composite op %3: %4ms\n")
			       .arg(testCount)
			       .arg(opacity)
			       .arg(op)
			       .arg(t.elapsed()));


	// ------------------------------------------------------------------------------
	// Big
	KisLayerSP big = new KisLayer(800, 800,
				      KisColorSpaceRegistry::singleton() -> get(cspace),
				      "big blit");

	pf.begin(big.data()) ;
	pf.fillRect(0, 0, 800, 800, KoColor::black());
	pf.end();

	t.restart();
	p.begin(img -> activeLayer().data());
	for (Q_UINT32 i = 0; i < testCount; ++i) {
		p.bitBlt(0, 0, op, big.data());

	}
	p.end();
	report = report.append(QString("   %1 blits of rectangles 800 x 800 with opacity %2 and composite op %3: %4ms\n")
			       .arg(testCount)
			       .arg(opacity)
			       .arg(op)
			       .arg(t.elapsed()));


	// ------------------------------------------------------------------------------
	// Outside

	KisLayerSP outside = new KisLayer(500, 500,
					  KisColorSpaceRegistry::singleton() -> get(cspace),
					  "outside blit");

	pf.begin(outside.data()) ;
	pf.fillRect(0, 0, 500, 500, KoColor::lightGray());
	pf.end();

	t.restart();
	p.begin(img -> activeLayer().data());
	for (Q_UINT32 i = 0; i < testCount; ++i) {
		p.bitBlt(600, 600, op, outside.data());

	}
	p.end();
	report = report.append(QString("   %1 blits of rectangles 500 x 500 at 600,600 with opacity %2 and composite op %3: %4ms\n")
			       .arg(testCount)
			       .arg(opacity)
			       .arg(op)
			       .arg(t.elapsed()));


	return report;

}

QString PerfTest::fillTest(Q_UINT32 testCount)
{
	QString report = QString("* Fill test\n");

	KisDoc * doc = m_view -> getDocument();
	QStringList l = KisColorSpaceRegistry::singleton() -> listColorSpaceNames();


	for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		report = report.append( "  Testing blitting on " + *it + "\n");

 		KisImage * img = doc -> newImage("fill-" + *it, 1000, 1000, KisColorSpaceRegistry::singleton() -> get(*it));
		doc -> addImage(img);
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
			p.fillRect(0, 0, 1000, 1000, KoColor::red());
		}
		report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i) {
			p.fillRect(50, 50, 500, 500, KoColor::green());
		}
		report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i, KoColor::blue()) {
			p.fillRect(-50, -50, 1100, 1100, KoColor::lightGray());
		}
		report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));
								       
		// Transparent rect fill
		
		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i) {
			p.fillRect(0, 0, 1000, 1000, KoColor::red(), OPACITY_OPAQUE / 2);
		}
		report = report.append(QString("    Opaque fill 1000 x 1000 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i) {
			p.fillRect(50, 50, 500, 500, KoColor::green(), OPACITY_OPAQUE / 2);
		}
		report = report.append(QString("    Opaque fill 500 x 500 layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));


		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i) {
			p.fillRect(-50, -50, 1100, 1100, KoColor::blue(), OPACITY_OPAQUE / 2);
		}
		report = report.append(QString("    Opaque fill rect bigger than layer %1 times: %2\n").arg(testCount).arg(t.elapsed()));
					
		// Colour fill

		t.restart();
		for (Q_UINT32 i = 0; i < testCount; ++i) {
			p.eraseRect(0, 0, 1000, 1000);
			p.setPaintColor(KoColor::yellow());
			p.setFillThreshold(15);
			p.setCompositeOp(COMPOSITE_OVER);
			p.fillColor(500, 500);
		}
		report = report.append(QString("    Opaque floodfill of whole layer (incl. erase) %1 times: %2\n").arg(testCount).arg(t.elapsed()));
					

		// Pattern fill
	}

	return report;
	

	return QString("Fill test\n");
}

QString PerfTest::gradientTest(Q_UINT32 testCount)
{
	return QString("Gradient test\n");
}

QString PerfTest::pixelTest(Q_UINT32 testCount)
{
	return QString("Pixel test\n");
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

#include "perftest.moc"
