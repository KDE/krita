/****************************************************************************
** $Id$
**
** Implementation of some internal classes
**
** Created :
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#ifndef QT_NO_COMPLEXTEXT
#include "KoRichText.h"
//#include "qfontdata_p.h"
#include "qfontmetrics.h"
#include "qrect.h"
//Added by qt3to4:
#include <Q3PointArray>
#include <Q3PtrList>

#include <stdlib.h>

// -----------------------------------------------------

/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
KoBidiContext::KoBidiContext( uchar l, QChar::Direction e, KoBidiContext *p, bool o )
    : level(l) , override(o), dir(e)
{
    if ( p )
	p->ref();
    parent = p;
    count = 0;
}

KoBidiContext::~KoBidiContext()
{
    if( parent && parent->deref() )
	delete parent;
}

static QChar *shapeBuffer = 0;
static int shapeBufSize = 0;

/*
   Arabic shaping obeys a number of rules according to the joining classes (see Unicode book, section on
   arabic).

   Each unicode char has a joining class (right, dual (left&right), center (joincausing) or transparent).
   transparent joining is not encoded in QChar::joining(), but applies to all combining marks and format marks.

   Right join-causing: dual + center
   Left join-causing: dual + right + center

   Rules are as follows (for a string already in visual order, as we have it here):

   R1 Transparent characters do not affect joining behaviour.
   R2 A right joining character, that has a right join-causing char on the right will get form XRight
   (R3 A left joining character, that has a left join-causing char on the left will get form XLeft)
   Note: the above rule is meaningless, as there are no pure left joining characters defined in Unicode
   R4 A dual joining character, that has a left join-causing char on the left and a right join-causing char on
	     the right will get form XMedial
   R5  A dual joining character, that has a right join causing char on the right, and no left join causing char on the left
	 will get form XRight
   R6 A dual joining character, that has a  left join causing char on the left, and no right join causing char on the right
	 will get form XLeft
   R7 Otherwise the character will get form XIsolated

   Additionally we have to do the minimal ligature support for lam-alef ligatures:

   L1 Transparent characters do not affect ligature behaviour.
   L2 Any sequence of Alef(XRight) + Lam(XMedial) will form the ligature Alef.Lam(XLeft)
   L3 Any sequence of Alef(XRight) + Lam(XLeft) will form the ligature Alef.Lam(XIsolated)

   The two functions defined in this class do shaping in visual and logical order. For logical order just replace right with
   previous and left with next in the above rules ;-)
*/

/*
  Two small helper functions for arabic shaping. They get the next shape causing character on either
  side of the char in question. Implements rule R1.

  leftChar() returns true if the char to the left is a left join-causing char
  rightChar() returns true if the char to the right is a right join-causing char
*/
static inline const QChar *prevChar( const QString &str, int pos )
{
    //kDebug() << "leftChar: pos=" << pos << endl;
    pos--;
    const QChar *ch = str.unicode() + pos;
    while( pos > -1 ) {
	if( !ch->isMark() )
	    return ch;
	pos--;
	ch--;
    }
    return &QChar::ReplacementCharacter;
}

static inline const QChar *nextChar( const QString &str, int pos)
{
    pos++;
    int len = str.length();
    const QChar *ch = str.unicode() + pos;
    while( pos < len ) {
	//kDebug() << "rightChar: " << pos << " isLetter=" << ch.isLetter() << ", joining=" << ch.joining() << endl;
	if( !ch->isMark() )
	    return ch;
	// assume it's a transparent char, this might not be 100% correct
	pos++;
	ch++;
    }
    return &QChar::ReplacementCharacter;
}

static inline bool prevVisualCharJoins( const QString &str, int pos)
{
    return (     prevChar( str, pos )->joining() != QChar::OtherJoining );
}

static inline bool nextVisualCharJoins( const QString &str, int pos)
{
    QChar::Joining join = nextChar( str, pos )->joining();
    return ( join == QChar::Dual || join == QChar::Center );
}


KoComplexText::Shape KoComplexText::glyphVariant( const QString &str, int pos)
{
    // ignores L1 - L3, done in the codec
    QChar::Joining joining = str[pos].joining();
    //kDebug() << "checking " << str[pos].unicode() << ", joining=" << joining << endl;
    switch ( joining ) {
	case QChar::OtherJoining:
	case QChar::Center:
	    // these don't change shape
	    return XIsolated;
	case QChar::Right:
	    // only rule R2 applies
	    if( nextVisualCharJoins( str, pos ) )
		return XFinal;
	    return XIsolated;
	case QChar::Dual:
	    bool right = nextVisualCharJoins( str, pos );
	    bool left = prevVisualCharJoins( str, pos );
	    //kDebug() << "dual: right=" << right << ", left=" << left << endl;
	    if( right && left )
		return XMedial;
	    else if ( right )
		return XFinal;
	    else if ( left )
		return XInitial;
	    else
		return XIsolated;
    }
    return XIsolated;
}

/* and the same thing for logical ordering :)
 */
static inline bool prevLogicalCharJoins( const QString &str, int pos)
{
    return (     nextChar( str, pos )->joining() != QChar::OtherJoining );
}

static inline bool nextLogicalCharJoins( const QString &str, int pos)
{
    QChar::Joining join = prevChar( str, pos )->joining();
    return ( join == QChar::Dual || join == QChar::Center );
}


