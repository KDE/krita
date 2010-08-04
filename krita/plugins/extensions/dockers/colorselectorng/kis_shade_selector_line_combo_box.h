#ifndef KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
#define KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H

#include <QComboBox>

class KisShadeSelectorLineComboBoxPrivate;
class KisShadeSelectorLine;

class KisShadeSelectorLineComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLineComboBox(QWidget *parent = 0);
    void hidePopup();
    void showPopup();
    void setConfiguration(const QString& stri);
    QString configuration() const;
    void setLineNumber(int n);
//    QSize sizeHint() const;

protected:
    void resizeEvent(QResizeEvent *e);

public slots:
    void updateSettings();
    void setGradient(bool);
    void setPatches(bool);
    void setPatchCount(int count);
    void setLineHeight(int height);

private:
    KisShadeSelectorLineComboBoxPrivate* m_private;
    KisShadeSelectorLine* m_currentLine;

};

#endif // KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
