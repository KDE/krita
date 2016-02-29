/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_guides_manager.h"
#include "kis_guides_decoration.h"
#include <KoGuidesData.h>
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signals_blocker.h"


struct KisGuidesManager::Private
{
    Private()
        : decoration(0) {}

    KisGuidesDecoration *decoration;
    KoGuidesData guidesData;
    QPointer<KisView> view;
};

KisGuidesManager::KisGuidesManager(QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
    /*
    m_d->guidesData.setShowGuides(true);
    m_d->guidesData.addGuideLine(Qt::Horizontal, 200);
    m_d->guidesData.addGuideLine(Qt::Horizontal, 300);
    m_d->guidesData.addGuideLine(Qt::Horizontal, 400);
    m_d->guidesData.addGuideLine(Qt::Vertical, 500);
    m_d->guidesData.addGuideLine(Qt::Vertical, 300);
    m_d->guidesData.addGuideLine(Qt::Vertical, 400);
    setGuidesDataImpl(m_d->guidesData);
    */
}

KisGuidesManager::~KisGuidesManager()
{
}

KisCanvasDecoration* KisGuidesManager::decoration() const
{
    return m_d->decoration;
}

void KisGuidesManager::setGuidesDataImpl(const KoGuidesData &value)
{
    if (!m_d->decoration || value == m_d->decoration->guidesData()) return;

    m_d->decoration->setVisible(value.showGuides());
    m_d->decoration->setGuidesData(value);
}

bool KisGuidesManager::showGuides() const
{
    return m_d->guidesData.showGuides();
}

void KisGuidesManager::setShowGuides(bool value)
{
    m_d->guidesData.setShowGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

bool KisGuidesManager::lockGuides() const
{
    return m_d->guidesData.lockGuides();
}

void KisGuidesManager::setLockGuides(bool value)
{
    m_d->guidesData.setLockGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

bool KisGuidesManager::snapToGuides() const
{
    return m_d->guidesData.snapToGuides();
}

void KisGuidesManager::setSnapToGuides(bool value)
{
    m_d->guidesData.setSnapToGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

void KisGuidesManager::setup(KisActionManager *actionManager)
{
    KisAction *action = 0;

    {
        action = actionManager->createAction("new_show_guides");
        connect(action, SIGNAL(triggered(bool)), this, SLOT(setShowGuides(bool)));
        KisSignalsBlocker l(action);
        action->setChecked(m_d->guidesData.showGuides());
    }

    {
        action = actionManager->createAction("new_lock_guides");
        connect(action, SIGNAL(triggered(bool)), this, SLOT(setLockGuides(bool)));
        KisSignalsBlocker l(action);
        action->setChecked(m_d->guidesData.lockGuides());
    }

    {
        action = actionManager->createAction("new_snap_to_guides");
        connect(action, SIGNAL(triggered(bool)), this, SLOT(setSnapToGuides(bool)));
        KisSignalsBlocker l(action);
        action->setChecked(m_d->guidesData.snapToGuides());
    }
}

void KisGuidesManager::setView(QPointer<KisView> view)
{
    if (m_d->view && m_d->view->canvasBase()) {
        m_d->decoration = 0;
    }

    m_d->view = view;

    if (m_d->view) {
        KisGuidesDecoration* decoration = qobject_cast<KisGuidesDecoration*>(m_d->view->canvasBase()->decoration(GUIDES_DECORATION_ID));
        if (!decoration) {
            decoration = new KisGuidesDecoration(m_d->view);
            m_d->view->canvasBase()->addDecoration(decoration);
        }
        m_d->decoration = decoration;
        setGuidesDataImpl(m_d->guidesData);
    }
}
