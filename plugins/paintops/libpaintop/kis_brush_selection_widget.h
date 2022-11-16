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

#include <lager/cursor.hpp>
#include <KisBrushModel.h>
#include <KisBrushOptionWidgetFlags.h>

class KisAutoBrushWidget;
class KisPredefinedBrushChooser;
class KisTextBrushChooser;
class KisCustomBrushWidget;
class KisClipboardBrushWidget;
class KisBrush;
class QStackedWidget;
class KisAutoBrushModel;
class KisPredefinedBrushModel;
class KisTextBrushModel;

/**
 *Compound widget that collects all the various brush selection widgets.
 */
class PAINTOP_EXPORT KisBrushSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisBrushSelectionWidget(int maxBrushSize,
                            KisAutoBrushModel *autoBrushModel,
                            KisPredefinedBrushModel *predefinedBrushModel,
                            KisTextBrushModel *textBrushModel,
                            lager::cursor<KisBrushModel::BrushType> brushType,
                            lager::cursor<KisBrushModel::PrecisionData> precisionData,
                            KisBrushOptionWidgetFlags flags,
                            QWidget *parent = 0);

    ~KisBrushSelectionWidget() override;

    KisBrushSP brush() const;

    void setImage(KisImageWSP image);
    void hideOptions(const QStringList &options);

    lager::reader<bool> lightnessModeEnabled() const;

Q_SIGNALS:

    void sigBrushChanged();
    void sigPrecisionChanged();

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
    QSize m_minimumSize;
    QStackedWidget *m_stackedWidget{0};

    KisAutoBrushWidget *m_autoBrushWidget {0};
    KisPredefinedBrushChooser *m_predefinedBrushWidget {0};
    KisTextBrushChooser *m_textBrushWidget {0};

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
