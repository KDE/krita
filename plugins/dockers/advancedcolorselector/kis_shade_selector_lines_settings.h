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

#ifndef KIS_SHADE_SELECTOR_LINES_SETTINGS_H
#define KIS_SHADE_SELECTOR_LINES_SETTINGS_H

#include <QWidget>

class KisShadeSelectorLineComboBox;

class KisShadeSelectorLinesSettings : public QWidget
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLinesSettings(QWidget *parent = 0);
    QString toString() const;
    void fromString(const QString& stri);

public Q_SLOTS:
    void updateSettings();
    void setLineCount(int count);

Q_SIGNALS:
    void setGradient(bool);
    void setPatches(bool);
    void setPatchCount(int count);
    void setLineHeight(int height);

Q_SIGNALS:
    void lineCountChanged(int newLineCount);

private:
    QList<KisShadeSelectorLineComboBox*> m_lineList;

};

#endif // KIS_SHADE_SELECTOR_LINES_SETTINGS_H
