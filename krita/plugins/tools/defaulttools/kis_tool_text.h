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

class QFont;
class QLabel;
class QWidget;
class QPushButton;
class KSqueezedTextLabel;

class KisToolText : public KisToolPaint {
    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolText();
    virtual ~KisToolText();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_FILL; }
    virtual void buttonRelease(KoPointerEvent *e);

    virtual QWidget* createOptionWidget(QWidget* parent);
public slots:
    virtual void setFont();

private:
    KisCanvasSubject *m_subject;
    QFont m_font;
    QLabel *m_lbFont;
    KSqueezedTextLabel *m_lbFontName;
    QPushButton *m_btnMoreFonts;
};


class KisToolTextFactory : public KoToolFactory {

public:
    KisToolTextFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolText", i18n( "Text" ))
        {
            setToolTip(i18n("Place a single line of uneditable text on the canvas"));
            setToolType(TOOL_TYPE_FILL);
            setPriority(0);
            setIcon("tool_text");
        }

    virtual ~KisToolTextFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolText(canvas);
    }

};


#endif // KIS_TOOL_TEXT_H_

