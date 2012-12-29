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

#ifndef __CSCOPE_MANAGEDPROJECT_H__
#define __CSCOPE_MANAGEDPROJECT_H__

#include <core/project.h>
#include <core/projectconfig.h>
#include "crossref.h"
#include "files.h"
#include "configwidget.h"

namespace KScope
{

namespace Cscope
{

/**
 * A managed Cscope project.
 * This is a managed project, since KScope has control over the code base, which
 * is kept as a cscope.files file.
 * @author Elad Lahav
 */
class ManagedProject : public Core::Project<Crossref, Files>
{
public:
	ManagedProject(const QString& projPath = QString());
	virtual ~ManagedProject();

	void create(const Params&);
	void updateConfig(const Params&);
};

}

namespace Core
{

/**
 * Template specialisation for Cscope managed projects.
 * @author Elad Lahav
 */
template<>
struct ProjectConfig<Cscope::ManagedProject>
{
	/**
	 * Creates a Cscope configuration widget.
	 * @param  project  The project for which parameters are shown (NULL for a
	 *                  new project)
	 * @param  parent   The parent widget
	 * @return A new configuration widget
	 */
	static QWidget* createConfigWidget(const Cscope::ManagedProject* project,
	                                   QWidget* parent) {
		Cscope::ConfigWidget* widget = new Cscope::ConfigWidget(parent);

		if (project) {
			// Existing project: set widget controls to reflect current
			// configuration.
			ProjectBase::Params params;
			project->getCurrentParams(params);

			QStringList args = params.engineString_.split("*");
			widget->kernelCheck_->setChecked(args.contains("-k"));
			widget->invIndexCheck_->setChecked(args.contains("-q"));
			widget->compressCheck_->setChecked(!args.contains("-c"));
		}
		else {
			// New project: set default configuration.
			widget->kernelCheck_->setChecked(false);
			widget->invIndexCheck_->setChecked(true);
			widget->compressCheck_->setChecked(true);
		}

		return widget;
	}

	/**
	 * Updates a project parameters structure to reflect the current selections
	 * in a configuration widget.
	 * @param  widget  The widget from which parameters are taken
	 * @param  params  The structure to fill
	 */
	static void paramsFromWidget(QWidget* widget, ProjectBase::Params& params) {
		params.engineString_ = params.projPath_;
		params.codebaseString_ = params.projPath_;

		Cscope::ConfigWidget* confWidget
			= dynamic_cast<Cscope::ConfigWidget*>(widget);
		if (confWidget) {
			if (confWidget->kernelCheck_->isChecked())
				params.engineString_ += "*-k";
			if (confWidget->invIndexCheck_->isChecked())
				params.engineString_ += "*-q";
			if (!confWidget->compressCheck_->isChecked())
				params.engineString_ += "*-c";
		}
	}
};

} // namespace Cscope

} // namespace KScope

#endif // __CSCOPE_MANAGEDPROJECT_H
