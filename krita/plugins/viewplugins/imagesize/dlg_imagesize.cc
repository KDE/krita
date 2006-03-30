/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
 *
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <kis_cmb_idlist.h>
#include <kis_filter_strategy.h>

#include "dlg_imagesize.h"
#include "wdg_imagesize.h"


// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)
DlgImageSize::DlgImageSize( QWidget *  parent,
                const char * name)
    : super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
    m_lock = false;

    m_page = new WdgImageSize(this, "image_size");
    Q_CHECK_PTR(m_page);

    m_page->cmbFilterType->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->cmbFilterType->setCurrentText("Mitchell");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    unblockAll();


    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

}

DlgImageSize::~DlgImageSize()
{
    delete m_page;
}

void DlgImageSize::hideScaleBox()
{
    m_page->grpResizeScale->hide();
}

void DlgImageSize::setWidth(quint32 w)
{
    blockAll();

    m_page->lblWidthOriginal->setNum((int)w);
    m_page->intWidth->setValue(w);
    m_oldW = w;
    m_origW = w;

    unblockAll();
}

void DlgImageSize::setWidthPercent(quint32 w)
{
    blockAll();

    m_page->intWidthPercent->setValue(w);
    m_oldWPercent = w;

    unblockAll();
}


void DlgImageSize::setMaximumWidth(quint32 w)
{
    m_page->intWidth->setMaxValue(w);
    m_maxW = w;
}

qint32 DlgImageSize::width()
{
    //return (qint32)qRound(m_oldW);
    return (qint32)qRound(m_page->intWidth->value());
}

void DlgImageSize::setHeight(quint32 h)
{
    blockAll();

    m_page->lblHeightOriginal->setNum((int)h);
    m_page->intHeight->setValue(h);
    m_oldH = h;
    m_origH = h;

    unblockAll();
}


void DlgImageSize::setHeightPercent(quint32 h)
{
    blockAll();

    m_page->intHeightPercent->setValue(h);
    m_oldHPercent = h;

    unblockAll();
}



void DlgImageSize::setMaximumHeight(quint32 h)
{
    m_page->intHeight->setMaxValue(h);
    m_maxH = h;
}


qint32 DlgImageSize::height()
{
    //return (qint32)qRound(m_oldH);
    return (qint32)qRound(m_page->intHeight->value());
}

bool DlgImageSize::scale()
{
    return m_page->radioScale->isChecked();
}

bool DlgImageSize::cropLayers()
{
    return m_page->chkCrop->isChecked();
}

KisFilterStrategy *DlgImageSize::filterType()
{
    KisID filterID = m_page->cmbFilterType->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->get(filterID);
    return filter;
}

// SLOTS

void DlgImageSize::okClicked()
{
    accept();
}

void DlgImageSize::slotWidthPixelsChanged(int w)
{
    blockAll();

    double wPercent = double(w) * 100 / double(m_origW);

    m_page->intWidthPercent->setValue(qRound(wPercent));

    // Set height in pixels and percent of necessary
    if (m_page->chkConstrain->isChecked()) {
        m_page->intHeightPercent->setValue(qRound(wPercent));

        m_oldH = qRound(m_origH * wPercent / 100);
        m_page->intHeight->setValue(qRound(m_oldH));

    }
    m_oldW = w;

    unblockAll();
}

void DlgImageSize::slotHeightPixelsChanged(int h)
{
    blockAll();

    double hPercent = double(h) * 100 / double(m_origH);

    m_page->intHeightPercent->setValue(qRound(hPercent));

    // Set width in pixels and percent of necessary
    if (m_page->chkConstrain->isChecked()) {
        m_page->intWidthPercent->setValue(qRound(hPercent));

        m_oldW = qRound(m_origW * hPercent / 100);
        m_page->intWidth->setValue(qRound(m_oldW));

    }
    m_oldH = h;

    unblockAll();
}

void DlgImageSize::slotWidthPercentChanged(int w)
{
    blockAll();

    m_page->intWidth->setValue(qRound(w * m_origW / 100));

    if (m_page->chkConstrain->isChecked()) {
        m_page->intHeightPercent->setValue(w);
        m_page->intHeight->setValue(qRound( w * m_origH / 100));
    }

    unblockAll();
}

void DlgImageSize::slotHeightPercentChanged(int h)
{
    blockAll();

    m_page->intHeight->setValue(qRound(h * m_origH / 100));
    if (m_page->chkConstrain->isChecked()) {
        m_page->intWidthPercent->setValue(h);
        m_page->intWidth->setValue(qRound( h * m_origW / 100));
    }

    unblockAll();

}


void DlgImageSize::blockAll()
{
    // XXX: more efficient to use blockSignals?
    m_page->intWidth->disconnect();
    m_page->intHeight->disconnect();
    m_page->intWidthPercent->disconnect();
    m_page->intHeightPercent->disconnect();

}

void DlgImageSize::unblockAll()
{
    // XXX: more efficient to use blockSignals?
    connect (m_page->intWidth, SIGNAL(valueChanged(int)),
         this, SLOT(slotWidthPixelsChanged(int)));

    connect (m_page->intHeight, SIGNAL(valueChanged(int)),
         this, SLOT(slotHeightPixelsChanged(int)));

    connect (m_page->intWidthPercent, SIGNAL(valueChanged(int)),
         this, SLOT(slotWidthPercentChanged(int)));

    connect (m_page->intHeightPercent, SIGNAL(valueChanged(int)),
         this, SLOT(slotHeightPercentChanged(int)));


}

#include "dlg_imagesize.moc"
