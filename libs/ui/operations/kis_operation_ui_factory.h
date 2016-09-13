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

#ifndef KIS_OPERATION_UI_FACTORY_H
#define KIS_OPERATION_UI_FACTORY_H

#include "kritaui_export.h"
#include <QString>
#include "operations/kis_operation_configuration.h"

class KisViewManager;


class KRITAUI_EXPORT KisOperationUIFactory
{
public:
   /**
    * Construct a Ui factory
    * @param id the id for the ui, has to be the same as the operation id of the KisAction
    */
    KisOperationUIFactory(const QString &id);
    virtual ~KisOperationUIFactory();

   /**
    * id for the UI registry
    */
    QString id() const;

   /**
    * Fetch the configuration for a QWidget or other UI
    * @param view the view
    * @param configuration the into which the setting will be written
    */
    virtual bool fetchConfiguration(KisViewManager* view, KisOperationConfigurationSP configuration) = 0;

private:
    class Private;
    Private* const d;
};

#endif // KIS_OPERATION_UI_FACTORY_H
