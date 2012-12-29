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

#ifndef __APP_VERSION_H__
#define __APP_VERSION_H__

namespace KScope
{

namespace App
{

/**
 * Generates version information.
 * @todo Would like to automate generation of version from external source
 *       (e.g., SVN).
 * @author  Elad Lahav
 */
template<int Major, int Minor, int Revision>
struct Version
{
	/**
	 * Major version number.
	 */
	static const int major_ = Major;

	/**
	 * Minor version number.
	 */
	static const int minor_ = Minor;

	/**
	 * Revision number.
	 */
	static const int revision_ = Revision;

	/**
	 * Translates the version numbers into a string.
	 * The string is of the form "MAJOR.MINOR.REVISION (BUILD_DATE)"
	 * @return  The formatted version
	 */
	static QString toString() {
		return QString("%1.%2.%3 (Built %4)").arg(major_).arg(minor_)
		                                     .arg(revision_).arg(__DATE__);
	}

	/**
	 * Determines whether the current version is an alpha/beta/RC release.
	 * This is signified by an odd minor number.
	 */
	static bool isDevel() {
		return (minor_ & 0x1) == 1;
	}
};

/**
 * Specifies the application version.
 */
typedef Version<1, 9, 4> AppVersion;

} // namespace App

} // namespace KScope

#endif // __APP_VERSION_H__
