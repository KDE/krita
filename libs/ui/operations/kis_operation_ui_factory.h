/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
