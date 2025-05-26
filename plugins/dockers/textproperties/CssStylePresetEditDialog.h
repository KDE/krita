/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CSSSTYLEPRESETEDITDIALOG_H
#define CSSSTYLEPRESETEDITDIALOG_H

#include <QObject>
#include <KoDialog.h>
#include <resources/KoCssStylePreset.h>
#include <QQuickWidget>
#include <lager/KoSvgTextPropertiesModel.h>

class CssStylePresetEditDialog : public KoDialog
{
    Q_OBJECT
public:
    CssStylePresetEditDialog(QWidget *parent = nullptr);
    ~CssStylePresetEditDialog();

    void setCurrentResource(KoCssStylePresetSP resource);
    KoCssStylePresetSP currentResource();

public Q_SLOTS:
    void slotUpdateTextProperties();
    QColor modalColorDialog(QColor oldColor);
    QString wwsFontFamilyName(QString familyName);

private:
    QQuickWidget *m_quickWidget {0};
    KoSvgTextPropertiesModel *m_model;
    KoCssStylePresetSP m_currentResource;
};

#endif // CSSSTYLEPRESETEDITDIALOG_H
