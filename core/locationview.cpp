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

#include <QDebug>
#include "locationview.h"
#include "locationlistmodel.h"
#include "locationtreemodel.h"
#include "textfilterdialog.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  parent  The parent widget
 * @param  type    Whether the view works in list or tree modes
 */
LocationView::LocationView(QWidget* parent, Type type)
	: QTreeView(parent), type_(type)
{
	// Set tree view properties.
	setRootIsDecorated(type_ == Tree);
	setUniformRowHeights(true);
	setExpandsOnDoubleClick(false);

	// Create the model proxy.
	LocationViewProxyModel* proxy = new LocationViewProxyModel(this);
	setModel(proxy);

	// Create a location model.
	switch (type_) {
	case List:
		proxy->setSourceModel(new LocationListModel(this));
		break;

	case Tree:
		proxy->setSourceModel(new LocationTreeModel(this));
		break;
	}

	// Emit requests for locations when an item is double-clicked.
	connect(this, SIGNAL(activated(const QModelIndex&)), this,
	        SLOT(requestLocation(const QModelIndex&)));

	// Create the context menu.
	menu_ = new QMenu(this);
	menu_->addAction(tr("&Filter..."), this, SLOT(promptFilter()));
	menu_->addAction(tr("C&lear filter"), this, SLOT(clearFilter()));
}

/**
 * Class destructor.
 */
LocationView::~LocationView()
{
}

/**
 * Adjusts all columns to fit the their contents.
 */
void LocationView::resizeColumns()
{
	for (int i = 0; i < locationModel()->columnCount(); i++)
		resizeColumnToContents(i);
}

/**
 * Creates an XML representation of the view, which can be used for storing the
 * model's data in a file.
 * @param  doc      The XML document object to use
 * @param  viewElem The element representing the view
 */
void LocationView::toXML(QDomDocument& doc, QDomElement& viewElem) const
{
	// Create an element for storing the view.
	viewElem.setAttribute("name", windowTitle());
	viewElem.setAttribute("type", QString::number(type_));

	// Create a "Columns" element.
	QDomElement colsElem = doc.createElement("Columns");
	viewElem.appendChild(colsElem);

	// Add an element for each column.
	foreach (Location::Fields field, locationModel()->columns()) {
		QDomElement colElem = doc.createElement("Column");
		colElem.setAttribute("field", QString::number(field));
		colsElem.appendChild(colElem);
	}

	// Add locations.
	locationToXML(doc, viewElem, QModelIndex());
}

/**
 * Loads a query view from an XML representation.
 * @param  root The root element for the query's XML representation
 */
void LocationView::fromXML(const QDomElement& viewElem)
{
	// Reset the model.
	locationModel()->clear(QModelIndex());

	// TODO: Is there a guarantee of order?
	QDomNodeList columnNodes = viewElem.elementsByTagName("Column");
	QList<Location::Fields> colList;
	for (int i = 0; i < columnNodes.size(); i++) {
		QDomElement elem = columnNodes.at(i).toElement();
		if (elem.isNull())
			continue;

		colList.append(static_cast<Location::Fields>
		               (elem.attribute("field").toUInt()));
	}
	locationModel()->setColumns(colList);

	// Find the <LocationList> element that is a child of the root element.
	QDomNodeList childNodes = viewElem.childNodes();
	for (int i = 0; i < childNodes.size(); i++) {
		QDomElement elem = childNodes.at(i).toElement();
		if (elem.isNull() || elem.tagName() != "LocationList")
			continue;

		// Load locations.
		locationFromXML(elem, QModelIndex());
	}

#ifndef QT_NO_DEBUG
	locationModel()->verify();
#endif
}

/**
 * Selects the next available index in the proxy.
 */
void LocationView::selectNext()
{
	QModelIndex index = moveCursor(MoveNext, 0);
	if (!index.isValid())
		return;

	setCurrentIndex(index);

	Location loc;
	if (locationModel()->locationFromIndex(proxy()->mapToSource(index), loc))
		emit locationRequested(loc);
}