KoComplexText::Shape KoComplexText::glyphVariantLogical( const QString &str, int pos)
{
    // ignores L1 - L3, ligatures are job of the codec
    QChar::Joining joining = str[pos].joining();
    //kDebug() << "checking " << str[pos].unicode() << ", joining=" << joining << endl;
    switch ( joining ) {
	case QChar::OtherJoining:
	case QChar::Center:
	    // these don't change shape
	    return XIsolated;
	case QChar::Right:
	    // only rule R2 applies
	    if( nextLogicalCharJoins( str, pos ) )
		return XFinal;
	    return XIsolated;
	case QChar::Dual:
	    bool right = nextLogicalCharJoins( str, pos );
	    bool left = prevLogicalCharJoins( str, pos );
	    //kDebug() << "dual: right=" << right << ", left=" << left << endl;
	    if( right && left )
		return XMedial;
	    else if ( right )
		return XFinal;
	    else if ( left )
		return XInitial;
	    else
		return XIsolated;
    }
    return XIsolated;
}

// -------------------------------------------------------------

// The unicode to unicode shaping codec.
// does only presentation forms B at the moment, but that should be enough for
// simple display
static const ushort arabicUnicodeMapping[256][2] = {
    // base of shaped forms, and number-1 of them ( 0 for non shaping,
    // 1 for right binding and 3 for dual binding
    { 0x0600, 0 }, // 0x600
    { 0x0601, 0 }, // 0x601
    { 0x0602, 0 }, // 0x602
    { 0x0603, 0 }, // 0x603
    { 0x0604, 0 }, // 0x604
    { 0x0605, 0 }, // 0x605
    { 0x0606, 0 }, // 0x606
    { 0x0607, 0 }, // 0x607
    { 0x0608, 0 }, // 0x608
    { 0x0609, 0 }, // 0x609
    { 0x060a, 0 }, // 0x60a
    { 0x060b, 0 }, // 0x60b
    { 0x060c, 0 }, // 0x60c     Arabic comma
    { 0x060d, 0 }, // 0x60d
    { 0x060e, 0 }, // 0x60e
    { 0x060f, 0 }, // 0x60f

    { 0x0610, 0 }, // 0x610
    { 0x0611, 0 }, // 0x611
    { 0x0612, 0 }, // 0x612
    { 0x0613, 0 }, // 0x613
    { 0x0614, 0 }, // 0x614
    { 0x0615, 0 }, // 0x615
    { 0x0616, 0 }, // 0x616
    { 0x0617, 0 }, // 0x617
    { 0x0618, 0 }, // 0x618
    { 0x0619, 0 }, // 0x619
    { 0x061a, 0 }, // 0x61a
    { 0x061b, 0 }, // 0x61b     Arabic semicolon
    { 0x061c, 0 }, // 0x61c
    { 0x061d, 0 }, // 0x61d
    { 0x061e, 0 }, // 0x61e
    { 0x061f, 0 }, // 0x61f     Arabic question mark

    { 0x0620, 0 }, // 0x620
    { 0xfe80, 0 }, // 0x621     Hamza
    { 0xfe81, 1 }, // 0x622     R       Alef with Madda above
    { 0xfe83, 1 }, // 0x623     R       Alef with Hamza above
    { 0xfe85, 1 }, // 0x624     R       Waw with Hamza above
    { 0xfe87, 1 }, // 0x625     R       Alef with Hamza below
    { 0xfe89, 3 }, // 0x626     D       Yeh with Hamza above
    { 0xfe8d, 1 }, // 0x627     R       Alef
    { 0xfe8f, 3 }, // 0x628     D       Beh
    { 0xfe93, 1 }, // 0x629     R       Teh Marbuta
    { 0xfe95, 3 }, // 0x62a     D       Teh
    { 0xfe99, 3 }, // 0x62b     D       Theh
    { 0xfe9d, 3 }, // 0x62c     D       Jeem
    { 0xfea1, 3 }, // 0x62d     D       Hah
    { 0xfea5, 3 }, // 0x62e     D       Khah
    { 0xfea9, 1 }, // 0x62f     R       Dal

    { 0xfeab, 1 }, // 0x630     R       Thal
    { 0xfead, 1 }, // 0x631     R       Reh
    { 0xfeaf, 1 }, // 0x632     R       Zain
    { 0xfeb1, 1 }, // 0x633     D       Seen
    { 0xfeb5, 3 }, // 0x634     D       Sheen
    { 0xfeb9, 3 }, // 0x635     D       Sad
    { 0xfebd, 3 }, // 0x636     D       Dad
    { 0xfec1, 3 }, // 0x637     D       Tah
    { 0xfec5, 3 }, // 0x638     D       Zah
    { 0xfec9, 3 }, // 0x639     D       Ain
    { 0xfecd, 3 }, // 0x63a     D       Ghain
    { 0x063b, 0 }, // 0x63b
    { 0x063c, 0 }, // 0x63c
    { 0x063d, 0 }, // 0x63d
    { 0x063e, 0 }, // 0x63e
    { 0x063f, 0 }, // 0x63f

    { 0x0640, 0 }, // 0x640     C       Tatweel
    { 0xfed1, 3 }, // 0x641     D       Feh
    { 0xfed5, 3 }, // 0x642     D       Qaf
    { 0xfed9, 3 }, // 0x643     D       Kaf
    { 0xfedd, 3 }, // 0x644     D       Lam
    { 0xfee1, 3 }, // 0x645     D       Meem
    { 0xfee5, 3 }, // 0x646     D       Noon
    { 0xfee9, 3 }, // 0x647     D       Heh
    { 0xfeed, 1 }, // 0x648     R       Waw
    { 0xfeef, 1 }, // 0x649     R       Alef Maksura // ### Dual according to newest arabicshaping.txt
    { 0xfef1, 3 }, // 0x64a     D       Yeh
    { 0x064b, 0 }, // 0x64b     Mark Fathatan
    { 0x064c, 0 }, // 0x64c     Mark Dammatan
    { 0x064d, 0 }, // 0x64d     Mark Kasratan
    { 0x064e, 0 }, // 0x64e     Mark Fatha
    { 0x064f, 0 }, // 0x64f     Mark Damma

    { 0x0650, 0 }, // 0x650     Mark Kasra
    { 0x0651, 0 }, // 0x651     Mark Shadda
    { 0x0652, 0 }, // 0x652     Mark Sukan
    // these do not exist in latin6 anymore:
    { 0x0653, 0 }, // 0x653     Mark Maddah above
    { 0x0654, 0 }, // 0x654     Mark Hamza above
    { 0x0655, 0 }, // 0x655     Mark Hamza below
    { 0x0656, 0 }, // 0x656
    { 0x0657, 0 }, // 0x657
    { 0x0658, 0 }, // 0x658
    { 0x0659, 0 }, // 0x659
    { 0x065a, 0 }, // 0x65a
    { 0x065b, 0 }, // 0x65b
    { 0x065c, 0 }, // 0x65c
    { 0x065d, 0 }, // 0x65d
    { 0x065e, 0 }, // 0x65e
    { 0x065f, 0 }, // 0x65f

    { 0x0660, 0 }, // 0x660     Arabic 0
    { 0x0661, 0 }, // 0x661     Arabic 1
    { 0x0662, 0 }, // 0x662     Arabic 2
    { 0x0663, 0 }, // 0x663     Arabic 3
    { 0x0664, 0 }, // 0x664     Arabic 4
    { 0x0665, 0 }, // 0x665     Arabic 5
    { 0x0666, 0 }, // 0x666     Arabic 6
    { 0x0667, 0 }, // 0x667     Arabic 7
    { 0x0668, 0 }, // 0x668     Arabic 8
    { 0x0669, 0 }, // 0x669     Arabic 9
    { 0x066a, 0 }, // 0x66a     Arabic % sign
    { 0x066b, 0 }, // 0x66b     Arabic decimal separator
    { 0x066c, 0 }, // 0x66c     Arabic thousands separator
    { 0x066d, 0 }, // 0x66d     Arabic five pointed star
    { 0x066e, 0 }, // 0x66e
    { 0x066f, 0 }, // 0x66f

	// ### some glyphs do not have shaped mappings in the presentation forms A.
	// these have the shaping set to 0 for the moment. Will have to find out better mappings for them.
    { 0x0670, 0 }, // 0x670
    { 0xfb50, 1 }, // 0x671 R   Alef Wasla
    { 0x0672, 0 }, // 0x672 R    Alef with wavy Hamza above
    { 0x0673, 0 }, // 0x673 R   Alef with wavy Hamza below
    { 0x0674, 0 }, // 0x674 U   High Hamza
    { 0x0675, 0 }, // 0x675 R   High Hamza Alef
    { 0x0676, 0 }, // 0x676 R   High Hamza Wav
    { 0xfbdd, 0 }, // 0x677 R   U with hamza above // ### only isolated form found...
    { 0x0678, 0 }, // 0x678 D   High hamza yeh
    { 0xfb66, 3 }, // 0x679 D   ttheh
    { 0xfb5e, 3 }, // 0x67a D   theheh
    { 0xfb52, 3 }, // 0x67b D   beeh
    { 0x067c, 0 }, // 0x67cD    teh with ring
    { 0x067d, 0 }, // 0x67d D   teh with three dots above downwards
    { 0xfb56, 3 }, // 0x67e D   peh
    { 0xfb62, 3 }, // 0x67f D   teheh

    { 0xfb5a, 3 }, // 0x680 D   beheh
    { 0x0681, 0 }, // 0x681 D   hah with hamza above
    { 0x0682, 0 }, // 0x682 D   hah with two dots vertical above
    { 0xfb76, 3 }, // 0x683 D   nyeh
    { 0xfb72, 3 }, // 0x684 D   dyeh
    { 0x0685, 0 }, // 0x685 D   hah with three dots above
    { 0xfb7a, 3 }, // 0x686 D   tcheh
    { 0xfb7e, 3 }, // 0x687 D   tcheheh
    { 0xfb88, 1 }, // 0x688 R   ddal
    { 0x0689, 0 }, // 0x689 R   dal with ring
    { 0x068a, 0 }, // 0x68a R   dal with dot
    { 0x068b, 0 }, // 0x68b R   dal with dot below and small tah
    { 0xfb84, 1 }, // 0x68cR    dahal
    { 0xfb82, 1 }, // 0x68d R   ddahal
    { 0xfb86, 1 }, // 0x68e R   dul
    { 0x068f, 0 }, // 0x68f R   dal with three dots above downwards

    { 0x0690, 0 }, // 0x690 R   dal with four dots above
    { 0xfb8c, 1 }, // 0x691 R   rreh
    { 0x0692, 0 }, // 0x692 R   reh with small v
    { 0x0693, 0 }, // 0x693 R   reh with ring
    { 0x0694, 0 }, // 0x694 R   reh with dot below
    { 0x0695, 0 }, // 0x695 R   reh with small v below
    { 0x0696, 0 }, // 0x696 R   reh with dot below and dot above
    { 0x0697, 0 }, // 0x697 R   reh with two dots above
    { 0xfb8a, 1 }, // 0x698 R   jeh
    { 0x0699, 0 }, // 0x699 R   reh with four dots above
    { 0x069a, 0 }, // 0x69a D   seen with dot below and dot above
    { 0x069b, 0 }, // 0x69b D   seen with three dots below
    { 0x069c, 0 }, // 0x69cD    seen with three dots below and three dots above
    { 0x069d, 0 }, // 0x69d D   sad with two dots below
    { 0x069e, 0 }, // 0x69e D   sad with three dots above
    { 0x069f, 0 }, // 0x69f D   tah with three dots above

    { 0x06a0, 0 }, // 0x6a0 D   ain with three dots above
    { 0x06a1, 0 }, // 0x6a1 D   dotless feh
    { 0x06a2, 0 }, // 0x6a2 D   feh with dot moved below
    { 0x06a3, 0 }, // 0x6a3 D   feh with dot below
    { 0xfb6a, 3 }, // 0x6a4 D   veh
    { 0x06a5, 0 }, // 0x6a5 D   feh with three dots below
    { 0xfb6e, 3 }, // 0x6a6 D   peheh
    { 0x06a7, 0 }, // 0x6a7 D   qaf with dot above
    { 0x06a8, 0 }, // 0x6a8 D   qaf woith three dots above
    { 0xfb8e, 3 }, // 0x6a9 D   keheh
    { 0x06aa, 0 }, // 0x6aa D   swash kaf
    { 0x06ab, 0 }, // 0x6ab D   kaf with ring
    { 0x06ac, 0 }, // 0x6acD    kaf with dot above
    { 0xfbd3, 3 }, // 0x6ad D   ng
    { 0x06ae, 0 }, // 0x6ae D   kaf with three dots below
    { 0xfb92, 3 }, // 0x6af D   gaf

    { 0x06b0, 0 }, // 0x6b0 D   gaf with ring
    { 0xfb9a, 3 }, // 0x6b1 D   ngoeh
    { 0x06b2, 0 }, // 0x6b2 D   gaf with two dots below
    { 0xfb96, 3 }, // 0x6b3 D   gueh
    { 0x06b4, 0 }, // 0x6b4 D   gaf with three dots above
    { 0x06b5, 0 }, // 0x6b5 D   lam with small v
    { 0x06b6, 0 }, // 0x6b6 D   lam with dot above
    { 0x06b7, 0 }, // 0x6b7 D   lam with three dots above
    { 0x06b8, 0 }, // 0x6b8 D   lam with three dots below
    { 0x06b9, 0 }, // 0x6b9 D   noon with dot below
    { 0xfb9e, 1 }, // 0x6ba R   noon ghunna
    { 0xfba0, 3 }, // 0x6bb D   rnoon
    { 0x06bc, 0 }, // 0x6bcD    noon with ring
    { 0x06bd, 0 }, // 0x6bd D   noon with three dots above
    { 0xfbaa, 3 }, // 0x6be D   heh doachashmee
    { 0x06bf, 0 }, // 0x6bf D   tcheh with dot above

    { 0xfba4, 1 }, // 0x6c0 R   heh with yeh above = ligature hamza on hah (06d5 + 0654)
    { 0xfba6, 3 }, // 0x6c1 D   heh goal
    { 0x06c2, 0 }, // 0x6c2 R   heh goal with hamza above (06c1 + 0654)
    { 0x06c3, 0 }, // 0x6c3 R   teh marbuta goal
    { 0x06c4, 0 }, // 0x6c4 R   waw with ring
    { 0xfbe0, 1 }, // 0x6c5 R   kirghiz oe
    { 0xfbd9, 1 }, // 0x6c6 R   oe
    { 0xfbd7, 1 }, // 0x6c7 R   u
    { 0xfbdb, 1 }, // 0x6c8 R   yu
    { 0xfbe2, 1 }, // 0x6c9 R   kirghiz yu
    { 0x06ca, 0 }, // 0x6ca R   waw with teo dots above
    { 0xfbde, 1 }, // 0x6cb R   ve
    { 0x06cc, 0 }, // 0x6cc D   farsi yeh
    { 0x06cd, 0 }, // 0x6cd R   yeh with tail
    { 0x06ce, 0 }, // 0x6ce D   yeh with small v
    { 0x06cf, 0 }, // 0x6cf R   waw with dot above

    { 0xfbe4, 3 }, // 0x6d0 D   e
    { 0x06d1, 0 }, // 0x6d1 D   yeh with three dots below
    { 0xfbae, 1 }, // 0x6d2 R   yeh barree
    { 0xfbb0, 1 }, // 0x6d3 R   yeh barree with hamza above
    { 0x06d4, 0 }, // 0x6d4 U   full stop
    { 0x06d5, 0 }, // 0x6d5 D   ae
    { 0x06d6, 0 }, // 0x6d6     koreanic annotaion signs
    { 0x06d7, 0 }, // 0x6d7     ...
    { 0x06d8, 0 }, // 0x6d8
    { 0x06d9, 0 }, // 0x6d9
    { 0x06da, 0 }, // 0x6da
    { 0x06db, 0 }, // 0x6db
    { 0x06dc, 0 }, // 0x6dc
    { 0x06dd, 0 }, // 0x6dd
    { 0x06de, 0 }, // 0x6de
    { 0x06df, 0 }, // 0x6df

    { 0x06e0, 0 }, // 0x6e0
    { 0x06e1, 0 }, // 0x6e1
    { 0x06e2, 0 }, // 0x6e2
    { 0x06e3, 0 }, // 0x6e3
    { 0x06e4, 0 }, // 0x6e4
    { 0x06e5, 0 }, // 0x6e5
    { 0x06e6, 0 }, // 0x6e6
    { 0x06e7, 0 }, // 0x6e7
    { 0x06e8, 0 }, // 0x6e8
    { 0x06e9, 0 }, // 0x6e9
    { 0x06ea, 0 }, // 0x6ea
    { 0x06eb, 0 }, // 0x6eb
    { 0x06ec, 0 }, // 0x6ec
    { 0x06ed, 0 }, // 0x6ed
    { 0x06ee, 0 }, // 0x6ee
    { 0x06ef, 0 }, // 0x6ef

    { 0x06f0, 0 }, // 0x6f0     Arabic indic digit 0
    { 0x06f1, 0 }, // 0x6f1
    { 0x06f2, 0 }, // 0x6f2
    { 0x06f3, 0 }, // 0x6f3
    { 0x06f4, 0 }, // 0x6f4
    { 0x06f5, 0 }, // 0x6f5
    { 0x06f6, 0 }, // 0x6f6
    { 0x06f7, 0 }, // 0x6f7
    { 0x06f8, 0 }, // 0x6f8
    { 0x06f9, 0 }, // 0x6f9     Arabic indic digit 9
    { 0x06fa, 0 }, // 0x6fa D   Sheen with dot below
    { 0x06fb, 0 }, // 0x6fb D   dad with dot below
    { 0x06fc, 0 }, // 0x6fc D   ghain with dot below
    { 0x06fd, 0 }, // 0x6fd     Sindhi ampersand
    { 0x06fe, 0 }, // 0x6fe     sindhi postposition
    { 0x06ff, 0 }, // 0x6ff

};

