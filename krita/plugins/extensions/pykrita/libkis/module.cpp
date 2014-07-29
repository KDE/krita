#include "module.h"

#include <KoApplication.h>
#include <KoPart.h>
#include <KoMainWindow.h>

#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_image.h"

Module::Module(QObject *parent) :
    QObject(parent)
{
}

QList<MainWindow *> Module::mainWindows()
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

QList<View *> Module::views()
{
    QList<View *> ret;
    foreach(MainWindow *mainWin, mainWindows()) {
        ret << mainWin->views();
    }
    return ret;
}

QList<Document *> Module::documents()
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

QList<Image *> Module::images()
{
    QList<Image *> ret;
    foreach(Document *doc, documents()) {
        ret << doc->image();
    }
    return ret;
}
