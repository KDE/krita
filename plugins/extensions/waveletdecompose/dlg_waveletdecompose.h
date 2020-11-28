/*
 *
 *  SPDX-FileCopyrightText: 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
