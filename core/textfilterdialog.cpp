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

#include "textfilterdialog.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  re     The regular expression to use by default
 * @param  parent Parent widget
 */
TextFilterDialog::TextFilterDialog(const QRegExp& re, QWidget* parent)
	: QDialog(parent), Ui::TextFilterDialog()
{
	setupUi(this);

	// Show the regular expression pattern.
	patternEdit_->setText(re.pattern());

	// Set the type of filter to use.
	if (re.isEmpty()) {
		// Special case: empty QRegExp.
		// We want the default to be a fixed-string match, while a QRegExp()
		// constructor defaults to a regular expression.
		stringButton_->setChecked(true);
	}
	else {
		// Non-empty QRegExp.
		// Use the type set for the object.
		switch (re.patternSyntax()) {
		case QRegExp::FixedString:
			stringButton_->setChecked(true);
			break;

		case QRegExp::RegExp:
		case QRegExp::RegExp2:
			regExpButton_->setChecked(true);
			break;

		case QRegExp::Wildcard:
			simpRegExpButton_->setChecked(true);
			break;
		}
	}

	// Determine whether the filter is case-sensitive.
	caseSensitiveCheck_->setChecked(re.caseSensitivity() == Qt::CaseSensitive);
}

/**
 * Class destructor.
 */
TextFilterDialog::~TextFilterDialog()
{
}

/**
 * Populates the "Filter By" combo-box.
 * @param  pairs Option names and values for the combo-box
 */
void TextFilterDialog::setFilterByList(const KeyValuePairs& pairs)
{
	KeyValuePairs::ConstIterator itr;
	for (itr = pairs.begin(); itr != pairs.end(); ++itr)
		filterByCombo_->addItem(itr.key(), itr.value());
}

/**
 * Selects an item in the "Filter By" combo-box.
 * @param  value The value by which the item is selected
 */
void TextFilterDialog::setFilterByValue(const QVariant& value)
{
	int index = filterByCombo_->findData(value);
	if (index >= 0)
		filterByCombo_->setCurrentIndex(index);
}

/**
 * Generates a QRegExp object from the parameters of the dialogue.
 * @return The generated QRegExp object
 */
QRegExp TextFilterDialog::filter() const
{
	// Determine the type of filter to use.
	QRegExp::PatternSyntax type;
	if (stringButton_->isChecked())
		type = QRegExp::FixedString;
	else if (regExpButton_->isChecked())
		type = QRegExp::RegExp;
	else if (simpRegExpButton_->isChecked())
		type = QRegExp::Wildcard;

	// Determine case sensitivity.
	Qt::CaseSensitivity cs = caseSensitiveCheck_->isChecked()
	                         ? Qt::CaseSensitive :  Qt::CaseInsensitive;

	// Create and return the object.
	return QRegExp(patternEdit_->text(), cs, type);
}

/**
 * @return The data stored in the current index of the "Filter By" combo
 */
QVariant TextFilterDialog::filterByValue() const
{
	return filterByCombo_->itemData(filterByCombo_->currentIndex());
}

} // namespace Core

} // namespace KScope
