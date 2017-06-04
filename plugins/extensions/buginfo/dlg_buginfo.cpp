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
#include <QSysInfo>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QWindow>

#include "kis_document_aware_spin_box_unit_manager.h"

DlgBugInfo::DlgBugInfo(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Please paste this information in your bug report"));
    setButtons(Ok);
    setDefaultButton(Ok);

    m_page = new WdgBugInfo(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    // OS information
    QString info;
    info.append("OS Information");
    info.append("  Build ABI: ").append(QSysInfo::buildAbi());
    info.append("\n  Build CPU: ").append(QSysInfo::buildCpuArchitecture());
    info.append("\n  CPU: ").append(QSysInfo::currentCpuArchitecture());
    info.append("\n  Kernel Type: ").append(QSysInfo::kernelType());
    info.append("\n  Kernel Version: ").append(QSysInfo::kernelVersion());
    info.append("\n  Hostname: ").append(QSysInfo::machineHostName());
    info.append("\n  Pretty Productname: ").append(QSysInfo::prettyProductName());
    info.append("\n  Product Type: ").append(QSysInfo::productType());
    info.append("\n  Product Version: ").append(QSysInfo::productVersion());
    info.append("\n\n");

    // OpenGL information
    // we need a QSurface active to get our GL functions from the context
    QWindow  surface;
    surface.setSurfaceType( QSurface::OpenGLSurface );
    surface.create();

    QOpenGLContext context;
    context.create();
    if (!context.isValid()) return;

    context.makeCurrent( &surface );

    QOpenGLFunctions  *funcs = context.functions();
    funcs->initializeOpenGLFunctions();

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif
    QString Renderer = QString((const char*)funcs->glGetString(GL_RENDERER));
    info.append("\nOpenGL Info");
    info.append("\n  Vendor: ").append(reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR)));
    info.append("\n  Renderer: ").append(Renderer);
    info.append("\n  Version: ").append(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));
    info.append("\n  Shading language: ").append(reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION)));

    int glMajorVersion = context.format().majorVersion();
    int glMinorVersion = context.format().minorVersion();
    bool supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);

    info.append(QString("\n   Version: %1.%2").arg(glMajorVersion).arg(glMinorVersion));
    info.append(QString("\n     Supports deprecated functions: %1").arg(supportsDeprecatedFunctions ? "true" : "false"));



    // Installation information


    m_page->txtBugInfo->setText(info);

}

DlgBugInfo::~DlgBugInfo()
{
    delete m_page;
}
