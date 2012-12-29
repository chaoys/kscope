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

#ifndef __CORE_ENGINE_H
#define __CORE_ENGINE_H

#include <QObject>
#include <QWidget>
#include "globals.h"

namespace KScope
{

namespace Core
{

/**
 * Abstract base-class for cross-reference databases.
 * This class is the at the heart of KScope's implementation. Its derivations
 * define methods for retrieving information used to analyse the code base.
 * @author Elad Lahav
 */
class Engine : public QObject
{
	Q_OBJECT

public:
	/**
	 * Engine's database status.
	 */
	enum Status {
		/**
		 * Default status.
		 * Do not build or query.
		 */
		Unknown,

		/**
		 * Requires initial build.
		 * No queries can be issued.
		 */
		Build,

		/**
		 * Requires rebuilding.
		 * Queries can be issued in the meantime.
		 */
		Rebuild,

		/**
		 * Up-to-date.
		 * Ready for querying
		 */
		Ready
	};

	/**
	 * Class constructor.
	 * @param  parent  Parent object
	 * @return
	 */
	Engine(QObject* parent = NULL) : QObject(parent) {}

	/**
	 * Class destructor.
	 */
	virtual ~Engine() {}

	/**
	 * Makes the database available for querying.
	 * @param  initString  Implementation-specific string
	 */
	virtual void open(const QString& initString, Callback<>* cb) = 0;

	/**
	 * Determines the status of the database: whether a build is required and
	 * whether queries can be issued.
	 */
	virtual Status status() const = 0;

	/**
	 * Returns a list of Location structure fields that are filled by the given
	 * query type.
	 * @param  The requested query type
	 * @return A list of Location fields
	 */
	virtual QList<Location::Fields> queryFields(Query::Type type) const = 0;

	/**
	 * Abstract base class for a controllable object.
	 * This allows an engine operation to be stopped.
	 */
	struct Controlled
	{
		virtual void stop() = 0;
	};

	/**
	 * Represents a single thread of execution in a database.
	 * Engine operations are expected to execute asynchronously. Therefore,
	 * methods of the Engine class that invoke such operations (query and build)
	 * take an Engine::Connection object as a parameter. This object is used to
	 * convey information on the executing operation, including its progress and
	 * results.
	 * From the caller's end, this object can be used to stop an on-going
	 * operation.
	 * @author  Elad Lahav
	 */
	struct Connection
	{
		/**
		 * Struct constructor.
		 */
		Connection() : ctrlObject_(NULL) {}

		/**
		 * @param  ctrlObject  A controlled object that allows the operation to
		 *                     be stopped
		 */
		void setCtrlObject(Controlled* ctrlObject) { ctrlObject_ = ctrlObject; }

		/**
		 * Stops the current operation.
		 */
		virtual void stop() {
			if (ctrlObject_)
				ctrlObject_->stop();
		}

		/**
		 * Called when query data is produced by the engine.
		 * @param  locList  A location list, holding query results
		 */
		virtual void onDataReady(const Core::LocationList& locList) = 0;

		/**
		 * Called when an engine operation terminates successfully.
		 */
		virtual void onFinished() = 0;

		/**
		 * Called when an engine operation terminates abnormally.
		 */
		virtual void onAborted() = 0;

		/**
		 * Called when an engine operation makes progress.
		 * @param  text  A message describing the kind of progress made
		 * @param  cur   The current value
		 * @param  total The expected final value
		 */
		virtual void onProgress(const QString& text, uint cur, uint total) = 0;

	protected:
		/**
		 * An object which can be used to stop the current operation.
		 */
		Controlled* ctrlObject_;
	};

public slots:
	/**
	 * Starts a query.
	 * @param  conn    Used for communication with the ongoing operation
	 * @param  query   The query to execute
	 */
	virtual void query(Connection* conn, const Query& query) const = 0;

	/**
	 * (Re)builds the symbols database.
	 * @param  conn    Used for communication with the ongoing operation
	 */
	virtual void build(Connection*) const = 0;
};

/**
 * A template for configuring engines.
 * Specialise to provide custom-behaviour for specific engines.
 */
template<class EngineT>
struct EngineConfig
{
	static QString name() { return QString(); }

	static void getConfig(KeyValuePairs& confParams) {
		(void)confParams;
	}

	static void setConfig(const KeyValuePairs& confParams) {
		(void)confParams;
	}

	static QWidget* createConfigWidget(QWidget* parent) {
		(void)parent;
		return NULL;
	}

	static void configFromWidget(QWidget* widget) {
		(void)widget;
	}
};

} // namespace Core

} // namespace KScope

#endif // __CORE_ENGINE_H