// this is a bit tricky. Alef always binds to the right, so the second parameter descibing the shape
// of the lam can be either initial of medial. So initial maps to the isolated form of the ligature,
// medial to the final form
static const ushort arabicUnicodeLamAlefMapping[6][4] = {
    { 0xfffd, 0xfffd, 0xfef5, 0xfef6 }, // 0x622        R       Alef with Madda above
    { 0xfffd, 0xfffd, 0xfef7, 0xfef8 }, // 0x623        R       Alef with Hamza above
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x624        R       Waw with Hamza above
    { 0xfffd, 0xfffd, 0xfef9, 0xfefa }, // 0x625        R       Alef with Hamza below
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x626        D       Yeh with Hamza above
    { 0xfffd, 0xfffd, 0xfefb, 0xfefc } // 0x627         R       Alef
};

static inline int getShape( const QChar * /* base */, uchar cell, int shape,
			    const QFontMetrics * /* fm */ )
{
    uint ch = arabicUnicodeMapping[cell][0] + shape;
    /*
    // we revert to the unshaped glyph in case the shaped version doesn't exist
    if ( fm && !fm->inFont( ch ) ) {
    switch( shape ) {
    case KoComplexText::XIsolated:
    break; // try base form
    case KoComplexText::XFinal:
    ch -= 1; // try isolated form
    break;
    case KoComplexText::XInitial:
    ch += 1; // try medial form
    break;
    case KoComplexText::XMedial:
    ch -= 1; // try initial form
    break;
    }
    if ( !fm->inFont( ch ) )
    ch = *base;
    }
    */
    return ch;
}

