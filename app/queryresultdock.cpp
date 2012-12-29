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

#include "queryresultdock.h"
#include "projectmanager.h"
#include "strings.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
QueryResultDock::QueryResultDock(QWidget* parent) :
	QDockWidget(tr("Query Results"), parent)
{
	setObjectName("QueryResultDock");
	setWidget(new StackWidget(this));
}

/**
 * Class destructor.
 */
QueryResultDock::~QueryResultDock()
{
}

/**
 * Runs a query and displays its results in a query view.
 * @param  query  The query to run
 */
void QueryResultDock::query(const Core::Query& query, bool tree)
{
	QueryView* view;

	if (tree) {
		QString title = tr("Call Tree: ") + Strings::toString(query);
		view = addView(title, Core::QueryView::Tree);
	}
	else {
		view = addView(Strings::toString(query), Core::QueryView::List);
	}

	// Run the query.
	try {
		view->query(query);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Stores the open query views in a session object.
 * @param  session The object to use for storing the views
 */
void QueryResultDock::saveSession(Session& session)
{
	QList<QWidget*> widgetList = tabWidget()->widgets();
	foreach (QWidget* widget, widgetList) {
		QueryView* view = static_cast<QueryView*>(widget);
		session.addQueryView(view);
	}
}

/**
 * Restores query views from a session object.
 * @param  session The session object to use
 */
void QueryResultDock::loadSession(Session& session)
{
	for (Session::QueryViewIterator itr = session.beginQueryIteration();
	     !itr.isAtEnd();
	     ++itr) {
		QueryView* view = addView(itr.title(), itr.type());
		itr.load(view);
		view->resizeColumns();
	}
}

/**
 * Selects the next location in the current view.
 */
void QueryResultDock::selectNextResult()
{
	QueryView* view	= static_cast<QueryView*>(tabWidget()->currentWidget());
	if (view != NULL)
		view->selectNext();
}

/**
 * Selects the next location in the current view.
 */
void QueryResultDock::selectPrevResult()
{
	QueryView* view	= static_cast<QueryView*>(tabWidget()->currentWidget());
	if (view != NULL)
		view->selectPrev();
}

/**
 * Closes all open query windows.
 */
void QueryResultDock::closeAll()
{
	tabWidget()->removeAll();
}

/**
 * Creates a new query view and adds it to the container widget.
 * @param  title  The title of the query view
 * @param  type   Whether to create a list or a tree view
 * @return The created widget
 */
QueryView* QueryResultDock::addView(const QString& title,
                                    Core::QueryView::Type type)
{
	// Create a new query view.
	QueryView* view = new QueryView(this, type);
	view->setWindowTitle(title);
	connect(view, SIGNAL(locationRequested(const Core::Location&)), this,
	        SIGNAL(locationRequested(const Core::Location&)));

	try {
		Core::LocationModel* model = view->locationModel();
		model->setRootPath(ProjectManager::project()->rootPath());
	}
	catch (Core::Exception* e) {
		delete e;
	}

	// TODO: Provide a configurable option to determine if a single result
	// should be selected automatically.
	if (type == Core::QueryView::List)
		view->setAutoSelectSingleResult(true);

	// Add to the tab widget.
	tabWidget()->addWidget(view);
	return view;
}

} // namespace App

} // namespace KScope
