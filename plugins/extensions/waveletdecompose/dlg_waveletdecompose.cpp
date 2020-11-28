/*
 *
 *  SPDX-FileCopyrightText: 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_waveletdecompose.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

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

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

}

DlgWaveletDecompose::~DlgWaveletDecompose()
{
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

