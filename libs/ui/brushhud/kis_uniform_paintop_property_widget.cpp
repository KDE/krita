/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_uniform_paintop_property_widget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>

#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"
#include "kis_slider_based_paintop_property.h"
#include "kis_combo_based_paintop_property.h"
#include "kis_debug.h"

/****************************************************************/
/*      KisUniformPaintOpPropertyWidget                         */
/****************************************************************/

struct KisUniformPaintOpPropertyWidget::Private
{
    Private(KisUniformPaintOpPropertySP _property)
        : property(_property) {}

    typedef KisUniformPaintOpProperty::Type Type;
    KisUniformPaintOpPropertySP property;
};

KisUniformPaintOpPropertyWidget::KisUniformPaintOpPropertyWidget(KisUniformPaintOpPropertySP property, QWidget *parent)
    : QWidget(parent),
      m_d(new Private(property))
{
    KisAcyclicSignalConnector *conn = new KisAcyclicSignalConnector(this);
    conn->connectForwardVariant(property.data(), SIGNAL(valueChanged(QVariant)),
                                this, SLOT(setValue(QVariant)));

    conn->connectBackwardVariant(this, SIGNAL(valueChanged(QVariant)),
                                 property.data(), SLOT(setValue(QVariant)));
}

KisUniformPaintOpPropertyWidget::~KisUniformPaintOpPropertyWidget()
{
}

KisUniformPaintOpPropertySP KisUniformPaintOpPropertyWidget::property() const
{
    return m_d->property;
}

void KisUniformPaintOpPropertyWidget::slotThemeChanged(QPalette pal)
{
    for(int i=0; i<this->children().size(); i++) {
        QWidget *w = qobject_cast<QWidget*>(this->children().at(i));
        if (w) {
            w->setPalette(pal);
        }
    }
}

/****************************************************************/
/*      KisUniformPaintOpPropertyIntSlider                      */
/****************************************************************/

KisUniformPaintOpPropertyIntSlider::KisUniformPaintOpPropertyIntSlider(KisUniformPaintOpPropertySP property, QWidget *parent)
    : KisUniformPaintOpPropertyWidget(property, parent)
{
    const QString prefix = QString("%1: ").arg(property->name());
    QVBoxLayout *layout = new QVBoxLayout(this);

    KisIntSliderBasedPaintOpProperty *sliderProperty =
        dynamic_cast<KisIntSliderBasedPaintOpProperty*>(property.data());
    KIS_ASSERT_RECOVER_RETURN(sliderProperty);

    m_slider = new KisSliderSpinBox(this);
    m_slider->setBlockUpdateSignalOnDrag(true);
    m_slider->setRange(sliderProperty->min(), sliderProperty->max());
    m_slider->setSingleStep(sliderProperty->singleStep());
    m_slider->setPageStep(sliderProperty->pageStep());
    m_slider->setPrefix(prefix);
    m_slider->setSuffix(sliderProperty->suffix());
    m_slider->setExponentRatio(sliderProperty->exponentRatio());

    m_slider->setValue(sliderProperty->value().toInt());
    connect(m_slider, SIGNAL(valueChanged(int)), SLOT(slotSliderChanged(int)));

    layout->addWidget(m_slider);
}

void KisUniformPaintOpPropertyIntSlider::setValue(const QVariant &value)
{
    m_slider->setValue(value.toInt());
}

void KisUniformPaintOpPropertyIntSlider::slotSliderChanged(int value)
{
    emit valueChanged(value);
}

/****************************************************************/
/*      KisUniformPaintOpPropertyDoubleSlider                   */
/****************************************************************/

