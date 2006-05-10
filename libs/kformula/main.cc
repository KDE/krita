
#include <iostream>
#include <memory>

#include <q3accel.h>
#include <qdom.h>
#include <QFile>
#include <QLayout>
#include <q3ptrlist.h>
#include <q3mainwindow.h>
#include <qpainter.h>
#include <QString>
#include <qtextstream.h>
#include <QWidget>
#include <qfileinfo.h>
//Added by qt3to4:
#include <QFocusEvent>
#include <QKeyEvent>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcommand.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>

#include "elementtype.h"
#include "kformulacommand.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "kformulawidget.h"
#include "scrollview.h"

using namespace KFormula;


class TestWidget : public KFormulaWidget {
public:
    TestWidget(Container* doc, QWidget* parent=0, const char* name=0, Qt::WFlags f=0)
            : KFormulaWidget(doc, parent, name, f) {}

protected:
    virtual void keyPressEvent(QKeyEvent* event);

private:
};


void save( const QString &filename, const QDomDocument& doc )
{
    QFile f( filename );
    if(!f.open(QIODevice::Truncate | QIODevice::ReadWrite)) {
        kWarning( DEBUGID ) << "Error opening file " << filename.latin1() << endl;
        return;
    }

    QTextStream stream(&f);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(stream, 2);
    f.close();
}


void load( KFormula::Document* document, const QString &filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning( DEBUGID ) << "Error opening file " << filename.latin1() << endl;
        return;
    }
    QTextStream stream(&f);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    QString content = stream.read();
    f.close();
    //kDebug( DEBUGID ) << content << endl;
    QDomDocument doc;
    if ( !doc.setContent( content ) ) {
        return;
    }
    if ( !document->loadXML( doc ) ) {
        kWarning( DEBUGID ) << "Failed." << endl;
    }
}


void saveMathML( KFormula::Container* formula, const QString &filename, bool oasisFormat )
{
    QFile f( filename );
    if ( !f.open( QIODevice::Truncate | QIODevice::ReadWrite ) ) {
        kWarning( DEBUGID ) << "Error opening file " << filename.latin1() << endl;
        return;
    }

    QTextStream stream( &f );
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    formula->saveMathML( stream, oasisFormat );
    f.close();
}


void loadMathML( KFormula::Container* formula, const QString &filename )
{
    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) ) {
        kWarning( DEBUGID ) << "Error opening file " << filename.latin1() << endl;
        return;
    }
    QTextStream stream( &f );
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    QString content = stream.read();

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if ( !doc.setContent( content, true,
                          &errorMsg, &errorLine, &errorColumn ) ) {
        kWarning( DEBUGID ) << "MathML built error: " << errorMsg
                             << " at line " << errorLine
                             << " and column " << errorColumn << endl;
        f.close();
        return;
    }

    /*kDebug( DEBUGID ) << "Container::loadMathML\n"
      << doc.toCString() << endl;*/

    if ( !formula->loadMathML( doc ) ) {
        kWarning( DEBUGID ) << "Failed." << endl;
    }
    f.close();
}


