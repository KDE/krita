/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TOOL_H_
#define KIS_TOOL_H_

#include <QObject>
#include <QString>
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>
#include <kicon.h>

#include <kaction.h>
#include <kicon.h>

#include <KoTool.h>

#include "kis_shared_ptr_vector.h"
#include <krita_export.h>

class QCursor;
class QEvent;
class QKeyEvent;
class QRect;
class QWidget;
class QActionGroup;

class KActionCollection;
class KAction;
class KDialog;
class KisBrush;
class KisGradient;
class KisPattern;
class KoPointerEvent;
class QPainter;

static const QString TOOL_TYPE_SHAPE = "Krita/Shape"; // Geometric shapes like ellipses and lines
static const QString TOOL_TYPE_FREEHAND = "Krita/Freehand"; // Freehand drawing tools
static const QString TOOL_TYPE_TRANSFORM = "Krita/Transform" // Tools that transform the layer;
static const QString TOOL_TYPE_FILL = "Krita/Fill"; // Tools that fill parts of the canvas
static const QString TOOL_TYPE_VIEW = "Krita/View"; // Tools that affect the canvas: pan, zoom, etc.
static const QString TOOL_TYPE_SELECTED = "Krita/Select"; // Tools that select pixels

class KRITAUI_EXPORT KisTool : public KoTool {

public:
    KisTool(const QString & name);
    virtual ~KisTool();

public:

    virtual void paint(QPainter& gc) = 0;
    virtual void paint(QPainter& gc, const QRect& rc) = 0;

    /**
     * This function is called after the creation of a tool to create the KAction corresponding
     * to the tool.
     *
     * The code should look like :
     * @code
     *
     * @endcode
     */
    virtual void setup(KActionCollection *collection) = 0;

    virtual void buttonPress(KoPointerEvent *e) = 0;
    virtual void move(KoPointerEvent *e) = 0;
    virtual void buttonRelease(KoPointerEvent *e) = 0;
    virtual void doubleClick(KoPointerEvent *e) = 0;
    virtual void keyPress(QKeyEvent *e) = 0;
    virtual void keyRelease(QKeyEvent *e) = 0;

    virtual QCursor cursor() = 0;
    virtual void setCursor(const QCursor& cursor) = 0;
    /**
     * This function is called to create the configuration widget of the tool.
     * @param parent the parent of the widget
     */
    virtual QWidget* createOptionWidget();
    /**
     * @return the current configuration widget.
     */
    virtual QWidget* optionWidget();
    KAction *action() const { return m_action; }

    /**
     * Return true if this tool wants auto canvas-scrolling to
     * work when this tool is active.
     */
    virtual bool wantsAutoScroll() const { return true; }

    // Methods for integration with karbon-style toolbox
    virtual quint32 priority() { return 0; }
    virtual enumToolType toolType() { return TOOL_FREEHAND; }
    virtual QIcon icon() { return m_action->icon(); }
    virtual QString quickHelp() const { return ""; }

public slots:
    /**
     * This slot is called when the tool is selected in the toolbox
     */
    virtual void activate() = 0;

    /**
     * deactivate is called when the tool gets deactivated because another
     * tool is selected. Tools can then clean up after themselves.
     */
    virtual void deactivate() = 0;

private:
    KisTool(const KisTool&);
    KisTool& operator=(const KisTool&);

protected:
    /**
     * The exclusive action group that all tools belong to.
     */
    QActionGroup *actionGroup() const;

    KAction *m_action;
    bool m_ownAction;

private:
    class KisToolPrivate;
    KisToolPrivate * d;

    static QActionGroup *toolActionGroup;
};

#endif // KIS_TOOL_H_

