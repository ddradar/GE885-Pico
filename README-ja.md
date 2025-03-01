# GE885-Pico

[![last commit](https://img.shields.io/github/last-commit/ddradar/GE885-Pico "last commit")](https://github.com/ddradar/ddradar/commits/main)
[![release version](https://img.shields.io/github/v/release/ddradar/GE885-Pico?sort=semver "release version")](https://github.com/ddradar/ddradar/releases)
[![License](https://img.shields.io/github/license/ddradar/GE885-Pico)](LICENSE)

> [!WARNING]
> :construction: このプロジェクトはまだ開発中です。:construction:

RP2040ベースのGE885-PWB(A) (DanceDanceRevolution リンクキット) エミュレーター

## Features

### Supported

- まだ

### Planed

- [ ] PS1 メモリーカードの読み書き
- [ ] コントローラー入力の読み取り (Guitar Freaksで使用)

### Not Planed (but PR is welcome)

- [ ] 他の開発ボードをサポートする (Raspberry Pi Picoなど)
- [ ] PCBデザイン
- [ ] メモリーカードのデータを他のストレージに保存する (SDカードなど)
- [ ] 認証して、メモリーカードのデータをダウンロード/アップロードする機能 (Brightwhiteのように)
- [ ] JVSのデイジーチェーン

## References

- [Controllers and Memory Cards - PlayStation Specifications - psx-spx](https://psx-spx.consoledev.net/controllersandmemorycards/)
- JAMMA VIDEO規格 (第3版)
- [MAMEのk573mcr実装](https://github.com/mamedev/mame/blob/master/src/mame/konami/k573mcr.cpp)
