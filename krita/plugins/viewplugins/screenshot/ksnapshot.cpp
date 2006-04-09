/*
 * KSnapshot
 *
 * (c) Richard J. Moore 1997-2002
 * (c) Matthias Ettrich 2000
 * (c) Aaron J. Seigo 2002
 * (c) Nadeem Hasan 2003
 * This adaptation: (c) Boudewijn Rempt 2005 
 *
 * Released under the GPL see file LICENSE for details.
 */

#include <kapplication.h>
#include <klocale.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kio/netaccess.h>
#include <ksavefile.h>
#include <ktempfile.h>
#include <kvbox.h>

#include <qbitmap.h>
#include <q3dragobject.h>
#include <qimage.h>
#include <qclipboard.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3MemArray>
#include <QEvent>
#include <QMouseEvent>

#include <kaccel.h>
#include <knotifyclient.h>
#include <khelpmenu.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kstartupinfo.h>

#include <qcursor.h>
#include <qregexp.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <q3whatsthis.h>

#include <stdlib.h>

#include "ksnapshot.h"
#include "regiongrabber.h"
#include "ksnapshotwidget.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <config.h>

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
#include <X11/extensions/shape.h>
#endif

#include <kglobal.h>
#include <QX11Info>

KSnapshot::KSnapshot(QWidget *parent, const char *name)
    : super(parent, name, false, QString::null, Ok|Cancel)
{
    grabber = new QWidget( 0, 0, Qt::WStyle_Customize | Qt::WX11BypassWM );
    Q_CHECK_PTR(grabber);
    grabber->move( -1000, -1000 );
    grabber->installEventFilter( this );

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
    int tmp1, tmp2;
    //Check whether the extension is available
    haveXShape = XShapeQueryExtension( QX11Info::display(), &tmp1, &tmp2 );
#endif

    KVBox *vbox = makeVBoxMainWidget();
    mainWidget = new KSnapshotWidget( vbox, "mainWidget" );
    Q_CHECK_PTR(mainWidget);

    connect(mainWidget, SIGNAL(startImageDrag()), SLOT(slotDragSnapshot()));

    connect( mainWidget, SIGNAL( newClicked() ), SLOT( slotGrab() ) );
    connect( mainWidget, SIGNAL( printClicked() ), SLOT( slotPrint() ) );

    grabber->show();
    grabber->grabMouse( Qt::waitCursor );
    
    snapshot = QPixmap::grabWindow( QX11Info::appRootWindow() );
    updatePreview();
    grabber->releaseMouse();
    grabber->hide();

    KConfig *conf=KGlobal::config();
    conf->setGroup("GENERAL");
    mainWidget->setDelay(conf->readNumEntry("delay",0));
    mainWidget->setMode( conf->readNumEntry( "mode", 0 ) );
    mainWidget->setIncludeDecorations(conf->readBoolEntry("includeDecorations",true));

    connect( &grabTimer, SIGNAL( timeout() ), this, SLOT(  grabTimerDone() ) );

    KAccel* accel = new KAccel(this);
    Q_CHECK_PTR(accel);
    accel->insert(KStdAccel::Print, this, SLOT(slotPrint()));
    accel->insert(KStdAccel::New, this, SLOT(slotGrab()));

    accel->insert( "Print2", Qt::Key_P, this, SLOT(slotPrint()));
    accel->insert( "New2", Qt::Key_N, this, SLOT(slotGrab()));
    accel->insert( "New3", Qt::Key_Space, this, SLOT(slotGrab()));

    mainWidget->btnNew->setFocus();
}

KSnapshot::~KSnapshot()
{
}

bool KSnapshot::save( const QString &filename )
{
    return save( KUrl::fromPathOrURL( filename ));
}

bool KSnapshot::save( const KUrl& url )
{
    KMimeType::Ptr mimeType = KMimeType::findByURL(url);
    QStringList types(KImageIO::typeForMime(mimeType->name()));

    QString type;

    if (types.isEmpty()) {
        type = "PNG";
    } else {
        type = types.at(0);
    }

    bool ok = false;

    if ( url.isLocalFile() ) {
        KSaveFile saveFile( url.path() );
        if ( saveFile.status() == 0 ) {
            if ( snapshot.save( saveFile.file(), type.latin1() ) )
                ok = saveFile.close();
        }
    }
    else {
        KTempFile tmpFile;
        tmpFile.setAutoDelete( true );
        if ( tmpFile.status() == 0 ) {
            if ( snapshot.save( tmpFile.file(), type.latin1() ) ) {
                if ( tmpFile.close() )
                    ok = KIO::NetAccess::upload( tmpFile.name(), url, this );
            }
        }
    }

    QApplication::restoreOverrideCursor();
    if ( !ok ) {
        kWarning() << "KSnapshot was unable to save the snapshot" << endl;

        QString caption = i18n("Unable to Save Image");
        QString text = i18n("KSnapshot was unable to save the image to\n%1.")
            .arg(url.prettyURL());
        KMessageBox::error(this, text, caption);
    }

    return ok;
}

