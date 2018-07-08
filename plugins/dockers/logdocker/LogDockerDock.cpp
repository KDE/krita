/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "LogDockerDock.h"

#include <QHBoxLayout>
#include <QToolButton>

#include <klocalizedstring.h>

#include <KoCanvasBase.h>

#include "kis_canvas2.h"
#include "KisViewManager.h"

#include <KoIcon.h>

LogDockerDock::LogDockerDock( )
    : QDockWidget(i18n("Log Viewer"))
{
    QWidget *page = new QWidget(this);
    setupUi(page);
    bnToggle->setIcon(koIcon("view-list-text"));
    bnClear->setIcon(koIcon("edit-clear"));
    bnSave->setIcon(koIcon("filesave"));
    bnSettings->setIcon(koIcon("settings"));
}

void LogDockerDock::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
    setEnabled(true);

}



