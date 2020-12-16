/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_DLG_GENERATORLAYER_H
#define KIS_DLG_GENERATORLAYER_H

#include <QDialog>
#include <QString>

class KisFilterConfiguration;
class KisViewManager;

#include "ui_wdgdlggeneratorlayer.h"
#include <generator/kis_generator.h>
#include <kis_thread_safe_signal_compressor.h>

/**
 * Create a new generator layer
 */
class KisDlgGeneratorLayer : public QDialog
{
public:

    Q_OBJECT

public:

    /**
     * Create a new generator layer
     * @param defaultLayerName the proposed name for this layer
     * @param arg_view the view manager
     * @param parent the widget parent of this dialog
     * @param glayer optional generator layer for editing
     * @param previousConfig optional configuration of layer being edited.
     */
    KisDlgGeneratorLayer(const QString &defaultLayerName, KisViewManager *arg_view, QWidget *parent, KisGeneratorLayerSP glayer, const KisFilterConfigurationSP previousConfig, const KisStrokeId stroke);
    ~KisDlgGeneratorLayer() override;

    void setConfiguration(const KisFilterConfigurationSP  config);
    KisFilterConfigurationSP  configuration() const;
    QString layerName() const;

protected Q_SLOTS:
    void slotNameChanged(const QString &);
    void previewGenerator();
    void slotDelayedPreviewGenerator();
    void saveLayer();
    void restoreLayer();

private:
    Ui_WdgDlgGeneratorLayer dlgWidget;
    KisGeneratorLayerSP layer;
    KisFilterConfigurationSP configBefore;
    KisViewManager *m_view;
    bool isEditing;

    bool m_customName;
    bool m_freezeName;
    KisStrokeId m_stroke;
    KisThreadSafeSignalCompressor m_compressor;
};

#endif
