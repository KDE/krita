/* This file is part of the KDE project
 *
   Copyright 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SVG_TEXT_TOOL
#define SVG_TEXT_TOOL

#include <KoToolBase.h>
#include <QPushButton>

class SvgTextEditor;
class KoSvgTextShape;

class SvgTextTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit SvgTextTool(KoCanvasBase *canvas);

    /// reimplemented from KoToolBase
    void paint(QPainter &, const KoViewConverter &) override;
    /// reimplemented from KoToolBase
    void mousePressEvent(KoPointerEvent *) override;
    /// reimplemented from superclass
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseMoveEvent(KoPointerEvent *) override;
    /// reimplemented from KoToolBase
    void mouseReleaseEvent(KoPointerEvent *) override;

    /// reimplemented from KoToolBase
    void activate(ToolActivation activation, const QSet<KoShape *> &shapes) override;
    /// reimplemented from KoToolBase
    void deactivate() override;

protected:
    /// reimplemented from KoToolBase
    virtual QWidget *createOptionWidget();

private Q_SLOTS:

    void showEditor() const;
    void textUpdated(const QString &svg, const QString &defs);

private:
    KoSvgTextShape *m_shape;
    SvgTextEditor *m_editor;
    QPushButton *m_edit;
};

#endif
