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

#ifndef __APP_OPENPROJECTDIALOG_H__
#define __APP_OPENPROJECTDIALOG_H__

#include <QDialog>
#include "ui_openprojectdialog.h"

namespace KScope
{

namespace App
{

/**
 * A dialogue that prompts the user for a project to open.
 * A project can be opened either by browsing for its path, or by choosing from
 * a list of recent projects.
 * It is also possible to select an option for creating a new project.
 * @author Elad Lahav
 */
class OpenProjectDialog : public QDialog, public Ui::OpenProjectDialog
{
	Q_OBJECT

public:
	OpenProjectDialog(QWidget* parent = 0);
	~OpenProjectDialog();

	/**
	 * Possible return values for exec().
	 */
	enum ExecResult {
		/** The user requested to open a project, using the provided path. */
		Open,
		/** The user requested to create a new project. */
		New,
		/** The "Cancel" button was clicked. */
		Cancel
	};

	ExecResult exec();

	/**
	 * @return The text currently selected in the path edit widget
	 */
	QString path() { return pathEdit_->text(); }

private slots:
	void browse();
	void newProject();
	void pathChanged(const QString&);
	void recentProjectSelected();
	void removeProject();
};

} // namespace App

} // namespace KScope

#endif // __APP_OPENPROJECTDIALOG_H__
