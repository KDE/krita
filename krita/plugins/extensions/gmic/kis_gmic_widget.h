/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _KIS_GMIC_WIDGET_H_
#define _KIS_GMIC_WIDGET_H_

#include <QTreeView>
#include <QGridLayout>
#include <QCheckBox>
#include "kis_gmic_filter_model.h"
#include "kis_gmic_filter_settings.h"

#include "ui_wdg_gmic.h"

class KisGmicUpdater;
class QCloseEvent;
class KisGmicInputOutputWidget;
class QPushButton;

class KisGmicWidget : public QWidget, public Ui::WdgGmic
{
    Q_OBJECT

public:
    // takes ownership of filter model
    KisGmicWidget(KisGmicFilterModel * filters, const QString &updateUrl = QString());
    ~KisGmicWidget();

    KisFilterPreviewWidget * previewWidget();

    void createMainLayout();
    virtual void closeEvent(QCloseEvent* );

signals:
    void sigFilterCurrentImage(KisGmicFilterSetting * setting); //TODO:const
    void sigPreviewFilterCommand(KisGmicFilterSetting * setting); //TODO:const
    void sigAcceptOnCanvasPreview();
    void sigCancelOnCanvasPreview();
    void sigPreviewActiveLayer();
    void sigClose();

private slots:
    void slotSelectedFilterChanged(const QItemSelection & newSelection, const QItemSelection & oldSelection);
    // buttons
    void slotApplyClicked();
    void slotOkClicked();
    void slotCancelClicked();
    void slotResetClicked();
    void slotMaximizeClicked();

    // internet updates slots
    void startUpdate();
    void finishUpdate();

    // preview
    void slotPreviewChanged(bool enabling);
    void slotPreviewSizeChanged();
    void slotConfigurationChanged();
    void slotNotImplemented();

private:
    KisGmicFilterSetting * currentFilterSettings();
    void requestComputePreview();
    void switchOptionsWidgetFor(QWidget * widget);



private:
    KisGmicFilterModel * m_filterModel;
    KisGmicUpdater * m_updater;

    QString m_updateUrl;

    int m_filterOptionsRow;
    int m_filterOptionsColumn;

    bool m_filterApplied;
    bool m_onCanvasPreviewActivated;
    bool m_onCanvasPreviewRequested;
};

#endif
