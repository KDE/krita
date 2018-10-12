/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_processing_visitor.h"

#include <KoUpdater.h>
#include <KoProgressUpdater.h>
#include "kis_node_progress_proxy.h"
#include "kis_node.h"
#include <KLocalizedString>

KisProcessingVisitor::ProgressHelper::ProgressHelper(const KisNode *node)
{
    KisNodeProgressProxy *progressProxy = node->nodeProgressProxy();

    if(progressProxy) {
        m_progressUpdater = new KoProgressUpdater(progressProxy);
        m_progressUpdater->setObjectName("ProgressHelper::m_progressUpdater");
        m_progressUpdater->start(100, i18n("Processing"));
        m_progressUpdater->moveToThread(node->thread());
    }
    else {
        m_progressUpdater = 0;
    }
}

KisProcessingVisitor::ProgressHelper::~ProgressHelper()
{
    if (m_progressUpdater) {
        m_progressUpdater->deleteLater();
    }
}

KoUpdater* KisProcessingVisitor::ProgressHelper::updater() const
{
    QMutexLocker l(&m_progressMutex);
    return m_progressUpdater ? m_progressUpdater->startSubtask() : 0;
}


KisProcessingVisitor::~KisProcessingVisitor()
{
}

KUndo2Command *KisProcessingVisitor::createInitCommand()
{
    return 0;
}
