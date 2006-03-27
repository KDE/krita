/* LibHnj is dual licensed under LGPL and MPL. Boilerplate for both
 * licenses follows.
 */

/* LibHnj - a library for high quality hyphenation and justification
 * Copyright (C) 1998 Raph Levien, (C) 2001 ALTLinux, Moscow
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA.
*/

/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "MPL"); you may not use this file except in
 * compliance with the MPL.  You may obtain a copy of the MPL at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the MPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the MPL
 * for the specific language governing rights and limitations under the
 * MPL.
 *
 */
/* wrappers for malloc */

#include <stdlib.h>
#include <stdio.h>

void *
hnj_malloc (int size)
{
  void *p;

  p = malloc (size);
  if (p == NULL)
    {
      fprintf (stderr, "can't allocate %d bytes\n", size);
      exit (1);
    }
  return p;
}

void *
hnj_realloc (void *p, int size)
{
  p = realloc (p, size);
  if (p == NULL)
    {
      fprintf (stderr, "can't allocate %d bytes\n", size);
      exit (1);
    }
  return p;
}

void
hnj_free (void *p)
{
  free (p);
}

