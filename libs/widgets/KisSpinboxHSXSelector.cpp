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

struct KisSpinboxHSXSelector::Private
{
    QList <QLabel*> labels;
    QList <KisDoubleParseSpinBox*> spinBoxes;
    QFormLayout *layout {0};
    KisVisualColorModelSP selectorModel;
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

void KisSpinboxHSXSelector::setModel(KisVisualColorModelSP model)
{
    if (m_d->selectorModel) {
        m_d->selectorModel->disconnect(this);
        disconnect(m_d->selectorModel.data());
    }
    m_d->selectorModel = model;
    if (model) {
        connect(model.data(), SIGNAL(sigColorModelChanged()), this, SLOT(slotColorModelChanged()));
        slotColorModelChanged();
        if (model->isHSXModel()) {
            slotChannelValuesChanged(model->channelValues());
        }
    }
}

void KisSpinboxHSXSelector::slotColorModelChanged()
{
    if (!m_d->selectorModel) {
        return;
    }

    if (m_d->selectorModel->isHSXModel()) {
        switch (m_d->selectorModel->colorModel()) {
        case KisVisualColorModel::HSV:
            m_d->labels[2]->setText(i18n("Value:"));
            break;
        case KisVisualColorModel::HSL:
            m_d->labels[2]->setText(i18n("Lightness:"));
            break;
        case KisVisualColorModel::HSI:
            m_d->labels[2]->setText(i18n("Intensity:"));
            break;
        case KisVisualColorModel::HSY:
            m_d->labels[2]->setText(i18n("Luma:"));
            break;
        default:
            break;
        }
        connect(m_d->selectorModel.data(), SIGNAL(sigChannelValuesChanged(QVector4D,quint32)),
                this, SLOT(slotChannelValuesChanged(QVector4D)), Qt::UniqueConnection);
        connect(this, SIGNAL(sigChannelValuesChanged(QVector4D)),
                m_d->selectorModel.data(), SLOT(slotSetChannelValues(QVector4D)), Qt::UniqueConnection);
    } else {
        m_d->selectorModel->disconnect(SIGNAL(sigChannelValuesChanged(QVector4D,quint32)), this);
        disconnect(SIGNAL(sigChannelValuesChanged(QVector4D)));
    }
}

void KisSpinboxHSXSelector::slotChannelValuesChanged(const QVector4D &values)
{
    const QSignalBlocker s1(m_d->spinBoxes[0]), s2(m_d->spinBoxes[1]), s3(m_d->spinBoxes[2]);
    m_d->spinBoxes[0]->setValue(values[0] * 360.0);
    m_d->spinBoxes[1]->setValue(values[1] * 100.0);
    m_d->spinBoxes[2]->setValue(values[2] * 100.0);
}

void KisSpinboxHSXSelector::slotSpinBoxChanged()
{
    QVector4D hsx(m_d->spinBoxes[0]->value() / 360.0,
                  m_d->spinBoxes[1]->value() / 100.0,
                  m_d->spinBoxes[2]->value() / 100.0,
                  0);
    Q_EMIT sigChannelValuesChanged(hsx);
}
