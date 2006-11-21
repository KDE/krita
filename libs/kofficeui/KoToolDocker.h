/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KO_TOOL_DOCKER
#define KO_TOOL_DOCKER

#include <QDockWidget>
#include <QStackedWidget>

#include <kdebug.h>

#include <KoDockFactory.h>
#include <KoCanvasBase.h>


/**
   The tool docker shows the tool option widget associtated with the
   current tool and the current canvas.
 */
class KoToolDocker : public QDockWidget
{
public:

    KoToolDocker(KoCanvasBase * canvas)
        : QDockWidget(i18n("Tool Options"))
        , m_canvas( canvas ) 
        {
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            m_stack = new QStackedWidget(this);
            setWidget(m_stack);
        }

    virtual ~KoToolDocker() {}

    void setOptionWidget(QWidget * widget)
        {
            if (widget && m_stack->indexOf(widget) == -1) {
                m_stack->addWidget(widget);
            }
            m_stack->setCurrentWidget(widget);
        }
private:
    KoToolDocker();

    QStackedWidget * m_stack;
    KoCanvasBase * m_canvas; 

};

class KoToolDockerFactory : public KoDockFactory
{
public:

    KoToolDockerFactory( KoCanvasBase * canvas )
        : m_canvas(canvas) {}

    ~KoToolDockerFactory() {}

    virtual QString dockId() const
        {
            return QString("KoToolOptionsDocker");
        }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
        {
            return Qt::RightDockWidgetArea;
        }

    virtual QDockWidget* createDockWidget()
        {
            KoToolDocker * dockWidget = new KoToolDocker(m_canvas);
            dockWidget->setObjectName( dockId() );
            return dockWidget;
        }
private:

    KoCanvasBase * m_canvas;
};

#endif