void TestWidget::keyPressEvent(QKeyEvent* event)
{
    Container* document = getDocument();

    //int action = event->key();
    int state = event->state();
    //MoveFlag flag = movementFlag(state);

    if ( ( state & Qt::ShiftModifier ) && ( state & Qt::ControlModifier ) ) {
        switch (event->key()) {
            case Qt::Key_B: document->document()->wrapper()->appendColumn(); return;
            case Qt::Key_I: document->document()->wrapper()->insertColumn(); return;
            case Qt::Key_R: document->document()->wrapper()->removeColumn(); return;
            case Qt::Key_Z: document->document()->wrapper()->redo(); return;
        case Qt::Key_F: saveMathML( document, "test.mml", true/*save to oasis format*/ ); return;
            case Qt::Key_M: saveMathML( document, "test.mml", false ); return;
            case Qt::Key_O: {
                QString file = KFileDialog::getOpenFileName();
                kDebug( DEBUGID ) << file << endl;
                if( !file.isEmpty() ) {
                    QFileInfo fi( file );
                    if ( fi.extension() == "mml" ) {
                        loadMathML( document, file );
                    }
                    else if ( fi.extension() == "xml" ) {
                        load( document->document(), file );
                    }
                }
                return;
        }
        }
    }
    else if (state & Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_1: document->document()->wrapper()->addSum(); return;
            case Qt::Key_2: document->document()->wrapper()->addProduct(); return;
            case Qt::Key_3: document->document()->wrapper()->addIntegral(); return;
            case Qt::Key_4: document->document()->wrapper()->addRoot(); return;
            case Qt::Key_5: document->document()->wrapper()->addFraction(); return;
            case Qt::Key_6: document->document()->wrapper()->addMatrix(); return;
	    case Qt::Key_7: document->document()->wrapper()->addOneByTwoMatrix(); return;
	    case Qt::Key_8: document->document()->wrapper()->addOverline(); return;
	    case Qt::Key_9: document->document()->wrapper()->addUnderline(); return;
            case Qt::Key_A: slotSelectAll(); return;
            case Qt::Key_B: document->document()->wrapper()->appendRow(); return;
            case Qt::Key_C: document->document()->wrapper()->copy(); return;
            case Qt::Key_D: document->document()->wrapper()->removeEnclosing(); return;
            case Qt::Key_G: document->document()->wrapper()->makeGreek(); return;
            case Qt::Key_I: document->document()->wrapper()->insertRow(); return;
            case Qt::Key_R: document->document()->wrapper()->removeRow(); return;
            case Qt::Key_K: document->document()->wrapper()->addMultiline(); return;
            case Qt::Key_L: document->document()->wrapper()->addGenericLowerIndex(); return;
            case Qt::Key_M: loadMathML( document, "test.mml" ); return;
            case Qt::Key_O: load( document->document(), "test.xml" ); return;
            case Qt::Key_Q: kapp->quit(); return;
            case Qt::Key_S: save( "test.xml", document->document()->saveXML() ); return;
            case Qt::Key_T: std::cout << document->texString().latin1() << std::endl; return;
            case Qt::Key_U: document->document()->wrapper()->addGenericUpperIndex(); return;
            case Qt::Key_V: document->document()->wrapper()->paste(); return;
            case Qt::Key_X: document->document()->wrapper()->cut(); return;
            case Qt::Key_Z: document->document()->wrapper()->undo(); return;
            default:
                //std::cerr << "Key: " << event->key() << std::endl;
                break;
        }
    }

    KFormulaWidget::keyPressEvent(event);
}


ScrollView::ScrollView()
        : Q3ScrollView(), child(0)
{
}

void ScrollView::addChild(KFormulaWidget* c, int x, int y)
{
    Q3ScrollView::addChild(c, x, y);
    child = c;
    connect(child, SIGNAL(cursorChanged(bool, bool)),
            this, SLOT(cursorChanged(bool, bool)));
}

void ScrollView::focusInEvent(QFocusEvent*)
{
    if (child != 0) child->setFocus();
}


void ScrollView::cursorChanged(bool visible, bool /*selecting*/)
{
    if (visible) {
        int x = child->getCursorPoint().x();
        int y = child->getCursorPoint().y();
        ensureVisible(x, y);
    }
}


static const KCmdLineOptions options[]= {
    { "+file", "File to open", 0 },
    KCmdLineLastOption
};

int main(int argc, char** argv)
{
    KAboutData aboutData("math test", "KFormula test",
                         "0.01", "test", KAboutData::License_GPL,
                         "(c) 2003, Ulrich Kuettler");
    aboutData.addAuthor("Ulrich Kuettler",0, "ulrich.kuettler@gmx.de");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    DocumentWrapper* wrapper = new DocumentWrapper( KGlobal::config(), 0 );
    Document* document = new Document;
    wrapper->document( document );
    Container* container1 = document->createFormula();

    ScrollView* scrollview1a = new ScrollView;

    KFormulaWidget* mw1a = new TestWidget(container1, scrollview1a, "test1a");

    scrollview1a->addChild(mw1a);
    scrollview1a->setCaption("Test1a of the formula engine");
    scrollview1a->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for ( int i = 0; i < args->count(); ++i ) {
        QFileInfo fi( args->url( i ).path() );
        if ( fi.extension() == "mml" )
            loadMathML( container1, args->url( i ).path() );
        else if ( fi.extension() == "xml" )
            load( container1->document(), args->url( i ).path() );
    }

    int result = app.exec();

    delete container1;
    delete wrapper;

    // Make sure there are no elements in the clipboard.
    // Okey for a debug app.
    QApplication::clipboard()->clear();

    int destruct = BasicElement::getEvilDestructionCount();
    if (destruct != 0) {
        std::cerr << "BasicElement::EvilDestructionCount: " << destruct << std::endl;
    }
    destruct = PlainCommand::getEvilDestructionCount();
    if (destruct != 0) {
        std::cerr << "PlainCommand::EvilDestructionCount: " << destruct << std::endl;
    }
    destruct = ElementType::getEvilDestructionCount();
    if (destruct != 0) {
        std::cerr << "ElementType::EvilDestructionCount: " << destruct << std::endl;
    }

    return result;
}

#include "scrollview.moc"
