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

#ifndef __CORE_CODEBASEMODEL_H
#define __CORE_CODEBASEMODEL_H

#include <QAbstractItemModel>
#include "treeitem.h"
#include "codebase.h"

namespace KScope
{

namespace Core
{

/**
 * @author Elad Lahav
 */
class CodebaseModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	CodebaseModel(const Codebase*, const QString& rootPath = "/",
	              QObject* parent = 0);
	~CodebaseModel();

	void addFiles(const QStringList&);
	void getFiles(QStringList&) const;

	virtual QModelIndex index(int row, int column,
	                          const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex&,
	                      int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int, Qt::Orientation,
	                            int role = Qt::DisplayRole) const;

private:
	typedef TreeItem<QString> ItemT;
	ItemT root_;

	void addFile(const QString&);
	void getFiles(const ItemT*, const QString&, QStringList&) const;

	static inline ItemT* indexData(QModelIndex index) {
		return static_cast<ItemT*>(index.internalPointer());
	}

	struct AddFilesCallback : public Core::Callback<const QString&>
	{
		CodebaseModel* model_;

		AddFilesCallback(CodebaseModel* model) : model_(model) {}

		void call(const QString& file) {
			model_->addFile(file);
		}
	};

	friend struct AddFilesCallback;

#ifndef QT_NO_DEBUG
	void verify(const QModelIndex&);
#endif
};

}

}

#endif // __CORE_CODEBASEMODEL_H
