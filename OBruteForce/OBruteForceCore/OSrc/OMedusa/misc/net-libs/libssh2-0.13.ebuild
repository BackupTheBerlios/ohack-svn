# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit base eutils

DESCRIPTION="Library implementing the SSH2 protocol."
HOMEPAGE="http://www.libssh2.org"
SRC_URI="mirror://sourceforge/libssh2/${P}.tar.gz"

LICENSE="BSD"
SLOT="0"
KEYWORDS="~x86"
IUSE=""
DEPEND="dev-libs/openssl sys-libs/zlib"
PATCHES="${FILESDIR}/${P}-banner-wait.patch"

src_install() {
	base_src_install
	dodoc README
}
