/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2009-2010 C. Boemann <cbo@kogmbh.com>
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
#ifndef TESTDOCUMENTLAYOUT_H
#define TESTDOCUMENTLAYOUT_H

#include <QObject>
#include <QPainter>

class QTextDocument;
class KoTextDocumentLayout;
class KoStyleManager;

class TestDocumentLayout : public QObject
{
    Q_OBJECT
public:
    TestDocumentLayout() {}

private Q_SLOTS:
    void initTestCase();

    /**
     * Test the hittest of KoTextDocumentLayout.
     */
    void testHitTest();

    /**
     * Test root-area with zero width.
     */
    void testRootAreaZeroWidth();

    /**
     * Test root-area with zero height.
     */
    void testRootAreaZeroHeight();

    /**
     * Test root-area with zero width and height.
     */
    void testRootAreaZeroWidthAndHeight();

private:
    void setupTest(const QString &initText = QString());

private:
    QTextDocument *m_doc;
    KoStyleManager *m_styleManager;
    KoTextDocumentLayout *m_layout;
};

#endif
