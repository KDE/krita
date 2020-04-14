/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_SELECTION_OPTIONS_H__
#define __KIS_SELECTION_OPTIONS_H__

#include <QWidget>
#include <QList>

#include "kritaui_export.h"

#include "ui_wdgselectionoptions.h"
#include "kis_image.h"
#include "kis_signal_compressor.h"
#include "kis_signal_auto_connection.h"

class KisCanvas2;
class QButtonGroup;
class QKeySequence;
class KisSignalCompressor;

class WdgSelectionOptions : public QWidget, public Ui::WdgSelectionOptions
{
    Q_OBJECT

public:
    WdgSelectionOptions(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 */
class KRITAUI_EXPORT KisSelectionOptions : public QWidget
{

    Q_OBJECT

public:
    KisSelectionOptions(KisCanvas2 * subject);
    ~KisSelectionOptions() override;

public:
    const QString SAMPLE_LAYERS_MODE_ALL = "sampleAllLayers";
    const QString SAMPLE_LAYERS_MODE_COLOR_LABELED = "sampleColorLabeledLayers";
    const QString SAMPLE_LAYERS_MODE_CURRENT = "sampleCurrentLayer";

public:
    int action();

    bool antiAliasSelection();
    QList<int> colorLabelsSelected();
    QString sampleLayersMode();

    void disableAntiAliasSelectionOption();
    void disableSelectionModeOption();

    void setAction(int);
    void setMode(int);
    void setAntiAliasSelection(bool value);
    void setSampleLayersMode(QString mode);

    void enablePixelOnlySelectionMode();
    void setColorLabelsEnabled(bool enabled);

    void updateActionButtonToolTip(int action, const QKeySequence &shortcut);

    void attachToImage(KisImageSP image, KisCanvas2* canvas);

    void activateConnectionToImage();
    void deactivateConnectionToImage();



Q_SIGNALS:
    void actionChanged(int);
    void modeChanged(int);
    void antiAliasSelectionChanged(bool);
    void selectedColorLabelsChanged();
    void sampleLayersModeChanged(QString mode);

private Q_SLOTS:
    void hideActionsForSelectionMode(int mode);
    void slotUpdateAvailableColorLabels();
    void slotSampleLayersModeChanged(int index);

private:
    QString sampleLayerModeToUserString(QString sampleLayersMode);
    void setCmbSampleLayersMode(QString sampleLayersModeId);

private:
    WdgSelectionOptions * m_page;
    QButtonGroup* m_mode;
    QButtonGroup* m_action;
    KisSignalCompressor m_colorLabelsCompressor;
    KisImageSP m_image;
    KisCanvas2* m_canvas;
    QString m_toolId;
    KisSignalAutoConnectionsStore m_nodesUpdatesConnectionsStore;
};

#endif

