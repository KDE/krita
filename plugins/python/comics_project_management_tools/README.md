

Comics Project Management Tools
===============================

This is the Comics Project Management Tools python plugin for Krita.

CPMT aims to simplify comics creation by:

* Giving the artist a way to organize and quickly access their pages.
* Helping the artist(s) deal with the boring bits meta data bits of a comic project by giving a meta-data editor that gives suggestions, explanation and occasionally a dab of humor.
* Making export set-and-forget type of affair where a single click can export to multiple formats with proper meta-data.

Export-wise, CPMT aims to support:

Advanced Comic Book Format
: An open comics format that has detailed markup as well as support for translations.

CBZ
: the most popular comic file format, with the following meta-data schemes:

    * ACBF - as above.
    * CoMet.xml
    * ComicBookInfo (Spec is unclear so not 100% certain)
    * ComicInfo.xml(Comic Rack)

Epub
: The epub publishing format. Not the most ideal format for handling comics, but most readers can open epub.

Usage - quick-start guide:
-------------------------

First, get the comic manager docker(settings &rarr; dockers &rarr; comic Management Docker). There, select *New Project*.

It will show a dialog asking for:

The project directory.
: This is where everything will be written to.

Concept
: so a simple sentence explaining what you want to write the comic about. This concept is just for you.

Project name.
: This is not the title, but more of a code name which will be used to create pages. For the impatient artist there is even a generator that produces code names.

Language
: The main language, used for all the meta data. By default set to the system locale.

Make a new directory with the project name.
: Whether to make a new project directory inside the selected directory. This allows you to have a generic comics directory that you always select and that CPMT will make directories named with the project name inside.

Pages
: The name for the directory to store the pages. This is where new pages are placed.

Export
: The name for the directory to store the export. This is where the comic will be exported to.

Templates
: The name for the directory to store the template. This is where the page templates get stored.

It will also allow you to edit meta data if you'd want already, but this is not mandatory.

Then after you finish, select *Open Project*, go to the location where you have stored your comics project. There should be a "comicsConfig.json" file there, next to the new folders for the pages, templates and export. Open that.

Now, click *Add Page* to add your first page. You will get a dialog asking for the template. Here you can generate one, or import one. CPMT will remember this as the default one.

Double click the new page to open in Krita.

The second column in the docker allows you to see the "subject" line in the document info if it's filled in.

You can press the arrow next to *Add Page* to get more features, like *Add Existing Page*, *Remove Page*, or *Batch Resize*.

Usage - Meta Data
------------------
You can edit the meta data by clicking the dropdown next to *Project Settings* and selecting *Meta Data*.

There's quite a few fields here, because there's quite a few different types of meta data. Hover over the fields to get an idea of what needs to be typed.

The meta data is intended to be filled out over the course of the project, so don't worry too much if you cannot instantly think of what a certain entry should be.

The meta data fields have auto completion wherever sensible. You can add your own meta data fields as noted in the following section: 

### Adding extra auto-completion keys.
First, you need to go to project settings, and there point the extra keys to a folder where extra keys can be found.

It will search that extra folder for the following folders:

* key_genre
* key_format
* key_rating
* key_characters
* key_author_roles
* key_other

You can add extra auto-completion keys by adding a text file with each new key on a separate line to one of the "key" folders. The name of the text file doesn't matter. This way you can add characters by universe, or archive specific keywords by archive name.

So for example, the following file has three superhero names on different lines, nothing more, nothing less.

```
 Spider-Man
 Hawkeye
 Jean Grey
```

When you then store it as marvel.txt put into the directory "key_characters", Krita will use the names from the list as suggestion for the character field in the meta-data.

The exception is the key_ratings, which uses CSV files, using the top row to determine the title, and then has the rating in the first column, and the description on the second. This allows the description to show up as tool-tips.

### The Author list
The author list is a table containing all the authors of the project. It allows a distinction between given, family, middle and nickname, as well as role, email and homepage.

You can rearrange the author list by drag and dropping the number at the left, as well as adding and removing authors.

Adding an author will always add "John Doe". You can double click the names and cells to change their contents. For the role, there are auto completion keys, so to encourage using standardized ways to describe their roles.

In the main docker, there's an option under the pages actions called *Scrape Authors*, this will make the comics project docker search the pages in the pages list for author info and append that to the author list. It will not attempt to check for duplicates, so be sure to the list afterwards.

Usage - Project Settings
-----------------------
The project settings allows you to change all the technical details of the project:

* the project name
* the concept
* the location of pages, export and templates
* the default template.
* the location of the extra auto-completion keys(see metadata)

Usage - Pages
-------------
There's several other things you can do with pages. You can either access these feature by clicking the drop-down next to *Add Page* or right-clicking the pages list.

