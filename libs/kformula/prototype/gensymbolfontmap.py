#!/usr/bin/env python

import time
from xml.sax import saxutils, handler, make_parser

unicodetable = { "normal":{}, "bold":{}, "italic":{},
                 "slant":{}, "boldItalic":{} }
fonttable = {}

class ContentGenerator(handler.ContentHandler):  

    def __init__(self):
        handler.ContentHandler.__init__(self)
        self.font = None
        
    def startElement(self, name, attrs):
        if name == 'unicodetable':
            self.font = None
            for (name, value) in attrs.items():
                if name == "font" and value:
                    self.font = value
                    if value not in fonttable:
                        fonttable[value] = []
        elif self.font and name == 'entry':
            number = ''
            for (name, value) in attrs.items():
                if name == "key": key = int(value)
                elif name == "number": number = value
                elif name == "name": latexName = value
                elif name == "class": charClass = value
                elif name == "style": style = value

            if number != '':
                unicodetable[style][number] = (latexName, charClass)
                fonttable[self.font].append((key, number, style))

def fontkey(font, style, number):
    for mapping in fonttable[font]:
        k, n, s = mapping
        if s == style and n == number:
            return k


def writeFontTable(fontname, f):
    f.write('\n\nstatic InternFontTable ' + fontname + 'Map[] = {\n')
    for style in unicodetable:
        for key in unicodetable[style]:
            latexName, charClass = unicodetable[style][key]
            pos = fontkey(fontname, style, key)
            if pos:
                f.write('    { ' + key + ', ' + hex(pos) + ', ' + charClass + ', ' + style + 'Char },\n')
    f.write('    { 0, 0, ORDINARY, normalChar }\n};\n\n')


def write_header(f):
    print >>f, '''//
// Created: ''' + time.ctime(time.time()) + '''
//      by: gensymbolfontmap.py
//    from: symbol.xml
//
// WARNING! All changes made in this file will be lost!
'''
    
def main():
    f = open('../symbolfontmapping.cc', 'w')
    write_header(f)
    writeFontTable("symbol", f)
    f.close()
    
    f = open('../esstixfontmapping.cc', 'w')
    write_header(f)
    fontnames = [ "esstixnine", 
                  "esstixthirteen", 
                  "esstixeleven", 
                  "esstixfourteen", 
                  "esstixfive", 
                  "esstixfifteen", 
                  "esstixeight", 
                  "esstixthree", 
                  "esstixten", 
                  "esstixsixteen", 
                  "esstixone", 
                  "esstixtwo", 
                  "esstixsix", 
                  "esstixseven", 
                  "esstixtwelve", 
                  "esstixseventeen", 
                  "esstixfour" ]
    for fn in fontnames:
        writeFontTable(fn, f)
    f.close()

    f = open('../cmmapping.cc', 'w')
    write_header(f)
    fontnames = [ "cmbx10", 
                  "cmex10", 
                  "cmmi10", 
                  "cmr10", 
                  #"cmsl10", 
                  "cmsy10", 
                  #"cmti10", 
                  #"cmtt10", 
                  "msam10",
                  "msbm10"
                  ]
    for fn in fontnames:
        writeFontTable(fn, f)
    f.close()

    f = open('../unicodenames.cc', 'w')
    write_header(f)
    print >>f, 'struct UnicodeNameTable { short unicode; const char* name; };'
    print >>f, 'static UnicodeNameTable nameTable[] = {'
    nameDir = {}
    table = {}
    for style in unicodetable:
        table.update(unicodetable[style])

    for key in table:
        latexName, charClass = table[key]
        if len(latexName) > 0:
            #for fn in fontnames:
            #    if fontkey(fn, style, key):
                    print >>f, '    { ' + key + ', "' + latexName + '" },'
                    #break
    print >>f, '    { 0, 0 }\n};'
    f.close()
    

   
def make_unicode_table():
    header = []
    codes = {}
    f = open('../config/unicode.tbl', 'r')
    for line in f.xreadlines():
        if line[0] == '#':
            header.append(line.strip())
        else:
            break
    for line in f.xreadlines():
        if len(line) > 0:
            codes[line.split(',')[0].strip()] = line
    f.close()
    
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        if len(latexName) > 0:
            codes[key] = key + ', ' + charClass + ', ' + latexName.replace('\\', '')
        else:
            codes[key] = key + ', ' + charClass
            
    f = open('../config/unicode.tbl', 'w')
    for line in header:
        print >> f, line
    for key in codes:
        print >> f, codes[key]
    f.close()

def make_font_table(font):
##    header = []
##    try:
##        f = open('../config/' + font + '.font', 'r')
##        for line in f.xreadlines():
##            if line[0] == '#':
##                header.append(line.strip())
##            else:
##                break
##        f.close()
##    except IOError:
##        pass
    
    #f = open('../config/' + font + '.font', 'w')
    f = open(font + '.font', 'w')
##    for line in header:
##        print >> f, line
    #print >> f, "name = " + font
    for key in unicodetable:
        latexName, charClass = unicodetable[key]
        pos = fontkey(font, key)
        if pos:
            print >> f, str(pos), key, charClass, latexName
    f.close()

def make_all_font_tables():
    for font in fonttable:
        make_font_table(font)


def symbol_entry(pos, unicode, charClass, name):
    return '    <entry key="%d" number="%s" name="%s" class="%s"/>' % \
           (pos, unicode, name, charClass)


def compare_font(font):
    for line in file(font+".font"):
        list = line.split()
        pos = int(list[0])
        unicode = list[1]
        charClass = list[2]
        if len(list)>3:
            name = list[3]
        else:
            name = ""

        if (pos, unicode) not in fonttable[font]:
            print "not in font", font, (pos, unicode)
            print symbol_entry(pos, unicode, charClass, name)
        if unicode not in unicodetable:
            print font, unicode, (name, charClass)
            print symbol_entry(pos, unicode, charClass, name)
        elif unicodetable[unicode] != (name, charClass):
            print font, unicode, pos, unicodetable[unicode], "!=", (name, charClass)

def compare():
    fontnames = [ "symbol",
                  "esstixnine", 
                  "esstixthirteen", 
                  "esstixeleven", 
                  "esstixfourteen", 
                  "esstixfive", 
                  "esstixfifteen", 
                  "esstixeight", 
                  "esstixthree", 
                  "esstixten", 
                  "esstixsixteen", 
                  "esstixone", 
                  "esstixtwo", 
                  "esstixsix", 
                  "esstixseven", 
                  "esstixtwelve", 
                  "esstixseventeen", 
                  "esstixfour" ]

    for font in fontnames:
        compare_font(font)

        
if __name__ == '__main__':
    parser = make_parser()
    parser.setContentHandler(ContentGenerator())
    parser.parse("symbol.xml")

    #print fonttable
    #print unicodetable

    #compare()
    
    main()
    #make_unicode_table()
    #make_all_font_tables()
