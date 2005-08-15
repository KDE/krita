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

#ifndef _KO_PALETTE_
#define _KO_PALETTE_

#include <qdockwindow.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qfont.h>
#include <qlayout.h>

#include <koView.h>

/**
 * A floating palette that allows the adding and removing of widgets
 * to its organzing principle.
 *
 * There is currently no titlebar with a shade button; I hope to be
 * able to use QDockWidget's toggle view action for that.
 */
class KoPalette : public QDockWindow {

Q_OBJECT

public:

    KoPalette(KoView * parent, const char * name);
    virtual ~KoPalette();

public:

    virtual void plug(QWidget * widget, const QString & name, int position) = 0;
    virtual void unplug(const QWidget * widget) = 0;
        virtual void showPage(QWidget *w) = 0;
        virtual void togglePageHidden(QWidget *w) = 0;
        virtual void makeVisible(bool v) = 0;
    virtual bool isHidden(QWidget * w) = 0;
        virtual int indexOf(QWidget *w) = 0;

    void setCaption(const QString & caption);

public slots:

    void slotShade(bool toggle);
    void slotPlaceChanged(QDockWindow::Place p);
    


protected:

    virtual void setMainWidget(QWidget * widget);
    QFont m_font;
    
private:
    QGridLayout * m_wdgDockerTabLayout;
    QLabel* m_lblCaption;
    QToolButton* m_bnShade;
    QPixmap m_pixShadeButton;
    QWidget * m_page;
    bool m_docked;
    bool m_shaded;

};

#endif //_KO_PALETTE_
