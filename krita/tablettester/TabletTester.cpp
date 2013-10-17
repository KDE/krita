/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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
 */
#include <QMainWindow>
#include <QTabletEvent>
#include <QWidget>
#include <QApplication>
#include <QEvent>
#include <QSplitter>
#include <QPen>
#include <QColor>
#include <QBrush>
#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QPlainTextEdit>

QString eventToString(QTabletEvent *event) {

    QString type;
    switch(event->type()) {
    case QEvent::TabletMove:
        type = "TabletMove";
        break;
    case QEvent::TabletPress:
        type = "TabletPress";
        break;
    case QEvent::TabletRelease:
        type = "TabletRelease";
        break;
    case QEvent::TabletEnterProximity:
        type = "TabletEnterProximity";
        break;
    case QEvent::TabletLeaveProximity:
        type = "TabletLeaveProximity";
        break;
    default:
        type = QString("Event Type: %1").arg(event->type());

    }

    QString pointerType;
    switch(event->pointerType()) {
    case QTabletEvent::UnknownPointer:
        pointerType = "Unknown Pointer";
        break;
    case QTabletEvent::Pen:
        pointerType = "Pen";
        break;
    case QTabletEvent::Cursor:
        pointerType = "Cursor";
        break;
    case QTabletEvent::Eraser:
        pointerType = "Eraser";
        break;
    default:
        pointerType = QString("Unknown Pointer Type: %1").arg(event->pointerType());
    }

    QString tabletDevice;
    switch(event->device()) {
    case QTabletEvent::NoDevice:
        tabletDevice = "NoDevice";
        break;
    case QTabletEvent::Puck:
        tabletDevice = "Puck";
        break;
    case QTabletEvent::Stylus:
        tabletDevice = "Stylus";
        break;
    case QTabletEvent::Airbrush:
        tabletDevice = "Airbrush";
        break;
    case QTabletEvent::FourDMouse:
        tabletDevice = "FourDMouse";
        break;
    case QTabletEvent::RotationStylus:
        tabletDevice = "RotationStylus";
        break;
    default:
        tabletDevice = QString("Unknown Tablet Device: %1").arg(event->device());
    }

    return QString("Tablet event. Type: %1."
                   " Pointer Type: %2."
                   " Device: %3."
                   " Global Pos: (%4, %5)."
                   " Hires Global Pos: (%6, %7)."
                   " Pressure: %8"
                   " Rotation: %9"
                   " Tangential pressure: %10"
                   " Unique id: %11"
                   " x: %12"
                   " y: %13"
                   " xTilt: %14"
                   " yTilt: %15"
                   )
            .arg(type)
            .arg(pointerType)
            .arg(tabletDevice)
            .arg(event->globalX())
            .arg(event->globalY())
            .arg(event->hiResGlobalX())
            .arg(event->hiResGlobalY())
            .arg(event->pressure())
            .arg(event->rotation())
            .arg(event->tangentialPressure())
            .arg(event->uniqueId())
            .arg(event->x())
            .arg(event->y())
            .arg(event->xTilt())
            .arg(event->yTilt())
            ;
}

class TabletCanvas : public QWidget {
public:
    TabletCanvas(QWidget *parent)
        : QWidget(parent)
    {
        initPixmap();
        setAutoFillBackground(true);
        deviceDown = false;
        m_color = Qt::black;
    }

    void setEventLogger(QPlainTextEdit *eventLogger) {
        m_eventLogger = eventLogger;
    }

protected:

    void tabletEvent(QTabletEvent *event)
    {
        m_eventLogger->appendPlainText("Canvas: " + eventToString(event));
        switch (event->type()) {
            case QEvent::TabletPress:
                if (!deviceDown) {
                    deviceDown = true;
                    m_polyLine[0] = m_polyLine[1] = m_polyLine[2] = event->pos();
                }
                break;
            case QEvent::TabletRelease:
                if (deviceDown)
                    deviceDown = false;
                break;
            case QEvent::TabletMove:
                m_polyLine[2] = m_polyLine[1];
                m_polyLine[1] = m_polyLine[0];
                m_polyLine[0] = event->pos();

                if (deviceDown) {
                    m_color.setAlpha(int(event->pressure() * 255.0));
                    m_pen.setWidthF(event->pressure() * 10 + 1);
                    m_brush.setColor(m_color);
                    m_pen.setColor(m_color);

                    QPainter painter(&pixmap);
                    painter.setBrush(m_brush);
                    painter.setPen(m_pen);
                    painter.drawLine(m_polyLine[1], event->pos());

                }
                break;
            default:
                break;
        }
        update();
        event->ignore();
    }

    void paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.drawPixmap(0, 0, pixmap);
    }

    void resizeEvent(QResizeEvent *event) {
        initPixmap();
        m_polyLine[0] = m_polyLine[1] = m_polyLine[2] = QPoint();
    }

    void mouseMoveEvent(QMouseEvent *event) {
        m_eventLogger->appendPlainText("received mouse move event");
        event->ignore();
    }

    void mousePressEvent(QMouseEvent *event) {
        m_eventLogger->appendPlainText("received mouse press event");
        event->ignore();
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        m_eventLogger->appendPlainText("received mouse release event");
        event->ignore();
    }

    void mouseDoubleClickEvent(QMouseEvent *event) {
        m_eventLogger->appendPlainText("received mouse double click event");
        event->ignore();
    }

private:

    void initPixmap() {
        QPixmap newPixmap = QPixmap(width(), height());
        newPixmap.fill(Qt::white);
        QPainter painter(&newPixmap);
        if (!pixmap.isNull())
            painter.drawPixmap(0, 0, pixmap);
        painter.end();
        pixmap = newPixmap;
    }

    QPlainTextEdit *m_eventLogger;
    QPixmap pixmap;
    QBrush m_brush;
    QPen m_pen;
    bool deviceDown;
    QPoint m_polyLine[3];
    QColor m_color;

};

class TabletApplication : public QApplication {
public:
    TabletApplication(int &argv, char **args)
        : QApplication(argv, args)
    {
    }

    bool event(QEvent *event) {
        if (event->type() == QEvent::TabletEnterProximity ||
            event->type() == QEvent::TabletLeaveProximity) {
            m_eventLogger->appendPlainText("Application: " + eventToString(static_cast<QTabletEvent *>(event)));
            return true;
        }
        return QApplication::event(event);
    }

    void setCanvas(TabletCanvas *canvas) {
        m_canvas = canvas;
    }

    void setEventLogger(QPlainTextEdit *eventLogger) {
        m_eventLogger = eventLogger;
    }

private:
    QPlainTextEdit *m_eventLogger;
    TabletCanvas *m_canvas;
};

class MainWindow : public QMainWindow {
public:
    MainWindow(TabletCanvas *canvas, QPlainTextEdit *eventLogger) {
        m_canvas = canvas;
        m_eventLogger = eventLogger;

        QSplitter *splitter = new QSplitter(Qt::Horizontal);
        splitter->addWidget(m_canvas);
        splitter->addWidget(m_eventLogger);

        setCentralWidget(splitter);
    }

private:

    QPlainTextEdit *m_eventLogger;
    TabletCanvas *m_canvas;

};


int main(int argv, char *args[])
{
    TabletApplication app(argv, args);
    TabletCanvas *canvas = new TabletCanvas(0);
    app.setCanvas(canvas);
    QPlainTextEdit *eventLogger = new QPlainTextEdit(0);
    eventLogger->setWordWrapMode(QTextOption::NoWrap);
    eventLogger->setReadOnly(true);
    canvas->setEventLogger(eventLogger);
    app.setEventLogger(eventLogger);

    MainWindow mainWindow(canvas, eventLogger);


    mainWindow.resize(500, 500);
    mainWindow.show();
    return app.exec();
}

