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

#include <KisPart.h>

#include <KisViewManager.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_action.h>
#include <kis_script_manager.h>

Krita* Krita::s_instance = 0;

Krita::Krita(QObject *parent) :
    QObject(parent)
{
}

QList<MainWindow *> Krita::mainWindows()
{
    QList<MainWindow *> ret;
    foreach(QPointer<KisMainWindow> mainWin, KisPart::instance()->mainWindows()) {
        ret << new MainWindow(mainWin, this);
    }
    return ret;
}

QList<View *> Krita::views()
{
    QList<View *> ret;
    foreach(QPointer<KisView> view, KisPart::instance()->views()) {
        ret << new View(view, this);
    }
    return ret;
}

QList<Document *> Krita::documents()
{
    QList<Document *> ret;
    foreach(QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        ret << new Document(doc, this);
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
    foreach(KisMainWindow *mainWin, KisPart::instance()->mainWindows()) {
        mainWin->viewManager()->scriptManager()->addAction(action);
    }
    return action;
}

Krita* Krita::instance()
{
    if (!s_instance)
    {
        s_instance = new Krita;
    }
    return s_instance;
}
