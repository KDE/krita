/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
