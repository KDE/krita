/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
#define KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H

#include <QComboBox>

class KisShadeSelectorLineComboBoxPopup;
class KisShadeSelectorLine;
class KisColorSelectorBaseProxy;

class KisShadeSelectorLineComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLineComboBox(QWidget *parent = 0);
    ~KisShadeSelectorLineComboBox() override;
    void hidePopup() override;
    void showPopup() override;
    QString configuration() const;
    void setLineNumber(int n);

protected:
    void resizeEvent(QResizeEvent *e) override;

public Q_SLOTS:
    void setConfiguration(const QString& stri);
    void updateSettings();
    void setGradient(bool);
    void setPatches(bool);
    void setPatchCount(int count);
    void setLineHeight(int height);

private:
    KisShadeSelectorLineComboBoxPopup* m_popup;
    QScopedPointer<KisColorSelectorBaseProxy> m_parentProxy;
    KisShadeSelectorLine* m_currentLine;

};

#endif // KIS_SHADE_SELECTOR_LINE_COMBO_BOX_H
