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

#ifndef __APP_STRINGS_H__
#define __APP_STRINGS_H__

#include <QString>

namespace KScope
{

namespace App
{

/**
 * Provides various conversions of types to strings.
 * @author Elad Lahav
 */
struct Strings
{
public:
	/**
	 * Converts a query type to a string.
	 * @param  type  The type to convert
	 * @return A string describing the query type
	 */
	static QString toString(Core::Query::Type type) {
		switch (type) {
		case Core::Query::Invalid:
			return QObject::tr("<INVALID>");

		case Core::Query::Text:
			return QObject::tr("Text Search");

		case Core::Query::Definition:
			return QObject::tr("Definition");

		case Core::Query::References:
			return QObject::tr("References");

		case Core::Query::CalledFunctions:
			return QObject::tr("Called Functions");

		case Core::Query::CallingFunctions:
			return QObject::tr("Calling Functions");

		case Core::Query::FindFile:
			return QObject::tr("Search for File");

		case Core::Query::IncludingFiles:
			return QObject::tr("Files #including");

		case Core::Query::LocalTags:
			return QObject::tr("Symbols in This File");
		}

		return QString();
	}

	/**
	 * Converts a query structure to a string.
	 * @param  query  The query to convert
	 * @return A string describing the query
	 */
	static QString toString(const Core::Query& query) {
		switch (query.type_) {
		case Core::Query::Invalid:
			return "<INVALID>";

		case Core::Query::Text:
			return QObject::tr("Text search '%1'").arg(query.pattern_);

		case Core::Query::Definition:
			return QObject::tr("Definition of '%1'").arg(query.pattern_);

		case Core::Query::References:
			return QObject::tr("References to '%1'").arg(query.pattern_);

		case Core::Query::CalledFunctions:
			return QObject::tr("Functions called by '%1'").arg(query.pattern_);

		case Core::Query::CallingFunctions:
			return QObject::tr("Functions calling '%1'").arg(query.pattern_);

		case Core::Query::FindFile:
			return QObject::tr("Find file '%1'").arg(query.pattern_);

		case Core::Query::IncludingFiles:
			return QObject::tr("Files #including '%1'").arg(query.pattern_);

		case Core::Query::LocalTags:
			return QObject::tr("Symbols in '%1'").arg(query.pattern_);
		}

		return QString();
	}
};

} // namespace App

} // namespace KScope

#endif // __APP_STRINGS_H__
