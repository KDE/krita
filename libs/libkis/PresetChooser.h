/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef PRESETCHOOSER_H
#define PRESETCHOOSER_H

#include <QObject>
#include <QWidget>

#include <kis_preset_chooser.h>

#include "kritalibkis_export.h"
#include "libkis.h"

class Resource;

/**
 * @brief The PresetChooser widget provides
 */
class KRITALIBKIS_EXPORT PresetChooser : public KisPresetChooser
{
    Q_OBJECT
public:
    PresetChooser(QWidget *parent = 0);
    virtual ~PresetChooser() {}

public Q_SLOTS:

    void setCurrentPreset(Resource *resource);
    Resource *currentPreset() const;

Q_SIGNALS:

    void presetSelected(Resource *resource);
    void presetClicked(Resource *resource);

private Q_SLOTS:

    void slotResourceSelected(KoResource *resource);
    void slotResourceClicked(KoResource *resource);
};

#endif // PRESETCHOOSER_H
