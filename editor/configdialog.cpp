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

#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <core/exception.h>
#include "configdialog.h"
#include "lexerstylemodel.h"
#include "lexerstyledelegate.h"

namespace KScope
{

namespace Editor
{

/**
 * Class constructor.
 * @param  config Holds the configuration parameters to display by default
 * @param  parent Parent widget
 */
ConfigDialog::ConfigDialog(const Config& config, QWidget* parent)
	: QDialog(parent), Ui::ConfigDialog()
{
	setupUi(this);

	// Update the controls to reflect the given configuration.
	hlCurLineCheck_->setChecked(config.hlCurLine_);
	visibleWSpaceCheck_->setChecked(config.visibleWSpace_);
	marginLineNumbersCheck_->setChecked(config.marginLineNumbers_);
	eolMarkerCheck_->setChecked(config.eolMarkerColumn_ > 0);
	eolMarkerSpin_->setValue(config.eolMarkerColumn_);
	indentTabsCheck_->setChecked(config.indentTabs_);
	tabWidthSpin_->setValue(config.tabWidth_);

	// We use a copy of the style model managed by the Config object, so that
	// changes to the model do not persist in case the dialogue is cancelled.
	styleModel_ = new LexerStyleModel(config.lexers_, this);
	styleModel_->copy(*config.styleModel_);
	connect(resetButton_, SIGNAL(clicked()), styleModel_, SLOT(resetStyles()));

	// Prepare the style editor.
	styleView_->setModel(styleModel_);
	propView_->setModel(styleModel_);
	connect(styleView_->selectionModel(),
	        SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(editStyle(const QModelIndex&)));

	// Create the delegate for editing properties.
	LexerStyleDelegate* delegate = new LexerStyleDelegate(this);
	connect(delegate,
	        SIGNAL(editProperty(const QModelIndex&, const QVariant&)),
	        this, SLOT(editProperty(const QModelIndex&, const QVariant&)));
	propView_->setItemDelegate(delegate);

	// Select the top index, so that the property view shows its properties,
	// rather than a tree of styles.
	QModelIndex index = styleModel_->index(0, 0);
	styleView_->setCurrentIndex(index);

	// Populate language combo-boxes.
	lexerModel_ = new QStandardItemModel(this);
	int row = 0;
	foreach (QsciLexer* lexer, config.lexers_) {
		QStandardItem* item = new QStandardItem(lexer->language());
		item->setData(QVariant::fromValue((void*)lexer));
		lexerModel_->setItem(row++, 0, item);
	}

	indentLanguageCombo_->setModel(lexerModel_);

	// Set up the Vi mode combo-box.
	viModeCombo_->insertItem(0, tr("Disabled"), ViScintilla::Disabled);
	viModeCombo_->insertItem(1, tr("Insert"), ViScintilla::InsertMode);
	viModeCombo_->insertItem(2, tr("Normal"), ViScintilla::NormalMode);
	int modeIndex = viModeCombo_->findData(config.viDefaultMode_);
	viModeCombo_->setCurrentIndex(modeIndex);
}

/**
 * Class destructor.
 */
ConfigDialog::~ConfigDialog()
{
}

/**
 * Copies the parameters in the dialogue to the given configuration object.
 * @param  config The configuration object
 */
void ConfigDialog::getConfig(Config& config)
{
	config.hlCurLine_ = hlCurLineCheck_->isChecked();
	config.visibleWSpace_ = visibleWSpaceCheck_->isChecked();
	config.marginLineNumbers_ = marginLineNumbersCheck_->isChecked();
	config.eolMarkerColumn_
		= eolMarkerCheck_->isChecked() ? eolMarkerSpin_->value() : 0;
	config.indentTabs_ = indentTabsCheck_->isChecked();
	config.tabWidth_ = tabWidthSpin_->value();

	int modeIndex = viModeCombo_->currentIndex();
	config.viDefaultMode_
		= static_cast<ViScintilla::EditMode>
	      (viModeCombo_->itemData(modeIndex).toUInt());

		config.styleModel_->copy(*styleModel_);
	config.styleModel_->updateLexers();
}

void ConfigDialog::indentLanguageChanged(int id)
{
	// TODO: Implement.
	(void)id;
}

/**
 * Changes all styles to use their default values.
 * If the "Use default font" check-box is checked, then the font property is
 * ignored (i.e., not reset).
 */
void ConfigDialog::resetStyles()
{
	LexerStyleModel* styleModel
		= static_cast<LexerStyleModel*>(styleView_->model());
	styleModel->resetStyles();
}

/**
 * Prompts the user for a settings file containing a style scheme, and loads
 * the styles into the model.
 */
void ConfigDialog::importStyles()
{
	// Prompt for a file.
	QString file = QFileDialog::getOpenFileName(this, tr("Import Styles"));
	if (file.isEmpty())
		return;

	// Load the styles.
	QSettings settings(file, QSettings::IniFormat);
	try {
		styleModel_->load(settings, false);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Allows the user to save the current style scheme to an external file.
 */
void ConfigDialog::exportStyles()
{
	// Prompt for a file.
	QString file = QFileDialog::getSaveFileName(this, tr("Export Styles"));
	if (file.isEmpty())
		return;

	// Store the styles.
	QSettings settings(file, QSettings::IniFormat);
	try {
		styleModel_->store(settings, false);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

void ConfigDialog::editStyle(const QModelIndex& index)
{
	QModelIndex propRoot = index.child(0, 2);
	propView_->setRootIndex(propRoot);
}

void ConfigDialog::editProperty(const QModelIndex& index,
                                const QVariant& currentValue)
{
	QVariant newValue;

	switch (currentValue.type()) {
	case QVariant::Font:
		{
			bool ok;
			QFont font = QFontDialog::getFont(&ok, currentValue.value<QFont>(),
			                                  this);
			if (!ok)
				return;

			newValue.setValue(font);
		}
		break;

	case QVariant::Color:
		{
			QColor clr = QColorDialog::getColor(currentValue.value<QColor>(),
			                                    this);
			if (!clr.isValid())
				return;

			newValue.setValue(clr);
		}
		break;

	default:
		Q_ASSERT(false);
		return;
	}

	LexerStyleModel* model = static_cast<LexerStyleModel*>(styleView_->model());
	model->setData(index, newValue, Qt::EditRole);
}

void ConfigDialog::applyInheritance()
{
	QModelIndex index = propView_->selectionModel()->currentIndex();
	if (index.isValid())
		styleModel_->applyInheritance(index);
}

} // namespace Editor

} // namespace KScope