KisUniformPaintOpPropertyDoubleSlider::KisUniformPaintOpPropertyDoubleSlider(KisUniformPaintOpPropertySP property, QWidget *parent)
    : KisUniformPaintOpPropertyWidget(property, parent)
{
    const QString prefix = QString("%1: ").arg(property->name());
    QVBoxLayout *layout = new QVBoxLayout(this);

    KisDoubleSliderBasedPaintOpProperty *sliderProperty =
        dynamic_cast<KisDoubleSliderBasedPaintOpProperty*>(property.data());
    KIS_ASSERT_RECOVER_RETURN(sliderProperty);

    m_slider = new KisDoubleSliderSpinBox(this);
    m_slider->setBlockUpdateSignalOnDrag(true);
    m_slider->setRange(sliderProperty->min(), sliderProperty->max(), sliderProperty->decimals());
    m_slider->setSingleStep(sliderProperty->singleStep());
    m_slider->setPrefix(prefix);
    m_slider->setSuffix(sliderProperty->suffix());
    m_slider->setExponentRatio(sliderProperty->exponentRatio());

    m_slider->setValue(sliderProperty->value().toReal());
    connect(m_slider, SIGNAL(valueChanged(qreal)), SLOT(slotSliderChanged(qreal)));

    layout->addWidget(m_slider);
}

void KisUniformPaintOpPropertyDoubleSlider::setValue(const QVariant &value)
{
    m_slider->setValue(value.toReal());
}

void KisUniformPaintOpPropertyDoubleSlider::slotSliderChanged(qreal value)
{
    emit valueChanged(value);
}

/****************************************************************/
/*      KisUniformPaintOpPropertyCheckBox                       */
/****************************************************************/

KisUniformPaintOpPropertyCheckBox::KisUniformPaintOpPropertyCheckBox(KisUniformPaintOpPropertySP property, QWidget *parent)
    : KisUniformPaintOpPropertyWidget(property, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_checkBox = new QCheckBox(property->name(), this);
    m_checkBox->setChecked(property->value().toBool());
    connect(m_checkBox, SIGNAL(toggled(bool)), SLOT(slotCheckBoxChanged(bool)));

    layout->addWidget(m_checkBox);
}

void KisUniformPaintOpPropertyCheckBox::setValue(const QVariant &value)
{
    m_checkBox->setChecked(value.toBool());
}

void KisUniformPaintOpPropertyCheckBox::slotCheckBoxChanged(bool value)
{
    emit valueChanged(value);
}

/****************************************************************/
/*      KisUniformPaintOpPropertyComboBox                       */
/****************************************************************/

KisUniformPaintOpPropertyComboBox::KisUniformPaintOpPropertyComboBox(KisUniformPaintOpPropertySP property, QWidget *parent)
    : KisUniformPaintOpPropertyWidget(property, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    KisComboBasedPaintOpProperty *comboProperty =
        dynamic_cast<KisComboBasedPaintOpProperty*>(property.data());
    KIS_ASSERT_RECOVER_RETURN(comboProperty);

    const QList<QString> items = comboProperty->items();
    const QList<QIcon> icons = comboProperty->icons();

    m_comboBox = new QComboBox(this);

    KIS_SAFE_ASSERT_RECOVER_RETURN(icons.isEmpty() ||
                                   items.size() == icons.size());

    if (!icons.isEmpty()) {
        auto itemIt = items.constBegin();
        auto iconIt = icons.constBegin();

        while (itemIt != items.constEnd() &&
               iconIt != icons.constEnd()) {

            m_comboBox->addItem(*iconIt, *itemIt);

            ++itemIt;
            ++iconIt;
        }
    } else {
        Q_FOREACH (const QString &item, items) {
            m_comboBox->addItem(item);
        }
    }

    m_comboBox->setCurrentIndex(property->value().toInt());
    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), SLOT(slotComboBoxChanged(int)));

    layout->addWidget(m_comboBox);
}

void KisUniformPaintOpPropertyComboBox::setValue(const QVariant &value)
{
    m_comboBox->setCurrentIndex(value.toInt());
}

void KisUniformPaintOpPropertyComboBox::slotComboBoxChanged(int value)
{
    emit valueChanged(value);
}
