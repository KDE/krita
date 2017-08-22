/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ThrottlePlugin.h"

#include <QApplication>
#include <QDesktopWidget>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kis_action.h>
#include "kis_config.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "Throttle.h"

K_PLUGIN_FACTORY_WITH_JSON(ThrottlePluginFactory, "krita_throttle.json", registerPlugin<ThrottlePlugin>();)


ThrottlePlugin::ThrottlePlugin(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
{
    KisAction *action = createAction("show_throttle");
    connect(action, SIGNAL(triggered()), this, SLOT(slotActivated()));

}

ThrottlePlugin::~ThrottlePlugin()
{
}

const int MARGIN = 70;

void ThrottlePlugin::slotActivated()
{
    if (!m_throttle) {
        m_throttle = new Throttle(m_view->mainWindow());
        m_throttle->setVisible(false);
        m_throttle->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
        m_throttle->setFocusPolicy(Qt::NoFocus);
        m_throttle->setAttribute(Qt::WA_ShowWithoutActivating);
        QRect screen = m_throttle->parentWidget()->geometry();
        screen.setTopLeft(m_throttle->parentWidget()->mapToGlobal(QPoint(MARGIN, MARGIN + 50)));
        m_throttle->setGeometry(QRect(screen.topLeft(), m_throttle->sizeHint()  ));
    }

    m_throttle->setVisible(!m_throttle->isVisible());

}

#include "ThrottlePlugin.moc"
