OSMExplorer
===========

OSMExplorer is an application that lets teams share information geographically. Users can join a shared
workspace and create and modify objects, such as markers, polygons, and tracks. Files can be attached to
objects as well, letting users share location-related data visually.

It's not a full-fledged GIS application as it's not designed to edit maps, but to use maps to organize
information that has geographical meaning. Use it to share hunting spots with friends, or good spots
for rockhounding with a club. Mark amateur radio contacts and share with your ham club. The list goes
on.

OSMExplorer is based on a custom [slippy map](https://en.wikipedia.org/wiki/Tiled_web_map) widget that
can use any standard map tile server to display maps.

## Why?

I had the idea for this app after making my own slippy map widget for Qt, called
[SlippyMap](https://github.com/archnemesis/SlippyMap). I was thinking about rockhounders and how they
might share information about different spots to collect rocks, and I though of a geographical
file-sharing application that let you see where things are. Will anyone want to use it? I'm not sure.

## How does it work?

It's backed by a PostgreSQL database running the PostGIS extension. A small REST server sits in between
the application and the database, and returns layers and objects depending on your "viewport." As you
move around the map, objects are queried and loaded. The goal is to be reasonably performant while
you're browsing around the map. The server is called
[OSMServer](https://github.com/archnemesis/OSMServer).

## What are it's features?

There are few features now, and even fewer fully complete:

* Multiple map layers (base layers and overlay layers). You can show radar tiles on top of the map,
  for instance.
* Multiple object layers, letting you organize your objects by whatever means you see fit. Sort of
  like layers in Photoshop.
* Objects that you can add to the map include markers (points), lines, polygons and more advanced
  objects like GPS tracks.
* Files can be attached to objects.
* GPX files can be imported.
* Any GPS that outputs NMEA data can be attached and you can see your live position.
* Undo/redo while editing objects on the map.
* Has a Windows installer.

## What is planned?

Here is a list of things I want to have:

* Share your GPS position with other users in real time.
* Ability to add comments and notes to objects.
* Whatever users want!

## How do I build it?

You should be able to build the project with a functioning Qt 5 development environment. A move to
Qt 6 is planned but has not happened yet.

### Requirements

* Qt 5
* Boost
* OpenSSL

### Build It

```shell
$ git clone ...
$ cd OSMExplorer
$ git submodule init
$ mkdir ../OSMExplorer-Build
$ cd ../OSMExplorer-Build
$ cmake ../OSMExplorer
$ make
```

## Can I help?

Yes you can, just drop me a line <robin@robingingras.com>.