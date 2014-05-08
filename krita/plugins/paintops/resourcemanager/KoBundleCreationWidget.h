#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <kdialog.h>

class KoXmlResourceBundleMeta;
class KoResourceManagerControl;

namespace Ui
{
class KoBundleCreationWidget;
}

class KoBundleCreationWidget : public KDialog
{
    Q_OBJECT

public:
    explicit KoBundleCreationWidget(KoXmlResourceBundleMeta* m_newMeta, QWidget *parent = 0);
    ~KoBundleCreationWidget();

signals:
    void status(QString text, int timeout = 0);

private slots:
    void createBundle();

private:
    QWidget *m_page;
    Ui::KoBundleCreationWidget *m_ui;
    KoXmlResourceBundleMeta *m_newMeta;
};

#endif // KOBUNDLECREATIONWIDGET_H
