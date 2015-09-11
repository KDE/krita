/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (c) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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
#ifndef TEST_KO_TEXT_EDITOR_H
#define TEST_KO_TEXT_EDITOR_H

#include <QObject>

class KoTextEditor;
class KoSection;
class KoSectionEnd;
class TestDocument;

class TestKoTextEditor : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    //FIXME: see cpp file: why it is commented out
//     void testInsertInlineObject();
    void testRemoveSelectedText();

    // Section tests
    void testBasicSectionCreation();

    void testInsertSectionHandling_data();
    void testInsertSectionHandling();

    void testDeleteSectionHandling_data();
    void testDeleteSectionHandling();

private:
    // Sections stuff
    struct SectionHandle
    {
        explicit SectionHandle(KoSection *_sec)
            : sec(_sec)
            , parent(0)
        {
        }

        KoSection *sec;
        KoSection *parent;
        QList<SectionHandle *> children;
    };

    bool checkEndings(const QVector<QString> &needEndings, KoTextEditor *editor);
    bool checkStartings(const QVector<QString> &needStartings, KoTextEditor *editor);
    void checkSectionFormattingLevel(
        TestDocument *doc,
        int neededBlockCount,
        const QVector< QVector<QString> > &needStartings,
        const QVector< QVector<QString> > &needEndings);
    void checkSectionModelLevel(TestDocument *doc);
    void checkSectionModelLevelRecursive(QModelIndex index, SectionHandle *handle);

    void pushSectionStart(int num, KoSection *sec, KoTextEditor *editor);
    void pushSectionEnd(int num, KoSectionEnd *secEnd, KoTextEditor *editor);
    void formSectionTestDocument(TestDocument *doc);
    void checkSectionTestDocument(TestDocument *doc);

    /**
     * This one is used to generate unittest data.
     * Use it if you are sure that current implementation is right
     * or double check results.
     */
    void dumpSectionFormattingLevel(TestDocument *doc);
};

#endif // TEST_KO_TEXT_EDITOR_H
