/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
