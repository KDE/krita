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

#ifndef _KIS_TEXT_BRUSH_H_ 
#define _KIS_TEXT_BRUSH_H_

#include "kis_wdg_text_brush.h"
#include "kis_brush.h"

class KisTextBrushResource : public KisBrush
{
	public:
		KisTextBrushResource() : KisBrush("")
		{
			setBrushType(MASK);
		}
		KisTextBrushResource(const QString& txt, const QFont& font) : KisBrush("")
		{
			setFont(font);
			setText(txt);
			updateBrush();
			setBrushType(MASK);
		};
	public:
		virtual bool loadAsync() { return false; };
		void setText(const QString& txt) { m_txt = txt; };
		void setFont(const QFont& font) { m_font = font; };
		void updateBrush();
	private:
		QFont m_font;
		QString m_txt;
};

class KisTextBrush : public KisWdgTextBrush
{
	Q_OBJECT
public:
	KisTextBrush(QWidget *parent, const char* name, const QString& caption);
signals:
	void activatedResource(KisResource *r);
private slots:
	void rebuildTextBrush();
private:
	KisTextBrushResource* m_textBrushRessource;
};




#endif
