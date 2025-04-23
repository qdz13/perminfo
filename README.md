# perminfo
perminfo is simple tool for displaying file permissions written in C.
<img src="preview.png" width="600">

## Installation
```sh
$ git clone https://github.com/qdz13/perminfo
$ cd perminfo
$ sudo make install
```
The colors can be changed by editing config.h.

## Usage
* Display permissions: `perminfo 755`
* Display file permissions: `perminfo example.txt`
* Display directory permissions: `perminfo /`
* Display help messages: `perminfo --help`
