/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_aspect_ratio_locker.h"

#include <QSpinBox>
#include <QDoubleSpinBox>

#include <KoAspectButton.h>

#include "kis_signals_blocker.h"
#include "kis_assert.h"
#include "kis_debug.h"
#include "kis_slider_spin_box.h"
#include "kis_int_parse_spin_box.h"
#include "kis_double_parse_spin_box.h"
#include "kis_double_parse_unit_spin_box.h"


struct SliderWrapper
{
    template <class Slider>
    SliderWrapper(Slider *slider)
        : m_slider(QVariant::fromValue(slider)),
          m_object(slider) {}

    void setValue(qreal value) {

        if (m_slider.canConvert<KisDoubleParseUnitSpinBox*>()) {
            m_slider.value<KisDoubleParseUnitSpinBox*>()->changeValue(value);

        } else if (m_slider.canConvert<KisDoubleParseSpinBox*>()) {
            m_slider.value<KisDoubleParseSpinBox*>()->setValue(value);

        } else if (m_slider.canConvert<KisDoubleSliderSpinBox*>()) {
            m_slider.value<KisDoubleSliderSpinBox*>()->setValue(value);

        } else if (m_slider.canConvert<QDoubleSpinBox*>()) {
            m_slider.value<QDoubleSpinBox*>()->setValue(value);

        } else if (m_slider.canConvert<KisIntParseSpinBox*>()) {
            m_slider.value<KisIntParseSpinBox*>()->setValue(qRound(value));

        } else if (m_slider.canConvert<KisSliderSpinBox*>()) {
            m_slider.value<KisSliderSpinBox*>()->setValue(qRound(value));

        } else if (m_slider.canConvert<QSpinBox*>()) {
            m_slider.value<QSpinBox*>()->setValue(qRound(value));
        }
    }

    qreal value() const {
        qreal result = 0.0;

        if (m_slider.canConvert<KisDoubleParseUnitSpinBox*>()) {
            result = m_slider.value<KisDoubleParseUnitSpinBox*>()->value();

        } else if (m_slider.canConvert<KisDoubleParseSpinBox*>()) {
            result = m_slider.value<KisDoubleParseSpinBox*>()->value();

        } else if (m_slider.canConvert<KisDoubleSliderSpinBox*>()) {
            result = m_slider.value<KisDoubleSliderSpinBox*>()->value();

        } else if (m_slider.canConvert<QDoubleSpinBox*>()) {
            result = m_slider.value<QDoubleSpinBox*>()->value();

        } else if (m_slider.canConvert<KisIntParseSpinBox*>()) {
            result = m_slider.value<KisIntParseSpinBox*>()->value();

        } else if (m_slider.canConvert<KisSliderSpinBox*>()) {
            result = m_slider.value<KisSliderSpinBox*>()->value();

        } else if (m_slider.canConvert<QSpinBox*>()) {
            result = m_slider.value<QSpinBox*>()->value();

        }

        return result;
    }

    bool isDragging() const {
        bool result = false;

        if (m_slider.canConvert<KisSliderSpinBox*>()) {
            result = m_slider.value<KisSliderSpinBox*>()->isDragging();

        } else if (m_slider.canConvert<KisDoubleSliderSpinBox*>()) {
            result = m_slider.value<KisDoubleSliderSpinBox*>()->isDragging();
        }

        return result;
    }

    QObject* object() const {
        return m_object;
    }

private:
    QVariant m_slider;
    QObject *m_object;
};

struct KisAspectRatioLocker::Private
{
    QScopedPointer<SliderWrapper> spinOne;
    QScopedPointer<SliderWrapper> spinTwo;
    KoAspectButton *aspectButton = 0;

    qreal aspectRatio = 1.0;
    bool blockUpdatesOnDrag = false;
};


KisAspectRatioLocker::KisAspectRatioLocker(QObject *parent)
    : QObject(parent),
      m_d(new Private)
{
}

KisAspectRatioLocker::~KisAspectRatioLocker()
{
}

template <class SpinBoxType>
void KisAspectRatioLocker::connectSpinBoxes(SpinBoxType *spinOne, SpinBoxType *spinTwo, KoAspectButton *aspectButton)
{
    m_d->spinOne.reset(new SliderWrapper(spinOne));
    m_d->spinTwo.reset(new SliderWrapper(spinTwo));
    m_d->aspectButton = aspectButton;

    if (QVariant::fromValue(spinOne->value()).type() == QVariant::Double) {
        connect(spinOne, SIGNAL(valueChanged(qreal)), SLOT(slotSpinOneChanged()));
        connect(spinTwo, SIGNAL(valueChanged(qreal)), SLOT(slotSpinTwoChanged()));
    } else {
        connect(spinOne, SIGNAL(valueChanged(int)), SLOT(slotSpinOneChanged()));
        connect(spinTwo, SIGNAL(valueChanged(int)), SLOT(slotSpinTwoChanged()));
    }

    connect(m_d->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), SLOT(slotAspectButtonChanged()));
    slotAspectButtonChanged();
}

template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(QSpinBox *spinOne, QSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(QDoubleSpinBox *spinOne, QDoubleSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(KisSliderSpinBox *spinOne, KisSliderSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(KisDoubleSliderSpinBox *spinOne, KisDoubleSliderSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(KisIntParseSpinBox *spinOne, KisIntParseSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(KisDoubleParseSpinBox *spinOne, KisDoubleParseSpinBox *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectSpinBoxes(KisDoubleParseUnitSpinBox *spinOne, KisDoubleParseUnitSpinBox *spinTwo, KoAspectButton *aspectButton);

void KisAspectRatioLocker::slotSpinOneChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinTwo->object());
        m_d->spinTwo->setValue(m_d->aspectRatio * m_d->spinOne->value());
    }

    if (!m_d->blockUpdatesOnDrag || !m_d->spinOne->isDragging()) {
        emit sliderValueChanged();
    }
}

void KisAspectRatioLocker::slotSpinTwoChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinOne->object());
        m_d->spinOne->setValue(m_d->spinTwo->value() / m_d->aspectRatio);
    }

    if (!m_d->blockUpdatesOnDrag || !m_d->spinTwo->isDragging()) {
        emit sliderValueChanged();
    }
}

void KisAspectRatioLocker::slotAspectButtonChanged()
{
    if (m_d->aspectButton->keepAspectRatio() &&
        m_d->spinTwo->value() > 0 &&
        m_d->spinOne->value() > 0) {

        m_d->aspectRatio = qreal(m_d->spinTwo->value()) / m_d->spinOne->value();
    } else {
        m_d->aspectRatio = 1.0;
    }

    if (!m_d->spinTwo->isDragging()) {
        emit aspectButtonChanged();
        emit aspectButtonToggled(m_d->aspectButton->keepAspectRatio());
    }
}

void KisAspectRatioLocker::setBlockUpdateSignalOnDrag(bool value)
{
    m_d->blockUpdatesOnDrag = value;
}

void KisAspectRatioLocker::updateAspect()
{
    KisSignalsBlocker b(this);
    slotAspectButtonChanged();
}
