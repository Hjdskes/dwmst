pkgname=dwm-status
pkgver=20130125
pkgrel=1
pkgdesc='A hardcoded statusbar for DWM'
arch=('any')
url='https://github.com/Unia/dwmst'
license=('custom')
depends=('libx11'
		 'wireless_tools'
		 'alsa-lib')
optdepends=('audacious: to have Audacious current song in the statusbar'
			'mpd: to have mpd current song in the statusbar')
makedepends=('git')
conflicts=()

_gitroot="https://github.com/Unia/dwmst"
_gitname="dwmst"

build() {
  cd "$srcdir"/
	msg "Connecting to GIT server...."

    if [ -d $_gitname ] ; then
    	cd $_gitname && git pull origin
   		msg "The local files are updated."
	else
		git clone --depth=1 $_gitroot $_gitname
		cd $_gitname
	fi
	 msg "GIT checkout done or server timeout"

  cd ${srcdir}/${_gitname}

  make
}

package() {
  cd ${srcdir}/${_gitname}
  make DESTDIR="$pkgdir" install
}
