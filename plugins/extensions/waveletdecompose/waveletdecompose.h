/*
 * SPDX-FileCopyrightText: 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef WAVELETDECOMPOSE_H
#define WAVELETDECOMPOSE_H

#include <QVariant>

#include <QUrl>
#include <KisActionPlugin.h>

class WaveletDecompose : public KisActionPlugin
{
    Q_OBJECT
public:
    WaveletDecompose(QObject *parent, const QVariantList &);
    ~WaveletDecompose() override;

private Q_SLOTS:

    void slotWaveletDecompose();

};

#endif // WAVELETDECOMPOSE_H
