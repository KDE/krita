/*
 * Copyright (C) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#include <kis_signal_compressor.h>

#include "KisSelectionPropertySlider.h"

struct KisSelectionPropertySliderBase::Private
{
    KisSignalCompressor *signalCompressor;
    QString normalPrefix;
    QString mixedPrefix;

    explicit Private(KisSelectionPropertySliderBase *q)
        : signalCompressor(new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE, q))
    {}
};

KisSelectionPropertySliderBase::KisSelectionPropertySliderBase(QWidget *parent)
    : KisDoubleSliderSpinBox(parent)
    , m_d(new Private(this))
{
    connect(m_d->signalCompressor, SIGNAL(timeout()), SLOT(slotCompressedUpdate()));
}

KisSelectionPropertySliderBase::~KisSelectionPropertySliderBase()
{}

void KisSelectionPropertySliderBase::setPrefixes(const QString &normalPrefix, const QString &mixedPrefix)
{
    m_d->normalPrefix = normalPrefix;
    m_d->mixedPrefix = mixedPrefix;
    setPrefix(normalPrefix);
}

void KisSelectionPropertySliderBase::setInternalValue(int _value, bool blockUpdateSignal)
{
    static const qreal eps = 1e-3;

    if (!hasSelection()) return;

    setPrivateValue(_value);

    const qreal newValue = value();
    const qreal commonValue = getCommonValue();
    if (qAbs(commonValue - newValue) < eps) {
        return;
    }

    if(!blockUpdateSignal) {
        m_d->signalCompressor->start();
    }
}

void KisSelectionPropertySliderBase::slotCompressedUpdate()
{
    emit(valueChanged(value()));
}

void KisSelectionPropertySliderBase::setSelectionValue(qreal commonValue, bool mixed)
{
    if (mixed) {
        setInternalValue(0.0, true); // BUG:409131
        setPrefix(m_d->mixedPrefix);
    } else {
        setValue(commonValue);
        setPrefix(m_d->normalPrefix);
    }
}

KisShapePropertySlider::KisShapePropertySlider(QWidget *parent)
    : KisSelectionPropertySlider<KoShape*>::KisSelectionPropertySlider(parent)
{}
