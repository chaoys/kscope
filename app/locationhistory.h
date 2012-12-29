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

#ifndef __APP_LOCATIONHISTORY_H__
#define __APP_LOCATIONHISTORY_H__

#include <QList>
#include <QDebug>
#include <core/globals.h>

namespace KScope
{

namespace App
{

/**
 * Keeps track of visited locations in the source code.
 * @author Elad Lahav
 */
class LocationHistory
{
public:
	/**
	 * Class constructor.
	 */
	LocationHistory() : pos_(-1) {}

	/**
	 * Class destructor.
	 */
	~LocationHistory() {}

	/**
	 * @return The list of locations
	 */
	const Core::LocationList& list() const { return locList_; }

	/**
	 * Adds a location descriptor to the list.
	 * The location is added immediately after the current position. If this
	 * is not the end of the list, all locations following the new one are
	 * discarded.
	 * @note   If the new location is "close" to the current one, than it just
	 *         replaces it in the history list. This keeps the list from being
	 *         cluttered by close moves. The current definition of close is
	 *         being on the same file, within 5 lines of each other.
	 * @param  loc  The descriptor to add
	 */
	void add(const Core::Location& loc) {
		qDebug() << "Add history" << loc.file_ << loc.line_;

		// Replace the current location if it is within 5 lines of new one.
		if (pos_ >= 0) {
			Core::Location curLoc = locList_.at(pos_);
			if ((curLoc.file_ == loc.file_)
			    && (qAbs((int)curLoc.line_ - (int)loc.line_) < 5)) {
				locList_[pos_] = loc;
				return;
			}
		}

		// Remove any location descriptors after the current position.
		while (pos_ < (locList_.size() - 1))
			locList_.removeLast();

		// Add the new descriptor.
		locList_.append(loc);
		pos_++;
	}

	/**
	 * Retrieves the next descriptor in the list, updating the current position.
	 * @param  loc  A descriptor to fill
	 * @return true if successful, false if already at the end of the list
	 */
	bool next(Core::Location& loc) {
		if (pos_ == (locList_.size() - 1))
			return false;

		// Update the current position.
		++pos_;

		// Get the new current descriptor.
		Q_ASSERT(pos_ >= 0 && pos_ < locList_.size());
		loc = locList_.at(pos_);
		return true;
	}

	/**
	 * Retrieves the previous descriptor in the list, updating the current
	 * position.
	 * @param  loc  A descriptor to fill
	 * @return true if successful, false if already at the beginning of the list
	 */
	bool prev(Core::Location& loc) {
		if (pos_ <= 0)
			return false;

		// Update the current position.
		--pos_;

		// Get the new current descriptor.
		Q_ASSERT(pos_ >= 0 && pos_ < locList_.size());
		loc = locList_.at(pos_);
		return true;
	}

	/**
	 * Removes all items.
	 */
	void clear() {
		locList_.clear();
		pos_ = -1;
	}

private:
	/**
	 * The history is kept as a list of locations.
	 */
	QList<Core::Location> locList_;

	/**
	 * The current position in the history.
	 */
	int pos_;
};

} // namespace App

} // namespace KScope

#endif // __APP_LOCATIONHISTORY_H__
