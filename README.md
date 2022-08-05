# yapa - Yet Another Photo Album

Yet Another Photo Album is a tool to create a static HTML album from a collection of pictures.

All config and meta files are plain ASCII text. This allows to process all files with pure shell commands. The allows to specify the order of images and directories for output, including a label.

Every directory can have a description. It is read from a file 
"directory.txt" in the same directroy for which the description is.
The description for pictures is read from a file <image filename>.txt
next to the image, where the suffix .txt can replace the suffix of
the image, or be added. Supported images or jpeg and png.
The content of the files with the description can be pure ASCII or HTML,
which will be added inside of <p></p> tags in the final HTML page.

The order of images is defined by the file yapa/images, the order
of directories is defined by yapa/directories. The first part of the
file contains the filename/directory name. Optional you can add a @
after the name followed by a short label, which is used instead of
the file/directory name in the HTML output.

The yapa directory in the root of the photo album directroy
hierachy contains the file "root". Here optional the name of the
album can be specified, the default is "gallery-name=Photo Gallery".

Every directory can have its own yapa/config file, where this options
from this file are valid for this directory and all subdirectories.
The currently known options are:

subdir-format=[0|1]
  - 0 means we use a table with all subdirectories
  - 1 means we use a list of all subdirectories

subdir-columns=N
  - number of columns a table with all subdriectories should have

image-columns=N
  - number of columns of images a index page should have at max.

image-rows=N
  - number of rows of images a index page should have at max.

thumbnail-size=N
  - NxN is the max. size of a thumbnail on the index page,
    the default is 128x128

midnail-size=N
  - NxN is the max. size of a nail on the image page,
    the default is 640x640

sort-directory=[0|1|2]
  - 0 means don't sort directories
  - 1 means add new directories sorted at the end of the existing list
  - 2 means sort whole list of directories

sort-images=[0|1|2]
  - 0 means don't sort images
  - 1 means add new pictures sorted at the end of the existing list
  - 2 means sort whole list of pictures


To link Images from another directory into the current one, a file
called <path>/yapa/links has to be created. The content of this file
is a line by line list of images relative to the <path> directory.
