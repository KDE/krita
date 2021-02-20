/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
