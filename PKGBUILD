# Maintainer: Jon Sangster <jon@ertt.ca>

pkgname=libsangster_atmega328p-git
pkgver=0.2
pkgrel=1
pkgdesc='A blah blah blah'
depends=('avr-libc>=2.0.0')
arch=('any')
url="https://github.com/sangster/embedded-sangster_atmega328p"
license=('LGPL')
source=('libsangster_atmega328p-git::git+http://github.com/sangster/embedded-sangster_atmega328p#branch=master')
md5sums=('SKIP')

build() {
  export CFLAGS=
  export LDFLAGS=
  cd "$srcdir"/$pkgname
  ./setup_build.sh
  make
}

package() {
  cd "$srcdir"/$pkgname
  make DESTDIR="$pkgdir" install
}

pkgver() {
  cd "$pkgname"
  ( set -o pipefail
  git describe --long 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
  )
}
