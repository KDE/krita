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

#include <QFontMetrics>
#include <QPainter>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>

#include <kfontdialog.h>
#include <klineedit.h>

#include "kis_text_brush.h"

void KisTextBrushResource::updateBrush()
{
	QFontMetrics metric(m_font);
	int w = metric.width(m_txt);
	int h = metric.height();
	QPixmap px(w,h);
	QPainter p;
	p.begin(&px);
	p.setFont( m_font );
	p.fillRect(0,0, w, h, Qt::white);
	p.setPen(Qt::black);
	p.drawText(0, metric.ascent(), m_txt );
	p.end();
	setImage(px.toImage());
}

KisTextBrush::KisTextBrush(QWidget *parent, const char* name, const QString& caption)
	: KisWdgTextBrush(parent, name),
	  m_textBrushResource(new KisTextBrushResource())
{
	setWindowTitle(caption);
	connect((QObject*)lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)bnFont, SIGNAL(clicked()), this, SLOT(getFont()));
	m_font = font();
	rebuildTextBrush();
}


void KisTextBrush::getFont()
{
	KFontDialog::getFont( m_font, false/*, QWidget* parent! */ );
	rebuildTextBrush();
}

void KisTextBrush::rebuildTextBrush()
{
	lblFont->setText(QString(m_font.family() + ", %1").arg(m_font.pointSize()));
	lblFont->setFont(m_font);
	m_textBrushResource->setFont(m_font);
	m_textBrushResource->setText(lineEdit->text());
	m_textBrushResource->updateBrush();
	emit(activatedResource(m_textBrushResource));
}

#include "kis_text_brush.moc"
