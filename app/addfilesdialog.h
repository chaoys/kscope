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

#ifndef __KSCOPE_ADDFILESDIALOG_H__
#define __KSCOPE_ADDFILESDIALOG_H__

#include <QDialog>
#include <QFileDialog>
#include "ui_addfilesdialog.h"

namespace KScope
{

namespace App
{

/**
 * A dialogue used to select files to be added to a project.
 * @author Elad Lahav
 */
class AddFilesDialog : public QDialog, public Ui::AddFilesDialog
{
	Q_OBJECT

public:
	AddFilesDialog(QWidget* parent = NULL);
	~AddFilesDialog();

	void fileList(QStringList&) const;

protected slots:
	void addFiles();
	void addDir();
	void addTree();
	void loadFilter();
	void saveFilter();
	void deleteSelectedFiles();

private:
	bool getFiles(QFileDialog::FileMode, QStringList&);
	void addTree(bool);
};

}

}

#endif // __KSCOPE_ADDFILESDIALOG_H__
