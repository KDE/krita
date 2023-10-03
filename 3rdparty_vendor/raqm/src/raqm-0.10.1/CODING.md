Coding Style Guide
==================

* Indent with 2 spaces, not tabs.

* In function declarations, put function name on a new line:
````
static int
move (int x);
````

* In function declarations, each parameter should be in a separate line, and
  aligned to the first parameter:
````
static void
move (double x,
      int    y,
      char  *total);
````

* Place curly brackets on new lines on the new line:
````
if (x == 3)
{
  ...
}
else
{
  ...
}
````

* There should be a space after `,` or `;` on functions call, functions headers
  or for loops definition but not before them. For example:
````
foo (x, y, count);
int draw (int x, int total, double sum);
````

* All declaration should be in the beginning of code block:
````
int
main ()
{
  int num1;
  int num2
  double sum;

  printf ("Enters two number to add\n");

  scanf ("%d %d", &num1, &num2);
  sum = add (num1, num2);         /* function call */
  printf ("sum=%d", sum);

  return 0;
}
````

* In variable declarations, the asterisk attaches to the variable:
````
Shape *shape;
Data *data;
````

* Put space before opening parentheses `(`, and no space after them, nor before
  closing parenthesis:
````
add (num1, num2);
for (i=0; i<3; i++)
{
  ...
}
````

* Multi-line comments, lines should be aligned to the first line:
````
/* Handling script detection for each character of the input string,
   if the character script is common or inherited it takes the script
   of the character before it except some special paired characters */
````

* Use C comments `/* */` not C++ comments `//`.

* Variables should be all lower case, and compound words separated by `_`:
````
int total_amount;
string first_stage;
````

* Avoid consecutive empty lines, the following code is a bad example:
````
/* to get number of runs */
run_count = get_visual_runs (types, length, par_type, levels, scripts, NULL);
run = (Run*) malloc (sizeof (Run) * run_count);


/* to populate run array */
get_visual_runs (types, length, par_type, levels, scripts, run);
````

* Operators like `*`, `+`, `-`, `/`, `=`, `==`, `<=`, `!=`, etc. should be
  surrounded by spaces:
````
if (someVar == 0)
x = y + z * m;
````
