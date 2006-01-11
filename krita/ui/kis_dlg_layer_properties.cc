/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <limits.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qslider.h>
#include <qstring.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include "kis_global.h"
#include "wdglayerproperties.h"
#include "kis_dlg_layer_properties.h"
#include "kis_cmb_composite.h"
#include "kis_colorspace.h"

KisDlgLayerProperties::KisDlgLayerProperties(const QString& deviceName,
                     Q_INT32 opacity,
                     const KisCompositeOp& compositeOp,
                     const KisColorSpace * colorSpace,
                     QWidget *parent, const char *name, WFlags f)
    : super(parent, name, f, name, Ok | Cancel)
{
    m_page = new WdgLayerProperties(this);
    m_page->layout()->setMargin(0);

    opacity = opacity * 100 / 255;

    if (opacity)
        opacity++;

    setCaption(i18n("Layer Properties"));
    setMainWidget(m_page);

    m_page->editName->setText(deviceName);
    connect( m_page->editName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );

    m_page->lblColorSpace->setText( colorSpace->id().name() );

    m_page->intOpacity -> setRange(0, 100, 13, true);
    m_page->intOpacity -> setValue(opacity);
    m_page->intOpacity -> setSuffix("%");

    m_page->cmbComposite -> setCompositeOpList(colorSpace -> userVisiblecompositeOps());
    m_page->cmbComposite -> setCurrentItem(compositeOp);

    slotNameChanged( m_page->editName->text() );
}

KisDlgLayerProperties::~KisDlgLayerProperties()
{
}

void KisDlgLayerProperties::slotNameChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

QString KisDlgLayerProperties::getName() const
{
    return m_page->editName -> text();
}

int KisDlgLayerProperties::getOpacity() const
{
    Q_INT32 opacity = m_page->intOpacity -> value();

    if (!opacity)
        return 0;

    opacity = opacity * 255 / 100;
    return opacity - 1;
}

KisCompositeOp KisDlgLayerProperties::getCompositeOp() const
{
    return m_page->cmbComposite -> currentItem();
}

#include "kis_dlg_layer_properties.moc"