QString KoComplexText::shapedString(const QString& uc, int from, int len, QPainter::LayoutDirection dir, const QFontMetrics *fm )
{
    if( len < 0 )
	len = uc.length() - from;
    if( len == 0 ) {
	return QString::null;
    }

    // we have to ignore NSMs at the beginning and add at the end.
    int num = uc.length() - from - len;
    const QChar *ch = uc.unicode() + from + len;
    while ( num > 0 && ch->combiningClass() != 0 ) {
	ch++;
	num--;
	len++;
    }
    ch = uc.unicode() + from;
    while ( len > 0 && ch->combiningClass() != 0 ) {
	ch++;
	len--;
	from++;
    }
    if ( len == 0 ) return QString::null;

    if( !shapeBuffer || len > shapeBufSize ) {
      if( shapeBuffer ) free( (void *) shapeBuffer );
      shapeBuffer = (QChar *) malloc( len*sizeof( QChar ) );
//        delete [] shapeBuffer;
//        shapeBuffer = new QChar[ len + 1];
	shapeBufSize = len;
    }

    int lenOut = 0;
    QChar *data = shapeBuffer;
    if ( dir == QPainter::RTL )
	ch += len - 1;
    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	uchar c = ch->cell();
	if ( r != 0x06 ) {
	    if ( dir == QPainter::RTL && ch->mirrored() )
		*data = ch->mirroredChar();
	    else
		*data = *ch;
	    data++;
	    lenOut++;
	} else {
	    int pos = i + from;
	    if ( dir == QPainter::RTL )
		pos = from + len - 1 - i;
	    int shape = glyphVariantLogical( uc, pos );
	    //kDebug() << "mapping U+" << ch->unicode() << " to shape " << shape << " glyph=0x" << arabicUnicodeMapping[ch->cell()][shape] << endl;
	    // take care of lam-alef ligatures (lam right of alef)
	    ushort map;
	    switch ( c ) {
		case 0x44: { // lam
		    const QChar *pch = nextChar( uc, pos );
		    if ( pch->row() == 0x06 ) {
			switch ( pch->cell() ) {
			    case 0x22:
			    case 0x23:
			    case 0x25:
			    case 0x27:
				//kDebug() << " lam of lam-alef ligature" << endl;
				map = arabicUnicodeLamAlefMapping[pch->cell() - 0x22][shape];
				goto next;
			    default:
				break;
			}
		    }
		    break;
		}
		case 0x22: // alef with madda
		case 0x23: // alef with hamza above
		case 0x25: // alef with hamza below
		case 0x27: // alef
		    if ( prevChar( uc, pos )->unicode() == 0x0644 ) {
			// have a lam alef ligature
			//kDebug() << " alef of lam-alef ligature" << endl;
			goto skip;
		    }
		default:
		    break;
	    }
	    map = getShape( ch, c, shape, fm );
	next:
	    *data = map;
	    data++;
	    lenOut++;
	}
    skip:
	if ( dir == QPainter::RTL )
	    ch--;
	else
	    ch++;
    }

    if ( dir == QPainter::Auto && !uc.simpleText() ) {
	return bidiReorderString( QConstString( shapeBuffer, lenOut ).string() );
    }
    if ( dir == QPainter::RTL ) {
	// reverses the non spacing marks to be again after the base char
	QChar *s = shapeBuffer;
	int i = 0;
	while ( i < lenOut ) {
	    if ( s->combiningClass() != 0 ) {
		// non spacing marks
		int clen = 1;
		QChar *ch = s;
		do {
		    ch++;
		    clen++;
		} while ( ch->combiningClass() != 0 );

		int j = 0;
		QChar *cp = s;
		while ( j < clen/2 ) {
		    QChar tmp = *cp;
		    *cp = *ch;
		    *ch = tmp;
		    cp++;
		    ch--;
		    j++;
		}
		s += clen;
		i += clen;
	    } else {
		s++;
		i++;
	    }
	}
    }

    return QConstString( shapeBuffer, lenOut ).string();
}

