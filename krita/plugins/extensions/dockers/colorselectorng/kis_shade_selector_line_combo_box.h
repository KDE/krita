/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
    QString configuration() const;
    void setLineNumber(int n);
//    QSize sizeHint() const;

protected:
    void resizeEvent(QResizeEvent *e);

public slots:
    void setConfiguration(const QString& stri);
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
