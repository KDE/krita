/* This file is part of the KDE project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

#include <priorityqueue.h>
#include <kdebug.h>
#include <q3ptrlist.h>
#include <q3asciidict.h>
#include <stdlib.h>
#include <time.h>

struct Node {
    Node( unsigned int key ) : m_key( key ), m_index( 0 ) {}

    unsigned int key() const { return m_key; }
    void setKey( unsigned int key ) { m_key = key; }

    int index() const { return m_index; }
    void setIndex( int i ) { m_index = i; }
private:
    unsigned int m_key;
    int m_index;
};

static const char* const keys[] = { "one",  "two", "three",  "four", "five",
                                    "six", "seven", "eight", "nine", "ten",
                                    "eleven", "twelve", 0 };

int main( int /*argc*/, char **/*argv*/ )
{
    Q3PtrList<Node> list;
    list.setAutoDelete( true );
    Q3AsciiDict<Node> dict;

    KOffice::PriorityQueue<Node> queue;

    srand( time( 0 ) );
    for ( int i = 0; i < 12; ++i ) {
        Node *n = new Node( rand() % 20 );
        list.append( n );
        queue.insert( n );
        // Check whether the AsciiDict CTOR is okay
        Node *n2 = new Node( *n );
        dict.insert( keys[ i ], n2 );
    }

    kDebug() << "##### Queue 1: " << endl;
    queue.dump();

    kDebug() << "##### Queue 2: " << endl;
    KOffice::PriorityQueue<Node> queue2( dict );
    queue2.dump();

    Node *n = list.at( 6 );
    kDebug() << "##### Decreasing node: " << n->key() << " at " << n->index() << endl;
    n->setKey( 2 );
    queue.keyDecreased( n );
    queue.dump();

    n = list.at( 2 );
    kDebug() << "##### Decreasing node: " << n->key() << " at " << n->index() << endl;
    n->setKey( 0 );
    queue.keyDecreased( n );
    queue.dump();

    n = queue.extractMinimum();
    while ( n ) {
        queue.dump();
        n = queue.extractMinimum();
    }
    return 0;
}