/**
 * Selects the previous available index in the proxy.
 */
void LocationView::selectPrev()
{
	QModelIndex index = moveCursor(MovePrevious, 0);
	if (!index.isValid())
		return;

	setCurrentIndex(index);

	Location loc;
	if (locationModel()->locationFromIndex(proxy()->mapToSource(index), loc))
		emit locationRequested(loc);
}

/**
 * Displays the context menu in response to the matching event.
 * @param  event Event parameters
 */
void LocationView::contextMenuEvent(QContextMenuEvent* event)
{
	menu_->popup(event->globalPos());
	menuIndex_ = indexAt(event->pos());
	event->accept();
}

/**
 * Recursively transforms the location hierarchy stored in the model to an XML
 * sub-tree.
 * The XML representation of locations is composed of a <LocationList> element
 * holding a list of <Location> elements. For a tree model, <Location> elements
 * may in turn hold a <LocationList> sub-element, and so on.
 * @param  doc        The XML document object to use
 * @param  parentElem XML element under which new location elements should be
 *                    created
 * @param  index      The source index to store (along with its children)
 */
void LocationView::locationToXML(QDomDocument& doc, QDomElement& parentElem,
                                 const QModelIndex& index) const
{
	QDomElement elem;

	if (index.isValid()) {
		// A non-root index.
		// Translate the index into a location information structure.
		Location loc;
		if (!locationModel()->locationFromIndex(index, loc))
			return;

		// Create an XML element for the location.
		elem = doc.createElement("Location");
		parentElem.appendChild(elem);

		// Add a text node for each structure member.
		const QList<Location::Fields>& colList = locationModel()->columns();
		foreach (Location::Fields field, colList) {
			QString name;
			QDomNode node;

			switch (field) {
			case Location::File:
				name = "File";
				node = doc.createTextNode(loc.file_);
				break;

			case Location::Line:
				name = "Line";
				node = doc.createTextNode(QString::number(loc.line_));
				break;

			case Location::Column:
				name = "Column";
				node = doc.createTextNode(QString::number(loc.column_));
				break;

			case Location::TagName:
				name = "TagName";
				node = doc.createTextNode(loc.tag_.name_);
				break;

			case Location::TagType:
				name = "TagType";
				node = doc.createTextNode(QString::number(loc.tag_.type_));
				break;

			case Location::Scope:
				name = "Scope";
				node = doc.createTextNode(loc.tag_.scope_);
				break;

			case Location::Text:
				name = "Text";
				node = doc.createCDATASection(loc.text_);
				break;
			}

			QDomElement child = doc.createElement(name);
			child.appendChild(node);
			elem.appendChild(child);
		}
	}
	else {
		// For the root index, use the given parent element as the parent of
		// the location list created by top-level items.
		elem = parentElem;
	}

	// Create an element list using the index's children.
	// A <LocationList> element is created for queried items (whether there
	// were any results or not). An element is not created for non-queried
	// items, so that locationFromXML() does not call add() for such items,
	// correctly restoring their status.
	if (locationModel()->isEmpty(index) != LocationModel::Unknown) {
		// Create the element.
		QDomElement locListElem = doc.createElement("LocationList");
		QModelIndex proxyIndex = proxy()->mapFromSource(index);
		locListElem.setAttribute("expanded",
		                         isExpanded(proxyIndex) ? "1" : "0");
		elem.appendChild(locListElem);

		// Add child locations.
		for (int i = 0; i < locationModel()->rowCount(index); i++)
			locationToXML(doc, locListElem, locationModel()->index(i, 0, index));
	}
}

/**
 * Loads a hierarchy of locations from an XML document into the model.
 * See locationToXML() for the XML format.
 * @param  locListElem A <LocationList> XML element
 * @param  parentIndex The source model index under which locations should be
 *                     added
 */
