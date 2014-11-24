/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (c) 2012 C. Boemann <cbo@kogmbh.com>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestKoDocumentRdf.h"

#include <QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCharFormat>

#include <KoRdfSemanticItem.h>
#include <KoDocumentRdf.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextInlineRdf.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );

void TestKoDocumentRdf::testCreate()
{
    KoDocumentRdf *rdfDoc = new KoDocumentRdf();
    Q_ASSERT(rdfDoc->model());
    delete rdfDoc;
}

void TestKoDocumentRdf::testRememberNewInlineRdfObject()
{
    KoDocumentRdf rdfDoc;
    QTextDocument doc;

    QTextCursor cur(&doc);
    KoBookmark bm(cur);
    bm.setName("test");

    KoTextInlineRdf inlineRdf(&doc, &bm);
    inlineRdf.setXmlId(inlineRdf.createXmlId());

    rdfDoc.rememberNewInlineRdfObject(&inlineRdf);

    Q_ASSERT(&inlineRdf == rdfDoc.findInlineRdfByID(inlineRdf.xmlId()));
}

QTEST_MAIN(TestKoDocumentRdf)

#include "TestKoDocumentRdf.moc"
