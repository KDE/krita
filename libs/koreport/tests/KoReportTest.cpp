/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2012 Dag Andersen <danders@get2net.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoReportTest.h"
#include <qtest_kde.h>

#include "KoReportPreRenderer.h"
#include "KoReportDesigner.h"
#include "krreportdata.h"
#include "reportpageoptions.h"

#include "Set.h"
#include "Property.h"

#include "KoUnit.h"
#include "KoDpi.h"

void KoReportTest::pageOptions()
{
    QString s;
    s += "<report:content xmlns:report=\"http://kexi-project.org/report/2.0\"";
    s += " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"";
    s += " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" >";
    s += "<report:title>Report</report:title>";
    s += "<report:grid report:grid-divisions=\"4\" report:grid-snap=\"1\" report:page-unit=\"cm\" report:grid-visible=\"1\" />";
    s += "<report:page-style report:print-orientation=\"portrait\"";
    s += " fo:margin-bottom=\"1.5cm\" fo:margin-top=\"2.0cm\"";
    s += " fo:margin-left=\"3.0cm\" fo:margin-right=\"4.0cm\"";
    s += " report:page-size=\"A5\" >predefined</report:page-style>";
    // needs detail section, or else designer crash
    s += "<report:body>";
    s += "<report:detail>";
    s += "<report:section svg:height=\"5.0cm\" fo:background-color=\"#ffffff\" report:section-type=\"detail\"/>";
    s += "</report:detail>";
    s += "</report:body>";
    
    QDomDocument doc;
    doc.setContent( s );
    KoReportDesigner designer(0, doc.documentElement());
    QCOMPARE(designer.propertySet()->property("page-size").value().toString(), QString("A5"));
    QCOMPARE(designer.propertySet()->property("margin-bottom").value().toDouble(), KoUnit::parseValue("1.5cm"));
    QCOMPARE(designer.propertySet()->property("margin-top").value().toDouble(), KoUnit::parseValue("2.0cm"));
    QCOMPARE(designer.propertySet()->property("margin-left").value().toDouble(), KoUnit::parseValue("3.0cm"));
    QCOMPARE(designer.propertySet()->property("margin-right").value().toDouble(), KoUnit::parseValue("4.0cm"));
    
    KoReportPreRenderer renderer( designer.document() );
    renderer.generate();
    ReportPageOptions opt = renderer.reportData()->pageOptions();

    QCOMPARE(opt.getPageSize(), QString("A5"));
    QCOMPARE(QString::number(INCH_TO_POINT(opt.getMarginBottom()) / KoDpi::dpiY()), QString::number(KoUnit::parseValue("1.5cm")));
    QCOMPARE(QString::number(INCH_TO_POINT(opt.getMarginTop()) / KoDpi::dpiY()), QString::number(KoUnit::parseValue("2.0cm")));
    QCOMPARE(QString::number(INCH_TO_POINT(opt.getMarginLeft()) / KoDpi::dpiY()), QString::number(KoUnit::parseValue("3.0cm")));
    QCOMPARE(QString::number(INCH_TO_POINT(opt.getMarginRight()) / KoDpi::dpiY()), QString::number(KoUnit::parseValue("4.0cm")));
}

QTEST_KDEMAIN(KoReportTest, GUI);

#include "moc_KoReportTest.cpp"
