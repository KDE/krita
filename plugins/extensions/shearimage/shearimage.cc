/*
 * shearimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "shearimage.h"

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>

#include "dlg_shearimage.h"

K_PLUGIN_FACTORY_WITH_JSON(ShearImageFactory, "kritashearimage.json", registerPlugin<ShearImage>();)

ShearImage::ShearImage(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action = createAction("shearimage");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearImage()));

    action = createAction("shearlayer");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearLayer()));
}

ShearImage::~ShearImage()
{
}

void ShearImage::slotShearImage()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(viewManager()->mainWindow(), "ShearImage");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Image"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();
        viewManager()->imageManager()->shearCurrentImage(angleX, angleY);
    }
    delete dlgShearImage;
}

void ShearImage::slotShearLayer()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(viewManager()->mainWindow(), "ShearLayer");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Layer"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();
        viewManager()->nodeManager()->shear(angleX, angleY);

    }
    delete dlgShearImage;
}

#include "shearimage.moc"
