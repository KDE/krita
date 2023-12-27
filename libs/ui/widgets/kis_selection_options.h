/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SELECTION_OPTIONS_H__
#define __KIS_SELECTION_OPTIONS_H__

#include "kritaui_export.h"

#include <KisOptionCollectionWidget.h>
#include <KisSelectionTags.h>

class QKeySequence;

class KRITAUI_EXPORT KisSelectionOptions : public KisOptionCollectionWidget
{
    Q_OBJECT

public:
    enum ReferenceLayers { CurrentLayer, AllLayers, ColorLabeledLayers };

    KisSelectionOptions(QWidget *parent = nullptr);
    ~KisSelectionOptions() override;

    SelectionMode mode() const;
    SelectionAction action() const;
    bool antiAliasSelection() const;
    int growSelection() const;
    bool stopGrowingAtDarkestPixel() const;
    int featherSelection() const;
    ReferenceLayers referenceLayers() const;
    QList<int> selectedColorLabels() const;

    void setMode(SelectionMode newMode);
    void setAction(SelectionAction newAction);
    void setAntiAliasSelection(bool newAntiAliasSelection);
    void setGrowSelection(int newGrowSelection);
    void setStopGrowingAtDarkestPixel(bool newStopGrowingAtDarkestPixel);
    void setFeatherSelection(int newFeatherSelection);
    void setReferenceLayers(ReferenceLayers newReferenceLayers);
    void setSelectedColorLabels(const QList<int> &newSelectedColorLabels);

    void setModeSectionVisible(bool visible);
    void setActionSectionVisible(bool visible);
    void setAdjustmentsSectionVisible(bool visible);
    void setStopGrowingAtDarkestPixelButtonVisible(bool visible);
    void setReferenceSectionVisible(bool visible);

    void updateActionButtonToolTip(SelectionAction action,
                                   const QKeySequence &shortcut);

Q_SIGNALS:
    void modeChanged(SelectionMode mode);
    void actionChanged(SelectionAction action);
    void antiAliasSelectionChanged(bool antiAliasSelection);
    void growSelectionChanged(int growSelection);
    void stopGrowingAtDarkestPixelChanged(bool stopGrowingAtDarkestPixel);
    void featherSelectionChanged(int featherSelection);
    void referenceLayersChanged(ReferenceLayers referenceLayers);
    void selectedColorLabelsChanged();

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif

