Kross uses PyCXX 5.3.1 (http://cxx.sourceforge.net/)
to access the Python C API.

Following patches where applied and send back to the PyCXX team.

- Unsigned patch
  http://sourceforge.net/tracker/index.php?func=detail&aid=1085205&group_id=3180&atid=303180
- isInstance patch
  http://sourceforge.net/tracker/index.php?func=detail&aid=1178048&group_id=3180&atid=303180
- dir patch
  http://sourceforge.net/tracker/index.php?func=detail&aid=1186676&group_id=3180&atid=303180
- fixed typos
  http://sourceforge.net/tracker/index.php?func=detail&aid=1266579&group_id=3180&atid=103180
  http://sourceforge.net/tracker/index.php?func=detail&aid=1293777&group_id=3180&atid=103180

I also changed the includes to get PyCXX compiled the way we use it.
