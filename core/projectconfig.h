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

#ifndef __CORE_PROJECTCONFIG_H__
#define __CORE_PROJECTCONFIG_H__

#include "project.h"

namespace KScope
{

namespace Core
{

/**
 * Class template for creating project-specific configuration widgets.
 * A project dialogue can use this template to add a configuration page that
 * controls parameters in an implementation-dependent way.
 * The default implementation of the static functions does nothing. Use template
 * specialisation to create project-specific configuration widgets.
 * @author Elad Lahav
 */
template<class ProjectT>
struct ProjectConfig
{
	/**
	 * Creates a configuration widget for the project.
	 * @param  project  The project to use (NULL for a new project)
	 * @param  parent   A parent for the new widget
	 * @return The created widget (NULL by default)
	 */
	static QWidget* createConfigWidget(const ProjectT* project,
	                                   QWidget* parent) {
		(void)project;
		(void)parent;
		return NULL;
	}

	/**
	 * Fills a project parameters structure with data stored in a configuration
	 * widget.
	 * @param  widget  The configuration widget to use
	 * @param  params  The structure to fill
	 */
	static void paramsFromWidget(QWidget* widget, ProjectBase::Params& params) {
		(void)widget;
		(void)params;
	}
};

} // namespace Core

} // namespace KScope

#endif // __CORE_PROJECTCONFIG_H__
