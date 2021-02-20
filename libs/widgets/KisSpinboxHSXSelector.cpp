/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSpinboxHSXSelector.h"

#include <QFormLayout>
#include <QLabel>
#include <QList>
#include <QSignalBlocker>
#include "kis_double_parse_spin_box.h"
#include "KisVisualColorSelector.h"

struct KisSpinboxHSXSelector::Private
{
    QList <QLabel*> labels;
    QList <KisDoubleParseSpinBox*> spinBoxes;
    QFormLayout *layout {0};
};

KisSpinboxHSXSelector::KisSpinboxHSXSelector(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->layout = new QFormLayout(this);
    for (int i = 0; i < 3; i++){
        m_d->labels.push_back(new QLabel(this));
        m_d->spinBoxes.push_back(new KisDoubleParseSpinBox(this));
        m_d->layout->addRow(m_d->labels[i], m_d->spinBoxes[i]);
        connect(m_d->spinBoxes.back(), SIGNAL(valueChanged(double)), this, SLOT(slotSpinBoxChanged()));
    }
    m_d->labels[0]->setText(i18n("Hue:"));
    m_d->labels[1]->setText(i18n("Saturation:"));
    m_d->labels[2]->setText("<x>:");
    m_d->spinBoxes[0]->setMaximum(360.0);
    m_d->spinBoxes[1]->setMaximum(100.0);
    m_d->spinBoxes[2]->setMaximum(100.0);
    m_d->spinBoxes[0]->setSuffix(" °");
    m_d->spinBoxes[1]->setSuffix(" %");
    m_d->spinBoxes[2]->setSuffix(" %");
}

KisSpinboxHSXSelector::~KisSpinboxHSXSelector()
{
}

void KisSpinboxHSXSelector::attachToSelector(KisVisualColorSelector *selector)
{
    connect(selector, SIGNAL(sigColorModelChanged()), this, SLOT(slotColorModelChanged()));
    connect(selector, SIGNAL(sigHSXChanged(QVector3D)), this, SLOT(slotHSXChanged(QVector3D)));
    connect(this, SIGNAL(sigHSXChanged(QVector3D)), selector, SLOT(slotSetHSX(QVector3D)));
}

void KisSpinboxHSXSelector::slotColorModelChanged()
{
    const KisVisualColorSelector *selector = qobject_cast<KisVisualColorSelector *>(sender());
    if (!selector) {
        return;
    }

    switch (selector->getColorModel()) {
    case KisVisualColorSelector::HSV:
        m_d->labels[2]->setText(i18n("Value:"));
        break;
    case KisVisualColorSelector::HSL:
        m_d->labels[2]->setText(i18n("Lightness:"));
        break;
    case KisVisualColorSelector::HSI:
        m_d->labels[2]->setText(i18n("Intensity:"));
        break;
    case KisVisualColorSelector::HSY:
        m_d->labels[2]->setText(i18n("Luma:"));
        break;
    default:
        break;
    }
}

void KisSpinboxHSXSelector::slotHSXChanged(const QVector3D &hsx)
{
    const QSignalBlocker s1(m_d->spinBoxes[0]), s2(m_d->spinBoxes[1]), s3(m_d->spinBoxes[2]);
    m_d->spinBoxes[0]->setValue(hsx[0] * 360.0);
    m_d->spinBoxes[1]->setValue(hsx[1] * 100.0);
    m_d->spinBoxes[2]->setValue(hsx[2] * 100.0);
}

void KisSpinboxHSXSelector::slotSpinBoxChanged()
{
    QVector3D hsx(m_d->spinBoxes[0]->value() / 360.0,
                  m_d->spinBoxes[1]->value() / 100.0,
                  m_d->spinBoxes[2]->value() / 100.0);
    emit sigHSXChanged(hsx);
}
