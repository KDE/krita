/* This file is part of the KDE project
   Copyright 2009 Jeremias Epperlein <jeeree@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TestCursor.h"

#include <qtest_kde.h>
#include "FormulaCursor.h"
#include "FormulaData.h"
#include <KoDocument.h>
#include "FormulaCommand.h"
#include "FormulaCommandUpdate.h"
#include "KoFormulaTool.h"
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoCanvasBase.h>
#include <kdebug.h>
#include <FormulaEditor.h>

class MockCanvas : public KoCanvasBase
{
public:
    KUndo2QStack *stack;
    KoShapeManager *manager;
    MockCanvas(): KoCanvasBase(0) {
        stack=new KUndo2QStack();
        manager=new KoShapeManager(this);
    }
    ~MockCanvas() {
        delete stack;
    }

    void gridSize(qreal *, qreal *) const {}
    bool snapToGrid() const  {
        return false;
    }

    void addCommand(KUndo2Command* c) {
//         c->redo();
        stack->push(c);
    }
    KoShapeManager *shapeManager() const  {
        return manager;
    }
    void updateCanvas(const QRectF&)  {}
    KoToolProxy * toolProxy() const {
        return 0;
    }
    KoViewConverter *viewConverter() const {
        return 0;
    }
    QWidget* canvasWidget() {
        return 0;
    }
    const QWidget* canvasWidget() const {
        return 0;
    }
    KoUnit unit() const {
        return KoUnit(KoUnit::Millimeter);
    }
    void updateInputMethodInfo() {}
    void setCursor(const QCursor &) {}
};

void TestCursor::moveCursor()
{
    MockCanvas* canvas=new MockCanvas();
    KoFormulaShape* shape = new KoFormulaShape(NULL); // FIXME: Do we need a real resourceManager here?
    canvas->shapeManager()->addShape(shape);
    canvas->shapeManager()->selection()->select(shape);
    QCOMPARE(canvas->shapeManager()->selection()->count(),1);
    KoFormulaTool* tool= new KoFormulaTool(canvas);
    QSet<KoShape*> selectedShapes;
    selectedShapes << shape;
    tool->activate(KoToolBase::DefaultActivation, selectedShapes);
    FormulaEditor* editor=tool->formulaEditor();
    FormulaElement* root=editor->formulaData()->formulaElement();
    canvas->addCommand(new FormulaCommandUpdate(shape,editor->insertText("ade")));
    editor->cursor().moveTo(root->childElements()[0],1);
   //(a|de)
    canvas->addCommand(new FormulaCommandUpdate(shape,editor->insertText("bc")));
    editor->cursor().moveTo(root->childElements()[0],6);
    //(abcde|)
    editor->cursor().move(MoveLeft);
    //(abcd|e)
    QCOMPARE(editor->cursor().position(),5);
    editor->cursor().moveTo(root->childElements()[0],0);
    editor->cursor().move(MoveLeft);
    //|(abcde)
    QCOMPARE(editor->cursor().position(),0);
    canvas->addCommand(new FormulaCommandUpdate(shape,editor->insertText("123")));
    QCOMPARE(root->childElements().count(),2);
    //(12)(abcde)
    canvas->stack->undo();
    //(abcde)
    canvas->stack->redo();
    //(12)(abcde)
    QCOMPARE(root->childElements().count(),2);
    canvas->stack->clear();
}


QTEST_KDEMAIN(TestCursor,GUI)
#include "TestCursor.moc"
