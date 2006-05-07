/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_PALETTE_
#define _KO_PALETTE_

#include <QDockWidget>
#include <QFont>

#include <koffice_export.h>

#include "kopalettemanager.h"

/**
 * A floating palette that allows the adding and removing of widgets
 * to its organzing principle.
 *
 * There is currently no titlebar with a shade button; I hope to be
 * able to use QDockWidget's toggle view action for that.
 */
class KOPALETTE_EXPORT KoPalette : public QDockWidget {

Q_OBJECT

public:

    KoPalette(QWidget * parent, const char * name);
    virtual ~KoPalette();

public:

    virtual void resetFont();
    
    void setStyle(enumKoPaletteStyle style) { m_style = style; };
    enumKoPaletteStyle style() const { return m_style; };

    virtual void plug(QWidget * widget, const QString & name, int position) = 0;
    virtual void unplug(const QWidget * widget) = 0;
    virtual void showPage(QWidget *w) = 0;
    virtual void togglePageHidden(QWidget *w) = 0;
    virtual void hidePage(QWidget * w) = 0;
    virtual void makeVisible(bool v) = 0;
    virtual bool isHidden(QWidget * w) = 0;
    virtual int indexOf(QWidget *w) = 0;

protected:

    virtual void setMainWidget(QWidget * widget);
    QFont m_font;
    enumKoPaletteStyle m_style;
    
private:
    QWidget * m_page;
    
};

#endif //_KO_PALETTE_
