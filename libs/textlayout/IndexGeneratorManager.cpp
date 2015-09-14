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

#include "IndexGeneratorManager.h"

#include "KoTextDocumentLayout.h"
#include "ToCGenerator.h"

#include <KoTextDocument.h>
#include <KoParagraphStyle.h>
#include <KoTableOfContentsGeneratorInfo.h>

#include <QTextDocument>
#include <TextLayoutDebug.h>

IndexGeneratorManager::IndexGeneratorManager(QTextDocument *document)
    : QObject(document)
    , m_document(document)
    , m_state(FirstRunNeeded)
{
    m_documentLayout = static_cast<KoTextDocumentLayout *>(document->documentLayout());

    // connect to layoutIsDirty
    connect(m_documentLayout, SIGNAL(layoutIsDirty()), this, SLOT(requestGeneration()));

    // connect to FinishedLayout
    connect(m_documentLayout, SIGNAL(finishedLayout()), this, SLOT(startDoneTimer()));

    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_updateTimer.setInterval(5000); // after 5 seconds of pause we update
    m_updateTimer.setSingleShot(true);

    connect(&m_doneTimer, SIGNAL(timeout()), this, SLOT(layoutDone()));
    m_doneTimer.setInterval(1000); // after 1 seconds of silence we assume layout is done
    m_doneTimer.setSingleShot(true);
}

IndexGeneratorManager::~IndexGeneratorManager()
{
}

IndexGeneratorManager *IndexGeneratorManager::instance(QTextDocument *document)
{
    QVariant resource = document->resource(KoTextDocument::IndexGeneratorManager, KoTextDocument::IndexGeneratorManagerUrl);

    IndexGeneratorManager *igm = resource.value<IndexGeneratorManager *>();

    if (!igm) {
        igm = new IndexGeneratorManager(document);

        resource.setValue(igm);

        document->addResource(KoTextDocument::IndexGeneratorManager, KoTextDocument::IndexGeneratorManagerUrl, resource);
    }

    return igm;
}

void IndexGeneratorManager::requestGeneration()
{
    if (m_state == FirstRun || m_state == SecondRun) {
        return;
    }
    if (m_document->characterCount() < 2) {
        return;
    }
    m_updateTimer.stop();
    m_updateTimer.start();
}

void IndexGeneratorManager::startDoneTimer()
{
    //we delay acting on the finishedLayout signal by 1 second. This way we
    // don't act on it until every header has had a chance to be layouted
    // in words (we assume that a new finishedLayout signal will arrive within that
    // 1 second)
    m_doneTimer.start();
}

void IndexGeneratorManager::timeout()
{
    m_updateTimer.stop();
    m_state = FirstRunNeeded;
    m_documentLayout->scheduleLayout();
}

bool IndexGeneratorManager::generate()
{
    if (m_state == Resting || m_state == FirstRunLayouting || m_state == SecondRunLayouting) {
        return false;
    }

    if (m_state == FirstRun || m_state == SecondRun) {
        return true;
    }

    if (m_document->characterCount() < 2) {
        return false;
    }

    if (m_state == FirstRunNeeded) {
        m_state = FirstRun;
    }

    if (m_state == SecondRunNeeded) {
        m_state = SecondRun;
    }

    QTextBlock block = m_document->firstBlock();

    bool success = true;
    while (block.isValid()) {
        QTextBlockFormat format = block.blockFormat();

        if (format.hasProperty(KoParagraphStyle::TableOfContentsData)) {
            QVariant data = format.property(KoParagraphStyle::TableOfContentsData);
            KoTableOfContentsGeneratorInfo *tocInfo = data.value<KoTableOfContentsGeneratorInfo *>();

            data = format.property(KoParagraphStyle::GeneratedDocument);
            QTextDocument *tocDocument = data.value<QTextDocument *>();

            ToCGenerator *generator = m_generators[tocInfo];
            if (!generator) {
                generator = new ToCGenerator(tocDocument, tocInfo);
                m_generators[tocInfo] = generator;
            }

            generator->setBlock(block);
            success &= generator->generate();
        }
        block = block.next();
    }


    if (m_state == FirstRun) {
        m_state = FirstRunLayouting;
    }

    if (m_state == SecondRun) {
        if (success) {
            m_state = SecondRunLayouting;
        } else {
            m_state = FirstRunLayouting;
        }
    }

    return false;
}

void IndexGeneratorManager::layoutDone()
{
    switch (m_state) {
        case FirstRunLayouting:
            m_state = SecondRunNeeded;
            m_documentLayout->scheduleLayout();
            break;
        case SecondRunLayouting:
            m_state = Resting;
            break;
        default:
            break;
    }
}
