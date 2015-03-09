/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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
    emitSignals << ModifiedSignal;

    return new KisProcessingApplicator(image, 0,
                                        KisProcessingApplicator::NONE,
                                        emitSignals, actionName);
}

void KisOperation::endAction(KisProcessingApplicator *applicator, const QString &xmlData) {
    Q_UNUSED(xmlData);
    applicator->end();
    delete applicator;
}
