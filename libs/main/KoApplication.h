/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __ko_app_h__
#define __ko_app_h__

#include <kapplication.h>
#include "komain_export.h"

class KoPart;

class KoApplicationPrivate;
class QSplashScreen;

/**
 *  @brief Base class for all %Calligra apps
 *
 *  This class handles arguments given on the command line and
 *  shows a generic about dialog for all Calligra apps.
 *
 *  In addition it adds the standard directories where Calligra applications
 *  can find their images etc.
 *
 *  If the last mainwindow becomes closed, KoApplication automatically
 *  calls QApplication::quit.
 */
class KOMAIN_EXPORT KoApplication : public KApplication
{
    Q_OBJECT

public:
    /**
     * Creates an application object, adds some standard directories and
     * initializes kimgio.
     */
    KoApplication();

    /**
     *  Destructor.
     */
    virtual ~KoApplication();

    // ######### Bad name
    /**
     * Call this to start the application.
     *
     * Parses command line arguments and creates the initial shells and docs
     * from them (or an empty doc if no cmd-line argument is specified ).
     *
     * You must call this method directly before calling QApplication::exec.
     *
     * It is valid behaviour not to call this method at all. In this case you
     * have to process your command line parameters by yourself.
     */
    virtual bool start();

    /**
     * @return true if the application is starting
     */
    static bool isStarting();

    /**
     * Tell KoApplication to show this splashscreen when you call start();
     * when start returns, the splashscreen is hidden. Use KSplashScreen
     * to have the splash show correctly on Xinerama displays. 
     */
    void setSplashScreen(QSplashScreen *splash);


    QList<KoPart*> partList() const;

    void addPart(KoPart* part);

signals:

    /// KoDocument needs to be able to emit document signals from here.
    friend class KoDocument; // remove this line when done
    friend class KoPart;
    /**
     * emitted when a new document is opened.
     */
    void documentOpened(const QString &ref);

    /**
     * emitted when an old document is closed.
     */
    void documentClosed(const QString &ref);

private:
    bool initHack();
    KoApplicationPrivate * const d;
    static bool m_starting ; ///< is the application starting or not
    class ResetStarting;
    friend class ResetStarting;
};

#endif
