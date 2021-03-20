/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "csv_read_line.h"

#include <kis_debug.h>

#include <QDebug>
#include <QIODevice>

#include <asl/kis_offset_keeper.h>
#include <asl/kis_asl_writer_utils.h>
#include <asl/kis_asl_reader_utils.h>

CSVReadLine::CSVReadLine()
    : m_separator(0)
    , m_row(0)
    , m_linebuf(0)
    , m_pos(-1)
{
}

CSVReadLine::~CSVReadLine()
{
}

// returns: 0 finished, + continue, - error
int CSVReadLine::nextLine(QIODevice *io)
{
    int retval= 0;
    m_pos= -1;

    try {
        m_linebuf= io->readLine();

        if (!m_linebuf.size())
            retval= 0; //finished
        else {
            if (!m_separator)
                m_separator= ((m_linebuf.size() > 5) && (m_linebuf[5] == ';')) ?
                             ';' : ',';
            m_pos= 0;
            retval= 1;
        }
    } catch(KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: CSV:" << e.what();
        retval= -1; //error
    }
    return retval;
}

QString CSVReadLine::nextField()
{
    char     strBuf[CSV_FIELD_MAX];
    char    *ptr;
    char     c;
    int      i,p,max;

    p= m_pos;

    if (p < 0) return QString();

    ptr= strBuf;
    max= m_linebuf.size();

    do {    if (p >= max) {
                ptr[0]= 0;
                m_pos= -1;
                return QString(strBuf);
            }
            c= m_linebuf[p++];
    } while((c == ' ') || (c == '\t'));

    i= 0;

    if (c == '\"') {
        //quoted
        while(p < max) {
            c= m_linebuf[p++];

            if (c == '\"') {

                if (p >= max) break;

                if (m_linebuf[p] != c) break;

                 //double quote escape sequence
                ++p;
            }
            if (i < (CSV_FIELD_MAX - 1))
                ptr[i++]= c;
        }

        while (p < max) {
            c= m_linebuf[p++];
            if (c == m_separator) break;
        }
    } else {
        //without quotes
        while (c != m_separator) {
            if (i < (CSV_FIELD_MAX - 1))
                ptr[i++]= c;

            if (p >= max) break;

            c= m_linebuf[p++];
        }

        while(i > 0) {
            c= ptr[--i];
            if ((c != ' ')  && (c != '\t') &&
                (c != '\r') && (c != '\n')) {
                ++i;
                break;
            }
        }
    }
    ptr[i]= 0;
    m_pos= (p < max) ? p : -1;
    return QString(strBuf);
}

void CSVReadLine::rewind()
{
    m_pos= 0;
}
