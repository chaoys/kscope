/***************************************************************************
 *   Copyright (C) 2007-2009 by Elad Lahav
 *   elad_lahav@users.sourceforge.net
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

#ifndef __CORE_IMAGES_H__
#define __CORE_IMAGES_H__

#include <QIcon>
#include "globals.h"

namespace KScope
{

namespace Core
{

/**
 * Provides images for various values.
 * @author Elad Lahav
 */
struct Images
{
	/**
	 * Generates an icon for a given tag type.
	 * TODO This should probably be moved somewhere else (like an Images class)
	 * @param  type  The type for which an icon is requested
	 * @return The matching icon
	 */
	static QIcon tagIcon(Tag::Type type) {
		switch (type) {
		case Tag::UnknownTag:
			return QIcon();

		case Tag::Variable:
			return QIcon(":/images/variable");

		case Tag::Function:
			return QIcon(":/images/function");

		case Tag::Struct:
			return QIcon(":/images/struct");

		case Tag::Union:
			return QIcon(":/images/union");

		case Tag::Member:
			return QIcon(":/images/member");

		case Tag::Enum:
			return QIcon(":/images/enum");

		case Tag::Enumerator:
			return QIcon(":/images/enumerator");

		case Tag::Typedef:
			return QIcon(":/images/typedef");

		case Tag::Define:
			return QIcon(":/images/define");

		case Tag::Include:
			return QIcon(":/images/include");

		case Tag::Label:
			return QIcon(":/images/label");

		default:
			Q_ASSERT(false);
		}

		return QIcon();
	}
};

}

} // namespace KScope

#endif // __CORE_IMAGES_H__