QChar KoComplexText::shapedCharacter( const QString &str, int pos, const QFontMetrics *fm )
{
    const QChar *ch = str.unicode() + pos;
    if ( ch->row() != 0x06 )
	return *ch;
    else {
	int shape = glyphVariantLogical( str, pos );
	//kDebug() << "mapping U+" << ch->unicode() << " to shape " << shape << " glyph=0x" << arabicUnicodeMapping[ch->cell()][shape] << endl;
	// lam aleph ligatures
	switch ( ch->cell() ) {
	    case 0x44: { // lam
		const QChar *nch = nextChar( str, pos );
		if ( nch->row() == 0x06 ) {
		    switch ( nch->cell() ) {
			case 0x22:
			case 0x23:
			case 0x25:
			case 0x27:
			    return QChar(arabicUnicodeLamAlefMapping[nch->cell() - 0x22][shape]);
			default:
			    break;
		    }
		}
		break;
	    }
	    case 0x22: // alef with madda
	    case 0x23: // alef with hamza above
	    case 0x25: // alef with hamza below
	    case 0x27: // alef
		if ( prevChar( str, pos )->unicode() == 0x0644 )
		    // have a lam alef ligature
		    return QChar(0);
	    default:
		break;
	}
	return QChar( getShape( ch, ch->cell(), shape, fm ) );
    }
}

