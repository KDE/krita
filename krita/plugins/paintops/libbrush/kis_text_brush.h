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

#ifndef _KIS_TEXT_BRUSH_H_
#define _KIS_TEXT_BRUSH_H_

#include <QFont>

#include "kis_brush.h"
#include "krita_export.h"

class BRUSH_EXPORT KisTextBrush : public KisBrush
{

public:

    KisTextBrush() {
        setBrushType(MASK);
    }

    KisTextBrush(const QString& txt, const QFont& font) {
        setFont(font);
        setText(txt);
        updateBrush();
        setBrushType(MASK);
    }

    virtual bool load() {
        return false;
    }

    void setText(const QString& txt) {
        m_txt = txt;
    }

    void setFont(const QFont& font) {
        m_font = font;
    }

    QFont font() {

        return m_font;
    }

    void updateBrush();

    void toXML(QDomDocument& , QDomElement&) const;

private:

    QFont m_font;
    QString m_txt;
};

#endif
