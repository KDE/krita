/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include "dlg_buginfo.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <opengl/kis_opengl.h>
#include <KritaVersionWrapper.h>
#include <QSysInfo>

#include <QDesktopWidget>
#include <QClipboard>

#include "kis_document_aware_spin_box_unit_manager.h"

DlgBugInfo::DlgBugInfo(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Please paste this information in your bug report"));

    setButtons(User1 | Ok);
    setButtonText(User1, i18n("Copy to clipboard"));
    setDefaultButton(Ok);

    m_page = new WdgBugInfo(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    QString info;

    // Krita version info
    info.append("Krita");
    info.append("\n  Version: ").append(KritaVersionWrapper::versionString(true));
    info.append("\n\n");

    info.append("Qt");
    info.append("\n  Version (compiled): ").append(QT_VERSION_STR);
    info.append("\n  Version (loaded): ").append(qVersion());
    info.append("\n\n");

    // OS information
    info.append("OS Information");
    info.append("\n  Build ABI: ").append(QSysInfo::buildAbi());
    info.append("\n  Build CPU: ").append(QSysInfo::buildCpuArchitecture());
    info.append("\n  CPU: ").append(QSysInfo::currentCpuArchitecture());
    info.append("\n  Kernel Type: ").append(QSysInfo::kernelType());
    info.append("\n  Kernel Version: ").append(QSysInfo::kernelVersion());
    info.append("\n  Pretty Productname: ").append(QSysInfo::prettyProductName());
    info.append("\n  Product Type: ").append(QSysInfo::productType());
    info.append("\n  Product Version: ").append(QSysInfo::productVersion());
    info.append("\n");

    // OpenGL information
    info.append("\n").append(KisOpenGL::getDebugText());

    // Installation information

    // calculate a default height for the widget
    int wheight = m_page->sizeHint().height();
    m_page->txtBugInfo->setText(info);

    QFontMetrics fm = m_page->txtBugInfo->fontMetrics();
    int target_height = fm.height() * info.split('\n').size() + wheight;

    QDesktopWidget dw;
    QRect screen_rect = dw.availableGeometry(dw.primaryScreen());

    resize(m_page->size().width(), target_height > screen_rect.height() ? screen_rect.height() : target_height);

    connect(this, &KoDialog::user1Clicked, this, [this](){
        QGuiApplication::clipboard()->setText(m_page->txtBugInfo->toPlainText());
        m_page->txtBugInfo->selectAll(); // feedback
    });
}

DlgBugInfo::~DlgBugInfo()
{
    delete m_page;
}
