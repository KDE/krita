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
#ifndef DLG_WAVELETDECOMPOSE
#define DLG_WAVELETDECOMPOSE

#include <KoDialog.h>

#include "ui_wdg_waveletdecompose.h"

class WdgWaveletDecompose : public QWidget, public Ui::WdgWaveletDecompose
{
    Q_OBJECT

public:
    WdgWaveletDecompose(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgWaveletDecompose: public KoDialog
{

    Q_OBJECT

public:

    DlgWaveletDecompose(QWidget * parent = 0,
                               const char* name = 0);
    ~DlgWaveletDecompose() override;

    void setScales(quint32 scales);
    qint32 scales();

private Q_SLOTS:

    void okClicked();

private:

    WdgWaveletDecompose * m_page;

};

#endif // DLG_WAVELETDECOMPOSE