void KSnapshot::slotCopy()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setPixmap( snapshot );
}

void KSnapshot::slotDragSnapshot()
{
    Q3DragObject *drobj = new Q3ImageDrag(snapshot.convertToImage(), this);
    Q_CHECK_PTR(drobj);
    drobj->setPixmap(mainWidget->preview());
    drobj->dragCopy();
}

void KSnapshot::slotGrab()
{
    hide();
    if ( mainWidget->mode() == Region )
    {
        rgnGrab = new RegionGrabber();
        Q_CHECK_PTR(rgnGrab);
        connect( rgnGrab, SIGNAL( regionGrabbed( const QPixmap & ) ),
             SLOT( slotRegionGrabbed( const QPixmap & ) ) );
    }
    else
    {
        if ( mainWidget->delay() )
            grabTimer.start( mainWidget->delay() * 1000, true );
        else {
            grabber->show();
            grabber->grabMouse( Qt::crossCursor );
        }
    }
}

void KSnapshot::slotPrint()
{
    KPrinter printer;
    if (snapshot.width() > snapshot.height())
        printer.setOrientation(KPrinter::Landscape);
    else
        printer.setOrientation(KPrinter::Portrait);

    qApp->processEvents();

    if (printer.setup(this, i18n("Print Screenshot")))
    {
        qApp->processEvents();

        QPainter painter(&printer);
        Q3PaintDeviceMetrics metrics(painter.device());

        float w = snapshot.width();
        float dw = w - metrics.width();
        float h = snapshot.height();
        float dh = h - metrics.height();
        bool scale = false;

        if ( (dw > 0.0) || (dh > 0.0) )
            scale = true;

        if ( scale ) {

            QImage img = snapshot.convertToImage();
            qApp->processEvents();

            float newh, neww;
            if ( dw > dh ) {
                neww = w-dw;
                newh = neww/w*h;
            }
            else {
                newh = h-dh;
                neww = newh/h*w;
            }

            img = img.smoothScale( int(neww), int(newh), Qt::KeepAspectRatio );
            qApp->processEvents();

            int x = (metrics.width()-img.width())/2;
            int y = (metrics.height()-img.height())/2;

            painter.drawImage( x, y, img);
        }
        else {
            int x = (metrics.width()-snapshot.width())/2;
            int y = (metrics.height()-snapshot.height())/2;
            painter.drawPixmap( x, y, snapshot );
        }
    }

    qApp->processEvents();
}

void KSnapshot::slotRegionGrabbed( const QPixmap &pix )
{
    if ( !pix.isNull() )
    {
        snapshot = pix;
        updatePreview();
        modified = true;
    }

    delete rgnGrab;
    QApplication::restoreOverrideCursor();
    show();
}

bool KSnapshot::eventFilter( QObject* o, QEvent* e)
{
    if ( o == grabber && e->type() == QEvent::MouseButtonPress ) {
        QMouseEvent* me = (QMouseEvent*) e;
        if ( QWidget::mouseGrabber() != grabber )
            return false;
        if ( me->button() == Qt::LeftButton )
            performGrab();
    }
    return false;
}

void KSnapshot::updatePreview()
{
    QImage img = snapshot.convertToImage();
    double r1 = ((double) snapshot.height() ) / snapshot.width();
    if ( r1 * mainWidget->previewWidth()  < mainWidget->previewHeight() )
        img = img.smoothScale( mainWidget->previewWidth(),
                       int( mainWidget->previewWidth() * r1 ));
    else
        img = img.smoothScale( (int) (((double)mainWidget->previewHeight()) / r1),
                       (mainWidget->previewHeight() ) );

    QPixmap pm;
    pm.convertFromImage( img );
    mainWidget->setPreview( pm );
}

void KSnapshot::grabTimerDone()
{
    performGrab();
    KNotifyClient::beep(i18n("The screen has been successfully grabbed."));
}

static 
Window findRealWindow( Window w, int depth = 0 )
{
    if( depth > 5 )
        return None;
    static Atom wm_state = XInternAtom( QX11Info::display(), "WM_STATE", False );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* prop;
    if( XGetWindowProperty( QX11Info::display(), w, wm_state, 0, 0, False, AnyPropertyType,
                &type, &format, &nitems, &after, &prop ) == Success ) {
        if( prop != NULL )
            XFree( prop );
        if( type != None )
            return w;
    }
    Window root, parent;
    Window* children;
    unsigned int nchildren;
    Window ret = None;
    if( XQueryTree( QX11Info::display(), w, &root, &parent, &children, &nchildren ) != 0 ) {
        for( unsigned int i = 0;
             i < nchildren && ret == None;
             ++i )
            ret = findRealWindow( children[ i ], depth + 1 );
        if( children != NULL )
            XFree( children );
    }
    return ret;
}

