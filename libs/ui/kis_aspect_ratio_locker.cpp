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
#include "kis_slider_spin_box.h"
#include "kis_int_parse_spin_box.h"
#include "kis_double_parse_spin_box.h"
#include "kis_double_parse_unit_spin_box.h"
#include "KisAngleSelector.h"
#include "KisAngleGauge.h"


struct SliderWrapper
{
    template <class Slider>
    SliderWrapper(Slider *slider)
        : m_slider(QVariant::fromValue(slider)),
          m_object(slider) {}

    void setValue(qreal value) {
        if (auto *slider = m_slider.value<KisDoubleParseUnitSpinBox*>()) {
            // assume value from a KisDoubleParseUnitSpinBox is always provided in Pt
            slider->changeValuePt(value);

        } else if (auto *slider = m_slider.value<KisDoubleParseSpinBox*>()) {
            slider->setValue(value);

        } else if (auto *slider = m_slider.value<KisDoubleSliderSpinBox*>()) {
            slider->setValue(value);

        } else if (auto *slider = m_slider.value<QDoubleSpinBox*>()) {
            slider->setValue(value);

        } else if (auto *slider = m_slider.value<KisIntParseSpinBox*>()) {
            slider->setValue(qRound(value));

        } else if (auto *slider = m_slider.value<KisSliderSpinBox*>()) {
            slider->setValue(qRound(value));

        } else if (auto *slider = m_slider.value<QSpinBox*>()) {
            slider->setValue(qRound(value));

        } else if (auto *slider = m_slider.value<KisAngleSelector*>()) {
            slider->setAngle(value);

        } else if (auto *slider = m_slider.value<KisAngleGauge*>()) {
            slider->setAngle(value);
        }
    }

    qreal value() const {
        qreal result = 0.0;

        if (auto *slider = m_slider.value<KisDoubleParseUnitSpinBox*>()) {
            result = slider->valuePt();
        } else if (auto *slider = m_slider.value<KisDoubleParseSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<KisDoubleSliderSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<QDoubleSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<KisIntParseSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<KisSliderSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<QSpinBox*>()) {
            result = slider->value();

        } else if (auto *slider = m_slider.value<KisAngleSelector*>()) {
            result = slider->angle();

        } else if (auto *slider = m_slider.value<KisAngleGauge*>()) {
            result = slider->angle();

        }

        return result;
    }

    bool isDragging() const {
        bool result = false;

        if (auto *slider = m_slider.value<KisSliderSpinBox*>()) {
            result = slider->isDragging();

        } else if (auto *slider = m_slider.value<KisDoubleSliderSpinBox*>()) {
            result = slider->isDragging();
        }

        return result;
    }

    void connectDraggingFinished(QObject *receiver, const char *amember) {

        if (auto *slider = m_slider.value<KisSliderSpinBox*>()) {
            QObject::connect(slider, SIGNAL(draggingFinished()),
                             receiver, amember);

        } else if (auto *slider = m_slider.value<KisDoubleSliderSpinBox*>()) {
            QObject::connect(slider, SIGNAL(draggingFinished()),
                             receiver, amember);
        }
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

    m_d->spinOne->connectDraggingFinished(this, SLOT(slotSpinDraggingFinished()));
    m_d->spinTwo->connectDraggingFinished(this, SLOT(slotSpinDraggingFinished()));

    connect(m_d->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), SLOT(slotAspectButtonChanged()));
    slotAspectButtonChanged();
}

template <class AngleBoxType>
void KisAspectRatioLocker::connectAngleBoxes(AngleBoxType *spinOne, AngleBoxType *spinTwo, KoAspectButton *aspectButton)
{
    m_d->spinOne.reset(new SliderWrapper(spinOne));
    m_d->spinTwo.reset(new SliderWrapper(spinTwo));
    m_d->aspectButton = aspectButton;

    connect(spinOne, SIGNAL(angleChanged(qreal)), SLOT(slotSpinOneChanged()));
    connect(spinTwo, SIGNAL(angleChanged(qreal)), SLOT(slotSpinTwoChanged()));

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
template KRITAUI_EXPORT void KisAspectRatioLocker::connectAngleBoxes(KisAngleSelector *spinOne, KisAngleSelector *spinTwo, KoAspectButton *aspectButton);
template KRITAUI_EXPORT void KisAspectRatioLocker::connectAngleBoxes(KisAngleGauge *spinOne, KisAngleGauge *spinTwo, KoAspectButton *aspectButton);

void KisAspectRatioLocker::slotSpinOneChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinTwo->object());
        m_d->spinTwo->setValue(m_d->aspectRatio * m_d->spinOne->value());
    }

    if (!m_d->blockUpdatesOnDrag || !m_d->spinOne->isDragging()) {
        Q_EMIT sliderValueChanged();
    }
}

void KisAspectRatioLocker::slotSpinTwoChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinOne->object());
        m_d->spinOne->setValue(m_d->spinTwo->value() / m_d->aspectRatio);
    }

    if (!m_d->blockUpdatesOnDrag || !m_d->spinTwo->isDragging()) {
        Q_EMIT sliderValueChanged();
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
        Q_EMIT aspectButtonChanged();
        Q_EMIT aspectButtonToggled(m_d->aspectButton->keepAspectRatio());
    }
}

void KisAspectRatioLocker::slotSpinDraggingFinished()
{
    if (m_d->blockUpdatesOnDrag) {
        Q_EMIT sliderValueChanged();
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
