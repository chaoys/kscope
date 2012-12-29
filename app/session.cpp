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
#include <core/exception.h>
#include "session.h"
#include "projectmanager.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  path  The path of the configuration directory
 */
Session::Session(const QString& path)
	: path_(path), queryViewDoc_("QueryViews")
{
}

/**
 * Class destructor.
 */
Session::~Session()
{
}

/**
 * Reads session information from the configuration file.
 */
void Session::load()
{
	QSettings settings(configFile(), QSettings::IniFormat);

	// Get a list of files being edited, along with the location of the
	// cursor on each one.
	int size = settings.beginReadArray("Editors");
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);

		Core::Location loc;
		loc.file_ = settings.value("Path").toString();
		loc.line_ = settings.value("Line").toUInt();
		loc.column_ = settings.value("Column").toUInt();

		editorList_.append(loc);
	}
	settings.endArray();

	// Get the path of the active editor.
	activeEditor_ = settings.value("ActiveEditor").toString();
	maxActiveEditor_ = settings.value("MaxActiveEditor", false).toBool();

	// Load the query XML file.
	QFile xmlFile(queryViewFile());
	if (xmlFile.open(QIODevice::ReadOnly))
		queryViewDoc_.setContent(xmlFile.readAll());
}

/**
 * Writes session information to the configuration file.
 */
void Session::save()
{
	QSettings settings(configFile(), QSettings::IniFormat);

	// Store a list of files being edited, along with the location of the
	// cursor on each one.
	settings.beginWriteArray("Editors");
	for (int i = 0; i < editorList_.size(); i++) {
		settings.setArrayIndex(i);

		const Core::Location& loc = editorList_.at(i);
		settings.setValue("Path", loc.file_);
		settings.setValue("Line", loc.line_);
		settings.setValue("Column", loc.column_);
	}
	settings.endArray();

	// Store other information on the editor container.
	settings.setValue("ActiveEditor", activeEditor_);
	settings.setValue("MaxActiveEditor", maxActiveEditor_);

	// Save the query view XML document.
	QFile xmlFile(queryViewFile());
	if (xmlFile.open(QIODevice::WriteOnly))
		xmlFile.write(queryViewDoc_.toByteArray());
}

/**
 * Creates an XML representation of a query view.
 * The representation is rooted at a "QueryView" element, which holds a
 * "Query" element for query information, and "Location" elements for query
 * results.
 * @param view
 */
void Session::addQueryView(const QueryView* view)
{
	// Get the root element.
	// Create one if it does not exists.
	QDomElement root = queryViewDoc_.documentElement();
	if (root.isNull()) {
		root = 	queryViewDoc_.createElement("Queries");
		queryViewDoc_.appendChild(root);
	}

	// Put location information under the element.
	QDomElement viewElem = queryViewDoc_.createElement("QueryView");
	root.appendChild(viewElem);
	view->toXML(queryViewDoc_, viewElem);
}

/**
 * Creates an iterator that is used for loading query views from an XML
 * representation.
 * Once an iterator has been created, a view object can use its load() method
 * to get the query information and locations.
 * @return
 */
Session::QueryViewIterator Session::beginQueryIteration() const
{
	QueryViewIterator itr;

	QDomElement root = queryViewDoc_.documentElement();
	itr.queryNodeList_ = root.elementsByTagName("QueryView");
	itr.listPos_ = -1;
	++itr;

	return itr;
}

void Session::QueryViewIterator::load(QueryView* view)
{
	// Ensure the iterator's XML element is valid.
	if (!elem_.isNull())
		view->fromXML(elem_);
}

} // namespace App

} // namespace KScope
