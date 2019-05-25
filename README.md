# ALFI

ALFI is a query language for graphical user interfaces.

ALFI works in a similar way to SQL but instead of talking to a database you talk
to an ALFI client (or ALFI browser). The ALFI browser will convert the ALFI
queries into graphical elements (i.e. widgets) and render them on the screen.

An ALFI query will tell the client to either insert, delete or update an
element. ALFI is stateless so a typical use-case would be to have a regular web
server serving ALFI queries as responses from regular HTTP requests (like GET,
POST, etc).

The biggest strength of ALFI is that it requires very little technical skill to
get started. If you know basic SQL you can easily write ALFI too. Writing an
ALFI application is much simpler than writing an application using
HTML/JavaScript/CSS.

NAVI is a proof of concept client/browser for ALFI. It takes ALFI queries and
renders widgets in OpenGL. It is currently very much under development.

## Building / Installing

You need to have the glfw3 and nanovg libraries installed in order to be able to
build NAVI. Installing these depends on your distribution of choice.

Build:

    $ make

Install:

    $ make install

And that's it.

## Running

To parse an alfi file:

    $ alfi < example.alfi

To start navi:

    $ navi

## Contact

jens.nyberg@gmail.com

http://github.com/jezze/alfi

