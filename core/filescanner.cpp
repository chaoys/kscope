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

#include <QApplication>
#include <QDebug>
#include "filescanner.h"

namespace KScope
{

namespace Core
{

/**
 * Class constructor.
 * @param  parent  Owner object
 */
FileScanner::FileScanner(QObject* parent) : QObject(parent),
                                            followSymLinks_(false)
{
}

/**
 * Class destructor.
 */
FileScanner::~FileScanner()
{
}

/**
 * Starts a scan on a directory, using the given filter.
 * @param  dir        The directory to scan
 * @param  filter     The filter to use
 * @param  recursive  true for recursive scan, false otherwise
 * @return true if successful, false if the scan was aborted
 */
bool FileScanner::scan(const QDir& dir, const FileFilter& filter,
	                   bool recursive)
{
	scanned_ = 0;
	fileList_.clear();
	filter_ = filter;
	stop_ = false;

	// In a recursive scan, add only files under directories matching the filter
	// (starting with this one).
	QString path = QDir::toNativeSeparators(dir.path());
	 
	if (!path.endsWith(QDir::separator()))
		path += QDir::separator();

	bool addFiles = recursive ? filter_.match(path, true) : true;
	qDebug() << "Scanning" << path << recursive << addFiles;
	return scan(dir, recursive, addFiles);
}

/**
 * Implements a directory scan.
 * @param dir        The directory to scan
 * @param recursive  true for recursive scan, false otherwise
 * @return true if successful, false if the scan was aborted
 */
bool FileScanner::scan(const QDir& dir, bool recursive, bool addFiles)
{
	// Get a list of all entries in the directory.
	QFileInfoList infos = dir.entryInfoList(QDir::Files | QDir::Dirs
	                                        | QDir::NoDotAndDotDot);

	// Mark this directory as visited (if following symbolic links).
	if (followSymLinks_)
		visitedDirs_.insert(QDir::toNativeSeparators(dir.absolutePath()));

	// Iterate over the list.
	QFileInfoList::Iterator itr;
	for (itr = infos.begin(); itr != infos.end(); ++itr) {
		// Make sure event processing continues during a long scan.
		qApp->processEvents();
		if (stop_)
			return false;

		// Update and emit progress information.
		scanned_++;
		if ((scanned_ & 0xff) == 0) {
			if (!progressMessage_.isEmpty()) {
				QString msg = progressMessage_.arg(scanned_)
				                              .arg(fileList_.size());
				emit progress(msg);
			}
			else {
				emit progress(scanned_, fileList_.size());
			}
		}

		// Get the file's path.
		QString path = QDir::toNativeSeparators((*itr).filePath());
		if ((*itr).isDir()) {
			// Directory: scan recursively, if needed.
			if (recursive) {
				// Handle symbolic links.
				if ((*itr).isSymLink()) {
					qDebug() << __func__ << (*itr).absoluteFilePath();

					// If symbolic links are followed, make sure we do not
					// descend into an already-visited directory.
					if (!followSymLinks_
						|| visitedDirs_.contains(QDir::toNativeSeparators(
												(*itr).absoluteFilePath()))) {
							continue;
					}
				}

				// Change directory.
				QDir childDir(dir);
				if (!childDir.cd((*itr).fileName()))
					continue;

				// Add a trailing "/" to directory names, so that the filter can
				// distinguish those from regular files.
				if (!path.endsWith(QDir::separator()))
					path += QDir::separator();

				// Scan recursively.
				// Filter behaviour for sub-directories:
				// 1. If an inclusion rule is matched, add files.
				// 2. If an exclusion rule is matched, do not add files.
				// 3. If no rule is matched, inherit the behaviour of the
				//    current directory.
				if (!scan(childDir, true, filter_.match(path, addFiles)))
					return false;
			}
		}
		else if (addFiles) {
			// File: add to the file list if the path matches the filter.
			// The default match is set to false, so that files not matched by
			// any rule will not be added.
			if (filter_.match(path, false))
				fileList_.append(QDir::toNativeSeparators(path));
		}
	}

	return true;
}

}

}
