/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_dlg_image_properties.h"

#include <QPushButton>
#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>

#include <klocale.h>
#include <kcolorcombo.h>

#include <KoUnitDoubleSpinBox.h>
#include <KoColorSpace.h>
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoID.h"
#include "kis_types.h"
#include "kis_image.h"

#include "kis_config.h"
#include "kis_factory2.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"

KisDlgImageProperties::KisDlgImageProperties(KisImageWSP image, QWidget *parent, const char *name)
        : KDialog(parent)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setCaption(i18n("Image Properties"));
    m_page = new WdgImageProperties(this);

    m_image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KisConfig cfg;

    m_page->lblWidthValue->setText(QString::number(image->width()));
    m_page->lblHeightValue->setText(QString::number(image->height()));

    m_page->lblResolutionValue->setText(KGlobal::locale()->formatNumber(image->xRes()*72, 2)); // XXX: separate values for x & y?

    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());

}

KisDlgImageProperties::~KisDlgImageProperties()
{
    delete m_page;
}

const KoColorSpace * KisDlgImageProperties::colorSpace()
{
    return m_page->colorSpaceSelector->currentColorSpace();
}

#include "kis_dlg_image_properties.moc"
