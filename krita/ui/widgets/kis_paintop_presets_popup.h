/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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
#ifndef KIS_PAINTOP_PRESETS_POPUP_H
#define KIS_PAINTOP_PRESETS_POPUP_H

#include <QWidget>
#include <QList>
#include <KoID.h>
#include <kis_types.h>
#include <brushengine/kis_paintop_factory.h>

class QString;
class KisCanvasResourceProvider;
class KoResource;

/**
 * Popup widget for presets with built-in functionality
 * for adding and removing presets.
 */
class KisPaintOpPresetsPopup : public QWidget
{
    Q_OBJECT

public:

    KisPaintOpPresetsPopup(KisCanvasResourceProvider * resourceProvider, QWidget * parent = 0);

    ~KisPaintOpPresetsPopup();

    void setPaintOpSettingsWidget(QWidget * widget);

    /**
     * @return the name entered in the preset name lineedit
     */
    QString getPresetName() const;

    ///Image for preset preview
    ///@return image cut out from the scratchpad
    QImage cutOutOverlay();

    void setPaintOpList(const QList<KisPaintOpFactory*>& list);

    void setCurrentPaintOp(const QString & paintOpId);
    QString currentPaintOp();
    
    ///fill the cutoutOverlay rect with the cotent of an image, used to get the image back when selecting a preset
    ///@param image image that will be used, should be image of an existing preset resource
    void setPresetImage(const QImage& image);

    virtual void resizeEvent(QResizeEvent* );

    bool detached() const;

    void updateViewSettings();

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);

public Q_SLOTS:
    void slotWatchPresetNameLineEdit();
    void switchDetached(bool show = true);
    void hideScratchPad();
    void showScratchPad();
    void resourceSelected(KoResource* resource);
    void updateThemedIcons();

    void slotUpdateLodAvailability();

Q_SIGNALS:
    void savePresetClicked();
    void defaultPresetClicked();
    void paintopActivated(const QString& presetName);
    void signalResourceSelected(KoResource* resource);
    void reloadPresetClicked();
    void dirtyPresetToggled(bool value);
    void eraserBrushSizeToggled(bool value);
    
    void sizeChanged();

private Q_SLOTS:
    void slotSwitchPresetStrip(bool visible);
    void slotSwitchScratchpad(bool visible);

private:

    struct Private;
    Private * const m_d;

};

#endif
