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

#ifndef __CORE_LOCATIONVIEW_H__
#define __CORE_LOCATIONVIEW_H__

#include <QTreeView>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QDomDocument>
#include <QDomElement>
#include <QContextMenuEvent>
#include "globals.h"
#include "locationmodel.h"

namespace KScope
{

namespace Core
{

/**
 * A proxy model used by LocationView.
 * @author Elad Lahav
 */
class LocationViewProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	/**
	 * Class constructor.
	 * @param  parent Parent object
	 */
	LocationViewProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

	/**
	 * Class destructor.
	 */
	~LocationViewProxyModel() {}

	/**
	 * Determines if the given index has children.
	 * Used to display an expansion button beside items which either have
	 * children, or for which the state is still unknown (for on-demand
	 * population of the model).
	 * @param  parent The parent index
	 * @return true to display an expansion button, false otherwise
	 */
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const {
		LocationModel* model = static_cast<LocationModel*>(sourceModel());
		if (model->isEmpty(mapToSource(parent)) == LocationModel::Unknown)
			return true;

		return QSortFilterProxyModel::hasChildren(parent);
	}
};

/**
 * A view for displaying LocationModel models.
 * Adds filtering and save/restore capabilities to a standard QTreeView.
 * @author Elad Lahav
 */
class LocationView : public QTreeView
{
	Q_OBJECT

public:
	/**
	 * The view can work in either list or tree modes.
	 */
	enum Type { List, Tree };

	LocationView(QWidget*, Type type = List);
	~LocationView();

	void resizeColumns();
	virtual void toXML(QDomDocument&, QDomElement&) const;
	virtual void fromXML(const QDomElement&);

	/**
	 * @return  The type of the view
	 */
	Type type() const { return type_; }

	/**
	 * @return The proxy model
	 */
	inline LocationViewProxyModel* proxy() {
		return static_cast<LocationViewProxyModel*>(model());
	}

	/**
	 * @return The proxy model
	 */
	inline const LocationViewProxyModel* proxy() const {
		return static_cast<LocationViewProxyModel*>(model());
	}

	/**
	 * @return The location model for this view
	 */
	inline LocationModel* locationModel() {
		return static_cast<LocationModel*>(proxy()->sourceModel());
	}

	/**
	 * @return The location model for this view
	 */
	inline const LocationModel* locationModel() const {
		return static_cast<LocationModel*>(proxy()->sourceModel());
	}

public slots:
	void selectNext();
	void selectPrev();

signals:
	/**
	 * Emitted when a location item is selected.
	 * @param  loc  The location descriptor
	 */
	void locationRequested(const Core::Location& loc);

	/**
	 * Emitted when the view needs to be visible.
	 * This is useful for creating containers that only become visible when
	 * there is something to show in the view (either progress or results).
	 */
	void needToShow();

	/**
	 * Emitted when the filter on the proxy model is changed.
	 * @param  filtered true if a filter is currently applied, false otherwise
	 */
	void isFiltered(bool filtered);

protected:
	/**
	 * Whether the view is in list or tree modes.
	 */
	Type type_;

	/**
	 * A context menu for the view widget.
	 */
	QMenu* menu_;

	/**
	 * The proxy index for which the context menu was displayed.
	 */
	QModelIndex menuIndex_;

	virtual void contextMenuEvent(QContextMenuEvent*);
	virtual void locationToXML(QDomDocument&, QDomElement&,
	                           const QModelIndex&) const;
	virtual void locationFromXML(const QDomElement&, const QModelIndex&);

protected slots:
	void requestLocation(const QModelIndex&);
	void promptFilter();
	void clearFilter();
};

} // namespace Core

} // namespace KScope

#endif // __CORE_LOCATIONVIEW_H__
