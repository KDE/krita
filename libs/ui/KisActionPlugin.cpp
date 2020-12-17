/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisActionPlugin.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "operations/kis_operation.h"

KisActionPlugin::KisActionPlugin(QObject* parent)
    : QObject(parent)
{
    m_viewManager = qobject_cast<KisViewManager*>(parent);
    KIS_ASSERT_RECOVER_NOOP(m_viewManager);
}

KisActionPlugin::~KisActionPlugin()
{
}

void KisActionPlugin::addAction(const QString& name, KisAction* action)
{
    if (m_viewManager) {
        m_viewManager->actionManager()->addAction(name, action);
    }
}

KisAction* KisActionPlugin::createAction(const QString& name)
{
    if (m_viewManager) {
        return m_viewManager->actionManager()->createAction(name);
    }
    return 0;
}

void KisActionPlugin::addUIFactory(KisOperationUIFactory* factory)
{
    if (m_viewManager) {
        m_viewManager->actionManager()->registerOperationUIFactory(factory);
    }
}

void KisActionPlugin::addOperation(KisOperation* operation)
{
    if (m_viewManager) {
        m_viewManager->actionManager()->registerOperation(operation);
    }
}

QPointer<KisViewManager> KisActionPlugin::viewManager() const
{
    return m_viewManager;
}
