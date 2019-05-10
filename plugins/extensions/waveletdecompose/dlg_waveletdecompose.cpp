/*
 *
 *  Copyright (c) 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
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

#include "dlg_waveletdecompose.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <KisDialogStateSaver.h>

DlgWaveletDecompose::DlgWaveletDecompose(QWidget *  parent,
                                         const char * name)
    : KoDialog(parent)
{
    setCaption(i18n("WaveletDecompose"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);

    m_page = new WdgWaveletDecompose(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("wavelet_decompose");
    KisDialogStateSaver::restoreState(m_page, "DlgWaveletDecompose");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

}

DlgWaveletDecompose::~DlgWaveletDecompose()
{
    KisDialogStateSaver::saveState(m_page, "DlgWaveletDecompose");
    delete m_page;
}

void DlgWaveletDecompose::setScales(quint32 scales)
{
    m_page->scales->setValue(scales);

}


qint32 DlgWaveletDecompose::scales()
{
    return m_page->scales->value();
}


// SLOTS

void DlgWaveletDecompose::okClicked()
{
    accept();
}

