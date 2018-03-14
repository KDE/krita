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


#ifndef KIS_VIEW_PLUGIN_H
#define KIS_VIEW_PLUGIN_H

#include <kritaui_export.h>
#include <QObject>
#include <QPointer>

class KisOperation;
class KisOperationUIFactory;
class KisAction;
class KisViewManager;

/**
 *  KisActionPlugin is the base for plugins which add actions to the main window
 */
class KRITAUI_EXPORT KisActionPlugin : public QObject
{
    Q_OBJECT
public:
    KisActionPlugin(QObject *parent = 0);
    ~KisActionPlugin() override;

protected:

   /**
    *  Registers a KisAction to the UI and action manager.
    *  @param name - title of the action in the krita4.xmlgui file
    *  @param action the action that should be added
    */
    void addAction(const QString& name, KisAction *action);

    KisAction *createAction(const QString &name);

    void addUIFactory(KisOperationUIFactory *factory);

    void addOperation(KisOperation *operation);

    QPointer<KisViewManager> viewManager() const;

private:
    QPointer<KisViewManager> m_viewManager;
};

#endif // KIS_VIEW_PLUGIN_H
