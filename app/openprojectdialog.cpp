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

#include <QFileDialog>
#include "openprojectdialog.h"
#include "application.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent Parent widget
 */
OpenProjectDialog::OpenProjectDialog(QWidget* parent)
	: QDialog(parent), Ui::OpenProjectDialog()
{
	setupUi(this);
}

/**
 * Class destructor.
 */
OpenProjectDialog::~OpenProjectDialog()
{
}

/**
 * Shows the dialogue in modal mode.
 * @return See ExecResult
 */
OpenProjectDialog::ExecResult OpenProjectDialog::exec()
{
	// Populate the "Recent Projects" list.
	bool first = true;
	foreach (Settings::RecentProject proj,
	         Application::settings().recentProjects()) {
		QListWidgetItem* item = new QListWidgetItem(proj.name_,
		                                            recentProjectsList_);
		item->setData(Qt::UserRole, proj.path_);

		// Select the first item.
		if (first) {
			item->setSelected(true);
			first = false;
		}
	}

	// Show the dialogue.
	if (QDialog::exec() == Rejected)
		return Cancel;

	return pathEdit_->text().isEmpty() ? New : Open;
}

/**
 * Called when the user clicks the "..." button next to the path edit widget.
 * Shows the standard "Select Directory" dialogue.
 * The directories available for selection are the visible ones, as well as
 * hidden directories called ".kscope".
 */
void OpenProjectDialog::browse()
{
	// Set up the dialogue.
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::DirectoryOnly);
	dlg.setNameFilter("[^\\.]* .kscope");
	dlg.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);

	// Show the dialogue.
	if ((dlg.exec() == QDialog::Rejected) || dlg.selectedFiles().isEmpty())
		return;

	// Get the selected path.
	QString path = dlg.selectedFiles().first();
	if (!path.isEmpty())
		pathEdit_->setText(path);
}

/**
 * Called when the user clicks the "New..." button.
 * Closes the dialogue, indicating that a new project was requested.
 */
void OpenProjectDialog::newProject()
{
	pathEdit_->setText("");
	accept();
}

/**
 * Enables/disables the "Open" button, depending on whether the path widget
 * contains text.
 * @param  path The text in the path edit widget
 */
void OpenProjectDialog::pathChanged(const QString& path)
{
	openButton_->setDisabled(path.isEmpty());
}

/**
 * Called when selection changes in the "Recent Projects" list.
 * If an item is selected, its path is set in the path edit widget.
 * The method also controls the "Remove" button, which is enabled if and only
 * is a project item is selected.
 */
void OpenProjectDialog::recentProjectSelected()
{
	// Check if any item was selected.
	if (recentProjectsList_->selectedItems().isEmpty()) {
		removeProjectButton_->setEnabled(false);
		return;
	}

	// Extract the project's path from the item.
	QListWidgetItem* item = recentProjectsList_->selectedItems().first();
	pathEdit_->setText(item->data(Qt::UserRole).toString());

	removeProjectButton_->setEnabled(true);
}

/**
 * Called when the "Remove" button is clicked.
 * Removes the currently selected project from the list of recent projects.
 */
void OpenProjectDialog::removeProject()
{
	// We can safely get the first item, since the "Remove" button is only
	// enabled when there is a non-empty selection list.
	QListWidgetItem* item = recentProjectsList_->selectedItems().first();
	QString path = item->data(Qt::UserRole).toString();
	Application::settings().removeRecentProject(path);
	delete item;
}

} // namespace App

} // namespace KScope