Adding pages
: You can add pages by pressing the *Add Page* button. The first time you press this, it'll ask for a template. After you create or select a template it will use this as the default. You can set the default in the project settings.

Adding pages from template:
: Adding pages from a template will always give the template dialog. This will allow you to have several different templates in the templates directory(it will show all the kra files in the templates directory), so that you can have spread, coverpages and other pages at your finger tips. The create template dialog will allow you to make a simple two layer image with a white background, and rulers for the bleeds and guides. Import template will copy selected templates to the template directory, keeping all the necessary files inside the comics project.

Remove a page
: This allows you to remove the selected page in the list from the pages list. It does NOT delete the page from the disk.

Adding existing pages
: This is for when you wish to add existing pages, either because you removed the page from the list, or because you already have a project going and wish to add the pages to the list.

Batch Resize
: This will show a window with resize options. After selecting the right options, all the pages will be resized as such. A progress dialog will pop up showing you which pages have been done and how long it will take based on the passed time.

View Page in Window
: This will pop up a dialog with the selected page's mergedimage.png. The dialog will update when doing this for the image of another page. This is so that you can have a quick reference for a single page in the event your other referencing tools cannot open kra files.

Scrape Authors
: This searches all the files from the pages list for author information and adds that to the author list. It will not check for copies, so you will need to clean up the author list yourself.

Rearranging pages
: You can rearrange pages by moving the number on the left of the page up or down.

Usage - Copy Location
--------------------
Copy location, the button underneath the export button, allows you to copy the current project location to clipboard. Just press it, and paste somewhere else. This is useful when using multiple programs and reference tools and you just want to quickly navigate to the project directory.

Usage - Export
--------------
CPMT will not allow export without any export methods set.

You can configure the export settings by going to the drop-down next to *Project Settings* and selecting *Export Settings*.

Here you can define...

* how much a page needs to be cropped
* which layers to remove by layer color-label
* to which formats to export, in what file-format and how to resize.

Once you've done that, press export. Krita will pop up a progress bar for you with the estimated time and progress, so you can estimate how long you will have to wait.

CPMT will store the resized files and meta data in separate folders in the export folder. This is so that you can perform optimization methods afterwards and update everything quickly.

### ACBF ###

ACBF is the advanced comic book format. It is a metadata file that can hold extra data like panels and text, and can even store translations for the text.

Krita has some support for frames and text. If you name a vector layer "text" or "panels" it will search those for shapes. The shapes that are text nodes will be added to the ACBF file as a text in the main language of the comic, using the bounding box of the text-shape. The shapes that aren't text will have their bounding boxes used as frames. The order of frames and text is determined by the shape z-order in Krita, with the bottom shape being the first and the top shape being the last.

When you generate a CBZ file, the ACBF file will be generated alongside of it. There's in fact two ACBF files being generated: The one in the metadata folder is the ACBF file as it is inside the CBZ. The other ACBF file, next to the CBZ is the standalonefile. This file has the pages embedded, but there's currently fewer viewers who can read it.

To fine tune the export to ACBF, you can go to file->document Information and add the following keywords:

acbf_title
: this will flag this page to be used as a table of contents bookmark inside ACBF. The content mark will use the "title" value in the document information to create a bookmark in the project language.

acbf_none
: Sets the page transition value to "none" explicitly.

acbf_fade
: Sets the page transitio to fade. Viewers that support it will fade to black into this page.

acbf_blend
: Sets the page transition to blend. Viewers that support it will fade the previous page to this one.

acbf_horizontal
: Sets the page transition to scroll_right. Viewers that support it will scroll right to a new page.

acbf_vertical
: Sets the page transition to scroll_down. Viewers that support it will scroll down to a new page.

TODO:
======
Things I still want to do:

* Krita:
	- Generate text from the author list. (Requires text api)
* clean up path relativeness. (Not sure how much better this can be done)
* Make label removal just a list? (unsure)
* PNG and JPEG export settings.
* maybe use python minidom for acbf(or export in general), because then we can create a prettier xml file, which is necessary for helping people edit the files in question. [partially done, epub still needs this]

ACBF list:

* support getting text info from the vector layers. [Partially done]
	- Don't forget text-rotation(Needs API)
	- Probably also support acbf_order[n], where n is used to be the definitive value of the order.
	- Text class battle plan:
            1. Support title and desc in vector shapes(c++)
            2. Support editing them from a right-click "properties" window.
            3. Support getting those values from python.
            4. have acbf_textclass read from the desc.
    - Jump: proly not gonna support this.
    - text url anchors: only used for references.
    - Font embedding: spec is super vague about this. (Not possible with QT)
