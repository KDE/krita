/*
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_TOOL_TEXT_H_
#define KIS_TOOL_TEXT_H_

#include "kis_tool_paint.h"

class QFont;
class QLabel;
class QWidget;
class QPushButton;
class KSqueezedTextLabel;

class KisToolText : public KisToolPaint {
	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolText();
	virtual ~KisToolText();

	virtual void update(KisCanvasSubject *subject);
	virtual void setup(KActionCollection *collection);
	virtual void buttonRelease(KisButtonReleaseEvent *e);

	virtual QWidget* optionWidget();
	virtual QWidget* createoptionWidget(QWidget* parent);
public slots:
	virtual void setFont();

private:
	KisCanvasSubject *m_subject;
	QFont m_font;
	QLabel *m_lbFont;
	KSqueezedTextLabel *m_lbFontName;
	QPushButton *m_btnMoreFonts;
	QWidget *m_optWidget;
};

#endif // KIS_TOOL_TEXT_H_
