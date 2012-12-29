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

#include <core/codebasemodel.h>
#include "application.h"
#include "projectfilesdialog.h"
#include "addfilesdialog.h"
#include "projectmanager.h"

namespace KScope
{

namespace App
{

ProjectFilesDialog::ProjectFilesDialog(QWidget* parent)
	: QDialog(parent), Ui::ProjectFilesDialog()
{
	setupUi(this);

	try {
		const Core::ProjectBase* proj = ProjectManager::project();
		const Core::Codebase* codebase = &ProjectManager::codebase();

		Core::CodebaseModel* model
			= new Core::CodebaseModel(codebase, proj->rootPath(), this);
		view_->setModel(model);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

ProjectFilesDialog::~ProjectFilesDialog()
{
}

void ProjectFilesDialog::accept()
{
	// Get all files from the model.
	Core::CodebaseModel* model
		= static_cast<Core::CodebaseModel*>(view_->model());
	QStringList fileList;
	model->getFiles(fileList);

	// Update the code base.
	try {
		ProjectManager::codebase().setFiles(fileList);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}

	QDialog::accept();
}

void ProjectFilesDialog::addFiles()
{
	AddFilesDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {
		QStringList fileList;
		dlg.fileList(fileList);

		Core::CodebaseModel* model
			= static_cast<Core::CodebaseModel*>(view_->model());
		model->addFiles(fileList);
	}
}

void ProjectFilesDialog::removeFiles()
{
	// TODO: Implement!
}

}

}
