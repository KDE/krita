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

class KisResourceItemListView;

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
    ~KisPresetSelectorStrip() override;

    void setPresetFilter(const QString& paintOpId);

    int iconSize();
    void setIconSize(int size);

public Q_SLOTS:

    /// saving the icon base size. This affects all preset selectors
    /// outside UI elements adjusting icon size
    void slotSetIconSize(int size);

    /// saves the icon size to the config file
    /// when UI element is released, it is ok to save icon size to config
    void slotSaveIconSize();

private Q_SLOTS:
    /// Scrolls the strip's item view to the left
    void on_leftScrollBtn_pressed();
    
    /// Scrolls the strip's item view to the right
    void on_rightScrollBtn_pressed();

    /// Changes the preset list view type
    void slotThumbnailMode();
    void slotDetailMode();

private:
    /**
    * This is a workaround to access members of KisPresetChooser using the QObject tree
    * instead of class methods
    */
    KisResourceItemListView* m_resourceItemView;
    QString m_currentPaintopID;
};


#endif // KIS_PRESET_SELECTOR_STRIP_H
