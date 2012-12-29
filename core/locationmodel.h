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

#ifndef __CORE_LOCATIONMODEL_H__
#define __CORE_LOCATIONMODEL_H__

#include <QAbstractItemModel>
#include "globals.h"

namespace KScope
{

namespace Core
{

/**
 * Abstract base-class for location models.
 * Provides a common base for LocationListModel and LocationTreeModel.
 * @author Elad Lahav
 */
class LocationModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	LocationModel(QObject* parent = 0);
	virtual ~LocationModel();

	void setRootPath(const QString&);

	/**
	 * Determines which fields of a location structure the model supports, and
	 * in what order.
	 * @param  colList  An ordered list of location structure fields
	 */
	void setColumns(const QList<Location::Fields>& colList) {
		colList_ = colList;
		reset();
	}

	/**
	 * @return The list of query fields presented by the model as columns
	 */
	const QList<Location::Fields>& columns() const {
		return colList_;
	}

	/**
	 * Adds a list of locations under the given index
	 * @param  list   The list of locations
	 * @param  parent The index under which locations should be added
	 */
	virtual void add(const LocationList& list, const QModelIndex& parent) = 0;

	/**
	 * Possible return values for the isEmpty() method.
	 */
	enum IsEmptyResult {
		/** add() was called on this index, but no children were created. */
		Empty,
		/** add() was called on this index and it has children. */
		Full,
		/** add() was not called on this index. */
		Unknown
	};

	/**
	 * Determines whether the given index has children, and if not, whether this
	 * is because add() was called with no locations or add() was never called
	 * for this index.
	 * This method is useful for distinguishing queried-but-empty indices from
	 * non-queried ones in a lazy-query view.
	 * @param  index The required index
	 * @return See IsEmptyResult
	 */
	virtual IsEmptyResult isEmpty(const QModelIndex& index) const = 0;

	/**
	 * Deletes all locations in the model rooted at the given index.
	 */
	virtual void clear(const QModelIndex& parent) = 0;

	/**
	 * Converts a model index into a location structure.
	 * @param  index The index to convert
	 * @param  loc   The structure to fill
	 * @return true if the index represents a valid location, false otherwise
	 */
	virtual bool locationFromIndex(const QModelIndex& index,
	                               Location& loc) const = 0;

	/**
	 * Fills a location structure with the information stored in the first
	 * index.
	 * @param  loc The structure to fill
	 * @return true if a valid index was located, false otherwise
	 */
	virtual bool firstLocation(Location& loc) const = 0;

	/**
	 * Finds the next index in the model.
	 * The definition of "next" is implementation-dependent.
	 * @param  index The index to start the search from
	 * @return The next index in the model
	 */
	virtual QModelIndex nextIndex(const QModelIndex& index) const = 0;

	/**
	 * Finds the previous index in the model.
	 * The definition of "previous" is implementation-dependent.
	 * @param  index The index to start the search from
	 * @return The previous index in the model
	 */
	virtual QModelIndex prevIndex(const QModelIndex& index) const = 0;

	// QAsbstractItemModel implementation.
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant headerData(int, Qt::Orientation,
	                            int role = Qt::DisplayRole) const;

#ifndef QT_NO_DEBUG
	void verify(const QModelIndex& parentIndex = QModelIndex()) const;
#endif

protected:
	/**
	 * The list of query fields presented by the model as columns.
	 */
	QList<Location::Fields> colList_;

	/**
	 * A common root path for all files in the model.
	 * Files for which this path is a prefix will be presented with a '$' sign
	 * as an abbreviation of the common path.
	 */
	QString rootPath_;

	QVariant locationData(const Location&, uint, int) const;
	QString columnText(Location::Fields) const;
};

} // namespace Core

} // namespace KScope

#endif // __CORE_LOCATIONMODEL_H__
