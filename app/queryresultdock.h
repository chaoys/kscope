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

#ifndef __APP_QUERYRESULTDOCK_H__
#define __APP_QUERYRESULTDOCK_H__

#include <QDockWidget>
#include "stackwidget.h"
#include "queryview.h"
#include "session.h"

namespace KScope
{

namespace App
{

/**
 * A docked widget that holds query views.
 * @author Elad Lahav
 */
class QueryResultDock : public QDockWidget
{
	Q_OBJECT

public:
	QueryResultDock(QWidget* parent = 0);
	~QueryResultDock();

	void query(const Core::Query&, bool);
	void saveSession(Session&);
	void loadSession(Session&);

public slots:
	void selectNextResult();
	void selectPrevResult();
	void closeAll();

signals:
	/**
	 * Forwards a locationRequested() signal from any of the query views.
	 * @param  loc  The requested location
	 */
	void locationRequested(const Core::Location& loc);

private:
	inline StackWidget* tabWidget() {
		return static_cast<StackWidget*>(widget());
	}

	QueryView* addView(const QString&, Core::QueryView::Type);
};

} // namespace App

} // namespace KScope

#endif  // __APP_QUERYRESULTDOCK_H__
