/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_text_brush_chooser.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QString>
#include <QFontDialog>

#define showSlider(input) input->setRange(input->minimum(), input->maximum())

KisTextBrushChooser::KisTextBrushChooser(QWidget *parent, const char* name, const QString& caption)
    : QWidget(parent)
    , m_textBrush(new KisTextBrush())
{
    setObjectName(name);
    setupUi(this);

    setWindowTitle(caption);
    connect((QObject*)lineEdit, SIGNAL(textChanged(QString)), this, SLOT(rebuildTextBrush()));
    connect((QObject*)bnFont, SIGNAL(clicked()), this, SLOT(getFont()));
    connect(pipeModeChbox, SIGNAL(toggled(bool)), this, SLOT(rebuildTextBrush()));
    m_font = font();
    inputSpacing->setRange(0.0, 10, 2);
    inputSpacing->setValue(0.1);
    rebuildTextBrush();
    connect(inputSpacing, SIGNAL(valueChanged(qreal)), this, SLOT(rebuildTextBrush()));
}


void KisTextBrushChooser::getFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_font);
    if (ok) {
        m_font = f;
        rebuildTextBrush();
    }
}

void KisTextBrushChooser::rebuildTextBrush()
{
    pipeModeChbox->setEnabled(!lineEdit->text().isEmpty());
    if (lineEdit->text().isEmpty()) {
        pipeModeChbox->setChecked(false);
    }

    lblFont->setText(QString(m_font.family() + ", %1").arg(m_font.pointSize()));
    lblFont->setFont(m_font);

    KisTextBrush* textBrush = dynamic_cast<KisTextBrush*>(m_textBrush.data());
    textBrush->setFont(m_font);
    textBrush->setText(lineEdit->text());
    textBrush->setPipeMode(pipeModeChbox->isChecked());
    textBrush->setSpacing(inputSpacing->value());
    textBrush->updateBrush();

    emit sigBrushChanged();
}

void KisTextBrushChooser::setBrush(KisBrushSP brush)
{
    bool b;

    m_textBrush = brush;

    KisTextBrush *textBrush = dynamic_cast<KisTextBrush*>(brush.data());

    m_font = textBrush->font();

    // we want to set all the gui widgets without triggering any signals
    // (and thus calling rebuildTextBrush)
    b = lineEdit->blockSignals(true);
    lineEdit->setText(textBrush->text());
    lineEdit->blockSignals(b);

    b = pipeModeChbox->blockSignals(true);
    pipeModeChbox->setChecked(textBrush->pipeMode());
    pipeModeChbox->blockSignals(b);

    // trigger rebuildTextBrush on the last change
    inputSpacing->setValue(textBrush->spacing());
}

