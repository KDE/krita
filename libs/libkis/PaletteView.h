/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef LIBKIS_PALETTE_VIEW_H
#define LIBKIS_PALETTE_VIEW_H

#include <QObject>
#include <QScopedPointer>
#include "kritalibkis_export.h"
#include "libkis.h"
#include "Palette.h"
#include "ManagedColor.h"
#include "KoColorSet.h"
#include <kis_palette_view.h>
#include <KisPaletteModel.h>

/**
 * @brief The PaletteView class is a wrapper around a MVC method for handling
 * palettes. This class shows a nice widget.
 */

class KRITALIBKIS_EXPORT PaletteView : public QWidget
{
    Q_OBJECT
public:
    PaletteView(QWidget *parent = 0);
    ~PaletteView();
public Q_SLOTS:
    void setPalette(Palette *palette);
    bool addEntryWithDialog(ManagedColor *color);
    bool addGroupWithDialog();
    bool removeSelectedEntryWithDialog();
    void trySelectClosestColor(ManagedColor *color);
Q_SIGNALS:
    void entrySelectedForeGround(KoColorSetEntry entry);
    void entrySelectedBackGround(KoColorSetEntry entry);
private:
    struct Private;
    const QScopedPointer<Private> d;

};

#endif // LIBKIS_PALETTE_VIEW_H
