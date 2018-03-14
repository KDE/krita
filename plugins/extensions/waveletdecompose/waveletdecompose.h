/*
 * Copyright (C) 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
