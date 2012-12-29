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

#include "progressbar.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
ProgressBar::ProgressBar(QWidget* parent) : QFrame(parent), Ui::ProgressBar()
{
	// Setup user interface.
	setFrameShape(QFrame::StyledPanel);
	setupUi(this);

	// Emit the cancelled() signal when the cancel button is clicked.
	connect(cancelButton_, SIGNAL(clicked()), this, SIGNAL(cancelled()));
}

/**
 * Class destructor.
 */
ProgressBar::~ProgressBar()
{
}

/**
 * Updates the progress values.
 * @param  cur   New current value
 * @param  total Expected final value
 */
void ProgressBar::setProgress(uint cur, uint total)
{
	progBar_->setRange(0, total);
	progBar_->setValue(cur);
}

/**
 * Changes the text inside the progress-bar.
 * @param  text  The new text to show
 */
void ProgressBar::setLabel(const QString& text)
{
	progBar_->setFormat(text + " %p%");
}

} // namespace Core

} // namespace KScope
