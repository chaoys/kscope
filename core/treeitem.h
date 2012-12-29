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

#ifndef __CORE_TREEITEM_H__
#define __CORE_TREEITEM_H__

#include <QList>

namespace KScope
{

namespace Core
{

/**
 * A generic ordered tree structure.
 * @author  Elad Lahav
 */
template<typename DataT>
class TreeItem
{
public:
	typedef TreeItem<DataT> SelfT;

	/**
	 * Class constructor.
	 * Creates a parent-less tree node.
	 * @param  data   Node data
	 */
	TreeItem(DataT data = DataT()) : data_(data), parent_(NULL), index_(0) {}

	/**
	 * Class destructor.
	 */
	~TreeItem() {}

	/**
	 * Data accessor.
	 * @return The node's data
	 */
	DataT& data() { return data_; }

	/**
	 * Data accessor (const version).
	 * @return The node's data
	 */
	const DataT& data() const { return data_; }

	/**
	 * Parent accessor.
	 * @return The node's parent
	 */
	SelfT* parent() const { return parent_; }

	/**
	 * Adds a child to this node.
	 * @param  data  The child's data
	 */
	SelfT* addChild(DataT data) {
		SelfT child(data);
		child.parent_ = this;
		child.index_ = childList_.size();

		childList_.append(child);

		return &childList_.last();
	}

	/**
	 * Child accessor.
	 * @param  index  The ordinal number of the requested child
	 * @return Pointer to the requested child, NULL if the index is out of
	 *         bounds
	 */
	SelfT* child(int index) {
		if (index < 0 || index >= childList_.size())
			return NULL;

		return &childList_[index];
	}

	/**
	 * Child accessor (const version).
	 * @param  index  The ordinal number of the requested child
	 * @return Pointer to the requested child, NULL if the index is out of
	 *         bounds
	 */
	const SelfT* child(int index) const {
		if (index < 0 || index >= childList_.size())
			return NULL;

		return &childList_[index];
	}

	/**
	 * Child-count accessor.
	 * @return The number of children of this node
	 */
	int childCount() const { return childList_.size(); }

	/**
	 * Self-index accessor.
	 * @return The index of the node with respect to its parent
	 */
	int index() const {
		if (index_ == -1)
			index_ = parent_->childList_.indexOf(*this);

		return index_;
	}

	/**
	 * Locates a child node holding the given data.
	 * @param  data  Used for finding the node
	 * @return The found child, NULL if no such child exists
	 */
	SelfT* findChild(DataT data) {
		for (int i = 0; i < childList_.size(); i++) {
			if (childList_[i].data_ == data)
				return &childList_[i];
		}

		return NULL;
	}

	/**
	 * Recursively removes all children nodes.
	 */
	void clear() {
		for (int i = 0; i < childList_.size(); i++)
			childList_[i].clear();

		childList_.clear();
	}

	/**
	 * Two items are the same if and only if they are the same object.
	 * @param  other  The item to compare with
	 * @return true if the other item is equal to this one, false otherwise
	 */
	bool operator==(const SelfT& other) {
		return (this == &other);
	}

private:
	/**
	 * Node's data.
	 */
	DataT data_;

	/**
	 * Node's parent, NULL for root.
	 */
	SelfT* parent_;

	/**
	 * The index of this item in its parent's list.
	 * A value of -1 indicates that the cached value is invalid, and needs to
	 * be obtained from the parent.
	 */
	mutable int index_;

	/**
	 * A list of children.
	 */
	QList<SelfT> childList_;
};

} // namespace Core

} // namespace KScope

#endif // __CORE_TREEITEM_H__
