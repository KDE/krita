/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
