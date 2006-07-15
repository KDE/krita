/*
 * Copyright (c) 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QLinkedList>

#include "KoRTree.h"
#include "Tool.h"

class QAction;
class QActionGroup;
class QListWidget;
class QMenu;
class QTextEdit;

class Data
{
public:
    Data(  QRectF rect )
        : m_rect(  rect )
        {}

    QRectF boundingBox() { return m_rect; }
    void paint(  QPainter & p )
    {
        p.save();
        QPen pen(  Qt::black );
        p.setPen(  pen );
        p.drawRect(  m_rect );
        p.restore();
    }

private:
    QRectF m_rect;
};


class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas();
    virtual ~Canvas() {};

    void updateCanvas();
    void insert( QRectF & rect );
    void select( QRectF & rect );
    void remove( QRectF & rect );

public slots:
    void selectInsertTool();
    void selectSelectTool();
    void selectRemoveTool();

    void replay();
    void debug();
    void replayStep();
    void paintTree( bool paintTree );

protected: 
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void paintEvent(QPaintEvent * e);

private:
    double m_zoom;
    QSet<Data*> m_rects;
    QList<Data*> m_found;
    QRectF m_insertRect;
    bool m_buttonPressed;
    KoRTree<Data*> m_rtree;
    Tool * m_tool;
    CreateTool m_createTool;
    SelectTool m_selectTool;
    RemoveTool m_removeTool;
    QFile m_file;
    QTextStream m_out;
    QStringList m_list;
    int m_listId;
    bool m_paintTree;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void about();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    Canvas * m_canvas;

    QMenu * m_fileMenu;
    QMenu * m_editMenu;
    QMenu * m_helpMenu;

    QAction * m_aboutAct;
    QAction * m_aboutQtAct;
    QAction * m_quitAct;

    QAction * m_insertAct;
    QAction * m_selectAct;
    QAction * m_removeAct;
    QActionGroup *m_toolAct;

    QAction * m_replayAct;
    QAction * m_debugAct;
    QAction * m_paintTreeAct;
};

#endif
