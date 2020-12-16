/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCloneDocumentStroke.h"

#include "KisDocument.h"
#include "kis_layer_utils.h"

#include <QApplication>


struct KRITAIMAGE_NO_EXPORT KisCloneDocumentStroke::Private
{
    Private(KisDocument *_document)
        : document(_document)
    {
    }

    KisDocument *document = 0;
};

KisCloneDocumentStroke::KisCloneDocumentStroke(KisDocument *document)
    : KisSimpleStrokeStrategy(QLatin1String("clone-document-stroke"), kundo2_i18n("Clone Document")),
      m_d(new Private(document))
{
    setClearsRedoOnStart(false);
    setRequestsOtherStrokesToEnd(false);
    enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
}

KisCloneDocumentStroke::~KisCloneDocumentStroke()
{
}

void KisCloneDocumentStroke::initStrokeCallback()
{
    KisLayerUtils::forceAllDelayedNodesUpdate(m_d->document->image()->root());
}

void KisCloneDocumentStroke::finishStrokeCallback()
{
    KisDocument *doc = m_d->document->clone();
    doc->moveToThread(qApp->thread());
    emit sigDocumentCloned(doc);
}
