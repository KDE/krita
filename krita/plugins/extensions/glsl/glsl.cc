/*
 * glsl.cc -- Part of Krita
 *
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "glsl.h"
#include <QApplication>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>

#include "KoColorSpaceRegistry.h"

#include "kis_view2.h"
#include "kis_image.h"


#include "dlg_glsl.h"

K_PLUGIN_FACTORY(GlslFactory, registerPlugin<Glsl>();)
K_EXPORT_PLUGIN(GlslFactory("krita"))

Glsl::Glsl(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{

    if (parent->inherits("KisView2")) {
        setComponentData(GlslFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/kritaglsl.rc"), true);

        m_view = static_cast<KisView2*>(parent);
        m_image = m_view->image();

        KAction *action  = new KAction(i18n("&OpenGL Shader Filter..."), this);
        actionCollection()->addAction("kritaglsl", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotActivate()));

    }
}

Glsl::~Glsl()
{
}

void Glsl::slotActivate()
{
    if (!m_image) return;

    DlgGlsl dlgGlsl(m_view, "Glsl");

    dlgGlsl.setCaption(i18n("OpenGL Shader Language Filter"));

    if (dlgGlsl.exec() == QDialog::Accepted) {
        // Execute the filter
    }
}

#include "glsl.moc"
