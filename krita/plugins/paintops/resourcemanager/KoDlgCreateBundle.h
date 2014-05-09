#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <kdialog.h>

namespace Ui
{
class WdgDlgCreateBundle;
}

class KoDlgCreateBundle : public KDialog
{
    Q_OBJECT

public:
    explicit KoDlgCreateBundle(QWidget *parent = 0);
    ~KoDlgCreateBundle();

    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;

private slots:

    void accept();

private:
    QWidget *m_page;
    Ui::WdgDlgCreateBundle *m_ui;
};

#endif // KOBUNDLECREATIONWIDGET_H
