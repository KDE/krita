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



#ifndef KIS_OPERATION_UI_WIDGET_FACTORY_H
#define KIS_OPERATION_UI_WIDGET_FACTORY_H

#include <KoDialog.h>
#include <klocalizedstring.h>

#include "kis_operation_ui_factory.h"
#include "operations/kis_operation_ui_widget.h"
#include "KisViewManager.h"

/**
 *  Factory to get operation configurations from QWidget based operation widgets
 *  T has to be a KisOperationUIWidget
 */
template <class T> class KisOperationUIWidgetFactory : public KisOperationUIFactory
{

public:
    KisOperationUIWidgetFactory(const QString &id) : KisOperationUIFactory(id)
    {
    }
    
    ~KisOperationUIWidgetFactory() override
    {
    }

    /**
    *  Reimplemented. Show a dialog the widget specify as T
    *  @param view the view
    *  @param configuration the configuration to the operation
    *  @returns true if the configuration could be constructed (not canceled)
    */
    bool fetchConfiguration(KisViewManager* view, KisOperationConfigurationSP configuration) override {
        KoDialog * dialog = new KoDialog(view->mainWindow());
        Q_CHECK_PTR(dialog);

        T* configWidget = new T(dialog, view);
        dialog->setCaption(configWidget->caption());
        dialog->setMainWidget(configWidget);
        bool success = false;
        if (dialog->exec() == QDialog::Accepted) {
            configWidget->getConfiguration(configuration);
            success = true;
        }
        delete dialog;

        return success;
    }
};

#endif // KIS_OPERATION_UI_WIDGET_FACTORY_H
