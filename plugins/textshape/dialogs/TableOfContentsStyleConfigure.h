#ifndef TABLEOFCONTENTSSTYLECONFIGURE_H
#define TABLEOFCONTENTSSTYLECONFIGURE_H

#include <QDialog>

namespace Ui {
    class TableOfContentsStyleConfigure;
}

class QStandardItemModel;
class KoStyleManager;


class TableOfContentsStyleConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit TableOfContentsStyleConfigure(KoStyleManager *manager, QWidget *parent = 0);
    ~TableOfContentsStyleConfigure();
    void initializeUi();

private:
    Ui::TableOfContentsStyleConfigure *ui;
    QStandardItemModel *m_stylesTree;
    KoStyleManager *m_styleManager;
};

#endif // TABLEOFCONTENTSSTYLECONFIGURE_H
