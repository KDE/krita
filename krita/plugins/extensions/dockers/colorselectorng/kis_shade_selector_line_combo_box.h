#ifndef KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
#define KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H

#include <QComboBox>

class KisShadeSelectorLineComboBoxPrivate;

class KisShadeSelectorLineComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLineComboBox(QWidget *parent = 0);
    void hidePopup();
    void showPopup();
    void setConfiguration(const QString& stri);

signals:

public slots:
private:
    KisShadeSelectorLineComboBoxPrivate* m_private;

};

#endif // KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
