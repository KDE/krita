#ifndef TABLEOFCONTENTSCONFIGURE_H
#define TABLEOFCONTENTSCONFIGURE_H

#include <QDialog>
#include "ui_TableOfContentsConfigure.h"

namespace Ui {
    class TableOfContentsConfigure;
}

class KoTextEditor;
class QTextBlock;
class TableOfContentsStyleConfigure;

class TableOfContentsConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit TableOfContentsConfigure(KoTextEditor *editor, QWidget *parent = 0);
    ~TableOfContentsConfigure();

public slots:
    void setDisplay();
    void save();

private slots:
    void tocListIndexChanged(int index);
    void showStyleConfiguration(bool show);

private:

    Ui::TableOfContentsConfigure ui;
    KoTextEditor *m_textEditor;
    TableOfContentsStyleConfigure *m_tocStyleConfigure;
    QTextDocument *document;
};

Q_DECLARE_METATYPE(QTextBlock)
#endif // TABLEOFCONTENTSCONFIGURE_H
