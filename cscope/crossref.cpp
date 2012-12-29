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

#include <QDir>
#include <QFileInfo>
#include <core/exception.h>
#include "crossref.h"
#include "ctags.h"

namespace KScope
{

namespace Cscope
{

/**
 * Class constructor.
 * @param  parent  Parent object
 */
Crossref::Crossref(QObject* parent) : Core::Engine(parent), status_(Unknown)
{
}

/**
 * Class destructor.
 */
Crossref::~Crossref()
{
}

/**
 * Opens a cscope cross-reference database.
 * The method expects to find a 'cscope.out' file under the given path.
 * The path is stored and used for all database operations.
 * The initialisation string should be colon-delimited, where the first section
 * is the project path (includes the cscope.out and cscope.files files),
 * followed by command-line arguments to Cscope (only the ones that apply to
 * building the database).
 * @param  initString  The initialisation string
 * @throw  Exception
 */
void Crossref::open(const QString& initString, Core::Callback<>* cb)
{
	// Parse the initialisation string.
	QStringList args = initString.split("*", QString::SkipEmptyParts);
	QString path = args.takeFirst();

	qDebug() << __func__ << initString << path;

	// Make sure the path exists.
	QDir dir(path);
	if (!dir.exists())
		throw new Core::Exception("Database directory does not exist");

	// Check if the cross-reference file exists.
	// If not, the databsae needs to be built. Otherwise, it is ready for
	// querying, but needs to be rebuilt.
	// We also ensure that if it exists it is readable.
	QFileInfo fi(dir, "cscope.out");
	Status status;
	if (!fi.exists())
		status = Build;
	else if (!fi.isReadable())
		throw new Core::Exception("Cannot read the 'cscope.out' file");
	else
		status = Ready;

	// Handle reopening with different parameters (i.e., after a change to the
	// project parameters).
	if (status_ != Unknown) {
		if ((path != path_) || args != args_)
			status = Rebuild;
	}

	// Store arguments for running Cscope.
	path_ = path;
	args_ = args;
	status_ = status;

	if (cb)
		cb->call();
}

/**
 * Builds a list of fields for each query type.
 * The list specifies the fields that carry useful information for the given
 * type of query.
 * @param  type  Query type
 * @return A list of Location structure fields
 */
QList<Core::Location::Fields>
Crossref::queryFields(Core::Query::Type type) const
{
	QList<Core::Location::Fields> fieldList;

	switch (type) {
	case Core::Query::FindFile:
		fieldList << Core::Location::File;
		break;

	case Core::Query::Text:
	case Core::Query::IncludingFiles:
		fieldList << Core::Location::File
		          << Core::Location::Line
		          << Core::Location::Text;
		break;

	case Core::Query::Definition:
		fieldList << Core::Location::TagName
		          << Core::Location::File
		          << Core::Location::Line
		          << Core::Location::Text;
		break;

	case Core::Query::References:
	case Core::Query::CalledFunctions:
	case Core::Query::CallingFunctions:
		fieldList << Core::Location::Scope
		          << Core::Location::File
		          << Core::Location::Line
		          << Core::Location::Text;
		break;

	case Core::Query::LocalTags:
		fieldList << Core::Location::TagName
		          << Core::Location::Scope
		          << Core::Location::Line
		          << Core::Location::TagType;
		break;

	default:
		;
	}

	return fieldList;
}

/**
 * Starts a Cscope query.
 * Creates a new Cscope process to handle the query.
 * @param  conn  Connection object to attach to the new process
 * @param  query Query information
 * @throw  Exception
 */
void Crossref::query(Core::Engine::Connection* conn,
                     const Core::Query& query) const
{
	Cscope::QueryType type;

	// Translate the requested type into a Cscope query number.
	switch (query.type_) {
	case Core::Query::Text:
		if (query.flags_ & Core::Query::RegExp)
			type = Cscope::EGrepPattern;
		else
			type = Cscope::Text;
		break;

	case Core::Query::References:
		type = Cscope::References;
		break;

	case Core::Query::Definition:
		type = Cscope::Definition;
		break;

	case Core::Query::CalledFunctions:
		type = Cscope::CalledFunctions;
		break;

	case Core::Query::CallingFunctions:
		type = Cscope::CallingFunctions;
		break;

	case Core::Query::FindFile:
		type = Cscope::FindFile;
		break;

	case Core::Query::IncludingFiles:
		type = Cscope::IncludingFiles;
		break;

	case Core::Query::LocalTags:
		{
			Ctags* ctags = new Ctags();
			ctags->setDeleteOnExit();
			ctags->query(conn, query.pattern_);
			return;
		}

	default:
		// Query type is not supported.
		// TODO: What happens if an exception is thrown from within a slot?
		throw new Core::Exception(QString("Unsupported query type '%1")
		                          .arg(query.type_));
	}

	// Create a new Cscope process object, and start the query.
	Cscope* cscope = new Cscope();
	cscope->setDeleteOnExit();
	cscope->query(conn, path_, type, query.pattern_);
}

/**
 * Starts a Cscope build process.
 * @param  conn  Connection object to attach to the new process
 */
void Crossref::build(Core::Engine::Connection* conn) const
{
	// Create the Cscope process object.
	Cscope* cscope = new Cscope();
	cscope->setDeleteOnExit();

	// Need to update the status upon successful termination.
	connect(cscope, SIGNAL(finished(int, QProcess::ExitStatus)), this,
	        SLOT(buildProcessFinished(int, QProcess::ExitStatus)));

	// Start the build process.
	cscope->build(conn, path_, args_);
}

void Crossref::buildProcessFinished(int code, QProcess::ExitStatus status)
{
	if ((code == 0) && (status == QProcess::NormalExit))
		status_ = Ready;
}

} // namespace Cscope

} // namespace KScope
