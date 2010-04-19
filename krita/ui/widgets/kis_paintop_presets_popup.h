/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#include <KoID.h>
#include <kis_types.h>

class QString;
class KoResource;
class KisPaintOpPreset;
class KisPropertiesConfiguration;
class KisCanvasResourceProvider;

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

    ///Set id for paintop to be accept by the proxy model
    ///@param paintopID id of the paintop for which the presets will be shown
    void setPresetFilter(const KoID & paintopID);

    ///Set preset for the scratchpad
    ///@param preset that will be used in the scratchpad
    void setPreset(KisPaintOpPresetSP preset);
    
    ///Image for preset preview
    ///@return image cut out from the scratchpad
    QImage cutOutOverlay();

protected:

    void contextMenuEvent(QContextMenuEvent *);

public slots:
    void switchDetached();

signals:
    void savePresetClicked();
    void defaultPresetClicked();
    void resourceSelected( KoResource * resource );

private slots:
    void fillScratchPadGradient();
    void fillScratchPadSolid();
    void fillScratchPadLayer();

private:

    class Private;
    Private * const m_d;

};

#endif
