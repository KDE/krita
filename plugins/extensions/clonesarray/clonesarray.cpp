/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "clonesarray.h"

#include <cmath>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_action.h>

#include "dlg_clonesarray.h"

K_PLUGIN_FACTORY_WITH_JSON(ClonesArrayFactory, "kritaclonesarray.json", registerPlugin<ClonesArray>();)

ClonesArray::ClonesArray(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *action = createAction("clones_array");
    connect(action, SIGNAL(triggered()), this, SLOT(slotCreateClonesArray()));
}


ClonesArray::~ClonesArray()
{
}


void ClonesArray::slotCreateClonesArray()
{
    KisImageWSP image = viewManager()->image();
    Q_ASSERT(image); Q_UNUSED(image);

    DlgClonesArray *dialog = new DlgClonesArray(viewManager(), viewManager()->mainWindow());
    Q_CHECK_PTR(dialog);

    if (dialog->exec() == QDialog::Accepted) {
    }

    delete dialog;
}

#include "clonesarray.moc"
