/*
 * SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
