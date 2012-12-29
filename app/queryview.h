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

#ifndef __APP_QUERYVIEW_H__
#define __APP_QUERYVIEW_H__

#include <core/queryview.h>
#include <core/exception.h>
#include "projectmanager.h"

namespace KScope
{

namespace App
{

/**
 * A specialisation of Core::QueryView, which uses the current engine, as
 * supplied by the project manager.
 * @author Elad Lahav
 */
class QueryView : public Core::QueryView
{
	Q_OBJECT

public:
	/**
	 * Class constructor.
	 * @param  parent  Parent widget
	 * @param  type    Whether to create a list or a tree view
	 */
	QueryView(QWidget* parent, Type type = List)
		: Core::QueryView(parent, type) {}

	/**
	 * Class destructor.
	 */
	~QueryView() {}

protected:
	/**
	 * @return The engine supplied by the project manager
	 * @throw  Exception
	 */
	Core::Engine* engine() {
		Core::Engine* eng;

		try {
			eng = &ProjectManager::engine();
		}
		catch (Core::Exception* e) {
			throw e;
		}

		return eng;
	}
};

} // namespace App

} // namespace KScope

#endif // __APP_QUERYVIEW_H__
