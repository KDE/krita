/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KEYSIGNATUREDIALOG_H
#define KEYSIGNATUREDIALOG_H

#include <ui_KeySignatureDialog.h>

#include <KDialog>
namespace MusicCore {
    class KeySignature;
}

class KeySignatureDialog : public KDialog {
    Q_OBJECT
public:
    explicit KeySignatureDialog(QWidget *parent = 0);
    
    void setBar(int bar);
    int accidentals();
    void setAccidentals(int accidentals);
    void setMusicStyle(MusicStyle* style);
    bool updateAllStaves();
    
    bool updateToNextChange();
    bool updateTillEndOfPiece();
    
    int startBar();
    int endBar();
private slots:
    void accidentalsChanged(int accidentals);
private:
    Ui::KeySignatureDialog widget;
    MusicCore::KeySignature* m_ks;
};

#endif // KEYSIGNATUREDIALOG_H
