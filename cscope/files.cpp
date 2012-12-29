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

#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <core/exception.h>
#include "files.h"

namespace KScope
{

namespace Cscope
{

/**
 * Class constructor.
 * @param  parent  Parent object
 */
Files::Files(QObject* parent) : Core::Codebase(parent), writable_(false)
{
}

/**
 * Class destructor.
 */
Files::~Files()
{
}

/**
 * Creates a new cscope.files file.
 * @param  path  The project path under which to create the new file
 */
void Files::create(const QString& path)
{
	// Make sure the directory exists.
	QDir dir(path);
	if (!dir.exists())
		throw new Core::Exception("File list directory does not exist");

	// Open the file for writing.
	QFile file(dir.filePath("cscope.files"));
	if (!file.open(QIODevice::WriteOnly))
		throw Core::Exception("Failed to create the 'cscope.files' file");

	path_ = dir.filePath("cscope.files");
	writable_ = true;
	empty_ = true;
}

void Files::open(const QString& path, Core::Callback<>* cb)
{
	// Make sure the directory exists.
	QDir dir(path);
	if (!dir.exists())
		throw new Core::Exception("File list directory does not exist");

	// Check if the file exists.
	QFileInfo fi(dir, "cscope.files");
	if (fi.exists()) {
		// Yes, make sure it can be read.
		if (!fi.isReadable())
			throw new Core::Exception("Cannot open 'cscope.files' for reading");

		path_ = dir.filePath("cscope.files");
		writable_ = fi.isWritable();
		empty_ = (fi.size() == 0);
	}
	else {
		// No, create a new file.
		try {
			create(path);
		}
		catch (Core::Exception* e) {
			throw e;
		}
	}

	if (cb)
		cb->call();
}

void Files::save(const QString& path)
{
	(void)path;
}

void Files::getFiles(Core::Callback<const QString&>& cb) const
{
	QFile file(path_);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream strm(&file);
	while (!strm.atEnd())
		cb.call(strm.readLine());
}

void Files::setFiles(const QStringList& fileList)
{
	QFile file(path_);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream strm(&file);
	QStringList::ConstIterator itr;
	for (itr = fileList.begin(); itr != fileList.end(); ++itr)
		strm << *itr << endl;

	empty_ = fileList.isEmpty();
}

} // namespace Cscope

} // namespace KScope
