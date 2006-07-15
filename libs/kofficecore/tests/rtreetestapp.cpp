/*
 *  Copyright (c) 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QtGui>
#include <QPainterPath>
#include <QLayout>
#include <QPoint>
#include <QPointF>
#include <QDebug>
#include "rtreetestapp.h"
#include "Tool.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWin;

    mainWin.show();
    return app.exec();
}

Canvas::Canvas( )
: QWidget()
, m_zoom( 1 )    
, m_rtree( 4, 2 )
//, m_rtree( 2, 1 )
, m_tool( 0 )
, m_createTool( this )    
, m_selectTool( this )    
, m_removeTool( this )    
, m_file( "data.trc" )    
, m_listId( 0 )
, m_paintTree( false )
{
    m_tool = &m_createTool;
    setBackgroundRole(QPalette::Base);
    m_file.open( QIODevice::WriteOnly );
    m_out.setDevice( &m_file );
}

void Canvas::updateCanvas()
{
    update();
}

void Canvas::insert( QRectF & rect )
{
    m_out << "i " << rect.left() << " " << rect.top() << " " << rect.width() << " " << rect.height() << "\n";
    Data * data = new Data( rect );
    m_rects.insert( data );
    m_rtree.insert( rect, data );
    update();
}

void Canvas::select( QRectF & rect )
{
    if ( rect.isEmpty() )
    {
        m_found = m_rtree.contains( rect.topLeft() );
    }
    else
    {
        m_found = m_rtree.intersects( rect );
    }
    update();
}

void Canvas::remove( QRectF & rect )
{
    m_out << "r " << rect.left() << " " << rect.top() << " " << rect.width() << " " << rect.height() << "\n";
    m_found = QList<Data *>();
    QList<Data *> remove = m_rtree.intersects( rect );
    foreach ( Data * data, remove )
    {
        m_rtree.remove( data );
        m_rects.remove( data );
        delete data;
    }
    update();
}

void Canvas::replay()
{
    if ( QCoreApplication::arguments().size() > 1 )
    {
        QString filename( QCoreApplication::arguments().at( 1 ) );
        qDebug() << "parameter:" << filename;
        QFile file( filename ); 
        file.open( QIODevice::ReadOnly | QIODevice::Text );
        QTextStream in( &file );
        while ( !in.atEnd() ) 
        {
            m_list.push_back( in.readLine() );
        }
        m_listId = 0;
        QTimer::singleShot( 1000, this, SLOT( replayStep() ) );
    }
}

void Canvas::replayStep()
{

    QString line = m_list.at( m_listId++ );
    qDebug() << "Line:" << line;
    QStringList values = line.split( " " );
    int left = values[1].toInt();
    int top = values[2].toInt();
    int right = values[3].toInt();
    int bottom = values[4].toInt();
    QRectF rect( left, top, right, bottom );
    if ( values[0] == "i" )
    {
        insert( rect );
    }
    else if ( values[0] == "r" )
    {
        remove( rect );
    }

    update();
    if ( m_listId < m_list.size() )
    {
        int sleep = 1000;
        if ( QCoreApplication::arguments().size() >= 3 )
        {
            sleep = QCoreApplication::arguments().at( 2 ).toInt();
        }
        QTimer::singleShot( sleep, this, SLOT( replayStep() ) );
    }
}

void Canvas::debug()
{
    m_rtree.debug();
}

void Canvas::paintTree( bool paintTree )
{
    m_paintTree = paintTree;
    update();
}
   
void Canvas::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter p( this );
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(m_zoom, m_zoom);
    
    if ( m_tool )
        m_tool->paint( p );

    QPen pen( Qt::black );
    p.setPen( pen );
    foreach ( Data * data, m_rects )
    {
        data->paint( p );
    }

    if ( m_paintTree )
    {
        m_rtree.paint( p );
    }

    foreach ( Data * data, m_found )
    {
        QColor c( Qt::yellow );
        c.setAlphaF( 0.1 );
        QBrush brush( c );
        p.setBrush( brush );
        p.drawRect( data->boundingBox() );
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    if ( m_tool )
    {
        m_tool->mouseMoveEvent( e );
    }
}


void Canvas::mousePressEvent(QMouseEvent *e)
{
    if ( m_tool )
    {
        m_tool->mousePressEvent( e );
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *e)
{
    if ( m_tool )
    {
        m_tool->mouseReleaseEvent( e );
    }
}

void Canvas::selectInsertTool()
{
    m_tool = &m_createTool;
}

void Canvas::selectSelectTool()
{
    m_tool = &m_selectTool;
}

void Canvas::selectRemoveTool()
{
    m_tool = &m_removeTool;
}

MainWindow::MainWindow()
{
    m_canvas = new Canvas();
    setCentralWidget(m_canvas);
    m_canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    resize( 640, 480 );
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowTitle(tr("R-Tree Library Test Application"));

    m_canvas->repaint();

    repaint();
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About test"),
            tr("R-Tree Library Test Application"));
}

void MainWindow::createActions()
{
    m_quitAct = new QAction(tr("&Quit"), this);
    m_quitAct->setShortcut(tr("Ctrl+Q"));
    m_quitAct->setStatusTip(tr("Quit the application"));
    connect(m_quitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_aboutQtAct = new QAction(tr("About &Qt"), this);
    m_aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(m_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    m_insertAct = new QAction(tr("&Insert"), this);
    m_insertAct->setStatusTip(tr("Insert Object"));
    connect(m_insertAct, SIGNAL(triggered()), m_canvas, SLOT( selectInsertTool() ) );

    m_selectAct = new QAction(tr("&Select"), this);
    m_selectAct->setStatusTip(tr("Select Objects"));
    connect(m_selectAct, SIGNAL(triggered()), m_canvas, SLOT( selectSelectTool() ) );

    m_removeAct = new QAction(tr("&Remove"), this);
    m_removeAct->setStatusTip(tr("Remove Object"));
    connect(m_removeAct, SIGNAL(triggered()), m_canvas, SLOT( selectRemoveTool() ) );

    m_replayAct = new QAction(tr("&Replay"), this);
    m_replayAct->setShortcut(tr("Ctrl+R"));
    m_replayAct->setStatusTip(tr("Replay"));
    connect(m_replayAct, SIGNAL(triggered()), m_canvas, SLOT( replay() ) );

    m_debugAct = new QAction(tr("&Debug"), this);
    m_debugAct->setShortcut(tr("Ctrl+D"));
    m_debugAct->setStatusTip(tr("Debug"));
    connect(m_debugAct, SIGNAL(triggered()), m_canvas, SLOT( debug() ) );

    m_paintTreeAct = new QAction(tr("&Paint Tree"), this);
    m_paintTreeAct->setShortcut(tr("Ctrl+P"));
    m_paintTreeAct->setStatusTip(tr("Paint Tree"));
    m_paintTreeAct->setCheckable( true );
    connect(m_paintTreeAct, SIGNAL(toggled( bool )), m_canvas, SLOT( paintTree( bool ) ) );
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_replayAct);
    m_fileMenu->addAction(m_debugAct);
    m_fileMenu->addAction(m_paintTreeAct);
    m_fileMenu->addAction(m_quitAct);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_insertAct);
    m_editMenu->addAction(m_selectAct);
    m_editMenu->addAction(m_removeAct);
    menuBar()->addSeparator();


    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
    m_helpMenu->addAction(m_aboutQtAct);
}

void MainWindow::createToolBars()
{
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

#include "rtreetestapp.moc"
