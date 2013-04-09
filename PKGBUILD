pkgname=dwm-status
pkgver=2013.03.28
pkgrel=1
pkgdesc='A hardcoded statusbar for DWM'
arch=(any)
groups=(custom)
url='https://github.com/Unia/dwmst'
license=(custom)
depends=(libx11 wireless_tools alsa-lib)
optdepends=('audacious: to have Audacious current song in the statusbar'
			'libmpdclient: to have mpd current song in the statusbar')
makedepends=(git)
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

pkgver() {
	cd "$srcdir/$_gitname"
	git log -1 --format="%cd" --date=short | sed 's|-|.|g'
}

package() {
	cd ${srcdir}/${_gitname}
	make DESTDIR="$pkgdir" install
}
