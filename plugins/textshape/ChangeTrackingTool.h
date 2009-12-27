/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef CHANGETRACKINGTOOL_H
#define CHANGETRACKINGTOOL_H

#include <KoTool.h>

#include <QModelIndex>

class KoCanvasBase;
class KoPointerEvent;
class KoTextEditor;
class KoTextShapeData;
class KoViewConverter;
class TextShape;
class TrackedChangeModel;

class QPainter;
class QKeyEvent;

/// This tool allows to manipulate the tracked changes of a document. You can accept or reject changes.

class ChangeTrackingTool : public KoTool
{
    Q_OBJECT
public:
    ChangeTrackingTool(KoCanvasBase *canvas);

    ~ChangeTrackingTool();

    virtual void mouseReleaseEvent(KoPointerEvent* event);
    virtual void mouseMoveEvent(KoPointerEvent* event);
    virtual void mousePressEvent(KoPointerEvent* event);
    virtual void paint(QPainter& painter, const KoViewConverter& converter);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void activate(bool temporary = false);
    virtual void deactivate();

protected:
    virtual QWidget* createOptionWidget();

private slots:
    void acceptChange();
    void rejectChange();
    void selectedChangeChanged(QModelIndex item);
    void setShapeData(KoTextShapeData *data);
    void showTrackedChangeManager();

private:
    bool m_disableShowChangesOnExit;
    KoTextEditor *m_textEditor;
    KoTextShapeData *m_textShapeData;
    TextShape *m_textShape;
    TrackedChangeModel *m_model;

    QModelIndex m_currentHighlightedChange;
};

#endif // CHANGETRACKINGTOOL_H
