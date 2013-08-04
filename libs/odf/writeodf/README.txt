Generated classes in the writeodf namespace.

The program rng2cpp compiles a given Relax NG (.rng) file into C++ headers. In Calligra, this is used in combination with the RNG for OpenDocument Format.

The generated code has an API with class names that resemble the names of the ODF tags. <text:h/> becomes writeodf::text_h, <office:automatic-styles> becomes writeodf::office_automatic_styles.

The generated code has advantages of directly using KoXMLWriter.
 - function names instead of strings gives autocompletion and catches typing errors at compile time.
 - since elements are added into other elements, the nesting is checked at compile time.
 - elements are automatically closed when they go out of scope (but end() can be called to close them sooner)
 - elements are automatically closed if another item (text or element or other) is added to its parent

Future improvements:
 - also generate code for reading elements
 - also generate code from OOXML Relax NG files
 - check data types (bool, int, string) at compile time
