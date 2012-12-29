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

#include "managedproject.h"

namespace KScope
{

namespace Cscope
{

/**
 * Class constructor.
 * @param  projPath The directory to use for this project
 */
ManagedProject::ManagedProject(const QString& projPath)
	: Core::Project<Crossref, Files>("project.conf", projPath)
{
}

/**
 * Class destructor.
 */
ManagedProject::~ManagedProject()
{
}

/**
 * Creates a new Cscope managed project.
 * @param  params Configuration parameters for the new project
 * @throw  Exception
 */
void ManagedProject::create(const Core::ProjectBase::Params& params)
{
	try {
		Core::Project<Crossref, Files>::create(params);
		Files().create(params.projPath_);
	}
	catch (Core::Exception* e) {
		throw e;
	}
}

/**
 * Modified configuration parameters for this project.
 * @param  params Updated configuration parameters.
 * @throw  Exception
 */
void ManagedProject::updateConfig(const Core::ProjectBase::Params& params)
{
	try {
		// Base class implementation.
		Core::Project<Crossref, Files>::updateConfig(params);

		// Apply changes to the engine.
		engine_.open(params.engineString_, NULL);
	}
	catch (Core::Exception* e) {
		throw e;
	}
}

} // namespace Cscope

} // namespace KScope
