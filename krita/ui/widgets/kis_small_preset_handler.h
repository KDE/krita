/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#ifndef KIS_SMALL_PRESET_HANDLER_H
#define KIS_SMALL_PRESET_HANDLER_H

#include <QWidget>
#include "ui_wdgsmallpresethandler.h"

class KoResourceItemView;

class WdgSmallPresetHandler : public QWidget, public Ui::WdgSmallPresetHandler
{
    Q_OBJECT
    
public:
    WdgSmallPresetHandler(QWidget *parent);
    virtual ~WdgSmallPresetHandler();

    virtual void showEvent(QShowEvent *event);
    
public slots:
    void currentPaintopChanged(QString paintOpID);
    void startRefreshingTimer();
    void repaintDeleteButton();

private slots:
    /**
    * Properly position the delete button on the bottom right corner of the currently
    * selected preset item
    */
    void prepareDeleteButton();
    
    /**
    * Those button events scroll the preset strip to the left and to the right respectively.
    *
    * The form file (.ui) uses a different repeat speed for the left and the right scroll button:
    * those were adjusted to make both buttons feel similar in scroll speed when pressed.
    * Factors like the position of each icon in the strip affect how far it scrolls with each
    * event, thus needing fine tuning according to how are icons placed in the widget.
    */
    void on_leftScrollBtn_pressed();
    void on_rightScrollBtn_pressed();
    
    /**
    * This small button will hover over a preset when it is selected;
    * and will remove it when pressed
    */
    void on_deletePresetBtn_pressed();
    
private:
    KoResourceItemView* antiOOPHack;
    QTimer* refresher;
};


#endif // KIS_SMALL_PRESET_HANDLER_H
