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

#include <QDebug>
#include <QDir>
#include "codebasemodel.h"

namespace KScope
{

namespace Core
{

CodebaseModel::CodebaseModel(const Codebase* cbase, const QString& rootPath,
                             QObject* parent) :
	QAbstractItemModel(parent),
	root_(rootPath)
{
	// TODO: rootPath may be invalid.
	// Need to set root_ to "/", and ignore rootPath.

	// Add all files in the code base.
	AddFilesCallback cb(this);
	cbase->getFiles(cb);
}

CodebaseModel::~CodebaseModel()
{
}

void CodebaseModel::addFiles(const QStringList& fileList)
{
	// Add each of the files in the list to the tree structure.
	QStringList::ConstIterator itr;
        beginResetModel();
	for (itr = fileList.begin(); itr != fileList.end(); ++itr)
		addFile(*itr);
        endResetModel();

	// It's much easier to just reset the model than to emit individual
	// dataChanged() signals, since files are added all over the tree.
        //reset();
}

/**
 * Converts the model's tree-structured data into a list of file paths.
 * @param  fileList  The list object to fill
 */
void CodebaseModel::getFiles(QStringList& fileList) const
{
	getFiles(&root_, "", fileList);
}

QModelIndex CodebaseModel::index(int row, int column,
                                 const QModelIndex& parent) const
{
	const ItemT* item;

	if (!parent.isValid()) {
		item = &root_;
	}
	else {
		item = indexData(parent);
		if (item == NULL)
			return QModelIndex();
	}

	return createIndex(row, column, (void*)item->child(row));
}

QModelIndex CodebaseModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	ItemT* item = indexData(index);
	if (item == NULL || item->parent() == &root_)
		return QModelIndex();

	return createIndex(item->parent()->index(), 0, (void*)item->parent());
}

QVariant CodebaseModel::headerData(int section, Qt::Orientation orient,
                                   int role) const
{
	(void)section;
	if (orient != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();

	return tr("Path");
}

int CodebaseModel::rowCount(const QModelIndex& parent) const
{
	const TreeItem<QString>* item;

	if (!parent.isValid()) {
		item = &root_;
	}
	else {
		item = static_cast<TreeItem<QString>*>(parent.internalPointer());
		if (item == NULL)
			return 0;
	}

	return item->childCount();
}

int CodebaseModel::columnCount(const QModelIndex& parent) const
{
	(void)parent;
	return 1;
}

QVariant CodebaseModel::data(const QModelIndex& index, int role) const
{
	QString file;

	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	TreeItem<QString>* item
		= static_cast<TreeItem<QString>*>(index.internalPointer());
	if (item == NULL)
		return QVariant();

	return item->data();
}

void CodebaseModel::addFile(const QString& path)
{
	// Remove the root path prefix.
	QString actPath;
	if (path.startsWith(root_.data()))
		actPath = path.mid(root_.data().length());
	else
		actPath = path;

	// Dissect the file path into its directory and file components.
	QStringList pathParts = actPath.split(QDir::separator(), QString::SkipEmptyParts);

	// Descend down the tree, following the path components.
	ItemT* item = &root_;
	QStringList::iterator itr;
	for (itr = pathParts.begin(); itr != pathParts.end(); ++itr) {
		// Find a child of the current item corresponding to the component.
		// Create a new one if the child could not be found.
		ItemT* child;
		if ((child = item->findChild(*itr)) != NULL)
			item = child;
		else
			item = item->addChild(*itr);
	}
}

/**
 * An internal, recursive version of getFiles().
 * @param  item      The current item in a DFS
 * @param  path      The path of the item's parent
 * @param  fileList  The list to fill
 */
void CodebaseModel::getFiles(const ItemT* item, const QString& path,
                             QStringList& fileList) const
{
	QString name = path + item->data();

	if (item->childCount()) {
		// Add directory separator, if required.
		if (!name.endsWith(QDir::separator()))
			name += QDir::separator();
		
		// Descend to sub-directories.
		for (int i = 0; i < item->childCount(); i++)
			getFiles(item->child(i), name, fileList);
	}
	else {
		// Found a file, add to the list.
		fileList.append(QDir::toNativeSeparators(name));
	}
}

#ifndef QT_NO_DEBUG
void CodebaseModel::verify(const QModelIndex& parent)
{
	int i;

	for (i = 0; i < rowCount(parent); i++) {
		QModelIndex child = index(i, 0, parent);
		if (child.parent() != parent) {
			QModelIndex cParent = child.parent();
			qDebug() << cParent.row() << cParent.column()
			         << cParent.internalPointer();
			qDebug() << parent.row() << parent.column()
			         << parent.internalPointer();
			Q_ASSERT(false);
		}
		verify(child);
	}
}
#endif

}

}
