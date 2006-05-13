#include <kapplication.h>
#include <kcmdlineargs.h>
#include <KoStore.h>
#include <q3textbrowser.h>
#include <QStringList>
#include <QBuffer>
#include <QClipboard>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QKeyEvent>
#include <Q3CString>
#include <QDropEvent>

class StoreDropTest : public Q3TextBrowser
{
public:
    StoreDropTest( QWidget* parent );
protected:
    virtual void contentsDragEnterEvent( QDragEnterEvent * e );
    virtual void contentsDragMoveEvent( QDragMoveEvent * e );
    virtual void contentsDropEvent( QDropEvent * e );
    virtual void keyPressEvent( QKeyEvent * e );
    virtual void paste();
private:
    bool processMimeSource( QMimeSource* ev );
    void showZipContents( QByteArray data, const char* mimeType, bool oasis );
    QString loadTextFile( KoStore* store, const QString& fileName );
};

int main( int argc, char** argv )
{
    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc, argv, "storedroptest", 0, 0, 0, 0);
    KApplication app;

    StoreDropTest* window = new StoreDropTest( 0 );
    window->resize( 500, 500 );
    window->show();

    QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    return app.exec();
}

StoreDropTest::StoreDropTest( QWidget* parent )
    : Q3TextBrowser( parent )
{
    setText( "KoStore drop/paste test\nDrop or paste a selection from a KOffice application into this widget to see the ZIP contents" );
    setAcceptDrops( true );
}

void StoreDropTest::contentsDragEnterEvent( QDragEnterEvent * ev )
{
    ev->acceptAction();
}

void StoreDropTest::contentsDragMoveEvent( QDragMoveEvent * ev )
{
    ev->acceptAction();
}

void StoreDropTest::keyPressEvent( QKeyEvent * e )
{
    if ( ( ( e->modifiers() & Qt::ShiftModifier ) && e->key() == Qt::Key_Insert ) ||
         ( ( e->modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_V ) )
        paste();
    //else
    //    QTextBrowser::keyPressEvent( e );
}

void StoreDropTest::paste()
{
    qDebug( "paste" );
    QMimeSource* m = QApplication::clipboard()->data();
    if ( !m )
        return;
    processMimeSource( m );
}

void StoreDropTest::contentsDropEvent( QDropEvent *ev )
{
    if ( processMimeSource( ev ) )
        ev->acceptAction();
    else
        ev->ignore();
}

bool StoreDropTest::processMimeSource( QMimeSource* ev )
{
    const Q3CString acceptMimeType = "application/vnd.oasis.opendocument.";
    const char* fmt;
    QStringList formats;
    for (int i=0; (fmt = ev->format(i)); i++) {
        formats += fmt;
        bool oasis = QString( fmt ).startsWith( acceptMimeType );
        if ( oasis || QString( fmt ) == "application/x-kpresenter" ) {
            QByteArray data = ev->encodedData( fmt );
            showZipContents( data, fmt, oasis );
            return true;
        }
    }
    setText( "No acceptable format found. All I got was:\n" + formats.join( "\n" ) );
    return false;
}

void StoreDropTest::showZipContents( QByteArray data, const char* mimeType, bool oasis )
{
    if ( data.isEmpty() ) {
        setText( "No data!" );
        return;
    }
    QBuffer buffer( &data );
    KoStore * store = KoStore::createStore( &buffer, KoStore::Read );
    if ( store->bad() ) {
        setText( "Invalid ZIP!" );
        return;
    }
    store->disallowNameExpansion();

    QString txt = QString( "Valid ZIP file found for format " ) + mimeType + "\n";

    if ( oasis ) {
        txt += loadTextFile( store, "content.xml" );
        txt += loadTextFile( store, "styles.xml" );
        txt += loadTextFile( store, "settings.xml" );
        txt += loadTextFile( store, "META-INF/manifest.xml" );
    } else {
        txt += loadTextFile( store, "maindoc.xml" );
    }
    setText( txt );
}

QString StoreDropTest::loadTextFile( KoStore* store, const QString& fileName )
{
    if ( !store->open( fileName ) )
        return QString( "%1 not found\n" ).arg( fileName );

    QByteArray data = store->device()->readAll();
    store->close();
    QString txt = QString( "Found %1: \n" ).arg( fileName );
    txt += QString::fromUtf8( data.data(), data.size() );
    txt += "\n";
    return txt;
}
