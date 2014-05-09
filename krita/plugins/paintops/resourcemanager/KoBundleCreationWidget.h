#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <kdialog.h>

namespace Ui
{
class KoBundleCreationWidget;
}

class KoBundleCreationWidget : public KDialog
{
    Q_OBJECT

public:
    explicit KoBundleCreationWidget(QWidget *parent = 0);
    ~KoBundleCreationWidget();

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
    Ui::KoBundleCreationWidget *m_ui;
};

#endif // KOBUNDLECREATIONWIDGET_H
