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
 
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qcheckbox.h> 
#include <qlabel.h>

#include <klineedit.h>
#include <kfontcombo.h>

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
	setImage(px.convertToImage ());
}

KisTextBrush::KisTextBrush(QWidget *parent, const char* name, const QString& caption) 
	: KisWdgTextBrush(parent, name), 
	  m_textBrushResource(new KisTextBrushResource())
{
	setCaption(caption);
	// XXX: Did I remove the wrong line here? BSAR
 	connect((QObject*)lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)cmbWeight, SIGNAL(activated(int)), this, SLOT(rebuildTextBrush()));
// 	connect((QObject*)buttonGroupBold, SIGNAL(clicked(int)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)checkBoxItalic, SIGNAL(toggled(bool)), this, SLOT(rebuildTextBrush()));
	connect((QObject*)spinBoxSize, SIGNAL(valueChanged(int)), this, SLOT(rebuildTextBrush()));
	fontCombo->setCurrentItem (0);
	
}

void KisTextBrush::rebuildTextBrush()
{
// 	kdDebug() << "KisTextBrush::rebuildTextBrush Font = " << fontCombo->currentFont() << " size = " << spinBoxSize->value() << " boldness = " << spinBoxCustomBoldness->value() << endl;

	if( cmbWeight -> currentItem() == 5)
	{
		textLabelCustom -> setEnabled(true);
		spinBoxCustomBoldness->setEnabled(true);
	} else {
		textLabelCustom->setEnabled(false);
		spinBoxCustomBoldness->setEnabled(false);

		if ( cmbWeight -> currentItem() == 0 )
		{
			spinBoxCustomBoldness->setValue(25);
		}
		else if ( cmbWeight -> currentItem() == 1 )
		{
			spinBoxCustomBoldness->setValue(50);
		}
		else if ( cmbWeight -> currentItem() == 2 )
		{
			spinBoxCustomBoldness->setValue(63);
		}
		else if ( cmbWeight -> currentItem() == 3 )
		{
			spinBoxCustomBoldness->setValue(75);
		}
		else if ( cmbWeight -> currentItem() == 4 )
		{
			spinBoxCustomBoldness->setValue(87);
		}
    }

	QFont font(fontCombo->currentText(), spinBoxSize->value(), spinBoxCustomBoldness->value(), checkBoxItalic->isChecked());
	lineEdit->setFont(font);
	m_textBrushResource->setFont(font);
	m_textBrushResource->setText(lineEdit->text());
	m_textBrushResource->updateBrush();
	emit(activatedResource(m_textBrushResource ));
}
