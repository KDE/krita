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

#ifndef KIS_INPUT_OUTPUT_WIDGET
#define KIS_INPUT_OUTPUT_WIDGET

#include <QWidget>
#include <QHash>
#include "kis_gmic_filter_settings.h"

#include "ui_wdg_gmic_input_output.h"

class QStringListModel;

/**
 * creates GUI for Input/Output configuration
 */
class KisGmicInputOutputWidget : public QWidget, public Ui::WdgGmicInputOutput
{
    Q_OBJECT

public:
    KisGmicInputOutputWidget(QWidget * parent);
    ~KisGmicInputOutputWidget();

    InputLayerMode inputMode() const { return m_inputMode; };
    OutputMode outputMode() const { return m_outputMode; };

    OutputPreviewMode previewMode() const { return m_previewMode; };
    PreviewSize previewSize() const { return m_previewSize; };

    KisFilterPreviewWidget * previewWidget();

Q_SIGNALS:
    void sigConfigurationChanged();

private:
    void createMainLayout();

private Q_SLOTS:
    void setIntputMode(int index);
    void setOutputMode(int index);
    void setPreviewMode(int index);
    void setPreviewSize(int index);

private:
    InputLayerMode m_inputMode;
    OutputMode m_outputMode;
    OutputPreviewMode m_previewMode;
    PreviewSize m_previewSize;

    QStringListModel *m_inputModel;
    QStringListModel *m_outputModel;
    QStringListModel *m_previewModeModel;
    QStringListModel *m_previewSizeModel;

};



#endif

