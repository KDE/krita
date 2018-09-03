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


#include "KisSelectionUpdateCompressor.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_layer_utils.h"
#include "kis_update_selection_job.h"


KisSelectionUpdateCompressor::KisSelectionUpdateCompressor(KisSelection *selection)
    : m_parentSelection(selection),
      m_updateSignalCompressor(new KisThreadSafeSignalCompressor(300, KisSignalCompressor::POSTPONE)),
      m_hasStalledUpdate(false)
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
