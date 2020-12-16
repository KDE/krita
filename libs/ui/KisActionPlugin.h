/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    *  @param name - title of the action in the krita5.xmlgui file
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
