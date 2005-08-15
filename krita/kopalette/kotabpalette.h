/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KO_TAB_PALETTE_
#define _KO_TAB_PALETTE_

#include <qdockwindow.h>

#include "kopalette.h"

class KoView;
class QWidget;
class QTabWidget;

/**
 * This is a palette with a tabwidget. It supports
 * reorganzing the pages and moving the pages to other
 * palettes with drag and drop,
 *
 */
class KoTabPalette : public KoPalette {

Q_OBJECT

public:

    KoTabPalette(KoView * parent, const char * name);
    virtual ~KoTabPalette();

public:

    virtual void plug(QWidget * widget, const QString & name, int position = -1);
    virtual void unplug(const QWidget * widget);
        void showPage(QWidget *w);
        void makeVisible(bool v);
        int indexOf(QWidget *w);
    bool isHidden(QWidget *w);
        virtual void togglePageHidden(QWidget *w);

private:

    QTabWidget * m_page;

    QMap<QWidget*, int> m_hiddenPages;
};

#endif //_KO_TAB_PALETTE_
