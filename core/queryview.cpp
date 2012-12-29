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

#include "queryview.h"
#include "exception.h"
#include "engine.h"
#include "progressbar.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  parent  The parent widget
 * @param  type    Whether the view works in list or tree modes
 */
QueryView::QueryView(QWidget* parent, Type type)
	: LocationView(parent, type), progBar_(NULL),
	  autoSelectSingleResult_(false)
{
	// Query child items when expanded (in a tree view).
	if (type_ == Tree) {
		connect(this, SIGNAL(expanded(const QModelIndex&)), this,
		        SLOT(queryTreeItem(const QModelIndex&)));
	}

	menu_->addAction(tr("&Rerun Query"), this, SLOT(requery()));
}

/**
 * Class destructor.
 */
QueryView::~QueryView()
{
}

/**
 * @param  query  The query to run
 */
void QueryView::query(const Query& query)
{
	// Delete the model data.
	locationModel()->clear(QModelIndex());

	try {
		// Get an engine for running the query.
		Engine* eng;
		if ((eng = engine()) != NULL) {
			// Run the query.
			query_ = query;
			locationModel()->setColumns(eng->queryFields(query_.type_));
			eng->query(this, query_);
		}
	}
	catch (Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Creates an XML representation of the view, which can be used for storing the
 * model's data in a file.
 * Adds query information to the representation created by
 * LocationView::toXML().
 * @param  doc      The XML document object to use
 * @param  viewElem The element representing the view
 */
void QueryView::toXML(QDomDocument& doc, QDomElement& viewElem) const
{
	// Store query information.
	QDomElement queryElem = doc.createElement("Query");
	queryElem.setAttribute("type", QString::number(query_.type_));
	queryElem.setAttribute("flags", QString::number(query_.flags_));
	queryElem.appendChild(doc.createCDATASection(query_.pattern_));
	viewElem.appendChild(queryElem);

	LocationView::toXML(doc, viewElem);
}

/**
 * Loads a query view from an XML representation.
 * @param  root The root element for the query's XML representation
 */
void QueryView::fromXML(const QDomElement& viewElem)
{
	// Get query information.
	QDomElement queryElem
		= viewElem.elementsByTagName("Query").at(0).toElement();
	if (queryElem.isNull())
		return;

	query_.type_ = static_cast<Core::Query::Type>
	               (queryElem.attribute("type").toUInt());
	query_.flags_ = queryElem.attribute("flags").toUInt();
	query_.pattern_ = queryElem.childNodes().at(0).toCDATASection().data();

	LocationView::fromXML(viewElem);
}

/**
 * Called by the engine when results are available.
 * Adds the list of locations to the model.
 * @param  locList  Query results
 */
void QueryView::onDataReady(const LocationList& locList)
{
	locationModel()->add(locList, queryIndex_);
}

/**
 * Displays progress information in a progress-bar at the top of the view.
 * @param  text  Progress message
 * @param  cur   Current value
 * @param  total Expected final value
 */
void QueryView::onProgress(const QString& text, uint cur, uint total)
{
	// Create the progress-bar widget, if it does not exist.
	if (!progBar_) {
		progBar_ = new ProgressBar(this);
		connect(progBar_, SIGNAL(cancelled()), this, SLOT(stopQuery()));
		progBar_->show();
	}

	// Update progress information in the progress bar.
	progBar_->setLabel(text);
	progBar_->setProgress(cur, total);

	if (!isVisible())
		emit needToShow();
}

/**
 * Called by the engine when a query terminates normally.
 */
void QueryView::onFinished()
{
	// Handle an empty result set.
	if (locationModel()->rowCount(queryIndex_) == 0)
		locationModel()->add(LocationList(), queryIndex_);

	// Destroy the progress-bar, if it exists.
	if (progBar_) {
		delete progBar_;
		progBar_ = NULL;
	}

	// Adjust column sizes.
	resizeColumns();

	// Auto-select a single result, if required.
	Location loc;
	if (autoSelectSingleResult_ && locationModel()->rowCount(queryIndex_) == 1
	                            && locationModel()->firstLocation(loc)) {
		emit locationRequested(loc);
	}
	else {
		setCurrentIndex(QModelIndex());
		if (!isVisible())
			emit needToShow();
	}
}

/**
 * Called by the engine when a query terminates abnormally.
 */
void QueryView::onAborted()
{
	// Destroy the progress-bar, if it exists.
	if (progBar_) {
		delete progBar_;
		progBar_ = NULL;
	}
}

/**
 * Called when the "Cancel" button is clicked in the progress-bar.
 * Informs the engine that the query process should be stopped.
 */
void QueryView::stopQuery()
{
	stop();
}

/**
 * Called when a tree item is expanded.
 * If this item was not queried before, a query is performed.
 * @param  index  The expanded item (proxy index)
 */
void QueryView::queryTreeItem(const QModelIndex& index)
{
	// Query previously-non-queried items only.
	QModelIndex srcIndex = proxy()->mapToSource(index);
	if (locationModel()->isEmpty(srcIndex) != LocationModel::Unknown)
		return;

	// Get the location information from the index.
	Location loc;
	if (!locationModel()->locationFromIndex(srcIndex, loc))
		return;

	// Run a query on this location.
	try {
		Engine* eng;
		if ((eng = engine()) != NULL) {
			queryIndex_ = srcIndex;
			eng->query(this, Query(query_.type_, loc.tag_.scope_));
		}
	}
	catch (Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Runs the current query again.
 */
void QueryView::requery()
{
	// Do nothing if there is no valid query associated with this view.
	if (query_.type_ == Query::Invalid)
		return;

	// For a list view, just re-execute the query.
	if (type_ == List) {
		query(query_);
		return;
	}

	// Tree view: rerun the current branch only.
	QModelIndex srcIndex = proxy()->mapToSource(menuIndex_);
	locationModel()->clear(srcIndex);
	queryTreeItem(menuIndex_);
}

} // namespace Core

} // namespace KScope
