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

#ifndef __CORE_STRINGS_H__
#define __CORE_STRINGS_H__

#include "globals.h"

namespace KScope
{

namespace Core
{

/**
 * Translates various values into strings.
 * This class inherits from QObject in order to have access to the tr() method.
 * @author Elad Lahav
 */
struct Strings : public QObject
{
	/**
	 * Generates an icon for a given tag type.
	 * TODO This should probably be moved somewhere else (like a Strings class)
	 * @param  type  The type for which a name is requested
	 * @return The matching name
	 */
	static QString tagName(Tag::Type type) {
		switch (type) {
		case Tag::UnknownTag:
			return QString();

		case Tag::Variable:
			return tr("Variable");

		case Tag::Function:
			return tr("Function");

		case Tag::Struct:
			return tr("Struct");

		case Tag::Union:
			return tr("Union");

		case Tag::Member:
			return tr("Struct/Union Member");

		case Tag::Enum:
			return tr("Enumeration");

		case Tag::Enumerator:
			return tr("Enumeration Value");

		case Tag::Typedef:
			return tr("Type Definition");

		case Tag::Define:
			return tr("Preprorcessor Definition");

		case Tag::Include:
			return tr("#include Directive");

		case Tag::Label:
			return tr("Go-to Label");

		default:
			Q_ASSERT(false);
		}

		return QString();
	}
};

}

} // namespace KScope

#endif // __CORE_STRINGS_H__
