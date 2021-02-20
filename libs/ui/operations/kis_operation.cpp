/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_operation.h"
#include "kis_processing_applicator.h"
#include "KisViewManager.h"
#include "kis_image.h"

KisOperation::KisOperation(const QString &id)
    : m_id(id)
{
}

KisOperation::~KisOperation()
{
}

QString KisOperation::id() const
{
    return m_id;
}

void KisOperation::runFromXML(KisViewManager *view, const KisOperationConfiguration &config)
{
    Q_UNUSED(view);
    Q_UNUSED(config);

    qFatal("Not implemented yet");
}

KisProcessingApplicator* KisOperation::beginAction(KisViewManager *view, const KUndo2MagicString &actionName) {
    KisImageSP image = view->image();
    Q_ASSERT(image);

    KisImageSignalVector emitSignals;

    return new KisProcessingApplicator(image, 0,
                                        KisProcessingApplicator::NONE,
                                        emitSignals, actionName);
}

void KisOperation::endAction(KisProcessingApplicator *applicator, const QString &xmlData) {
    Q_UNUSED(xmlData);
    applicator->end();
    delete applicator;
}
