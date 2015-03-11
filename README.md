museomix-pimp-my-room
=====================

A Project for Museomix 2013 in Paris' Art Décoratif museum

This Software allows displaying a room and a collection of objects and
tapisseries, user can select them and decorate the room with them.


Client:
-------

The client uses selected images to display the room and objects to use for
decoration, when selected or moved, the coordinates of these objects are sent
to the server software, to be displayed on the wall.

The client is realised using Python and the Kivy framework.

To start it, go to the client subdirectory and start

    python main.py


Server:
-------

written in C++ using Cinder

Installation:
++++++++++++++

Install CINDER 8.5 for windows : http://libcinder.org/
Install visual studio 2012 for desktop
set environment variable CINDER_805 on CINDER's root.


Control :
to manage the window:
	shift escape : quit
	f	     : change fullscreen
	i	     : display infos

to manage warping : 
	see help_warping.png
	shift escape to save

to manage masks :
	v : displaay mask points
	n : new mask
	b : change mask selection
	x : delete mask
	shift escape to save masks

