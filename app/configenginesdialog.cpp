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

#include <QLabel>
#include <cscope/crossref.h>
#include "application.h"
#include "configenginesdialog.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
ConfigEnginesDialog::ConfigEnginesDialog(QWidget* parent)
	: QDialog(parent), Ui::ConfigEnginesDialog()
{
	setupUi(this);

	// TODO: We'd like a list of engines that can be iterated over in compile
	// time to generate multi-engine code.
	typedef Core::EngineConfig<Cscope::Crossref> Config;

	QWidget* widget = Config::createConfigWidget(this);
	QString title;
	if (widget) {
		title = widget->windowTitle();
	}
	else {
		widget = new QLabel(tr("Configuration not available"), this);
		title = Config::name();
	}

	tabWidget_->addTab(widget, title);
}

/**
 * Class destructor.
 */
ConfigEnginesDialog::~ConfigEnginesDialog()
{
}

/**
 * Called when the user clicks the "OK" button.
 * Applies the configuration to the engines, and exits the dialogue.
 */
void ConfigEnginesDialog::accept()
{
	// TODO: We'd like a list of engines that can be iterated over in compile
	// time to generate multi-engine code.
	typedef Core::EngineConfig<Cscope::Crossref> Config;

	// Apply configuration to the engine.
	Config::configFromWidget(tabWidget_->widget(0));

	// Get the new set of parameters.
	Core::KeyValuePairs params;
	Config::getConfig(params);

	Settings& settings = Application::settings();

	// Prefix group with "Engine_" so that engines do not overrun application
	// groups by accident.
	settings.beginGroup(QString("Engine_") + Config::name());

	// Write the new configuration parameters.
	Core::KeyValuePairs::Iterator itr;
	for (itr = params.begin(); itr != params.end(); ++itr)
		settings.setValue(itr.key(), itr.value());

	settings.endGroup();

	QDialog::accept();
}

} // namespace App

} // namespace Kscope
