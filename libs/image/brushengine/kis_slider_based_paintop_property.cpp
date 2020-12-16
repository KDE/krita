/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_slider_based_paintop_property.h"

#include "kis_paintop_settings.h"


template <typename T>
KisSliderBasedPaintOpProperty<T>::KisSliderBasedPaintOpProperty(Type type,
                                                                const QString &id,
                                                                const QString &name,
                                                                KisPaintOpSettingsRestrictedSP settings,
                                                                QObject *parent)
    : KisUniformPaintOpProperty(type, id, name, settings, parent),
      m_min(T(0)),
      m_max(T(100)),
      m_singleStep(T(1)),
      m_pageStep(T(10)),
      m_exponentRatio(1.0),
      m_decimals(2)
{
}

template <typename T>
KisSliderBasedPaintOpProperty<T>::KisSliderBasedPaintOpProperty(const QString &id,
                                                                const QString &name,
                                                                KisPaintOpSettingsRestrictedSP settings,
                                                                QObject *parent)
    : KisUniformPaintOpProperty(Int, id, name, settings, parent),
      m_min(T(0)),
      m_max(T(100)),
      m_singleStep(T(1)),
      m_pageStep(T(10)),
      m_exponentRatio(1.0),
      m_decimals(2)
{
    qFatal("Should have never been called!");
}

template <typename T>
T KisSliderBasedPaintOpProperty<T>::min() const
{
    return m_min;
}

template <typename T>
T KisSliderBasedPaintOpProperty<T>::max() const
{
    return m_max;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setRange(T min, T max)
{
    m_min = min;
    m_max = max;
}

template <typename T>
T KisSliderBasedPaintOpProperty<T>::singleStep() const
{
    return m_singleStep;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setSingleStep(T value)
{
    m_singleStep = value;
}

template <typename T>
T KisSliderBasedPaintOpProperty<T>::pageStep() const
{
    return m_pageStep;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setPageStep(T value)
{
    m_pageStep = value;
}

template <typename T>
qreal KisSliderBasedPaintOpProperty<T>::exponentRatio() const
{
    return m_exponentRatio;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setExponentRatio(qreal value)
{
    m_exponentRatio = value;
}

template <typename T>
int KisSliderBasedPaintOpProperty<T>::decimals() const
{
    return m_decimals;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setDecimals(int value)
{
    m_decimals = value;
}

template <typename T>
QString KisSliderBasedPaintOpProperty<T>::suffix() const
{
    return m_suffix;
}

template <typename T>
void KisSliderBasedPaintOpProperty<T>::setSuffix(QString value)
{
    m_suffix = value;
}


#include "kis_callback_based_paintop_property_impl.h"

template class KisSliderBasedPaintOpProperty<int>;
template class KisSliderBasedPaintOpProperty<qreal>;

template class KisCallbackBasedPaintopProperty<KisSliderBasedPaintOpProperty<int>>;
template class KisCallbackBasedPaintopProperty<KisSliderBasedPaintOpProperty<qreal>>;
