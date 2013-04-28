# Maintainer: Unia <jthidskes@outlook.com> 

pkgname=dwmst-git
_gitname=dwmst
pkgver=2013.04.18
pkgrel=1
pkgdesc="Hardcoded statusbar for DWM"
arch=('any')
url="https://github.com/Unia/dwmst"
license=('GPL2')
groups=('custom')
depends=('libx11' 'wireless_tools' 'alsa-lib' 'libcanberra')
makedepends=('git')
optdepends=('audacious: to have Audacious current song in the statusbar'
            'libmpdclient: to have mpd current song in the statusbar')
source=('git://github.com/Unia/dwmst.git')
md5sums=('SKIP')

pkgver() {
  cd $_gitname
  git log -1 --format="%cd" --date=short | sed 's|-|.|g'
}

build() {
  cd $_gitname
  make
}

package() {
  cd $_gitname
  make PREFIX=/usr DESTDIR="$pkgdir" install
}
