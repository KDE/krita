/* This file is part of the KDE project
 * Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PAINTOP_PRESETS_CHOOSER_POPUP_H
#define KIS_PAINTOP_PRESETS_CHOOSER_POPUP_H

#include <QWidget>
#include <KoID.h>

class KoResource;

class KisPaintOpPresetsChooserPopup : public QWidget
{
    Q_OBJECT
public:
    KisPaintOpPresetsChooserPopup(QWidget * parent = 0);
    virtual ~KisPaintOpPresetsChooserPopup();
    
    ///Set id for paintop to be accept by the proxy model
    ///@param paintopID id of the paintop for which the presets will be shown
    void setPresetFilter(const KoID & paintopID);

signals:
    void resourceSelected( KoResource * resource );
    
private slots:
    void slotThumbnailMode();
    void slotDetailMode();
   
private:

    class Private;
    Private * const m_d;
};

#endif // KIS_PAINTOP_PRESETS_CHOOSER_POPUP_H
