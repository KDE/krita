/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Remot <boud@valdyas.org>
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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>

#include "kis_global.h"
#include "kis_cmb_composite.h"
#include "kis_cmb_idlist.h"
#include "kis_dlg_new_layer.h"
#include "kis_colorspace_registry.h"
#include "kis_abstract_colorspace.h"

NewLayerDialog::NewLayerDialog(const KisID colorSpaceID,
                   const QString & deviceName,
                   QWidget *parent,
                   const char *name)
    : super(parent, name, true, "", Ok | Cancel)
{
    QWidget *page = new QWidget(this);

    QGridLayout *grid;
    QLabel *lbl;

    setCaption(i18n("New Layer"));

    setMainWidget(page);
    grid = new QGridLayout(page, 8, 2, 0, 6);

    // Name
    lbl = new QLabel(i18n("Name:"), page);
    m_name = new KLineEdit(deviceName, page);
    grid -> addWidget(lbl, 0, 0);
    grid -> addWidget(m_name, 0, 1);

    // Opacity
    lbl = new QLabel(i18n("Opacity:"), page);
    m_opacity = new KIntNumInput(page);
    m_opacity -> setRange(0, 100, 13, true);
    m_opacity -> setValue(100);
    m_opacity -> setSuffix("%");
    grid -> addWidget(lbl, 1, 0);
    grid -> addWidget(m_opacity, 1, 1);

    // Composite mode
    lbl = new QLabel(i18n("Composite mode:"), page);
    m_cmbComposite = new KisCmbComposite(page);
    grid -> addWidget(lbl, 2, 0);
    grid -> addWidget(m_cmbComposite, 2, 1);

    // Layer type
    lbl = new QLabel(i18n("Layer type:"), page);
    m_cmbImageType = new KisCmbIDList(page);
    m_cmbImageType -> setIDList(KisColorSpaceRegistry::instance() -> listKeys());
    m_cmbImageType -> setCurrent(colorSpaceID);

    grid -> addWidget(lbl, 3, 0);
    grid -> addWidget(m_cmbImageType, 3, 1);

    slotSetColorSpace(colorSpaceID);

        connect( m_name, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );
    connect(m_cmbImageType, SIGNAL(activated(const KisID &)), this, SLOT(slotSetColorSpace(const KisID &)));
        slotNameChanged( m_name->text() );
}

void NewLayerDialog::slotNameChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

int NewLayerDialog::opacity() const
{
    Q_INT32 opacity = m_opacity -> value();

    if (!opacity)
        return 0;

    opacity = opacity * 255 / 100;
    return opacity - 1;
}

KisCompositeOp NewLayerDialog::compositeOp() const
{
    return m_cmbComposite -> currentItem();
}

KisID NewLayerDialog::colorSpaceID() const
{
    return m_cmbImageType -> currentItem();
}

QString NewLayerDialog::layerName() const
{
    return m_name -> text();
}

void NewLayerDialog::slotSetColorSpace(const KisID &colorSpaceId)
{
    KisAbstractColorSpace * cs = KisColorSpaceRegistry::instance() -> get(colorSpaceId);
    if (cs) {
        m_cmbComposite -> setCompositeOpList(cs -> userVisiblecompositeOps());
    }
}

#include "kis_dlg_new_layer.moc"

