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

#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include "querydialog.h"
#include "strings.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
QueryDialog::QueryDialog(QWidget* parent)
	: QDialog(parent), Ui::QueryDialog()
{
	setupUi(this);
}

/**
 * Class destructor.
 */
QueryDialog::~QueryDialog()
{
}

/**
 * Displays a modal dialogue.
 * @param  defType   Default query type
 * @param  typeList  The types to show
 */
int QueryDialog::exec(Core::Query::Type defType, const TypeList& typeList)
{
	// Prepare the query type combo box.
	typeCombo_->clear();
	if (typeList.size() > 0) {
		foreach (Core::Query::Type type, typeList)
			typeCombo_->addItem(Strings::toString(type), type);
	}
	else {
		// Create a list with all supported query types.
		TypeList typeList;
		typeList << Core::Query::Text << Core::Query::References
		         << Core::Query::Definition << Core::Query::CalledFunctions
		         << Core::Query::CallingFunctions << Core::Query::FindFile
		         << Core::Query::IncludingFiles;

		foreach (Core::Query::Type type, typeList)
			typeCombo_->addItem(Strings::toString(type), type);
	}

	// Select the default type.
	typeCombo_->setCurrentIndex(typeCombo_->findData(defType));

	return QDialog::exec();
}

/**
 * @return The text in the pattern editor
 */
QString QueryDialog::pattern()
{
	return patternCombo_->lineEdit()->text();
}

/**
 * @param  pattern  Initial text for the pattern editor
 */
void QueryDialog::setPattern(const QString& pattern)
{
	patternCombo_->lineEdit()->setText(pattern);
	patternCombo_->lineEdit()->selectAll();
}

/**
 * @return The selected query type.
 */
Core::Query::Type QueryDialog::type()
{
	int index;
	QVariant data;

	index = typeCombo_->currentIndex();
	if (index == -1)
		return Core::Query::References;

	data = typeCombo_->itemData(index);
	return static_cast<Core::Query::Type>(data.toUInt());
}

/**
 * Deletes all items in the pattern combo-box.
 */
void QueryDialog::clear()
{
	patternCombo_->clear();
}

/**
 * Called when the user clicks the "OK" button.
 * Removes all white space from before and after the entered text.
 */
void QueryDialog::accept()
{
	// Get a trimmed version of the current text.
	QString text = patternCombo_->lineEdit()->text().trimmed();
	if (text.isEmpty()) {
		QMessageBox::warning(this, tr("Invalid Pattern"),
		                     tr("Please enter a non-empty pattern"));
		return;
	}

	// Remove an existing copy of the pattern.
	int i = patternCombo_->findText(text);
	if (i >= 0) {
		patternCombo_->removeItem(i);
	}

	// Add to the top of the history list.
	patternCombo_->insertItem(i, text);

	// Make sure the list does not exceed 20 items.
	while (patternCombo_->count() >= 20)
		patternCombo_->removeItem(patternCombo_->count() - 1);

	// Set as the current text.
	patternCombo_->lineEdit()->setText(text);

	QDialog::accept();
}

} // namespace App

} // namespace KScope
