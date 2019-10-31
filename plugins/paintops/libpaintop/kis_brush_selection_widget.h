/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Mohit Goyal    <mohit.bits2011@gmail.com>
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
 * Compound widget that collects all the various brush selection widgets.
 */
class PAINTOP_EXPORT KisBrushSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisBrushSelectionWidget(QWidget * parent = 0);

    ~KisBrushSelectionWidget() override;

    KisBrushSP brush() const;

    void setAutoBrush(bool on);
    void setPredefinedBrushes(bool on);
    void setCustomBrush(bool on);
    void setClipboardBrush(bool on);
    void setTextBrush(bool on);

    void setImage(KisImageWSP image);

    void setCurrentBrush(KisBrushSP brush);

    bool presetIsValid() {
        return m_presetIsValid;
    }

    void writeOptionSetting(KisPropertiesConfigurationSP settings) const;
    void readOptionSetting(const KisPropertiesConfigurationSP setting);

    void setPrecisionEnabled(bool value);
    bool autoPrecisionEnabled();

    void hideOptions(const QStringList &options);

Q_SIGNALS:

    void sigBrushChanged();
    void sigPrecisionChanged();

private Q_SLOTS:
    void buttonClicked(int id);
    void precisionChanged(int value);
    void setAutoPrecisionEnabled(int value);

private:
    void setCurrentWidget(QWidget * widget);
    void addChooser(const QString & text, QWidget * widget, int id, KoGroupButton::GroupPosition pos);

private:
    enum Type {
        AUTOBRUSH,
        PREDEFINEDBRUSH,
        CUSTOMBRUSH,
        TEXTBRUSH,
        CLIPBOARDBRUSH
    };

    bool m_presetIsValid;

    Ui_WdgBrushChooser uiWdgBrushChooser;
    QGridLayout * m_layout;
    QWidget * m_currentBrushWidget;
    QHash<int, QWidget*> m_chooserMap;
    QButtonGroup * m_buttonGroup;
    QSize m_mininmumSize;

    KisAutoBrushWidget * m_autoBrushWidget;
    KisPredefinedBrushChooser * m_predefinedBrushWidget;
    KisTextBrushChooser * m_textBrushWidget;

    KisPrecisionOption m_precisionOption;
};

#endif
