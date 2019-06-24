/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
