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

#include <QList>
#include <QSortFilterProxyModel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDebug>
#include <core/filefilter.h>
#include <core/filescanner.h>
#include "addfilesdialog.h"
#include "projectmanager.h"

namespace KScope
{

namespace App
{

struct FileFilter
{
	QString filter_;
	bool include_;
};

typedef QList<FileFilter> FileFilterList;

/**
 * A filter proxy model for QFileDialog.
 * The model is used to to display filtered items in the QFileDialog used for
 * selecting files to add to the project.
 * @note There is a bug in Qt versions 4.4.3 and below, which causes the
 *       file dialogue to crash when using a proxy. This bug is supposed to be
 *       fixed in 4.4.4.
 * @author Elad Lahav
 */
class FileFilterProxy : public QSortFilterProxyModel
{
public:
	FileFilterProxy(const QString& filter, QObject* parent = NULL)
		: QSortFilterProxyModel(parent), filter_(filter) {}

protected:
	bool filterAcceptsRow (int row, const QModelIndex& parent) const {
		// Get the inspected index from the source model.
		QModelIndex index = sourceModel()->index(row, 0, parent);
		if (!index.isValid())
			return false;

		// We need to distinguish directories from files somehow. We do this by
		// appending a trailing '/' to the file name.
		// TODO: Is there a safer way to check for directories than querying
		// about index children?
		QString name = sourceModel()->data(index).toString();
		if (sourceModel()->hasChildren(index))
			name += QDir::separator();

		// Query the filter.
		return filter_.match(name, true);
	}

private:
	Core::FileFilter filter_;
};

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
AddFilesDialog::AddFilesDialog(QWidget* parent) : QDialog(parent),
                                                  Ui::AddFilesDialog()
{
	setupUi(this);
}

/**
 * Class destructor.
 */
AddFilesDialog::~AddFilesDialog()
{
}

/**
 * Generates a list of all the files selected for addition.
 * @param  list  The list object to fill
 */
void AddFilesDialog::fileList(QStringList& list) const
{
	QListWidgetItem* item;
	int i = 0;

	while ((item = fileList_->item(i++)))
		list.append(QDir::toNativeSeparators(item->text()));
}

/**
 * Prompts the user for files to add.
 */
void AddFilesDialog::addFiles()
{
	QStringList list;
	if (getFiles(QFileDialog::ExistingFiles, list))
		fileList_->addItems(list);
}

/**
 * Prompts the user for a directory to add.
 * These include all files under the selected directory which match the current
 * filter.
 */
void AddFilesDialog::addDir()
{
	addTree(false);
}

/**
 * Prompts the user for a directory to add.
 * These include all files under the selected directory, as well as any of its
 * sub-directories, which match the current filter.
 */
void AddFilesDialog::addTree()
{
	addTree(true);
}

/**
 * Prompts the user for a text file which contains a file filter.
 * The first line in the selected file is set as the current filter.
 */
void AddFilesDialog::loadFilter()
{
	// Prompt for a file.
	QString path = QFileDialog::getOpenFileName(this, tr("Load Filter"));
	if (path.isEmpty())
		return;

	// Open the file.
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		QString msg = tr("Failed to open '%1'").arg(path);
		QMessageBox::critical(this, tr("File Error"), msg);
		return;
	}

	// Set the first line as the current filter.
	QTextStream str(&file);
	filterEdit_->setText(str.readLine());
}

/**
 * Prompts the user for a file, into which the current filter is saved.
 */
void AddFilesDialog::saveFilter()
{
	// Prompt for a file.
	QString path = QFileDialog::getSaveFileName(this, tr("Save Filter"));
	if (path.isEmpty())
		return;

	// Open the file.
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QString msg = tr("Failed to open '%1'").arg(path);
		QMessageBox::critical(this, tr("File Error"), msg);
		return;
	}

	// Store the current filter.
	QTextStream str(&file);
	str << Core::FileFilter(filterEdit_->text()).toString();
}

/**
 * Deletes the selected items from the file list.
 */
void AddFilesDialog::deleteSelectedFiles()
{
	// Get the currently selected items.
	QList<QListWidgetItem*> selectedItems = fileList_->selectedItems();

	// Delete the selected items.
	QList<QListWidgetItem*>::Iterator itr;
	for (itr = selectedItems.begin(); itr != selectedItems.end(); ++itr)
		delete *itr;
}

/**
 * Creates a file dialog.
 * This method is used by the various add() functions.
 * @param  mode  The type of file dialogue to display
 * @param  list  A list object to fill with selected file/directory names
 * @return true if successful, false if the user cancelled selection
 */
bool AddFilesDialog::getFiles(QFileDialog::FileMode mode, QStringList& list)
{
	QFileDialog dlg(this);
	dlg.setDirectory(ProjectManager::project()->rootPath());
	dlg.setFileMode(mode);

	// TODO: Re-enable when the proxy bug in QFileDialog gets fixed.
#if 0
	FileFilterProxy* proxy = new FileFilterProxy(filterEdit_->text());
	dlg.setProxyModel(proxy);
#endif

	if (dlg.exec() == QDialog::Rejected)
		return false;

	list = dlg.selectedFiles();
	return true;
}

/**
 * Service method for addDir() and addTree().
 * Prompts the user for a directory, and then adds all files under this
 * directory that match the current filter.
 * @param  recursive  true to descend into sub-directories, false otherwise
 */
void AddFilesDialog::addTree(bool recursive)
{
	// Prompt the user for a directory.
	QStringList list;
	if (!getFiles(QFileDialog::DirectoryOnly, list))
		return;

	// Create the scanner object.
	Core::FileScanner scanner(this);
	scanner.setProgressMessage(tr("Found %2 of %1 files"));

	// Create a modal progress dialogue.
	// This will be used to display progress information, as well as allow the
	// user to cancel scanning.
	QProgressDialog dlg(tr("Scanning..."), tr("Cancel"), 0, 0, this);
	dlg.setModal(true);
	connect(&dlg, SIGNAL(canceled()), &scanner, SLOT(stop()));
	connect(&scanner, SIGNAL(progress(const QString&)), &dlg,
	        SLOT(setLabelText(const QString&)));
	dlg.show();

	QString filter = filterEdit_->text();

	// Scan for files.
	if (scanner.scan(list.first(), Core::FileFilter(filter), recursive))
		fileList_->addItems(scanner.matchedFiles());
}

}

}