// Avoid using QFontPrivate, to which we don't have access. We don't use positionMarks() anyway
#if 0
Q3PointArray KoComplexText::positionMarks( QFontPrivate *f, const QString &str,
					 int pos, QRect *boundingRect )
{
    int len = str.length();
    int nmarks = 0;
    while ( pos + nmarks < len && str[pos+nmarks +1].combiningClass() > 0 )
	nmarks++;

    if ( !nmarks )
	return Q3PointArray();

    QChar baseChar = KoComplexText::shapedCharacter( str, pos );
    QRect baseRect = f->boundingRect( baseChar );
    int baseOffset = f->textWidth( str, pos, 1 );

    //kDebug() << "base char: bounding rect at " << baseRect.x() << "/" << baseRect.y() << " (" << baseRect.width() << "/" << baseRect.height() << ")" << endl;
    int offset = f->actual.pixelSize / 10 + 1;
    //kDebug() << "offset = " << offset << endl;
    Q3PointArray pa( nmarks );
    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    if ( boundingRect )
	*boundingRect = baseRect;
    for( i = 0; i < nmarks; i++ ) {
	QChar mark = str[pos+i+1];
	unsigned char cmb = mark.combiningClass();
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew and thai.

	    // ### add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 )
		offset +=1;
	    // below
	    if ( (cmb >= 10 && cmb <= 18) ||
		 cmb == 20 || cmb == 22 ||
		 cmb == 29 || cmb == 32 )
		cmb = QChar::Combining_Below;
	    // above
	    else if ( cmb == 23 || cmb == 27 || cmb == 28 ||
		      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36 ) )
		cmb = QChar::Combining_Above;
	    //below-right
	    else if ( cmb == 103 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 )
		cmb = QChar::Combining_AboveRight;
	    else if ( cmb == 25 )
		cmb = QChar::Combining_AboveLeft;
	    // fixed:
	    //  19 21

	}

	// combining marks of different class don't interact. Reset the rectangle.
	if ( cmb != lastCmb ) {
	    //kDebug() << "resetting rect" << endl;
	    attachmentRect = baseRect;
	}

	QPoint p;
	QRect markRect = f->boundingRect( mark );
	switch( cmb ) {
	case QChar::Combining_DoubleBelow:
		// ### wrong in rtl context!
	case QChar::Combining_BelowLeft:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowLeftAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    break;
	case QChar::Combining_Below:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_BelowRight:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowRightAttached:
	    p += attachmentRect.bottomRight() - markRect.topRight();
	    break;
	    case QChar::Combining_Left:
	    p += QPoint( -offset, 0 );
	case QChar::Combining_LeftAttached:
	    break;
	    case QChar::Combining_Right:
	    p += QPoint( offset, 0 );
	case QChar::Combining_RightAttached:
	    break;
	case QChar::Combining_DoubleAbove:
	    // ### wrong in RTL context!
	case QChar::Combining_AboveLeft:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveLeftAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    break;
	    case QChar::Combining_Above:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_AboveRight:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveRightAttached:
	    p += attachmentRect.topRight() - markRect.bottomRight();
	    break;

	case QChar::Combining_IotaSubscript:
	    default:
		break;
	}
	//kDebug() << "char=" << mark.unicode() << " combiningClass = " << cmb << " offset=" << p.x() << "/" << p.y() << endl;
	markRect.moveBy( p.x(), p.y() );
	p += QPoint( -baseOffset, 0 );
	attachmentRect |= markRect;
	if ( boundingRect )
	    *boundingRect |= markRect;
	lastCmb = cmb;
	pa.setPoint( i, p );
    }
    return pa;
}
#endif

