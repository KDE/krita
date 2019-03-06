/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_APPLICATION_H
#define KIS_APPLICATION_H

#include <QPointer>
#include <QScopedPointer>
#include <qtsingleapplication/qtsingleapplication.h>
#include "kritaui_export.h"

class KisMainWindow;
class KisApplicationPrivate;
class QWidget;
class KisApplicationArguments;
class KisAutoSaveRecoveryDialog;

#include <KisImportExportManager.h>

/**
 *  @brief Base class for the %Krita app
 *
 *  This class handles arguments given on the command line and
 *  shows a generic about dialog for the Krita app.
 *
 *  In addition it adds the standard directories where Krita
 *  can find its images etc.
 *
 *  If the last mainwindow becomes closed, KisApplication automatically
 *  calls QApplication::quit.
 */
class KRITAUI_EXPORT KisApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    /**
     * Creates an application object, adds some standard directories and
     * initializes kimgio.
     */
    explicit KisApplication(const QString &key, int &argc, char **argv);

    /**
     *  Destructor.
     */
    ~KisApplication() override;

    /**
     * Call this to start the application.
     *
     * Parses command line arguments and creates the initial main windowss and docs
     * from them (or an empty doc if no cmd-line argument is specified ).
     *
     * You must call this method directly before calling QApplication::exec.
     *
     * It is valid behaviour not to call this method at all. In this case you
     * have to process your command line parameters by yourself.
     */
    virtual bool start(const KisApplicationArguments &args);

    /**
     * Checks if user is holding ctrl+alt+shift keys and asks if the settings file should be cleared.
     *
     * Typically called during startup before reading the config.
     */
    void askClearConfig();

    /**
     * Tell KisApplication to show this splashscreen when you call start();
     * when start returns, the splashscreen is hidden. Use KSplashScreen
     * to have the splash show correctly on Xinerama displays.
     */
    void setSplashScreen(QWidget *splash);
    void hideSplashScreen();

    /// Overridden to handle exceptions from event handlers.
    bool notify(QObject *receiver, QEvent *event) override;

    void addResourceTypes();
    bool registerResources();
    void loadResourceTags();
    void loadPlugins();
    void loadGuiPlugins();
    void initializeGlobals(const KisApplicationArguments &args);

public Q_SLOTS:

    void remoteArguments(QByteArray message, QObject*socket);
    void fileOpenRequested(const QString & url);
    void setSplashScreenLoadingText(const QString&);

private:
    /// @return the number of autosavefiles opened
    void checkAutosaveFiles();
    bool createNewDocFromTemplate(const QString &fileName, KisMainWindow *m_mainWindow);
    void clearConfig();

private:
    class Private;
    QScopedPointer<Private> d;
    class ResetStarting;
    friend class ResetStarting;
};

#endif
