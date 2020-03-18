/* This file is part of the KDE project
 * Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
 * Copyright 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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
#include <kritaui_export.h>
#include <brushengine/kis_paintop_preset.h>

class KoResource;

class KRITAUI_EXPORT KisPaintOpPresetsChooserPopup : public QWidget
{
    Q_OBJECT
public:
    KisPaintOpPresetsChooserPopup(QWidget * parent = 0);
    ~KisPaintOpPresetsChooserPopup() override;
    
    void showButtons(bool show);
    void updateViewSettings();
public Q_SLOTS:
    void canvasResourceChanged(KisPaintOpPresetSP  preset);
    void slotThemeChanged();

Q_SIGNALS:
    void resourceSelected(KoResourceSP resource);
    void resourceClicked(KoResourceSP resource);
    
private Q_SLOTS:
    void slotThumbnailMode();
    void slotDetailMode();
    void paintEvent(QPaintEvent *) override;
   
private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_PAINTOP_PRESETS_CHOOSER_POPUP_H
