# Maintainer: Jon Sangster <jon@ertt.ca>

pkgname=libsangster_atmega328p-git
pkgver=0.1
pkgrel=1
pkgdesc='A blah blah blah'
arch=('avr')
url="https://github.com/sangster/embedded-sangster_atmega328p"
license=('LGPL')
source=('libsangster_atmega328p-git::git+http://github.com/sangster/embedded-sangster_atmega328p#branch=master')

build() {
  cd "$srcdir"/$pkgname-$pkgver
  ./bootstrap.sh
  ./configure --prefix=/usr --host=avr
  make
}

package() {
  cd "$srcdir"/$pkgname-$pkgver
  make DESTDIR="$pkgdir" install
}

pkgver() {
  cd "$pkgname"
  ( set -o pipefail
  git describe --long 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
  )
}
