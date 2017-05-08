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

#ifndef REVIEWTOOL_H
#define REVIEWTOOL_H

class KoCanvasBase;
class KoPointerEvent;
class KoTextEditor;
class KoTextShapeData;
class KoViewConverter;
class TextShape;

class QAction;

class QPainter;
class QKeyEvent;
template <class T> class QVector;
/// This tool allows to manipulate the tracked changes of a document. You can accept or reject changes.

#include <TextTool.h>

class ReviewTool : public TextTool
{
    Q_OBJECT
public:
    explicit ReviewTool(KoCanvasBase *canvas);
    ~ReviewTool() override;

    void mouseReleaseEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mousePressEvent(KoPointerEvent *event) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void keyPressEvent(QKeyEvent *event) override;
    void activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes) override;
    void deactivate() override;
    void createActions() override;

    QList<QPointer<QWidget> > createOptionWidgets() override;

public Q_SLOTS:
    void removeAnnotation();

private:

    KoTextEditor *m_textEditor;
    KoTextShapeData *m_textShapeData;
    KoCanvasBase *m_canvas;
    TextShape *m_textShape;
    QAction *m_removeAnnotationAction;
    KoShape *m_currentAnnotationShape;
};

#endif // REVIEWTOOL_H
