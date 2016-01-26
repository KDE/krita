/*
 *
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef __KIS_TOOL_TEXT_H__
#define __KIS_TOOL_TEXT_H__

#include <QRect>
#include "KoToolFactoryBase.h"

#include <kis_tool_rectangle_base.h>
#include "kis_text_tool_option_widget.h"
#include <kis_icon.h>

#include <kconfig.h>
#include <kconfiggroup.h>



/**
 * You are now reading a beautiful solution to a hideous abstraction. The old
 * Calligra suite developed text interaction methods that were intended to be
 * appropriate for an entire suite of office apps at once. This tool hooks up
 * Krita to the two old text interaction methods, one designed to be like a word
 * processor for a paragraph of text ("texttool"), the other designed to make a
 * very pretty single line of text ("artistictexttool"), and squishes them
 * together into a single digital-painting wrapper. Surprisingly, the base class
 * is KisToolRectangle. This is because the first interaction the user makes
 * will be drawing an outline for the text area. After the shape is drawn, the
 * rectangle tool will deactivate, switching to one of the two text tools
 * which will be used to interact with the new shape on canvas.
 *
 * I call "not dibs" on cleaning this up.
 *
 * TODO: Disallow the appearance of the Artistic Text Shape tool in the toolbox.
 */
class KisToolText : public KisToolRectangleBase
{
    Q_OBJECT

public:
    KisToolText(KoCanvasBase * canvas);
    virtual ~KisToolText();

    /// These functions are inherited from KisToolRectangleBase. They simply
    /// draw a rectangle. endPrimaryAction calls finishRect.
    virtual void beginPrimaryAction(KoPointerEvent *event);
    virtual void continuePrimaryAction(KoPointerEvent *event);
    virtual void endPrimaryAction(KoPointerEvent *event);

    virtual QList<QPointer<QWidget> > createOptionWidgets();

    virtual KisPainter::FillStyle fillStyle();

private:
    KConfigGroup m_configGroup;

private Q_SLOTS:

    /// slotActivateTextTool signals to the ToolManager that the Rectangle Tool
    /// work is done, and it should switch to a new tool for text editing.
    void slotActivateTextTool();
    void styleIndexChanged(int index);
    void textTypeIndexChanged(int index);

protected:
    /// finishRect places the text box on canvas and calls slotActivateTextTool.
    virtual void finishRect(const QRectF& rect);
    KisTextToolOptionWidget* m_optionsWidget;
};

class KisToolTextFactory : public KoToolFactoryBase
{

public:
    KisToolTextFactory()
            : KoToolFactoryBase("KritaShape/KisToolText") {
        setToolTip(i18n("Text Tool"));

        setToolType(mainToolType());
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("draw-text"));
        setPriority(2);
    }

    virtual ~KisToolTextFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolText(canvas);
    }

};


#endif // __KIS_TOOL_TEXT_H__

