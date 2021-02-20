/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
 * @brief The PresetChooser widget wraps the KisPresetChooser widget.
 * The widget provides for selecting brush presets. It has a tagging
 * bar and a filter field. It is not automatically synchronized with 
 * the currently selected preset in the current Windows.
 */
class KRITALIBKIS_EXPORT PresetChooser : public KisPresetChooser
{
    Q_OBJECT
public:
    PresetChooser(QWidget *parent = 0);
    ~PresetChooser() override {}

public Q_SLOTS:

    /**
     * Make the given preset active.
     */
    void setCurrentPreset(Resource *resource);
    
    /**
     * @return a Resource wrapper around the currently selected
     * preset. 
     */
    Resource *currentPreset() const;

Q_SIGNALS:

    /**
     * Emitted whenever a user selects the given preset.
     */
    void presetSelected(Resource resource);
    
    /**
     * Emitted whenever a user clicks on the given preset.
     */
    void presetClicked(Resource resource);

private Q_SLOTS:

    void slotResourceSelected(KoResourceSP resource);
    void slotResourceClicked(KoResourceSP resource);
};

#endif // PRESETCHOOSER_H
