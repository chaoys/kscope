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
#include "settings.h"

namespace KScope
{

namespace App
{

Settings::Settings() : QSettings()
{
}

Settings::~Settings()
{
}

void Settings::load()
{
	beginGroup("RecentProjects");

	int size = beginReadArray("Project");
	for (int i = 0; i < size; i++) {
		setArrayIndex(i);
		RecentProject proj;
		proj.path_ = value("Path").toString();
		proj.name_ = value("Name").toString();
		recentProjects_.append(proj);
	}
	endArray();

	endGroup();
}

void Settings::store()
{
	beginGroup("RecentProjects");

	beginWriteArray("Project");
	QLinkedList<RecentProject>::iterator itr;
	int i;
	for (itr = recentProjects_.begin(), i = 0; itr != recentProjects_.end();
	     ++itr, i++) {
		setArrayIndex(i);
		setValue("Path", (*itr).path_);
		setValue("Name", (*itr).name_);
	}
	endArray();

	endGroup();
}

void Settings::addRecentProject(const QString& path, const QString& name)
{
	// If this is already the most recent project, only need to make sure that
	// the project name is up-to-date.
	if (!recentProjects_.isEmpty() && recentProjects_.first().path_ == path) {
		recentProjects_.first().name_ = name;
		return;
	}

	// Remove the path from the current list.
	removeRecentProject(path);

	// Prepend the project to the list.
	RecentProject proj;
	proj.path_ = path;
	proj.name_ = name;
	recentProjects_.prepend(proj);

	// Truncate list if it contains more than 10 projects.
	while (recentProjects_.size() > 10)
		recentProjects_.removeLast();
}

void Settings::removeRecentProject(const QString& path)
{
	QLinkedList<RecentProject>::iterator itr;
	for (itr = recentProjects_.begin(); itr != recentProjects_.end(); ++itr) {
		if ((*itr).path_ == path)
			itr = recentProjects_.erase(itr);
	}
}

} // namespace App

} // namespace KScope
