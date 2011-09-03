/* This file is part of the KDE project
 * Copyright (C) 2011 Ko GmbH <cbo@kogmbh.com>
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
#ifndef INDEXGENERATORMANAGER_H
#define INDEXGENERATORMANAGER_H


#include <QObject>
#include <QMetaType>
#include <QHash>
#include <QTimer>

class QTextDocument;
class KoTextDocumentLayout;
class KoTableOfContentsGeneratorInfo;
class ToCGenerator;

class IndexGeneratorManager : public QObject
{
    Q_OBJECT
private:
    explicit IndexGeneratorManager(QTextDocument *document);
public:
    virtual ~IndexGeneratorManager();

    static IndexGeneratorManager *instance(QTextDocument *document);

    bool generate();

public slots:
    void requestGeneration();

private slots:
    void layoutDone();
    void timeout();

private:
    enum State {
        Resting,
        FirstRunNeeded,
        FirstRun,
        FirstRunLayouting,
        SecondRunNeeded,
        SecondRun,
        SecondRunLayouting
    };
    QTextDocument *m_document;
    KoTextDocumentLayout *m_documentLayout;
    QHash<KoTableOfContentsGeneratorInfo *, ToCGenerator *> m_generators;
    State m_state;
    QTimer m_updateTimer;
};

Q_DECLARE_METATYPE(IndexGeneratorManager *)

#endif
