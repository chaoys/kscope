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

#include "locationtreemodel.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  parent   Parent object
 */
LocationTreeModel::LocationTreeModel(QObject* parent)
	: LocationModel(parent), root_(Location())
{
}

/**
 * Class destructor.
 */
LocationTreeModel::~LocationTreeModel()
{
}

/**
 * Appends the given list to the one held by the model.
 * @param  locList  Result information
 * @param  parent   Index under which to add the results
 */
void LocationTreeModel::add(const LocationList& locList,
                            const QModelIndex& parent)
{
	Node* node;

	// Determine the node under which to add the results.
	if (!parent.isValid()) {
		node = &root_;
	}
	else {
		node = static_cast<Node*>(parent.internalPointer());
		if (node == NULL)
			return;
	}

	// Mark the item for use with isEmpty().
	// TODO: Is there a way to force the view to repaint this item?
	node->data().locationsAdded_ = true;

	// Determine the first and last rows for the new items.
	int firstRow = node->childCount();
	int lastRow = firstRow + locList.size() - 1;
	if (lastRow < firstRow)
		return;

	// Begin row insertion.
	// This is required by QAbstractItemModel.
	beginInsertRows(parent, firstRow, lastRow);

	// Add the entries.
	foreach (Location loc, locList)
		node->addChild(loc);

	// End row insertion.
	// This is required by QAbstractItemModel.
	endInsertRows();
}

/**
 * Determines whether the index has children, and if not, for what reason.
 * @param  index The index to check
 * @return See LocationModel::IsEmptyResult
 */
LocationModel::IsEmptyResult
LocationTreeModel::isEmpty(const QModelIndex& index) const
{
	const Node* node;

	// Get the node from the index.
	if (!index.isValid()) {
		node = &root_;
	}
	else {
		node = static_cast<Node*>(index.internalPointer());
		if (node == NULL)
			return Unknown;
	}

	// Return Unknown if locations were never added under this item.
	if (!node->data().locationsAdded_)
		return Unknown;

	// Return Empty or Full, based on the existence of children.
	return node->childCount() == 0 ? Empty : Full;
}

/**
 * Removes all tree nodes rooted at the given index.
 * @param  parent The index to start from
 */
void LocationTreeModel::clear(const QModelIndex& parent)
{
	// Handle the root node (removing all data from the model).
	if (!parent.isValid()) {
		if (root_.childCount() > 0) {
			root_.clear();
			root_.data().locationsAdded_ = false;
			reset();
		}
		return;
	}

	// Get the node from the index.
	Node* node = static_cast<Node*>(parent.internalPointer());
	if (node == NULL || node->childCount() == 0)
		return;

	// Delete all descendants.
	beginRemoveRows(parent, 0, node->childCount() - 1);
	node->clear();
	node->data().locationsAdded_ = false;
	endRemoveRows();
}

/**
 * Converts an index into a location descriptor.
 * @param  idx  The index to convert
 * @param  loc    An object to fill with the location information
 * @return true if successful, false if the index does not describe a valid
 *         position in the location tree
 */
bool LocationTreeModel::locationFromIndex(const QModelIndex& idx,
                                          Location& loc) const
{
	// Make sure the index is valid.
	if (!idx.isValid())
		return false;

	Node* node = static_cast<Node*>(idx.internalPointer());
	if (node == NULL)
		return false;

	loc = node->data().loc_;
	return true;
}

/**
 * Returns the location at the root of the tree.
 * @param  loc  An object to fill with the location information
 * @return true if successful, false if the list is empty
 */
bool LocationTreeModel::firstLocation(Location& loc) const
{
	if (root_.childCount() == 0)
		return false;

	loc = root_.child(0)->data().loc_;
	return true;
}

/**
 * Finds the successor of the given index.
 * @param  idx  The index for which to find a successor
 * @return The successor index
 */
QModelIndex LocationTreeModel::nextIndex(const QModelIndex& idx) const
{
	// If the given index is invalid, return the index of the first item on the
	// list.
	if (!idx.isValid())
		return index(0, 0, QModelIndex());

	// Get the tree item for the index.
	const Node* node = static_cast<Node*>(idx.internalPointer());
	if (node == NULL)
		return QModelIndex();

	// Go up the tree, looking for the first immediate sibling.
	const Node* parent;
	while ((parent = node->parent()) != NULL) {
		// Get the node's sibling.
		node = parent->child(node->index() + 1);
		if (node != NULL)
			return createIndex(node->index(), 0, (void*)node);

		node = parent;
	}

	return QModelIndex();
}

