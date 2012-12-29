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

#ifndef __APP_SETTINGS_H__
#define __APP_SETTINGS_H__

#include <QSettings>
#include <QLinkedList>

namespace KScope
{

namespace App
{

/**
 * Manages application-wide configuration.
 * @author Elad Lahav
 */
class Settings : public QSettings
{
public:
	Settings();
	~Settings();

	void load();
	void store();

	struct RecentProject
	{
		QString path_;
		QString name_;
	};

	void addRecentProject(const QString&, const QString&);
	void removeRecentProject(const QString&);
	const QLinkedList<RecentProject>& recentProjects() const {
		return recentProjects_;
	}

private:
	QLinkedList<RecentProject> recentProjects_;
};

} // namespace App

} // namespace KScope

#endif // __APP_SETTINGS_H__