void LocationView::locationFromXML(const QDomElement& locListElem,
                                   const QModelIndex& parentIndex)
{
	// Get a list of location elements.
	QDomNodeList nodes = locListElem.childNodes();

	// Translate elements into a list of location objects.
	// The list is used to store sub-lists encountered inside the location
	// element. These will be loaded later. It's better to first construct the
	// list of locations for the current level, rather than follow the XML
	// tree depth-first, due to the behaviour of the add() method.
	LocationList locList;
	QList< QPair<int, QDomElement> > childLists;
	for (int i = 0; i < nodes.size(); i++) {
		// Get the current location element.
		QDomElement elem = nodes.at(i).toElement();
		if (elem.isNull() || elem.tagName() != "Location")
			continue;

		// Iterate over the sub-elements, which represent either location
		// properties, or nested location lists. We expect at most one of the
		// latter.
		Location loc;
		QDomNodeList childNodes = elem.childNodes();
		for (int j = 0; j < childNodes.size(); j++) {
			// Has to be an element.
			QDomElement child = childNodes.at(j).toElement();
			if (child.isNull())
				continue;

			// Extract location data from the element.
			if (child.tagName() == "File")
				loc.file_ = child.text();
			else if (child.tagName() == "Line")
				loc.line_ = child.text().toUInt();
			else if (child.tagName() == "Column")
				loc.column_ = child.text().toUInt();
			else if (child.tagName() == "TagName")
				loc.tag_.name_ = child.text();
			else if (child.tagName() == "TagType")
				loc.tag_.type_ = static_cast<Tag::Type>(child.text().toUInt());
			else if (child.tagName() == "Scope")
				loc.tag_.scope_ = child.text();
			else if (child.tagName() == "Text")
				loc.text_ = child.firstChild().toCDATASection().data();
			else if (child.tagName() == "LocationList")
				childLists.append(QPair<int, QDomElement>(i, child));
		}

		// Add to the location list.
		locList.append(loc);
	}

	// Store locations in the model.
	locationModel()->add(locList, parentIndex);

	// Load any sub-lists encountered earlier.
	QList< QPair<int, QDomElement> >::Iterator itr;
	for (itr = childLists.begin(); itr != childLists.end(); ++itr) {
		locationFromXML((*itr).second,
		                locationModel()->index((*itr).first, 0, parentIndex));
	}

	// Expand the item if required.
	if (locListElem.attribute("expanded").toUInt())
		expand(proxy()->mapFromSource(parentIndex));
}

/**
 * Called when the user double-clicks a location item in the list.
 * Emits the locationRequested() signal for this location.
 * @param  index  The clicked item (proxy index)
 */
void LocationView::requestLocation(const QModelIndex& index)
{
	Location loc;
	if (locationModel()->locationFromIndex(proxy()->mapToSource(index), loc))
		emit locationRequested(loc);
}

/**
 * Displays the text filter dialogue.
 * If a filter is specified by the user, it is applied to the proxy model.
 */
void LocationView::promptFilter()
{
	// Create the dialogue.
	TextFilterDialog dlg(proxy()->filterRegExp());

	// Populate the "Filter By" list.
	KeyValuePairs pairs;
	for (int i = 0; i < locationModel()->columnCount(); i++) {
		QString colName = locationModel()->headerData(i, Qt::Horizontal,
		                                              Qt::DisplayRole)
		                                             .toString();
		pairs[colName] = i;
	}
	dlg.setFilterByList(pairs);
	dlg.setFilterByValue(menuIndex_.column());

	// Show the dialogue.
	if (dlg.exec() != QDialog::Accepted)
		return;

	// Apply the filter.
	proxy()->setFilterKeyColumn(dlg.filterByValue().toInt());
	QRegExp filter = dlg.filter();
	proxy()->setFilterRegExp(filter);
	emit isFiltered(filter.isEmpty());
}

/**
 * Removes any filters from the proxy.
 */
void LocationView::clearFilter()
{
	proxy()->setFilterRegExp(QRegExp());
	emit isFiltered(false);
}

} // namespace Core

} // namespace KScope
