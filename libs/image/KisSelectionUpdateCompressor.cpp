/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisSelectionUpdateCompressor.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_layer_utils.h"
#include "kis_update_selection_job.h"


KisSelectionUpdateCompressor::KisSelectionUpdateCompressor(KisSelection *selection)
    : m_parentSelection(selection)
    , m_updateSignalCompressor(new KisThreadSafeSignalCompressor(100, KisSignalCompressor::POSTPONE))
{
    connect(m_updateSignalCompressor, SIGNAL(timeout()), this, SLOT(startUpdateJob()));

    this->moveToThread(m_updateSignalCompressor->thread());
}

KisSelectionUpdateCompressor::~KisSelectionUpdateCompressor()
{
    m_updateSignalCompressor->deleteLater();
}

void KisSelectionUpdateCompressor::requestUpdate(const QRect &updateRect)
{
    m_fullUpdateRequested |= updateRect.isEmpty();
    m_updateRect = !m_fullUpdateRequested ? m_updateRect | updateRect : QRect();
    m_updateSignalCompressor->start();
}

void KisSelectionUpdateCompressor::tryProcessStalledUpdate()
{
    if (m_hasStalledUpdate) {
        m_updateSignalCompressor->start();
    }
}

void KisSelectionUpdateCompressor::startUpdateJob()
{
    KisNodeSP parentNode = m_parentSelection->parentNode();
    if (!parentNode) {
        m_hasStalledUpdate = true;
        return;
    }

    // FIXME: we cannot use parentNode->image() here because masks don't
    //        have the pointer initialized for some reason.
    KisImageSP image = KisLayerUtils::findImageByHierarchy(parentNode);
    if (!image) {
        m_hasStalledUpdate = true;
        return;
    }

    if (image) {
        image->addSpontaneousJob(new KisUpdateSelectionJob(m_parentSelection, m_updateRect));
    }
    m_updateRect = QRect();
    m_fullUpdateRequested = false;
    m_hasStalledUpdate = false;
}
