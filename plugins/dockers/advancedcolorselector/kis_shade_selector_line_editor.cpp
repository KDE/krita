/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QMouseEvent>

#include "kis_shade_selector_line_editor.h"
#include "kis_double_parse_spin_box.h"
#include "kis_config.h"

KisShadeSelectorLineEditor::KisShadeSelectorLineEditor(QWidget* parent, KisShadeSelectorLine* preview)
    : KisShadeSelectorLineBase(parent)
    , m_line_preview(preview)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout* lineOne = new QHBoxLayout();
    layout->addLayout(lineOne);
    lineOne->addWidget(new QLabel(i18n("Delta: ")));

    m_hueDelta = new KisDoubleParseSpinBox();
    lineOne->addWidget(m_hueDelta);
    m_saturationDelta = new KisDoubleParseSpinBox();
    lineOne->addWidget(m_saturationDelta);
    m_valueDelta = new KisDoubleParseSpinBox();
    lineOne->addWidget(m_valueDelta);

    QHBoxLayout* lineTwo = new QHBoxLayout();
    layout->addLayout(lineTwo);
    lineTwo->addWidget(new QLabel(i18n("Shift: ")));

    m_hueShift = new KisDoubleParseSpinBox();
    lineTwo->addWidget(m_hueShift);
    m_saturationShift = new KisDoubleParseSpinBox();
    lineTwo->addWidget(m_saturationShift);
    m_valueShift = new KisDoubleParseSpinBox();
    lineTwo->addWidget(m_valueShift);


    m_hueDelta->setRange(-1, 1);
    m_saturationDelta->setRange(-1, 1);
    m_valueDelta->setRange(-1, 1);
    m_hueShift->setRange(-1, 1);
    m_saturationShift->setRange(-1, 1);
    m_valueShift->setRange(-1, 1);

    m_hueDelta->setSingleStep(0.1);
    m_saturationDelta->setSingleStep(0.1);
    m_valueDelta->setSingleStep(0.1);
    m_hueShift->setSingleStep(0.05);
    m_saturationShift->setSingleStep(0.05);
    m_valueShift->setSingleStep(0.05);

    connect(m_hueDelta, SIGNAL(valueChanged(double)), SLOT(valueChanged()));
    connect(m_saturationDelta, SIGNAL(valueChanged(double)), SLOT(valueChanged()));
    connect(m_valueDelta, SIGNAL(valueChanged(double)), SLOT(valueChanged()));
    connect(m_hueShift, SIGNAL(valueChanged(double)), SLOT(valueChanged()));
    connect(m_saturationShift, SIGNAL(valueChanged(double)), SLOT(valueChanged()));
    connect(m_valueShift, SIGNAL(valueChanged(double)), SLOT(valueChanged()));

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    QString lineset = cfg.readEntry(
                "minimalShadeSelectorLineConfig", "0|0.2|0|0|0|0|0;1|0|1|1|0|0|0;2|0|-1|1|0|0|0;").split(";").at(0);
    fromString(lineset);

    updatePreview();
}

void KisShadeSelectorLineEditor::updatePreview(){
    m_line_preview->setParam(
                m_hueDelta->value(), m_saturationDelta->value(), m_valueDelta->value(),
                m_hueShift->value(), m_saturationShift->value(), m_valueShift->value()
                );
    this->parentWidget()->update(m_line_preview->geometry());
}

QString KisShadeSelectorLineEditor::toString() const
{
    return QString("%1|%2|%3|%4|%5|%6|%7")
        .arg(m_lineNumber)
        .arg(m_hueDelta->value())
        .arg(m_saturationDelta->value())
        .arg(m_valueDelta->value())
        .arg(m_hueShift->value())
        .arg(m_saturationShift->value())
        .arg(m_valueShift->value());
}

void KisShadeSelectorLineEditor::fromString(const QString &string)
{
    QStringList strili = string.split('|');
    m_lineNumber = strili.at(0).toInt();
    m_hueDelta->setValue(strili.at(1).toDouble());
    m_saturationDelta->setValue(strili.at(2).toDouble());
    m_valueDelta->setValue(strili.at(3).toDouble());
    if(strili.size()==4) return;            // don't crash, if reading old config files.
    m_hueShift->setValue(strili.at(4).toDouble());
    m_saturationShift->setValue(strili.at(5).toDouble());
    m_valueShift->setValue(strili.at(6).toDouble());
}

void KisShadeSelectorLineEditor::valueChanged() {
    updatePreview();
    emit requestActivateLine(this);
}

void KisShadeSelectorLineEditor::mousePressEvent(QMouseEvent* e) {
    e->accept();
}

