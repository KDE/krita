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
#include <KoIcon.h>

class KisToolText : public KisToolRectangleBase
{
    Q_OBJECT

public:
    KisToolText(KoCanvasBase * canvas);
    virtual ~KisToolText();

    virtual QList< QWidget* > createOptionWidgets();

    virtual KisPainter::FillStyle fillStyle();

private slots:
    void slotActivateTextTool();

protected:
    virtual void finishRect(const QRectF& rect);
    KisTextToolOptionWidget* m_optionWidget;
};

class KisToolTextFactory : public KoToolFactoryBase
{

public:
    KisToolTextFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolText") {
        setToolTip(i18n("Text tool"));

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

