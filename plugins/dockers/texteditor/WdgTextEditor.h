#ifndef WDGTEXTEDITOR_H
#define WDGTEXTEDITOR_H

#include <QWidget>

namespace Ui {
class WdgTextEditor;
}

class WdgTextEditor : public QWidget
{
    Q_OBJECT

public:
    explicit WdgTextEditor(QWidget *parent = 0);
    ~WdgTextEditor();

private:
    Ui::WdgTextEditor *ui;
};

#endif // WDGTEXTEDITOR_H
