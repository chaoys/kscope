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

#include "queryresultdialog.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
QueryResultDialog::QueryResultDialog(QWidget* parent)
	: QDialog(parent), Ui::QueryResultDialog()
{
	setupUi(this);

	// Close the dialogue when a selection is made.
	connect(view_, SIGNAL(locationRequested(const Core::Location&)), this,
	        SLOT(accept()));
}

/**
 * Class destructor.
 */
QueryResultDialog::~QueryResultDialog()
{
}

} // namespace App

} // namespace KScope
