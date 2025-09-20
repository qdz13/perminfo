# perminfo
English | [日本語](README-ja.md)

perminfo is simple tool for displaying file permissions written in C.

<img src="preview.jpg" width="500">

## Installation
```sh
git clone https://codeberg.org/qdz13/perminfo.git
cd perminfo
make
sudo make install
```
> [!TIP]
> The colors can be changed by editing config.h.

## Usage
* Display file permissions: `perminfo example.txt`
* Display directory permissions: `perminfo /path/to/dir`
* Display help messages: `perminfo --help`