//#define BIDI_DEBUG
#ifdef BIDI_DEBUG
#include <iostream>
#endif

static QChar::Direction basicDirection(const QString &str, int start = 0)
{
    int len = str.length();
    int pos = start > len ? len -1 : start;
    const QChar *uc = str.unicode() + pos;
    while( pos < len ) {
	switch( uc->direction() )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return QChar::DirL;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return QChar::DirR;
	default:
	    break;
	}
	++pos;
	++uc;
    }
    if ( start != 0 )
	return basicDirection( str );
    return QChar::DirL;
}

// transforms one line of the paragraph to visual order
// the caller is responisble to delete the returned list of KoTextRuns.
Q3PtrList<KoTextRun> *KoComplexText::bidiReorderLine( KoBidiControl *control, const QString &text, int start, int len,
						   QChar::Direction basicDir )
{
    int last = start + len - 1;
    //printf("doing BiDi reordering from %d to %d!\n", start, last);

    Q3PtrList<KoTextRun> *runs = new Q3PtrList<KoTextRun>;
    runs->setAutoDelete(TRUE);

    KoBidiContext *context = control->context;
    if ( !context ) {
	// first line
	//if( start != 0 )
	//    kDebug() << "bidiReorderLine::internal error" << endl;
	if( basicDir == QChar::DirR || (basicDir == QChar::DirON && text.isRightToLeft() ) ) {
	    context = new KoBidiContext( 1, QChar::DirR );
	    control->status.last = QChar::DirR;
	} else {
	    context = new KoBidiContext( 0, QChar::DirL );
	    control->status.last = QChar::DirL;
	}
    }

    KoBidiStatus status = control->status;
    QChar::Direction dir = QChar::DirON;

    int sor = start;
    int eor = start;

    int current = start;
    while(current <= last) {
	QChar::Direction dirCurrent;
	if(current == (int)text.length()) {
	    KoBidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else if ( current == last ) {
	    dirCurrent = ( basicDir != QChar::DirON ? basicDir : basicDirection( text, current ) );
	} else
	    dirCurrent = text.at(current).direction();


#ifdef BIDI_DEBUG
	cout << "directions: dir=" << dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << " level =" << (int)context->level << endl;
#endif

	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new KoBidiContext(level, QChar::DirR, context);
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new KoBidiContext(level, QChar::DirL, context);
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirRLO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new KoBidiContext(level, QChar::DirR, context, TRUE);
		    dir = QChar::DirR;
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new KoBidiContext(level, QChar::DirL, context, TRUE);
		    dir = QChar::DirL;
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		KoBidiContext *c = context->parent;
		if(c) {
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    status.last = context->dir;
		    if( context->deref() ) delete context;
		    context = c;
		    if(context->override)
			dir = context->dir;
		    else
			dir = QChar::DirON;
		    status.lastStrong = context->dir;
		}
		break;
	    }

	    // strong types
	case QChar::DirL:
	    if(dir == QChar::DirON)
		dir = QChar::DirL;
	    switch(status.last)
		{
		case QChar::DirL:
		    eor = current; status.eor = QChar::DirL; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(dir != QChar::DirL) {
			//last stuff takes embedding dir
			if( context->dir == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				runs->append( new KoTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    else
				eor = current - 1;
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			} else {
			    if(status.eor != QChar::DirL) {
				runs->append( new KoTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirL;
			    } else {
				eor = current; status.eor = QChar::DirL; break;
			    }
			}
		    } else {
			eor = current; status.eor = QChar::DirL;
		    }
		default:
		    break;
		}
	    status.lastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(dir == QChar::DirON) dir = QChar::DirR;
	    switch(status.last)
		{
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if( status.eor != QChar::DirR && status.eor != QChar::DirAL ) {
			//last stuff takes embedding dir
			if(context->dir == QChar::DirR || status.lastStrong == QChar::DirR) {
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			}
		    } else {
			eor = current; status.eor = QChar::DirR;
		    }
		default:
		    break;
		}
	    status.lastStrong = dirCurrent;
	    break;

	    // weak types:

	case QChar::DirNSM:
	    // ### if @sor, set dir to dirSor
	    break;
	case QChar::DirEN:
	    if(status.lastStrong != QChar::DirAL) {
		// if last strong was AL change EN to AL
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirL)
			dir = QChar::DirL;
		    else
			dir = QChar::DirAN;
		}
		switch(status.last)
		    {
		    case QChar::DirET:
			if ( status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL ) {
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			// fall through
		    case QChar::DirEN:
		    case QChar::DirL:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			runs->append( new KoTextRun(sor, eor, context, dir) );
			++eor; sor = eor; status.eor = QChar::DirEN;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN || dir == QChar::DirAN) {
			    eor = current; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = current - 1;
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirEN;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				runs->append( new KoTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				runs->append( new KoTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirAN;
			    } else {
				eor = current; status.eor = dirCurrent;
			    }
			}
		    default:
			break;
		    }
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(dir == QChar::DirON) dir = QChar::DirAN;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirAN:
		    eor = current; status.eor = QChar::DirAN; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		    runs->append( new KoTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; status.eor = QChar::DirR; break;
		    }
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(status.eor == QChar::DirR) {
			// neutrals go to R
			eor = current - 1;
			runs->append( new KoTextRun(sor, eor, context, dir) );
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    runs->append( new KoTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			} else {
			    eor = current; status.eor = dirCurrent;
			}
		    }
		default:
		    break;
		}
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(status.last == QChar::DirEN) {
		dirCurrent = QChar::DirEN;
		eor = current; status.eor = dirCurrent;
		break;
	    }
	    break;

	    // boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
	    // neutrals
	case QChar::DirB:
	    // ### what do we do with newline and paragraph separators that come to here?
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << endl;

	if(current >= (int)text.length()) break;

	// set status.last as needed.
	switch(dirCurrent)
	    {
	    case QChar::DirET:
	    case QChar::DirES:
	    case QChar::DirCS:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		switch(status.last)
		    {
		    case QChar::DirL:
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirEN:
		    case QChar::DirAN:
			status.last = dirCurrent;
			break;
		    default:
			status.last = QChar::DirON;
		    }
		break;
	    case QChar::DirNSM:
	    case QChar::DirBN:
		// ignore these
		break;
	    default:
		status.last = dirCurrent;
	    }

	++current;
    }

#ifdef BIDI_DEBUG
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if ( sor <= eor )
	runs->append( new KoTextRun(sor, eor, context, dir) );

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    KoTextRun *r = runs->first();
    while ( r ) {
	//printf("level = %d\n", r->level);
	if ( r->level > levelHigh )
	    levelHigh = r->level;
	if ( r->level < levelLow )
	    levelLow = r->level;
	r = runs->next();
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#ifdef BIDI_DEBUG
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    cout << "logical order is:" << endl;
    Q3PtrListIterator<KoTextRun> it2(*runs);
    KoTextRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
	cout << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = runs->count() - 1;

    while(levelHigh >= levelLow)
    {
	int i = 0;
	while ( i < count )
	{
	    while(i < count && runs->at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i <= count && runs->at(i)->level >= levelHigh) i++;
	    int end = i-1;

	    if(start != end)
	    {
		//cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    KoTextRun *first = runs->take(start+j);
		    KoTextRun *last = runs->take(end-j-1);
		    runs->insert(start+j, last);
		    runs->insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }

#ifdef BIDI_DEBUG
    cout << "visual order is:" << endl;
    Q3PtrListIterator<KoTextRun> it3(*runs);
    KoTextRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	cout << "    " << r3 << endl;
    }
#endif

    control->setContext( context );
    control->status = status;

    return runs;
}


QString KoComplexText::bidiReorderString( const QString &str, QChar::Direction /*basicDir*/ )
{

// ### fix basic direction
    KoBidiControl control;
    int lineStart = 0;
    int lineEnd = 0;
    int len = str.length();
    QString visual;
    visual.setUnicode( 0, len );
    QChar *vch = (QChar *)visual.unicode();
    const QChar *ch = str.unicode();
    while( lineStart < len ) {
	lineEnd = lineStart;
	while( *ch != '\n' && lineEnd < len ) {
	    ch++;
	    lineEnd++;
	}
	lineEnd++;
	Q3PtrList<KoTextRun> *runs = bidiReorderLine( &control, str, lineStart, lineEnd - lineStart );

	// reorder the content of the line, and output to visual
	KoTextRun *r = runs->first();
	while ( r ) {
	    if(r->level %2) {
		// odd level, need to reverse the string
		int pos = r->stop;
		while(pos >= r->start) {
		    *vch = str[pos];
		    if ( vch->mirrored() )
			*vch = vch->mirroredChar();
		    vch++;
		    pos--;
		}
	    } else {
		int pos = r->start;
		while(pos <= r->stop) {
		    *vch = str[pos];
		    vch++;
		    pos++;
		}
	    }
	    r = runs->next();
	}
	if ( *ch == '\n' ) {
	    *vch = *ch;
	    vch++;
	    ch++;
	    lineEnd++;
	}
	lineStart = lineEnd;
    }
    return visual;
}

KoTextRun::KoTextRun(int _start, int _stop, KoBidiContext *context, QChar::Direction dir) {
    start = _start;
    stop = _stop;
    if(dir == QChar::DirON) dir = context->dir;

    level = context->level;

    // add level of run (cases I1 & I2)
    if( level % 2 ) {
	if(dir == QChar::DirL || dir == QChar::DirAN)
	    level++;
    } else {
	if( dir == QChar::DirR )
	    level++;
	else if( dir == QChar::DirAN )
	    level += 2;
    }
#ifdef BIDI_DEBUG
    printf("new run: dir=%d from %d, to %d level = %d\n", dir, _start, _stop, level);
#endif
}

#endif //QT_NO_COMPLEXTEXT
