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

#include "findtextdialog.h"

namespace KScope
{

namespace Editor
{

QStringList FindTextDialog::historyList_;
Editor::SearchOptions FindTextDialog::options_;

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
FindTextDialog::FindTextDialog(QWidget* parent)
	: QDialog(parent), Ui::FindTextDialog()
{
	setupUi(this);
}

/**
 * Class destructor.
 */
FindTextDialog::~FindTextDialog()
{
}

/**
 * Displays the search dialogue.
 * This is a convenience static function, which remembers the last settings, as
 * well as keeps a history of searched patterns.
 * @param  pattern  Default pattern (input) and selected pattern (output)
 * @param  options  Holds the selected search options
 * @param  parent   Parent widget
 * @return The result of calling exec() for a dialogue object
 */
int FindTextDialog::promptPattern(QString& pattern,
                                  Editor::SearchOptions& options,
                                  QWidget* parent)
{
	FindTextDialog dlg(parent);

	// Adjust dialogue controls.
	dlg.patternCombo_->addItems(historyList_);
	dlg.patternCombo_->lineEdit()->setText(pattern);
	dlg.patternCombo_->lineEdit()->selectAll();
	dlg.regExpCheck_->setChecked(options_.regExp_);
	dlg.caseSensitiveCheck_->setChecked(options_.caseSensitive_);
	dlg.wholeWordsCheck_->setChecked(options_.wholeWordsOnly_);
	dlg.wrapCheck_->setChecked(options_.wrap_);
	dlg.backwardsCheck_->setChecked(options_.backward_);

	// Show the dialogue.
	if (dlg.exec() != QDialog::Accepted)
		return QDialog::Rejected;

	// Get the current search parameters.
	// This will both save the current options as well as create a copy for the
	// caller.
	pattern = dlg.patternCombo_->lineEdit()->text();
	options_.regExp_ = dlg.regExpCheck_->isChecked();
	options_.caseSensitive_ = dlg.caseSensitiveCheck_->isChecked();;
	options_.wholeWordsOnly_ = dlg.wholeWordsCheck_->isChecked();
	options_.wrap_ = dlg.wrapCheck_->isChecked();
	options_.backward_ = dlg.backwardsCheck_->isChecked();
	options = options_;

	// Add to the history list, if pattern is not already there.
	if (!historyList_.contains(pattern)) {
		// Restrict the size of the list to 20 entries.
		if (historyList_.size() == 20)
			historyList_.removeLast();

		historyList_.prepend(pattern);
	}

	return QDialog::Accepted;
}

} // namespace Editor

} // namespace KScope
