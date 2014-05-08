#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <QDialog>

class KoXmlResourceBundleMeta;
class KoResourceManagerControl;

namespace Ui
{
class KoBundleCreationWidget;
}

class KoBundleCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit KoBundleCreationWidget(KoXmlResourceBundleMeta* m_newMeta, QWidget *parent = 0);
    ~KoBundleCreationWidget();

    void initializeUI();

signals:
    void status(QString text, int timeout = 0);

private slots:
    void createBundle();
    void showHide();

private:
    Ui::KoBundleCreationWidget *m_ui;
    KoXmlResourceBundleMeta *m_newMeta;
    QString m_kritaPath;
};

#endif // KOBUNDLECREATIONWIDGET_H
