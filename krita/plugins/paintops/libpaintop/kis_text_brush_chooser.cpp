/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_text_brush_chooser.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QString>

#include <kfontdialog.h>
#include <klineedit.h>

#define showSlider(input) input->setRange(input->minimum(), input->maximum())

KisTextBrushChooser::KisTextBrushChooser(QWidget *parent, const char* name, const QString& caption)
        : QWidget(parent)
        , m_textBrush(new KisTextBrush())
{
    setObjectName(name);
    setupUi(this);

    setWindowTitle(caption);
    connect((QObject*)lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(rebuildTextBrush()));
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
    KFontDialog::getFont(m_font);
    rebuildTextBrush();
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
    m_textBrush = brush;
    m_font = static_cast<KisTextBrush*>(brush.data())->font();
}

#include "kis_text_brush_chooser.moc"
