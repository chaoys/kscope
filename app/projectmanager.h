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

#ifndef __APP_PROJECTMANAGER_H__
#define __APP_PROJECTMANAGER_H__

#include <QObject>
#include <core/project.h>
#include "application.h"

namespace KScope
{

namespace App
{

/**
 * Proxy for emitting signals from the ProjectManager class.
 * The reason is to allow ProjectManager to use only static members, and not
 * be a Q_OBJECT (which would limit templating).
 * @author Elad Lahav
 */
class ProjectManagerSignals : public QObject
{
	Q_OBJECT

signals:
	void hasProject(bool has);
	void buildProject();

private:
	ProjectManagerSignals() : QObject() {}

	void emitHasProject(bool has) {
		emit hasProject(has);
	}

	void emitBuildProject() {
		emit buildProject();
	}

	friend class ProjectManager;
};

/**
 * Provides the active project for the application.
 * Maintains a ProjectBase object, which is the one and only active project in
 * the application. Also, provides safe access to the engine and code base
 * objects of the project.
 * @author Elad Lahav
 */
class ProjectManager
{
public:
	static bool hasProject() { return proj_ != NULL; }
	static const Core::ProjectBase* project();
	static Core::Engine& engine();
	static Core::Codebase& codebase();
	static const ProjectManagerSignals* signalProxy();

	template<class ProjectT>
	static void load(const QString& projPath) {
		// Do not load if another project is currently loaded.
		if (proj_)
			return;

		// Create and open a project.
		try {
			proj_ = new ProjectT(projPath);
			proj_->open(new OpenCallback());
		}
		catch (Core::Exception* e) {
			throw e;
		}

		// Save the project path.
		Application::settings().addRecentProject(projPath, proj_->name());
	}

	static void updateConfig(Core::ProjectBase::Params&);
	static void close();

private:
	static Core::ProjectBase* proj_;
	static ProjectManagerSignals signals_;

	static void finishLoad();

	struct OpenCallback : public Core::Callback<>
	{
		void call() {
			ProjectManager::finishLoad();
			delete this;
		}
	};
};

} // namespace App

} // namespace KScope

#endif // __APP_PROJECTMANAGER_H__
