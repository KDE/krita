#!/usr/bin/env python

import os, sys

dcopiface_header = """/* This file is part of the KDE project
 *  Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef %(classname_upper)sIFACE_H
#define %(classname_upper)sIFACE_H

#include <dcopref.h>
#include <dcopobj.h>

#include <QString>

class %(classname)s;

class %(classname)sIface : virtual public DCOPObject
{
	K_DCOP
public:
	%(classname)sIface( %(classname)s * parent );
k_dcop:

private:

	%(classname)s *m_parent;
};

#endif
"""

dcopiface_template = """/*
 *  This file is part of the KDE project
 *
 *  Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <kapplication.h>


#include "%(ifaceheader)s"

#include "%(classheader)s"

#include <dcopclient.h>

%(classname)sIface::%(classname)sIface( %(classname)s * parent )
	: DCOPObject()
{
	m_parent = parent;
}

"""

def parseHeader(headerfile, classname):
	# parse the source class header to get a list of functions we're going to wrap
	functions = []
	if (headerfile.find("private:") > -1):
		lines = headerfile[headerfile.find(classname):headerfile.find("private")].splitlines()
	else:
		lines = headerfile[headerfile.find(classname):headerfile.find("#endif")].splitlines()
	i = 0
	while i < len(lines):
		line = lines[i].strip()
		if (line.startswith("/") or 
			line.startswith("public:") or 
			line.startswith("*") or 
			line.startswith(classname) or
			line.startswith("class") or
			line.startswith("Q_OBJECT") or
			line.startswith("#") or
			line.startswith("}") or
			line.startswith("public slots:") or
			line.find("~")  != -1 or
			len(line) == 0
			):
			i+=1	
			continue
		if (line.startswith("protected")):
			return functions
		# by now we are reasonable sure that this is a function. We need to find the end of the function definition, and then 
		# if the return type is not primitive, replace it with dcopref.
		function = line
		complete = 0
		# strip the inline implementation
		if (line.find("{") > -1):
			function = line[:line.find("{")]
			if function.find("}") > -1:
				function += line[line.find("}") + 1:]
				complete = 1
			else:
				i += 1
				# search for the missing } on the next lines
				while i < len(lines):
					if (lines[i].find("}") > -1):
						function += lines[i][lines[i].find("}") + 1:]
						complete = 1
					i += 1
		else:
			complete = 1

		if complete == 0:
			i+=1
			continue

		if (function.endswith("= 0;")):
			function = function[:-4] + ";"
		print "\t", function
		i+=1
			

def createDCOP(header):

	# Determine filenames and classnames

	implementation = header[:-1] + "cc"
	classname = ""	
	classname_upper ="_"
	for part in header[:-2].split("_"):
		classname = classname + part.capitalize()
		classname_upper = classname_upper + part.upper() + "_"
	ifaceheader = header[:-2] + "_iface.h"
	ifaceimplementation = header[:-2] + "_iface.cc"
	ifaceclass = classname + "Iface"

	#print "with: ", implementation, classname, classname_upper, ifaceheader, ifaceimplementation, ifaceclass
	file(ifaceheader, "w+").write(dcopiface_header % { "classname_upper" : classname_upper,
							   "classname" : classname})
	file(ifaceimplementation, "w+").write(dcopiface_template % {"ifaceheader" : ifaceheader,
								    "classheader" : header,
								    "classname" : classname })
	functions = parseHeader(open(header).read(), classname)

def main(args):
	for line in args[1:]:
		print "Going to create a dcop interface for:", line[:-1]
		createDCOP(line.strip())

if __name__=="__main__":
    main(sys.argv)


