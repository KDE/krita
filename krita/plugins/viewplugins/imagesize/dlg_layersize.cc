/*
 *  dlg_layersize.cc - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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

#include <QRadioButton>
#include <QCheckBox>
#include <q3buttongroup.h>
#include <QLabel>
#include <QComboBox>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <kis_cmb_idlist.h>
#include <kis_filter_strategy.h>

#include "dlg_layersize.h"

// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)
DlgLayerSize::DlgLayerSize( QWidget *  parent,
                const char * name)
    : super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
    m_lock = false;

    m_page = new WdgLayerSize(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("layer_size");
    
    m_page->cmbFilterType->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->cmbFilterType->setCurrent("Mitchell");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    unblockAll();


    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

}

DlgLayerSize::~DlgLayerSize()
{
    delete m_page;
}

void DlgLayerSize::setWidth(quint32 w)
{
    blockAll();

    m_page->lblWidthOriginal->setNum((int)w);
    m_page->intWidth->setValue(w);
    m_oldW = w;
    m_origW = w;

    unblockAll();
}

void DlgLayerSize::setWidthPercent(quint32 w)
{
    blockAll();

    m_page->intWidthPercent->setValue(w);
    m_oldWPercent = w;

    unblockAll();
}


void DlgLayerSize::setMaximumWidth(quint32 w)
{
    m_page->intWidth->setMaximum(w);
    m_maxW = w;
}

qint32 DlgLayerSize::width()
{
    //return (qint32)qRound(m_oldW);
    return (qint32)qRound(m_page->intWidth->value());
}

void DlgLayerSize::setHeight(quint32 h)
{
    blockAll();

    m_page->lblHeightOriginal->setNum((int)h);
    m_page->intHeight->setValue(h);
    m_oldH = h;
    m_origH = h;

    unblockAll();
}


void DlgLayerSize::setHeightPercent(quint32 h)
{
    blockAll();

    m_page->intHeightPercent->setValue(h);
    m_oldHPercent = h;

    unblockAll();
}

void DlgLayerSize::setMaximumHeight(quint32 h)
{
    m_page->intHeight->setMaximum(h);
    m_maxH = h;
}

qint32 DlgLayerSize::height()
{
    //return (qint32)qRound(m_oldH);
    return (qint32)qRound(m_page->intHeight->value());
}

KisFilterStrategy *DlgLayerSize::filterType()
{
    KisID filterID = m_page->cmbFilterType->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->get(filterID);
    return filter;
}


// SLOTS

void DlgLayerSize::okClicked()
{
    accept();
}

void DlgLayerSize::slotWidthPixelsChanged(int w)
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

void DlgLayerSize::slotHeightPixelsChanged(int h)
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

void DlgLayerSize::slotWidthPercentChanged(int w)
{
    blockAll();

    m_page->intWidth->setValue(qRound(w * m_origW / 100));

    if (m_page->chkConstrain->isChecked()) {
        m_page->intHeightPercent->setValue(w);
        m_page->intHeight->setValue(qRound( w * m_origH / 100));
    }

    unblockAll();
}

void DlgLayerSize::slotHeightPercentChanged(int h)
{
    blockAll();

    m_page->intHeight->setValue(qRound(h * m_origH / 100));
    if (m_page->chkConstrain->isChecked()) {
        m_page->intWidthPercent->setValue(h);
        m_page->intWidth->setValue(qRound( h * m_origW / 100));
    }

    unblockAll();

}


void DlgLayerSize::blockAll()
{
    // XXX: more efficient to use blockSignals?
    m_page->intWidth->disconnect();
    m_page->intHeight->disconnect();
    m_page->intWidthPercent->disconnect();
    m_page->intHeightPercent->disconnect();

}

void DlgLayerSize::unblockAll()
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

#include "dlg_layersize.moc"
