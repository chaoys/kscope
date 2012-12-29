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

#ifndef __CSCOPE_CSCOPE_H__
#define __CSCOPE_CSCOPE_H__

#include <core/process.h>
#include <core/globals.h>
#include <core/engine.h>

namespace KScope
{

namespace Cscope
{

/**
 * Front-end to a Cscope process.
 * This object can be used for both querying and building the Cscope
 * cross-reference file.
 * @author Elad Lahav
 */
class Cscope : public Core::Process, public Core::Engine::Controlled
{
public:
	Cscope();
	~Cscope();

	/**
	 * Cscope query types.
	 * These are enumerated by the numeric value assigned to each query type
	 * in Cscope's command-line interface (i.e., the parameter to the -L
	 * option).
	 */
	enum QueryType {
		References = 0,
		Definition = 1,
		CalledFunctions = 2,
		CallingFunctions = 3,
		Text = 4,
		EGrepPattern = 6,
		FindFile = 7,
		IncludingFiles = 8
	};

	void query(Core::Engine::Connection*, const QString&, QueryType,
	           const QString&);
	void build(Core::Engine::Connection*, const QString&, const QStringList&);

	/**
	 * Stops a query/build process.
	 */
	virtual void stop() { kill(); }

	static QString execPath_;

protected slots:
	virtual void handleFinished(int, QProcess::ExitStatus);

private:
	/**
	 * The current connection object, used to communicate progress and result
	 * information.
	 */
	Core::Engine::Connection* conn_;

	/**
	 * Total number of result lines.
	 */
	uint resNum_;

	/**
	 * Number of parsed result lines (used to provide parsing progress
	 * information).
	 */
	uint resParsed_;

	/**
	 * Initial state for when building the database.
	 */
	State buildInitState_;

	/**
	 * Build progress state.
	 */
	State buildProgState_;

	/**
	 * Query progress state.
	 */
	State queryProgState_;

	/**
	 * Query results state.
	 */
	State queryResultState_;

	/**
	 * List of locations.
	 * The list is constructed when result lines are parsed.
	 */
	Core::LocationList locList_;

	/**
	 * The type of the current query.
	 */
	QueryType type_;

	/**
	 * Functor for progress-states transition-functions.
	 */
	struct ProgAction
	{
		/**
		 * Struct constructor.
		 * @param  self  The owner Cscope object
		 * @param  text  Used to build progress messages
		 */
		ProgAction(Cscope& self, const QString& text) : self_(self),
			text_(text) {}

		/**
		 * Functor operator.
		 * Provides a call-back into the connection's onProgress() method.
		 * @param  capList  List of captured strings
		 */
		void operator()(const Parser::CapList& capList) const {
			self_.conn_->onProgress(text_, capList[0].toUInt(),
			                        capList[1].toUInt());
		}

		/**
		 * The owner Cscope object.
		 */
		Cscope& self_;

		/**
		 * Used to build progress messages
		 */
		QString text_;
	};

	/**
	 * Functor for the end-of-query-state transition-function.
	 */
	struct QueryEndAction
	{
		/**
		 * Struct constructor.
		 * @param  self  The owner Cscope object
		 */
		QueryEndAction(Cscope& self) : self_(self) {}

		/**
		 * Functor operator.
		 * Provides a call-back into the connection's onProgress() method.
		 * @param  capList  List of captured strings
		 */
		void operator()(const Parser::CapList& capList) const {
			self_.resNum_ = capList[0].toUInt();
			self_.resParsed_ = 0;
			self_.conn_->onProgress(tr("Parsing..."), 0, self_.resNum_);
		}

		/**
		 * The owner Cscope object.
		 */
		Cscope& self_;
	};

	/**
	 * Functor for the query-result-state transition-function.
	 */
	struct QueryResultAction
	{
		/**
		 * Struct constructor.
		 * @param  self  The owner Cscope object
		 */
		QueryResultAction(Cscope& self) : self_(self) {}

		/**
		 * Functor operator.
		 * Parses result lines.
		 * @param  capList  List of captured strings
		 */
		void operator()(const Parser::CapList& capList) const {
			// Fill-in a Location object, using the parsed result information.
			Core::Location loc;
			loc.file_ = capList[0].toString();
			loc.line_ = capList[2].toUInt();
			loc.column_ = 0;
			loc.text_ = capList[3].toString();
			loc.tag_.type_ = Core::Tag::UnknownTag;

			// Cscope's "Scope" result field should be handled differently
			// for each query type.
			switch (self_.type_) {
			case Cscope::References:
			case Cscope::CalledFunctions:
			case Cscope::CallingFunctions:
				loc.tag_.scope_ = capList[1].toString();
				break;

			case Cscope::Definition:
				loc.tag_.name_ = capList[1].toString();
				break;

			default:
				;
			}

			// Add to the list of parsed locations.
			self_.locList_.append(loc);
			self_.resParsed_++;

			// Provide progress information for result-parsing.
			if ((self_.resParsed_ & 0xff) == 0) {
				self_.conn_->onProgress(tr("Parsing..."), self_.resParsed_,
				                        self_.resNum_);
			}
		}

		/**
		 * The owner Cscope object.
		 */
		Cscope& self_;
	};
};

} // namespace Cscope

} // namespace KScope

#endif // __CSCOPE_CSCOPE_H__
