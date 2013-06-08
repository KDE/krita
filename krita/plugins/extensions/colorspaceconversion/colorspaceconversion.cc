/*
 * colorspaceconversion.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "colorspaceconversion.h"

#include <QApplication>
#include <QCursor>

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

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
#include <kis_colorspace_convert_visitor.h>

#include <kis_view2.h>
#include <kis_paint_device.h>
#include <kis_action.h>
#include <kis_group_layer.h>

#include "dlg_colorspaceconversion.h"

K_PLUGIN_FACTORY(ColorSpaceConversionFactory, registerPlugin<ColorSpaceConversion>();)
K_EXPORT_PLUGIN(ColorSpaceConversionFactory("krita"))


ColorSpaceConversion::ColorSpaceConversion(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/colorspaceconversion.rc")
{
    KisAction *action  = new KisAction(i18n("&Convert Image Color Space..."), this);
    addAction("imagecolorspaceconversion", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageColorSpaceConversion()));

    action  = new KisAction(i18n("&Convert Layer Color Space..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("layercolorspaceconversion", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotLayerColorSpaceConversion()));
}

ColorSpaceConversion::~ColorSpaceConversion()
{
}

void ColorSpaceConversion::slotImageColorSpaceConversion()
{
    KisImageWSP image = m_view->image();

    if (!image) return;


    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    bool allowLCMSOptimization = KisConfig().allowLCMSOptimization();
    dlgColorSpaceConversion->m_page->chkAllowLCMSOptimization->setChecked(allowLCMSOptimization);
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert All Layers From ") + image->colorSpace()->name());

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

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert Current Layer From") + layer->colorSpace()->name());

    if (dlgColorSpaceConversion->exec() == QDialog::Accepted) {
        const KoColorSpace * cs = dlgColorSpaceConversion->m_page->colorSpaceSelector->currentColorSpace();
        if (cs) {

            QApplication::setOverrideCursor(KisCursor::waitCursor());

            image->undoAdapter()->beginMacro(i18n("Convert Layer Type"));

            KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::HighQuality;
            if (dlgColorSpaceConversion->m_page->chkBlackpointCompensation->isChecked()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
            if (!dlgColorSpaceConversion->m_page->chkAllowLCMSOptimization->isChecked()) conversionFlags |= KoColorConversionTransformation::NoOptimization;
            KisColorSpaceConvertVisitor visitor(image, layer->colorSpace(), cs, (KoColorConversionTransformation::Intent)dlgColorSpaceConversion->m_intentButtonGroup.checkedId(), conversionFlags);
            layer->accept(visitor);

            image->undoAdapter()->endMacro();

            QApplication::restoreOverrideCursor();
            m_view->nodeManager()->nodesUpdated();
        }
    }
    delete dlgColorSpaceConversion;
}

#include "colorspaceconversion.moc"
