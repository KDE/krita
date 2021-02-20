/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_DLG_FILE_LAYER_H
#define KIS_DLG_FILE_LAYER_H

#include <KoDialog.h>
#include <QString>


#include <kis_file_layer.h>

#include "ui_wdgdlgfilelayer.h"

/**
 * Create a new file layer
 */
class KisDlgFileLayer : public KoDialog
{
public:

    Q_OBJECT

public:

    /**
     * Create a new file layer
     * @param basePath the base path of the layer
     * @param name the proposed name for this layer
     * @param parent the widget parent of this dialog
     */
    KisDlgFileLayer(const QString &basePath, const QString &name, QWidget *parent = 0);
    QString fileName() const;
    QString layerName() const;
    KisFileLayer::ScalingMethod scaleToImageResolution() const;

    void setFileName(QString fileName);
    void setScalingMethod(KisFileLayer::ScalingMethod method);

protected Q_SLOTS:
    void slotNameChanged(const QString &);

private:

    Ui_WdgDlgFileLayer dlgWidget;
    QString m_basePath;
};

#endif
