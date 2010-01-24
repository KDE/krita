/* This file is part of the KDE project
   Copyright (C) 2003 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2005 Rob Buis <buis@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#if 0

// 8x8 pixels

static const char* const buttonnone[]={
"8 8 7 1",
"c c #0a0000",
". c #0a0a0a",
"b c #330000",
"a c #331f1f",
"# c #333333",
"e c #ff0000",
"d c #ffffff",
".####abc",
"#ddddeeb",
"#dddeeea",
"#ddeeed#",
"#deeedd#",
"aeeeddd#",
"beedddd#",
"cba####."};

static const char* const buttonsolid[]={
"8 8 1 1",
". c #000000",
"........",
"........",
"........",
"........",
"........",
"........",
"........",
"........"};

static const char* const buttongradient[]={
"8 8 14 1",
"f c #000000",
"e c #040404",
". c #0a0a0a",
"d c #0f0f0f",
"c c #181818",
"b c #212121",
"a c #292929",
"# c #303030",
"g c #333333",
"l c #484848",
"k c #787878",
"j c #a7a7a7",
"i c #cdcdcd",
"h c #f1f1f1",
".#abcdef",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
"ghijkl#f",
".#abcdef"};

static const char* const buttonpattern[]={
"8 8 4 1",
". c #0a0a0a",
"# c #333333",
"a c #a0a0a0",
"b c #ffffff",
".######.",
"#aabbaa#",
"#aabbaa#",
"#bbaabb#",
"#bbaabb#",
"#aabbaa#",
"#aabbaa#",
".######."};

#else

// 16x16 pixels

static const char* const buttonnone[]={
"16 16 3 1",
"# c #333333",
"e c #ff0000",
"- c #ffffff",
"################",
"#------------ee#",
"#-----------eee#",
"#----------eee-#",
"#---------eee--#",
"#--------eee---#",
"#-------eee----#",
"#------eee-----#",
"#-----eee------#",
"#----eee-------#",
"#---eee--------#",
"#--eee---------#",
"#-eee----------#",
"#eee-----------#",
"#ee------------#",
"################"};

static const char* const buttonsolid[]={
"16 16 2 1",
"# c #000000",
". c #969696",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};


// FIXME: Smoother gradient button.

static const char* const buttongradient[]={
"16 16 15 1",
"# c #000000",
"n c #101010",
"m c #202020",
"l c #303030",
"k c #404040",
"j c #505050",
"i c #606060",
"h c #707070",
"g c #808080",
"f c #909090",
"e c #a0a0a0",
"d c #b0b0b0",
"c c #c0c0c0",
"b c #d0d0d0",
"a c #e0e0e0",
"################",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"#abcdefghijklmn#",
"################"};

static const char* const buttonpattern[]={
"16 16 4 1",
". c #0a0a0a",
"# c #333333",
"a c #a0a0a0",
"b c #ffffffff",
"################",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#bbbbbaaaabbbbb#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"#aaaaabbbbaaaaa#",
"################"};

static const char* const buttonevenodd[]={
"16 16 3 1",
"# c #0a0a0a",
". c #969696",
"o c #ffffffff",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#....######....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....#oooo#....#",
"#....######....#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};

static const char* const buttonwinding[]={
"16 16 2 1",
"# c #0a0a0a",
". c #969696",
"################",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"#....######....#",
"#....#....#....#",
"#....#....#....#",
"#....#....#....#",
"#....#....#....#",
"#....######....#",
"#..............#",
"#..............#",
"#..............#",
"#..............#",
"################"};

#endif


#include "StyleButtonBox.h"

#include <klocale.h>
#include <kicon.h>

#include <QtGui/QPixmap>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>

class StyleButtonBox::Private
{
public:
    Private()
    : group(0)
    , rowCount(1)
    , columnCount(6)
    {}

    QButtonGroup* group;
    int rowCount;
    int columnCount;
};

StyleButtonBox::StyleButtonBox(QWidget* parent)
: QWidget(parent)
, d(new Private())
{
    //setMinimumSize(45, 70);
    setContentsMargins(0, 0, 0, 0);

    QGridLayout * layout = new QGridLayout(this);
    d->group = new QButtonGroup(this);

    // The button for no fill
    QToolButton* button = new QToolButton(this);
    //button->setIcon(QPixmap((const char **) buttonnone));
    button->setIcon(KIcon("edit-delete"));
    button->setToolTip(i18nc("No stroke or fill", "None"));
    d->group->addButton(button, None);

    // The button for solid fill
    button = new QToolButton(this);
    button->setIcon(QPixmap((const char **) buttonsolid));
    button->setToolTip(i18nc("Solid color stroke or fill", "Solid"));
    d->group->addButton(button, Solid);

    // The button for gradient fill
    button = new QToolButton(this);
    button->setIcon(QPixmap((const char **) buttongradient));
    button->setToolTip(i18n("Gradient"));
    d->group->addButton(button, Gradient);

    // The button for pattern fill
    button = new QToolButton(this);
    button->setIcon(QPixmap((const char **) buttonpattern));
    button->setToolTip(i18n("Pattern"));
    d->group->addButton(button, Pattern);

    // The button for even-odd fill rule
    button = new QToolButton(this);
    button->setIcon(QPixmap((const char **) buttonevenodd));
    button->setToolTip(i18n("Even-Odd Fill"));
    d->group->addButton(button, EvenOdd);

    // The button for winding fill-rule
    button = new QToolButton(this);
    button->setIcon(QPixmap((const char **) buttonwinding));
    button->setToolTip(i18n("Winding Fill"));
    d->group->addButton(button, Winding);

    int index = 1;
    for(int row = 0; row < d->rowCount; ++row) {
        for(int col = 0; col < d->columnCount; ++col) {
            layout->addWidget(d->group->button(index), row, col);
            index = index<<1;
            if(index > Winding)
                break;
        }
        if(index > Winding)
            break;
    }

    layout->setMargin(0);
    layout->setSpacing(1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(3, 1);

    connect(d->group, SIGNAL(buttonClicked(int)), this, SIGNAL(buttonPressed(int)));
}

StyleButtonBox::~StyleButtonBox()
{
    delete d;
}

void StyleButtonBox::showButtons(StyleButtons buttons)
{
    foreach(QAbstractButton * b, d->group->buttons()) {
        b->setVisible(buttons & d->group->id(b));
    }
}

#include <StyleButtonBox.moc>
