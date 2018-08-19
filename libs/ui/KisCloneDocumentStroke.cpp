/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
    : KisSimpleStrokeStrategy("clone-document-stroke", kundo2_i18n("Clone Document")),
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
