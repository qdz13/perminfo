# perminfo
[English](README.md) | [日本語]

perminfoはC言語で書かれた、ファイルのパーミッションを表示するシンプルなツールです。
<img src="preview.png" width="600">

## インストール
```sh
$ git clone https://github.com/qdz13/perminfo
$ cd perminfo
$ sudo make install
```
色はconfig.hを編集することで変更できます。

## 使い方
* パーミッションを表示: `perminfo 755`
* ファイルのパーミッションを表示: `perminfo example.txt`
* ディレクトリのパーミッションを表示: `perminfo /`
* ヘルプメッセージを表示: `perminfo --help`
