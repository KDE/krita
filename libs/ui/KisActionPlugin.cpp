/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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
