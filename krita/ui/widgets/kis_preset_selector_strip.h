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

#ifndef KIS_PRESET_SELECTOR_STRIP_H
#define KIS_PRESET_SELECTOR_STRIP_H

#include <QWidget>
#include "ui_wdgpresetselectorstrip.h"

class KoResourceItemView;

/**
*
* KisPresetSelectorStrip is a composite widget around KisPresetChooser. It provides
* a strip of icons with two scroll buttons at the sides and a small delete button 
* that appears when a user selects a preset icon.
*
* KisPresetSelectorStrip makes it possible to quickly select and modify presets.
*
* Note that KisPresetSelectorStrip uses the QObject tree to access properties of the contained
* classes, and uses heuristics to approximate pixel offsets, times, and other
* properties that cannot be accessed through the QObject tree.
*
*/
class KisPresetSelectorStrip : public QWidget, public Ui::WdgPresetSelectorStrip
{
    Q_OBJECT
    
public:
    KisPresetSelectorStrip(QWidget *parent);
    virtual ~KisPresetSelectorStrip();

    virtual void showEvent(QShowEvent *event);
    
public slots:
    /// Updates the contents of the strip item view, show only presets for @param paintOpID
    void currentPaintopChanged(QString paintOpID);
    
    /**
    * startRefreshingTimer() and repaintDeleteButton() are heuristics
    * to reposition the delete button when the strip auto-scrolls 
    * to show a selected item that's partly outside the item view
    */
    void startRefreshingTimer();
    void repaintDeleteButton();

private slots:
    /**
    * Properly position the delete button on the bottom right corner of the currently
    * selected preset item
    */
    void prepareDeleteButton();
    
    /// Scrolls the strip's item view to the left
    void on_leftScrollBtn_pressed();
    
    /// Scrolls the strip's item view to the right
    void on_rightScrollBtn_pressed();
    
    /**
    * This small button will hover over a preset when it is selected;
    * and will remove the preset when clicked
    */
    void on_deletePresetBtn_clicked();
    
private:
    /**
    * This is a workaround to access members of KisPresetChooser using the QObject tree
    * instead of class methods
    */
    KoResourceItemView* m_resourceItemView;
    
    QTimer* m_refresher;
};


#endif // KIS_PRESET_SELECTOR_STRIP_H
