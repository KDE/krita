/* This file is part of the KDE project
 * Copyright (C) 2013 Elvis Stansvik <elvstone@gmail.com>
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
#ifndef TESTSTYLEMANAGER_H
#define TESTSTYLEMANAGER_H

#include <QObject>

class KoStyleManager;
class KoTextDocument;

class QTextDocument;

class TestStyleManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void testAddRemoveCharacterStyle();
    void testAddRemoveParagraphStyle();
    void testAddRemoveListStyle();
    void testAddRemoveTableStyle();
    void testAddRemoveTableColumnStyle();
    void testAddRemoveTableRowStyle();
    void testAddRemoveTableCellStyle();
    void testAddRemoveSectionStyle();
    void testAddAppliedCharacterStyle();
    void testApplyAddedCharacterStyle();
    void testAddAppliedParagraphStyle();
    void testApplyAddedParagraphStyle();
    void cleanup();

private:
    KoStyleManager *m_styleManager;
    KoTextDocument *m_koDoc;
    QTextDocument *m_doc;
};

#endif // TESTSTYLEMANAGER_H