/**
 * Finds the predecessor of the given index.
 * @param  idx  The index for which to find a predecessor
 * @return The predecessor index
 */
QModelIndex LocationTreeModel::prevIndex(const QModelIndex& idx) const
{
	// If the given index is invalid, return the index of the first item on the
	// list.
	// TODO: What's the best default for a previous index on an invalid index?
	if (!idx.isValid())
		return index(0, 0, QModelIndex());

	// Get the tree item for the index.
	const Node* node = static_cast<Node*>(idx.internalPointer());
	if (node == NULL)
		return QModelIndex();

	// Go up the tree, looking for the first immediate sibling.
	const Node* parent;
	while ((parent = node->parent()) != NULL) {
		// Get the node's sibling.
		node = parent->child(node->index() - 1);
		if (node != NULL)
			return createIndex(node->index(), 0, (void*)node);

		node = parent;
	}

	return QModelIndex();
}

/**
 * Creates an index for the given parameters.
 * @param  row     Row number, with respect to the parent
 * @param  column  Column number
 * @param  parent  Parent index
 * @return The new index, if created, an invalid index otherwise
 */
QModelIndex LocationTreeModel::index(int row, int column,
									 const QModelIndex& parent) const
{
	// Extract the node object from the index.
	const Node* node;
	if (!parent.isValid()) {
		node = &root_;
	}
	else {
		// Get the tree item for the parent.
		node = static_cast<Node*>(parent.internalPointer());
		if (node == NULL)
			return QModelIndex();
	}

	// Get the child at the row'th position.
	node = node->child(row);
	if (node == NULL)
		return QModelIndex();

	return createIndex(row, column, (void*)node);
}

/**
 * Returns an index for the parent of the given one.
 * @param  idx  The index for which the parent is to be returned
 * @return An invalid index
 */
QModelIndex LocationTreeModel::parent(const QModelIndex& idx) const
{
	if (!idx.isValid())
		return QModelIndex();

	// Get the tree item for the index.
	const Node* node = static_cast<Node*>(idx.internalPointer());
	if (node == NULL)
		return QModelIndex();

	// Get the parent node.
	node = node->parent();
	if ((node == NULL) || (node == &root_))
		return QModelIndex();

	return createIndex(node->index(), 0, (void*)node);
}

/**
 * Determines the number of children for the given parent index.
 * @param  parent  The parent index
 * @return The number of child indices belonging to the parent
 */
int LocationTreeModel::rowCount(const QModelIndex& parent) const
{
	const Node* node;
	if (!parent.isValid()) {
		// An invalid index represents the root item.
		node = &root_;
	}
	else {
		// Get the tree item for the index.
		node = static_cast<Node*>(parent.internalPointer());
		if (node == NULL)
			return 0;
	}

	return node->childCount();
}

/**
 * Determines whether the node at the given index has children.
 * We return true if one of two conditions are met:
 * 1. The node has children
 * 2. The node represents a function that was not yet queried
 * Case (2) will include an expansion symbol next to the function, which, when
 * clicked, will result in a query for called/calling functions.
 * @param  parent  The parent index
 * @return true if the parent has children (or can have children), false
 *         otherwise
 */
bool LocationTreeModel::hasChildren(const QModelIndex& parent) const
{
	const Node* node;
	if (!parent.isValid()) {
		// An invalid index represents the root item.
		node = &root_;
	}
	else {
		// Get the tree item for the index.
		node = static_cast<Node*>(parent.internalPointer());
		if (node == NULL)
			return 0;
	}

	return node->childCount() > 0;
}

/**
 * Extracts location data from the given index.
 * @param   index  The index for which data is requested
 * @param   role   The role of the data
 * @return
 */
QVariant LocationTreeModel::data(const QModelIndex& idx, int role) const
{
	// No data for invalid indices.
	if (!idx.isValid()) {
		if (idx.column() == 0)
			return "<ROOT>";

		return QVariant();
	}

	// Get the location for the index's row.
	Node* node = static_cast<Node*>(idx.internalPointer());
	if (node == NULL)
		return false;

	// Get the column-specific data.
	return locationData(node->data().loc_, idx.column(), role);
}

} // namespace Core

} // namespace KScope
