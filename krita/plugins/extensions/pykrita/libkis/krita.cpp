/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "krita.h"

#include <KoApplication.h>
#include <KoPart.h>
#include <KoMainWindow.h>

#include <kis_view2.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_action.h>
#include <kis_script_manager.h>

Krita::Krita(QObject *parent) :
    QObject(parent)
{
}

QList<MainWindow *> Krita::mainWindows()
{
    QList<MainWindow *> ret;
    foreach(KoPart *part, koApp->partList()) {
        if (part) {
            foreach(KoMainWindow *mainWin, part->mainWindows()) {
                ret << new MainWindow(mainWin, this);
            }
        }
    }
    return ret;
}

QList<View *> Krita::views()
{
    QList<View *> ret;
    foreach(MainWindow *mainWin, mainWindows()) {
        ret << mainWin->views();
    }
    return ret;
}

QList<Document *> Krita::documents()
{
    QList<Document *> ret;
    foreach(KoPart *part, koApp->partList()) {
        if (part) {
            KisDoc2 *doc = qobject_cast<KisDoc2*>(part->document());
            if (doc) {
                ret << new Document(doc, this);
            }
        }
    }
    return ret;

}

QList<Image *> Krita::images()
{
    QList<Image *> ret;
    foreach(Document *doc, documents()) {
        ret << doc->image();
    }
    return ret;
}

QAction *Krita::createAction(const QString &text)
{
    KisAction *action = new KisAction(text, this);
    foreach(KoPart *part, koApp->partList()) {
        if (part) {
            foreach(KoMainWindow *mainWin, part->mainWindows()) {
                if (mainWin && mainWin->rootView()) {
                    KisView2 *view = qobject_cast<KisView2*>(view);
                    if (view) {
                        view->scriptManager()->addAction(action);
                    }
                }
            }
        }
    }
    return action;
}
