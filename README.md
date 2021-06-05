pacghost: List pacman packages by size or last use.
===================================================
pacghost identifies large and/or not frequently used packages on your pacman
managed package management system.

There's probably a -Q option in pacman to do this already, but sometimes it's
nice to have a dedicated tool.

Usage
-----
See the -h option.  The default should list the top 10 largest pacman packages
installed globally on your system.  If the package cannot be accessed (e.g.,
due to file permisions), no error is reported, the analysis continues.

Building
--------
Run `make`

Contact
-------
Matt Davis: https://github.com/enferex
