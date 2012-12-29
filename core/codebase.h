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

#ifndef __CORE_CODEBASE_H__
#define __CORE_CODEBASE_H__

#include <QStringList>
#include "globals.h"

namespace KScope
{

namespace Core
{

/**
 * An abstract base class representing a set of source files.
 * KScope describes a project as a set of source files and a data engine that
 * indexes these files and can be queried for cross-reference information.
 * While the engine can potentially be responsible for managing the code base,
 * this is not always the case: Cscope can hold the list of files to work on in
 * a cscope.files file that is separate from the corss-reference database.
 * This class adds flexibility by allowing separate management of the set of
 * source files and their index.
 * @author Elad Lahav
 */
class Codebase : public QObject
{
	Q_OBJECT

public:
	Codebase(QObject* parent = 0) : QObject(parent) {}
	~Codebase() {}

	virtual void open(const QString& initString, Callback<>* cb) = 0;
	virtual void save(const QString& initString) = 0;
	virtual void getFiles(Callback<const QString&>& cb) const = 0;
	virtual void setFiles(const QStringList& fileList) = 0;
	virtual bool canModify() = 0;
	virtual bool needFiles() { return false; }

signals:
	void loaded();
	void modified();
};

}

}

#endif // __CORE_CODEBASE_H__
