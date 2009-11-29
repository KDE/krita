/*
 *  dlg_backgrounds.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "dlg_backgrounds.h"

#include <klocale.h>
#include <kis_debug.h>

#include <kis_view2.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_background.h>

DlgBackgrounds::DlgBackgrounds(KisView2* view)
        : KDialog(view)
        , m_view(view)
{
    setCaption(i18n("Select a Background"));
    setButtons(Apply | Close);
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()),
            this, SLOT(applyClicked()));

    m_page = new WdgBackgrounds(this);
    setMainWidget(m_page);
}

DlgBackgrounds::~DlgBackgrounds()
{
}

KisBackgroundSP DlgBackgrounds::background()
{
    QListWidgetItem* item = m_page->lstBackgrounds->currentItem();
    QString file = item->data(Qt::UserRole + 1).value<QString>();
    if (file.isEmpty()) {
        return 0;
    } else {
        QImage image = item->data(Qt::DecorationRole).value<QImage>();
        return new KisBackground(image);
    }
}

void DlgBackgrounds::applyClicked()
{
    accept();
}

#include "dlg_backgrounds.moc"
