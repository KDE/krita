
#ifndef SCROLLVIEW_H
#define SCROLLVIEW_H

#include <q3scrollview.h>
//Added by qt3to4:
#include <QFocusEvent>

#include "kformuladefs.h"

class KFormulaWidget;

using namespace KFormula;


class ScrollView : public Q3ScrollView {
    Q_OBJECT
public:
    ScrollView();

    virtual void addChild(KFormulaWidget* c, int x=0, int y=0);

protected:
    virtual void focusInEvent(QFocusEvent* event);

protected slots:

    void cursorChanged(bool visible, bool selecting);

private:
    KFormulaWidget* child;
};

#endif // SCROLLVIEW_H
