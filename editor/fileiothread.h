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

#ifndef __EDITOR_FILEIOTHREAD_H__
#define __EDITOR_FILEIOTHREAD_H__

#include <QThread>
#include <QFile>
#include <QTextStream>

namespace KScope
{

namespace Editor
{

/**
 * Thread-based asynchronous file loading/storing.
 * @author Elad Lahav
 */
class FileIoThread : public QThread
{
	Q_OBJECT

public:
	FileIoThread(QObject* parent) : QThread(parent) {}

	bool load(const QString& path) {
		file_.setFileName(path);

		if (!file_.open(QIODevice::ReadOnly | QIODevice::Text))
			return false;

		start();
		return true;
	}

	virtual void run() {
		QTextStream strm(&file_);
		QString text;

		text = strm.readAll();
		file_.close();

		emit done(text);
	}

signals:
	void done(const QString& text);

private:
	QFile file_;
};

} // namespace Editor

} // namespace KScope

#endif  // __EDITOR_FILEIOTHREAD_H__
