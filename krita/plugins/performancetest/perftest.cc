/*
 * perftest.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler
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
	dlgPerfTest -> setCaption(i18n("Performance test"));
	
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
	return QString("bitBlt test\n");

	m_view -> a

}

QString PerfTest::fillTest(Q_UINT32 testCount)
{
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


#include "perftest.moc"
