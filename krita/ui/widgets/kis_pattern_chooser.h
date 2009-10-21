/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PATTERN_CHOOSER_H_
#define KIS_PATTERN_CHOOSER_H_

#include <QFrame>
#include <KoResourceItemChooser.h>

class QLabel;

class KisPatternChooser : public QFrame
{

    Q_OBJECT

public:
    KisPatternChooser(QWidget *parent = 0, const char *name = 0);
    virtual ~KisPatternChooser();

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResource * currentResource();

    void setCurrentItem(int row, int column);

signals:
    /// Emitted when a resource was selected
    void resourceSelected(KoResource * resource);

private slots:
    void update(KoResource * resource);

private:
    QLabel *m_lbName;
    KoResourceItemChooser * m_itemChooser;
};

#endif // KIS_PATTERN_CHOOSER_H_

