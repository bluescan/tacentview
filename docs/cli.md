---
title: CLI

---
## Tacent View CLI
This page is an overview of the features available in _Tacent View_ while using the command line interface. All operations that may be performed in the GUI may also be accessed via the command-line allowing many images to be processed by a single command. For example, a simple CLI command can resize all png files in a list and then save them as jpg files at a desired quality level.

---
## Examples
The easiest way to demonstrate using _Tacent View_ from the command-line is by example. The following assumes a) _tacentview_ or _tacentview.exe_ is in the path or in the current directory, and b) there are multiple images of various types in the current directory. Since Windows can handle not specifying the _exe_ extension, it has been omitted. These examples do not exhaustively demonstrate all options and parameters -- see the following _usage_ section for all the options.\
\
\
{% include_relative examples.md %}

---
## Usage
This is a printout of the CLI usage instructions built into _Tacent View_. To get this printout simply issue the command `tacentview --help`

```
{% include_relative usage.md %}
```
