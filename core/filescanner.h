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

#ifndef __CORE_FILESCANNER_H__
#define __CORE_FILESCANNER_H__

#include <QObject>
#include <QDir>
#include <QSet>
#include "filefilter.h"

namespace KScope
{

namespace Core
{

/**
 * A FileFilter-based directory scanner.
 * Scans can be performed on the given directory only, or recursively. By
 * default, symbolic links are not followed, to simplify the scan. However, it
 * is possible to set an option for following symbolic links, which somewhat
 * degrades performance (as the scanner needs to keep track of visited
 * directories to avoid loops).
 * @author Elad Lahav
 */
class FileScanner : public QObject
{
	Q_OBJECT

public:
	FileScanner(QObject* parent = NULL);
	~FileScanner();

	bool scan(const QDir&, const FileFilter&, bool recursive = false);

	/**
	 * Modifies the symbolic-link behaviour
	 * @param  follow  true to follow symbolic links, false to not
	 */
	void setFollowSymLinks(bool follow) {
		followSymLinks_ = follow;
	}

	/**
	 * Changes the type of progress signal emitted to be a formatted message.
	 * The given string should include a '%1' marker to be replaced by the
	 * number of scanned files, and a '%2' marker to be replaced by the
	 * number of files matching the filter.
	 * @param msg
	 */
	void setProgressMessage(const QString& msg) {
		progressMessage_ = msg;
	}

	/**
	 * Returns the list of files matched during the last scan.
	 * @return The matching file list
	 */
	const QStringList& matchedFiles() const { return fileList_; }

public slots:
	/**
	 * Signals the scan process to stop.
	 */
	void stop() { stop_ = true; }

signals:
	void progress(int scanned, int matched);
	void progress(const QString& msg);

private:
	/**
	 * true to follow symbolic links, false (default) to skip them.
	 */
	bool followSymLinks_;
	int scanned_;
	QStringList fileList_;
	FileFilter filter_;
	bool stop_;
	QString progressMessage_;
	QSet<QString> visitedDirs_;

	bool scan(const QDir&, bool, bool);
};

}

}

#endif // __CORE_FILESCANNER_H__
