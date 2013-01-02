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
#include <QMessageBox>
#include <core/exception.h>
#include "cscope.h"

namespace KScope
{

namespace Cscope
{

QString Cscope::execPath_("/usr/bin/cscope");

/**
 * Class constructor.
 * Creates the parser objects used for parsing Cscope output.
 */
Cscope::Cscope()
	: Process(),
	  conn_(NULL),
	  buildInitState_("BuildInit"),
	  buildProgState_("BuildProgress"),
	  queryProgState_("QueryProgress"),
	  queryResultState_("QueryResults")
{
	addRule(buildInitState_, Parser::Literal("Building cross-reference...\n"),
	        buildProgState_);
	addRule(buildProgState_, Parser::Literal("> Building symbol database ")
	                         << Parser::Number()
	                         << Parser::Literal(" of ")
	                         << Parser::Number()
	                         << Parser::Literal("\n"),
	        buildProgState_, ProgAction(*this, tr("Building database...")));
	addRule(queryProgState_, Parser::Literal("> Symbols matched ")
	                         << Parser::Number()
	                         << Parser::Literal(" of ")
	                         << Parser::Number()
                             << Parser::Literal("\n"),
	        queryProgState_, ProgAction(*this, tr("Querying...")));
	addRule(queryProgState_, Parser::Literal("> Possible references retrieved ")
	                         << Parser::Number()
	                         << Parser::Literal(" of ")
	                         << Parser::Number()
                             << Parser::Literal("\n"),
		    queryProgState_, ProgAction(*this, tr("Querying...")));
	addRule(queryProgState_, Parser::Literal("> Search ")
	                         << Parser::Number()
	                         << Parser::Literal(" of ")
	                         << Parser::Number()
                             << Parser::Literal("\n"),
	        queryProgState_, ProgAction(*this, tr("Querying...")));
	addRule(queryProgState_, Parser::Literal("cscope: ")
	                         << Parser::Number()
	                         << Parser::Literal(" lines\n"),
	        queryResultState_, QueryEndAction(*this));
	addRule(queryResultState_, Parser::String<>(' ')
	                           << Parser::Whitespace()
	                           << Parser::String<>(' ')
	  	                       << Parser::Whitespace()
	                           << Parser::Number()
	  	                       << Parser::Whitespace()
	                           << Parser::String<>('\n')
	                           << Parser::Literal("\n"),
	        queryResultState_, QueryResultAction(*this));
}

/**
 * Class destructor.
 */
Cscope::~Cscope()
{
}

/**
 * Transform integer flags to string
 */
QStringList Cscope::flags2Str(uint flags)
{
    QStringList args;
    //dummy arg, in case flags contains nothing
    args << "-";
	if (flags & Core::Query::IgnoreCase)
        args << "-C";
	return args;
}

/**
 * Starts a Cscope query process.
 * The process performs a one-time, line-oriented query on the database, without
 * rebuilding it.
 * @param  conn     A connection object used for reporting progress and data
 * @param  path     The directory to execute under
 * @param  type     The type of query to run
 * @param  pattern  The pattern to query
 * @throw  Exception
 */
void Cscope::query(Core::Engine::Connection* conn, const QString& path,
                   QueryArg extraArgs, const QString& pattern)
{
	// Abort if a process is already running.
	if (state() != QProcess::NotRunning || conn_ != NULL)
		throw Core::Exception("Process already running");

	// Prepare the argument list.
	QStringList args;
	args << "-d";
	args << "-v";
    args += flags2Str(extraArgs.flags);
	args << QString("-L%1").arg(extraArgs.type);
	args << pattern;
	setWorkingDirectory(path);

	// Initialise parsing.
	conn_ = conn;
	conn_->setCtrlObject(this);
	setState(queryProgState_);
	locList_.clear();
	type_ = extraArgs.type;

	// Start the process.
	qDebug() << "Running" << execPath_ << args << "in" << path;
	start(execPath_, args);
}

/**
 * Starts a Cscope build process.
 * @param  conn      A connection object used for reporting progress and data
 * @param  path      The directory to execute under
 * @param  buildArgs Command-line arguments for building the database
 * @throw  Exception
 */
void Cscope::build(Core::Engine::Connection* conn, const QString& path,
                   const QStringList& buildArgs)
{
	// Abort if a process is already running.
	if (state() != QProcess::NotRunning || conn_ != NULL)
		throw Core::Exception("Process already running");

	// TODO: Make the Cscope path configurable.
	QString prog = execPath_;

	// Prepare the argument list.
	QStringList args = buildArgs;
	args << "-b";
	args << "-v";
	setWorkingDirectory(path);

	// Initialise parsing.
	conn_ = conn;
	conn_->setCtrlObject(this);
	setState(buildInitState_);

	// Start the process.
	qDebug() << "Running cscope:" << args << "in" << path;
	start(prog, args);
}

/**
 * Called when the process terminates.
 * @param  code    The exit code of the process
 * @param  status  Used to indicate process crashes
 */
void Cscope::handleFinished(int code, QProcess::ExitStatus status)
{
	Process::handleFinished(code, status);

	// Hand over data to the other side of the connection.
	if (!locList_.isEmpty())
		conn_->onDataReady(locList_);

	// Signal normal termination.
	conn_->onFinished();

	// Detach from the connection object.
	conn_->setCtrlObject(NULL);
	conn_ = NULL;
}

} // namespace Cscope

} // namespace KScope
