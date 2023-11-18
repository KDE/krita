/*
 * SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KisSpinBoxI18nHelper.h>
#include <kis_signal_compressor.h>

#include "KisSelectionPropertySlider.h"

template class KisSelectionPropertySlider<KoShape *>;

struct KisSelectionPropertySliderBase::Private
{
    KisSignalCompressor *signalCompressor {nullptr};
    QString normalTemplate;
    QString mixedTemplate;

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

void KisSelectionPropertySliderBase::setTextTemplates(const QString &normalTemplate, const QString &mixedTemplate)
{
    m_d->normalTemplate = normalTemplate;
    m_d->mixedTemplate = mixedTemplate;
    KisSpinBoxI18nHelper::setText(static_cast<QDoubleSpinBox *>(this), normalTemplate);
}

void KisSelectionPropertySliderBase::setInternalValue(qreal _value, bool blockUpdateSignal)
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
        KisSpinBoxI18nHelper::setText(static_cast<QDoubleSpinBox *>(this), m_d->mixedTemplate);
    } else {
        setValue(commonValue);
        KisSpinBoxI18nHelper::setText(static_cast<QDoubleSpinBox *>(this), m_d->normalTemplate);
    }
}

KisShapePropertySlider::KisShapePropertySlider(QWidget *parent)
    : KisSelectionPropertySlider<KoShape*>::KisSelectionPropertySlider(parent)
{}
