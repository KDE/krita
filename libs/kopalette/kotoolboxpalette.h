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

#ifndef _KO_TOOLBOX_PALETTE_
#define _KO_TOOLBOX_PALETTE_

#include <limits.h>

#include <qtoolbox.h>

#include <kopalettemanager.h>

/**
 * A palette based on a toolbox widget. This does not support drag and drop
 * configuration of palette widgets
 */
class KoToolBoxPalette : public KoPalette {

Q_OBJECT

public:

    KoToolBoxPalette(QWidget * parent, const char * name);
    ~KoToolBoxPalette();

public:
    void resetFont();
    virtual void plug(QWidget * widget, const QString & name, int position = INT_MAX);
    virtual void unplug(const QWidget * widget);
    void showPage(QWidget *w);
    void makeVisible(bool v);
    virtual void hidePage(QWidget * w);
    int indexOf(QWidget *w);
    bool isHidden(QWidget *w);
    void togglePageHidden(QWidget *w);
private:

    QToolBox * m_page;

    QMap<QWidget*, int> m_hiddenPages;
};

#endif //_KO_TOOLBOX_PALETTE_
