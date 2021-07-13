/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_BRUSH_SELECTION_WIDGET_H
#define KIS_BRUSH_SELECTION_WIDGET_H

#include <QWidget>

#include <KoGroupButton.h>

#include <kis_properties_configuration.h>
#include <kis_brush.h>

#include "kis_precision_option.h"
#include "ui_wdgbrushchooser.h"

class KisAutoBrushWidget;
class KisPredefinedBrushChooser;
class KisTextBrushChooser;
class KisCustomBrushWidget;
class KisClipboardBrushWidget;
class KisBrush;


/**
 *Compound widget that collects all the various brush selection widgets.
 */
class PAINTOP_EXPORT KisBrushSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisBrushSelectionWidget(QWidget *parent = 0);

    ~KisBrushSelectionWidget() override;

    KisBrushSP brush() const;

    void setImage(KisImageWSP image);

    void setCurrentBrush(KisBrushSP brush);

    void writeOptionSetting(KisPropertiesConfigurationSP settings) const;
    void readOptionSetting(const KisPropertiesConfigurationSP setting);

    void setPrecisionEnabled(bool value);
    bool autoPrecisionEnabled();

    void hideOptions(const QStringList &options);

    void setHSLBrushTipEnabled(bool value);
    bool hslBrushTipEnabled() const;


Q_SIGNALS:

    void sigBrushChanged();
    void sigPrecisionChanged();

private Q_SLOTS:
    void buttonClicked(int id);
    void precisionChanged(int value);
    void setAutoPrecisionEnabled(int value);

private:
    void setCurrentWidget(QWidget *widget);
    void addChooser(const QString & text, QWidget *widget, int id, KoGroupButton::GroupPosition pos);

private:
    enum Type {
        AUTOBRUSH,
        PREDEFINEDBRUSH,
        TEXTBRUSH,
    };

    Ui_WdgBrushChooser uiWdgBrushChooser;
    QGridLayout *m_layout {0};
    QWidget *m_currentBrushWidget {0};
    QHash<int, QWidget*> m_chooserMap;
    QButtonGroup *m_buttonGroup {0};
    QSize m_mininmumSize;

    KisAutoBrushWidget *m_autoBrushWidget {0};
    KisPredefinedBrushChooser *m_predefinedBrushWidget {0};
    KisTextBrushChooser *m_textBrushWidget {0};

    KisPrecisionOption m_precisionOption;
};

#endif