void KSnapshot::performGrab()
{
    grabber->releaseMouse();
    grabber->hide();
    grabTimer.stop();
    XGrabServer( QX11Info::display());
    if ( mainWidget->mode() == WindowUnderCursor ) {
        Window root;
        Window child;
        uint mask;
        int rootX, rootY, winX, winY;
        XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
                   &rootX, &rootY, &winX, &winY,
                   &mask);
        if( child == None )
            child = QX11Info::appRootWindow();
        if( !mainWidget->includeDecorations()) {
            Window real_child = findRealWindow( child );
            if( real_child != None ) // test just in case
                child = real_child;
        }
        int x, y;
        unsigned int w, h;
        unsigned int border;
        unsigned int depth;
        XGetGeometry( QX11Info::display(), child, &root, &x, &y,
                  &w, &h, &border, &depth );
        w += 2 * border;
        h += 2 * border;

        Window parent;
        Window* children;
        unsigned int nchildren;
        if( XQueryTree( QX11Info::display(), child, &root, &parent,
                &children, &nchildren ) != 0 ) {
            if( children != NULL )
                XFree( children );
            int newx, newy;
            Window dummy;
            if( XTranslateCoordinates( QX11Info::display(), parent, QX11Info::appRootWindow(),
                           x, y, &newx, &newy, &dummy )) {
                x = newx;
                y = newy;
            }
        }

        snapshot = QPixmap::grabWindow( QX11Info::appRootWindow(), x, y, w, h );

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
        //No XShape - no work.
        if (haveXShape) {
            QBitmap mask(w, h);
            //As the first step, get the mask from XShape.
            int count, order;
            XRectangle* rects = XShapeGetRectangles( QX11Info::display(), child,
                                 ShapeBounding, &count, &order);
            //The ShapeBounding region is the outermost shape of the window;
            //ShapeBounding - ShapeClipping is defined to be the border.
            //Since the border area is part of the window, we use bounding
            // to limit our work region
            if (rects) {
                //Create a QRegion from the rectangles describing the bounding mask.
                QRegion contents;
                for (int pos = 0; pos < count; pos++)
                    contents += QRegion(rects[pos].x, rects[pos].y,
                                rects[pos].width, rects[pos].height);
                XFree(rects);

                //Create the bounding box.
                QRegion bbox(0, 0, snapshot.width(), snapshot.height());
                
                if( border > 0 ) {
                    contents.translate( border, border );
                    contents += QRegion( 0, 0, border, h );
                    contents += QRegion( 0, 0, w, border );
                    contents += QRegion( 0, h - border, w, border );
                    contents += QRegion( w - border, 0, border, h );
                }
                
                //Get the masked away area.
                QRegion maskedAway = bbox - contents;
                Q3MemArray<QRect> maskedAwayRects = maskedAway.rects();

                //Construct a bitmap mask from the rectangles
                QPainter p(&mask);
                p.fillRect(0, 0, w, h, Qt::color1);
                for (uint pos = 0; pos < maskedAwayRects.count(); pos++)
                    p.fillRect(maskedAwayRects[pos], Qt::color0);
                p.end();

                snapshot.setMask(mask);
            }
        }
#endif
    }
    else {
        snapshot = QPixmap::grabWindow( QX11Info::appRootWindow() );
    }
    XUngrabServer( QX11Info::display());
    updatePreview();
    QApplication::restoreOverrideCursor();
    modified = true;
    show();
}

void KSnapshot::setTime(int newTime)
{
    mainWidget->setDelay(newTime);
}

void KSnapshot::setURL( const QString &url )
{
    KUrl newURL = KUrl::fromPathOrURL( url );
    if ( newURL == filename )
        return;

    filename = newURL;
}

void KSnapshot::setGrabMode( int m )
{
    mainWidget->setMode( m );
}

void KSnapshot::slotMovePointer(int x, int y)
{
    QCursor::setPos( x, y );
}

void KSnapshot::exit()
{

    KConfig *conf=KGlobal::config();
    conf->setGroup("GENERAL");
    conf->writeEntry("delay",mainWidget->delay());
    conf->writeEntry("mode",mainWidget->mode());
    conf->writeEntry("includeDecorations",mainWidget->includeDecorations());
    KUrl url = filename;
    url.setPass( QString::null );
    conf->writePathEntry("filename",url.url());

    reject();
}

void KSnapshot::slotOk()
{

    KConfig *conf=KGlobal::config();
    conf->setGroup("GENERAL");
    conf->writeEntry("delay",mainWidget->delay());
    conf->writeEntry("mode",mainWidget->mode());
    conf->writeEntry("includeDecorations",mainWidget->includeDecorations());
    KUrl url = filename;
    url.setPass( QString::null );
    conf->writePathEntry("filename",url.url());

    emit screenGrabbed();

    accept();
}

#include "ksnapshot.moc"
