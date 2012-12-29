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
#include "projectdialog.h"
#include "application.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
ProjectDialog::ProjectDialog(QWidget* parent)
	: QDialog(parent), Ui::ProjectDialog(), confWidget_(NULL)
{
	setupUi(this);
}

/**
 * Class destructor.
 */
ProjectDialog::~ProjectDialog()
{
}

void ProjectDialog::browseRootPath()
{
	QString dir = QFileDialog::getExistingDirectory(this,
	                                                tr("Select Directory"));
	if (!dir.isEmpty())
		rootPathEdit_->setText(dir);
}

void ProjectDialog::browseProjectPath()
{
	QString dir = QFileDialog::getExistingDirectory(this,
	                                                tr("Select Directory"));
	if (!dir.isEmpty())
		projectPathEdit_->setText(dir);
}

void ProjectDialog::updateProjectPath()
{
	QString path;
	if (namedDirButton_->isChecked()) {
		path = projectPathEdit_->text();
		if (!path.endsWith(QDir::separator()))
			path += QDir::separator();
		path += nameEdit_->text();
	}
	else {
		path = rootPathEdit_->text();
		if (!path.endsWith(QDir::separator()))
			path += QDir::separator();
		path += ".kscope";
	}

	projectPathLabel_->setText(path);
}

} // namespace App

} // namespace KScope
