/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ExtensionsManagerWindow.h"

#include "ui_ExtensionsManagerWidget.h"
#include <kfiledialog.h>
#include "ExtensionsManager.h"
#include <kmessagebox.h>

ExtensionsManagerWindow::ExtensionsManagerWindow() : m_emWidget(new Ui_ExtensionsManagerWidget)
{
    m_emWidget->setupUi(this);
    connect(m_emWidget->pushButtonInstall, SIGNAL(released()), SLOT(installFromFile()));
    connect(m_emWidget->pushButtonClose, SIGNAL(released()), SLOT(close()));
}

ExtensionsManagerWindow::~ExtensionsManagerWindow()
{
    delete m_emWidget;
}

void ExtensionsManagerWindow::installFromFile()
{
    KUrl url = KFileDialog::getOpenFileName(KUrl(), "*.koffice-extension");
    if (!url.isEmpty()) {
        if (ExtensionsManager::instance()->installExtension(url)) {
            KMessageBox::information(this, i18n("The installation was successful, you will need to restart Krita to use the extensions"), i18n("Success"));
        }
    }
}
