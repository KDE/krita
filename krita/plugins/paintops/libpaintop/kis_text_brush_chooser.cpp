/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
    m_font = font();
    rebuildTextBrush();
    showSlider(inputSpacing);
    connect((QObject*)inputSpacing, SIGNAL(valueChanged(double)), this, SLOT(rebuildTextBrush()));
}


void KisTextBrushChooser::getFont()
{
    KFontDialog::getFont(m_font, false/*, QWidget* parent! */);
    rebuildTextBrush();
}

void KisTextBrushChooser::rebuildTextBrush()
{
    lblFont->setText(QString(m_font.family() + ", %1").arg(m_font.pointSize()));
    lblFont->setFont(m_font);
    KisTextBrush* textBrush = dynamic_cast<KisTextBrush*>(m_textBrush.data());
    textBrush->setFont(m_font);
    textBrush->setText(lineEdit->text());
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
