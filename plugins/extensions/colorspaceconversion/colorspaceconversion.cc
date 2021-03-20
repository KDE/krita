/*
 * colorspaceconversion.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "colorspaceconversion.h"

#include <QApplication>
#include <QCursor>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>

#include <kis_undo_adapter.h>
#include <kis_transaction.h>
#include <kis_annotation.h>
#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_node_manager.h>
#include <kis_layer.h>
#include <kis_types.h>

#include <KisViewManager.h>
#include <kis_paint_device.h>
#include <kis_action.h>
#include <kis_group_layer.h>

#include "dlg_colorspaceconversion.h"
#include "kis_action_manager.h"

K_PLUGIN_FACTORY_WITH_JSON(ColorSpaceConversionFactory, "kritacolorspaceconversion.json", registerPlugin<ColorSpaceConversion>();)


ColorSpaceConversion::ColorSpaceConversion(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *action  = viewManager()->actionManager()->createAction("imagecolorspaceconversion");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageColorSpaceConversion()));

    action  = viewManager()->actionManager()->createAction("layercolorspaceconversion");
    connect(action, SIGNAL(triggered()), this, SLOT(slotLayerColorSpaceConversion()));
}

ColorSpaceConversion::~ColorSpaceConversion()
{
}

void ColorSpaceConversion::slotImageColorSpaceConversion()
{
    KisImageSP image = viewManager()->image().toStrongRef();
    if (!image) return;

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(viewManager()->mainWindow(), "ColorSpaceConversion");
    bool allowLCMSOptimization = KisConfig(true).allowLCMSOptimization();
    dlgColorSpaceConversion->m_page->chkAllowLCMSOptimization->setChecked(allowLCMSOptimization);
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert All Layers From %1", image->colorSpace()->name()));
    dlgColorSpaceConversion->setInitialColorSpace(image->colorSpace(), image);

    if (dlgColorSpaceConversion->exec() == QDialog::Accepted) {

        const KoColorSpace * cs = dlgColorSpaceConversion->m_page->colorSpaceSelector->currentColorSpace();
        if (cs) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());
            KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::HighQuality;
            if (dlgColorSpaceConversion->m_page->chkBlackpointCompensation->isChecked()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
            if (!dlgColorSpaceConversion->m_page->chkAllowLCMSOptimization->isChecked()) conversionFlags |= KoColorConversionTransformation::NoOptimization;
            image->convertImageColorSpace(cs, (KoColorConversionTransformation::Intent)dlgColorSpaceConversion->m_intentButtonGroup.checkedId(), conversionFlags);
            QApplication::restoreOverrideCursor();
        }
    }
    delete dlgColorSpaceConversion;
}

void ColorSpaceConversion::slotLayerColorSpaceConversion()
{
    KisImageSP image = viewManager()->image().toStrongRef();
    if (!image) return;

    KisLayerSP layer = viewManager()->activeLayer();
    if (!layer) return;

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(viewManager()->mainWindow(), "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert Current Layer From %1", layer->colorSpace()->name()));
    dlgColorSpaceConversion->setInitialColorSpace(layer->colorSpace(), 0);

    if (dlgColorSpaceConversion->exec() == QDialog::Accepted) {
        const KoColorSpace * cs = dlgColorSpaceConversion->m_page->colorSpaceSelector->currentColorSpace();
        if (cs) {
            KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::HighQuality;
            if (dlgColorSpaceConversion->m_page->chkBlackpointCompensation->isChecked()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
            if (!dlgColorSpaceConversion->m_page->chkAllowLCMSOptimization->isChecked()) conversionFlags |= KoColorConversionTransformation::NoOptimization;
            image->convertLayerColorSpace(layer, cs, (KoColorConversionTransformation::Intent)dlgColorSpaceConversion->m_intentButtonGroup.checkedId(), conversionFlags);
        }
    }
    delete dlgColorSpaceConversion;
}

#include "colorspaceconversion.moc"
