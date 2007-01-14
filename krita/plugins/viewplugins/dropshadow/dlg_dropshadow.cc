/*
 *  dlg_dropshadow.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QColor>

#include <kconfig.h>
#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>
#include <kcolorbutton.h>

#include "dlg_dropshadow.h"

DlgDropshadow::DlgDropshadow( const QString & /*imageCS*/,
                          const QString & /*layerCS*/,
                          QWidget *  parent,
                          const char * name)
    : super (parent)
{
    setCaption( i18n("Drop Shadow") );
    setButtons(  Ok | Cancel);
    setDefaultButton( Ok );
    setObjectName(name);
    m_page = new WdgDropshadow(this, "dropshadow");
    Q_CHECK_PTR(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KConfig * cfg = KGlobal::config();
    cfg->setGroup("dropshadow");
    m_page->xOffsetSpinBox->setValue( cfg->readEntry<int>("x", 8) );
    m_page->yOffsetSpinBox->setValue( cfg->readEntry<int>("y", 8) );
    m_page->blurRadiusSpinBox->setValue( cfg->readEntry<int>("blurRadius", 5) );
    QColor black(0,0,0);
    m_page->shadowColorButton->setColor( cfg->readEntry<QColor>("color", Qt::black) );
    m_page->opacitySlider->setValue( cfg->readEntry<int>("opacity", 80 ) );
    m_page->opacitySpinBox->setValue( cfg->readEntry<int>("opacity", 80 ) );
    m_page->allowResizingCheckBox->setChecked( cfg->readEntry<bool>("resizing", true ) );
    cfg->setGroup("");
    
    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));
}

DlgDropshadow::~DlgDropshadow()
{
    delete m_page;
}

qint32 DlgDropshadow::getXOffset()
{
    return m_page->xOffsetSpinBox->value();
}

qint32 DlgDropshadow::getYOffset()
{
    return m_page->yOffsetSpinBox->value();
}

qint32 DlgDropshadow::getBlurRadius()
{
    return m_page->blurRadiusSpinBox->value();
}

quint8 DlgDropshadow::getShadowOpacity()
{
    double opacity = (double)m_page->opacitySpinBox->value();
    //convert percent to a 8 bit opacity value
    return (quint8)(opacity / 100 * 255);
}

QColor DlgDropshadow::getShadowColor()
{
    return m_page->shadowColorButton->color();
}

bool DlgDropshadow::allowResizingChecked()
{
    return m_page->allowResizingCheckBox->isChecked();
}

// SLOTS

void DlgDropshadow::okClicked()
{
    KConfig * cfg = KGlobal::config();
    cfg->setGroup("dropshadow");
    cfg->writeEntry("x", m_page->xOffsetSpinBox->value());
    cfg->writeEntry("y", m_page->yOffsetSpinBox->value());
    cfg->writeEntry("blurRadius", m_page->blurRadiusSpinBox->value());
    cfg->writeEntry("color", m_page->shadowColorButton->color());
    cfg->writeEntry("opacity", m_page->opacitySpinBox->value());
    cfg->writeEntry("resizing", m_page->allowResizingCheckBox->isChecked());
    cfg->setGroup("");
    accept();
}

#include "dlg_dropshadow.moc"
