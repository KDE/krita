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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
#include "kis_text_brush.h"
#include <qfontmetrics.h>
#include <qpainter.h>
#include <kfontcombo.h>
#include <qspinbox.h>
#include <qcheckbox.h> 
#include <klineedit.h>

void KisTextBrushResource::updateBrush()
{
	QFontMetrics metric(m_font);
	int w = metric.width(m_txt);
	int h = metric.height();
// 	kdDebug() << "KisTextBrushResource::updateBrush : m_txt = " << m_txt << " width = " << w << " height = " << h << " font name = " << m_font.family() << endl;
	QPixmap px(w,h);
	QPainter p;
	p.begin(&px);
	p.setFont( m_font );
	p.fillRect(0,0, w, h, Qt::white);
	p.setPen(Qt::black);
	p.drawText(0, metric.ascent(), m_txt );
	p.end();
	setImage(px.convertToImage ());
}

KisTextBrush::KisTextBrush(QWidget *parent, const char* name, const QString& caption) : KisWdgTextBrush(parent, name), m_textBrushRessource(new KisTextBrushResource())
{
	setCaption(caption);
 	connect((QObject*)lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)spinBoxCustomBoldness, SIGNAL(valueChanged(int)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)buttonGroupBold, SIGNAL(clicked(int)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)checkBoxItalic, SIGNAL(toggled(bool)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)spinBoxSize, SIGNAL(valueChanged(int)), this, SLOT(rebuildTextBrush()));
	fontCombo->setCurrentItem (0);
	
}

void KisTextBrush::rebuildTextBrush()
{
// 	kdDebug() << "KisTextBrush::rebuildTextBrush Font = " << fontCombo->currentFont() << " size = " << spinBoxSize->value() << " boldness = " << spinBoxCustomBoldness->value() << endl;
	QFont font(fontCombo->currentText(), spinBoxSize->value(), spinBoxCustomBoldness->value(), checkBoxItalic->isChecked());
	lineEdit->setFont(font);
	m_textBrushRessource->setFont(font);
	m_textBrushRessource->setText(lineEdit->text());
	m_textBrushRessource->updateBrush();
	emit(activatedResource(m_textBrushRessource ));
}
