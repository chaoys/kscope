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

#ifndef __EDITOR_CONFIGDIALOG_H__
#define __EDITOR_CONFIGDIALOG_H__

#include <QDialog>
#include <QStandardItemModel>
#include "ui_configdialog.h"
#include "config.h"

namespace KScope
{

namespace Editor
{

class LexerStyleModel;

/**
 * A dialogue for configuring a QScintilla editor.
 * Unfortunately, QScintilla does not provide such a dialogue
 * @author Elad Lahav
 */
class ConfigDialog : public QDialog, public Ui::ConfigDialog
{
	Q_OBJECT

public:
	ConfigDialog(const Config&, QWidget* parent = NULL);
	~ConfigDialog();

	void getConfig(Config&);

signals:
	/**
	 * Used to notify the lexer model of a change to the default font.
	 */
	void defaultFontChanged();

private:
	QStandardItemModel* lexerModel_;
	LexerStyleModel* styleModel_;

private slots:
	void indentLanguageChanged(int);
	void resetStyles();
	void importStyles();
	void exportStyles();
	void editStyle(const QModelIndex&);
	void editProperty(const QModelIndex&, const QVariant&);
	void applyInheritance();
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_CONFIGDIALOG_H__
