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

#include <KConfigGroup>
#include <KoToolBase.h>
#include <QPushButton>
#include <QFontComboBox>
#include <QPointer>

class KoSelection;
class SvgTextEditor;
class KoSvgTextShape;

class SvgTextTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit SvgTextTool(KoCanvasBase *canvas);
    ~SvgTextTool() override;
    /// reimplemented from KoToolBase
    void paint(QPainter &gc, const KoViewConverter &converter) override;
    /// reimplemented from KoToolBase
    void mousePressEvent(KoPointerEvent *event) override;
    /// reimplemented from superclass
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseMoveEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseReleaseEvent(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    /// reimplemented from KoToolBase
    void activate(ToolActivation activation, const QSet<KoShape *> &shapes) override;
    /// reimplemented from KoToolBase
    void deactivate() override;

protected:
    /// reimplemented from KoToolBase
    virtual QWidget *createOptionWidget() override;

    KoSelection *koSelection() const;
    KoSvgTextShape *selectedShape() const;

private Q_SLOTS:

    void showEditor();
    void textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs, bool richTextUpdated);

    /**
     * @brief generateDefs
     * This generates a defs section with the appropriate
     * css and css strings assigned. This allows the artist
     * to select settings that new texts will be created with.
     * @return a string containing the defs.
     */
    QString generateDefs();

    /**
     * @brief storeDefaults
     * store default font and point size when they change.
     */
    void storeDefaults();

private:
    QPointer<SvgTextEditor> m_editor;
    QPushButton *m_edit;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    bool m_dragging;
    QFontComboBox *m_defFont;
    QComboBox *m_defPointSize;
    QButtonGroup *m_defAlignment;
    KConfigGroup m_configGroup;

    QRectF m_hoveredShapeHighlightRect;
};

#endif
