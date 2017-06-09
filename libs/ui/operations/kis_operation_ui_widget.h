/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef KIS_OPERATION_UI_WIDGET_H
#define KIS_OPERATION_UI_WIDGET_H

#include <QWidget>
#include <kritaui_export.h>
#include "operations/kis_operation_configuration.h"

/**
*  Base class for the QWidget based operation config widgets
*/
class KRITAUI_EXPORT KisOperationUIWidget : public QWidget
{

public:
    explicit KisOperationUIWidget(const QString& caption, QWidget* parent = 0);
    ~KisOperationUIWidget() override;

   /**
    * Caption of the operation widget, used in dialog caption
    */
    QString caption() const;

   /**
    * Fetch the setting from the config widet
    * @param config configuration to which the setting will be written
    */
    virtual void getConfiguration(KisOperationConfigurationSP config) = 0;

private:
    class Private;
    Private* const d;
};

#endif // KIS_OPERATION_UI_WIDGET_H
