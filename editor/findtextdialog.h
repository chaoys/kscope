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

#ifndef __EDITOR_FINDTEXTDIALOG_H__
#define __EDITOR_FINDTEXTDIALOG_H__

#include <QDialog>
#include <QLineEdit>
#include "ui_findtextdialog.h"
#include "editor.h"

namespace KScope
{

namespace Editor
{

/**
 * A simple dialogue for searching text within an editor window.
 * @author  Elad Lahav
 */
class FindTextDialog : public QDialog, public Ui::FindTextDialog
{
	Q_OBJECT

public:
	FindTextDialog(QWidget* parent = 0);
	~FindTextDialog();

	static int promptPattern(QString&, Editor::SearchOptions&,
	                         QWidget* parent = NULL);

private:
	/**
	 * A list of previously-searched patterns.
	 */
	static QStringList historyList_;

	/**
	 *
	 */
	static Editor::SearchOptions options_;
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_FINDTEXTDIALOG_H__
