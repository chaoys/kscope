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

#ifndef __APP_PROJECTDIALOG_H__
#define __APP_PROJECTDIALOG_H__

#include <QDialog>
#include <core/project.h>
#include <core/projectconfig.h>
#include "ui_projectdialog.h"

namespace KScope
{

namespace App
{

/**
 * Allows the configuration of project properties.
 * Used either to create a new project, or to modify an existing one.
 * @author Elad Lahav
 */
class ProjectDialog : public QDialog, public Ui::ProjectDialog
{
	Q_OBJECT

public:
	ProjectDialog(QWidget* parent = NULL);
	~ProjectDialog();

	/**
	 * @param  proj  The project from which to read parameters, NULL for a new
	 *               project
	 */
	template <class ProjectT>
	void setParamsForProject(const ProjectT* proj) {
		if (proj) {
			// Display properties for an existing project.
			setWindowTitle(tr("Project Properties"));
			projectPathGroup_->setEnabled(false);

			// Get the current project properties.
			Core::ProjectBase::Params params;
			proj->getCurrentParams(params);

			// Update dialogue controls.
			nameEdit_->setText(params.name_);
			rootPathEdit_->setText(params.rootPath_);
			projectPathLabel_->setText(params.projPath_);
		}
		else {
			// New project dialogue.
			setWindowTitle(tr("New Project"));
			projectPathEdit_->setText(QDir::toNativeSeparators(QDir::home().path()));
		}

		// Add a project-specific configuration page.
		confWidget_
			= Core::ProjectConfig<ProjectT>::createConfigWidget(proj, this);
		if (confWidget_)
			configTabs_->addTab(confWidget_, confWidget_->windowTitle());
	}

	/**
	 * Fills a project parameters structure with values reflecting the current
	 * selections in the dialogue.
	 * @param  params  The structure to fill
	 */
	template <class ProjectT>
	void getParams(Core::ProjectBase::Params& params) {
		// Fill parameters from the main page.
		params.projPath_ = projectPathLabel_->text();
		params.name_ = nameEdit_->text();
		params.rootPath_ = rootPathEdit_->text();

		// Get parameters from the project-specific configuration page.
		if (confWidget_) {
			Core::ProjectConfig<ProjectT>
			    ::paramsFromWidget(confWidget_, params);
		}
	}

protected slots:
	void browseRootPath();
	void browseProjectPath();
	void updateProjectPath();

private:
	QWidget* confWidget_;
};

} // namespace App

} // namespace KScope

#endif // __APP_PROJECTDIALOG_H__
