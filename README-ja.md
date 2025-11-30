# perminfo
[English](README.md) | 日本語

perminfoはC言語で書かれた、ファイルのパーミッションを表示するシンプルなツールです。

<img src="preview.jpg" width="500">

## 必要な物
* C23コンパイラ

## インストール
```sh
git clone https://codeberg.org/qdz13/perminfo.git
cd perminfo
make
sudo make install
```
> [!TIP]
> 色はconfig.hを編集することで変更できます。

## 使い方
* ファイルのパーミッションを表示: `perminfo example.txt`
* ディレクトリのパーミッションを表示: `perminfo /path/to/dir`
* ヘルプメッセージを表示: `perminfo --help`
