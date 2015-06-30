#ifndef KIS_ONION_SKIN_DIALOG_H
#define KIS_ONION_SKIN_DIALOG_H

#include <QDialog>
#include <QSlider>

namespace Ui {
class KisOnionSkinDialog;
}

class KisOnionSkinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KisOnionSkinDialog(QWidget *parent = 0);
    ~KisOnionSkinDialog();

private:
    Ui::KisOnionSkinDialog *ui;

    QSlider numberOfSkins;
    QVector<QSlider*> forwardOpacities;
    QVector<QSlider*> backwardOpacities;

private slots:
    void changed();
};

#endif // KIS_ONION_SKIN_DIALOG_H
