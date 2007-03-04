/*
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_TOOL_TEXT_H_
#define KIS_TOOL_TEXT_H_

#include "kis_tool_paint.h"

#include "KoToolFactory.h"
#include <kis_layer_shape.h>

class QFont;
class QLabel;
class QWidget;
class QPushButton;
class KSqueezedTextLabel;

class KisToolText : public KisToolPaint {
    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolText(KoCanvasBase * canvas);
    virtual ~KisToolText();

    //virtual enumToolType toolType() { return TOOL_FILL; }
    virtual void mouseReleaseEvent(KoPointerEvent *e);

    virtual QWidget* createOptionWidget();
public slots:
    virtual void setFont();

private:

    QFont m_font;
    QLabel *m_lbFont;
    KSqueezedTextLabel *m_lbFontName;
    QPushButton *m_btnMoreFonts;
};


class KisToolTextFactory : public KoToolFactory {

public:
    KisToolTextFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KritaFill/KisToolText", i18n( "Text" ))
        {
            setToolTip(i18n("Place a single line of uneditable text on the canvas"));
            //setToolType(TOOL_TYPE_FILL);
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
            setToolType( dynamicToolType() );
            setPriority(0);
            setIcon("tool_text");
        }

    virtual ~KisToolTextFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolText(canvas);
    }

};


#endif // KIS_TOOL_TEXT_H_

