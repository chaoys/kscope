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

#ifndef __CORE_LOCATIONLISTMODEL_H__
#define __CORE_LOCATIONLISTMODEL_H__

#include "locationmodel.h"

namespace KScope
{

namespace Core
{

/**
 * A list model for displaying locations.
 * This model should be used for all location displays that do not require
 * a tree-like structure, as its internal storage is more compact and faster
 * to update.
 * @author Elad Lahav
 */
class LocationListModel : public LocationModel
{
	Q_OBJECT

public:
	LocationListModel(QObject* parent = 0);
	~LocationListModel();

	// LocationMode implementation.
	void add(const LocationList&, const QModelIndex& index = QModelIndex());
	IsEmptyResult isEmpty(const QModelIndex&) const;
	void clear(const QModelIndex& parent = QModelIndex());
	bool locationFromIndex(const QModelIndex&, Location&) const;
	bool firstLocation(Location&) const;
	QModelIndex nextIndex(const QModelIndex&) const;
	QModelIndex prevIndex(const QModelIndex&) const;

	// QAsbstractItemModel implementation.
	virtual QModelIndex index(int row, int column,
							  const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex&,
	                      int role = Qt::DisplayRole) const;

private:
	/**
	 * Result list.
	 */
	LocationList locList_;

	/**
	 * Whether add() was called.
	 * Required by the locationsAdded() method.
	 */
	bool locationsAdded_;
};

} // namespace Core

} // namespace KScope

#endif // __CORE_LOCATIONLISTMODEL_H__
