#include "krita.h"

#include <KoApplication.h>
#include <KoPart.h>
#include <KoMainWindow.h>

#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_image.h"

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
