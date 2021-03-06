/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#ifndef QTLUAMEMBER_HXX_
#define QTLUAMEMBER_HXX_

#include <internal/qtluamember.hh>
#include <QtLua/qtluauserdata.hxx>

namespace QtLua {

Member::Member(const QMetaObject *mo, int index)
	: _mo(mo)
	, _index(index)
{
}

int Member::get_index() const
{
	return _index;
}

}

#endif
