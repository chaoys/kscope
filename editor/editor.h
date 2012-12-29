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

#ifndef __EDITOR_EDITOR_H__
#define __EDITOR_EDITOR_H__

#include <QSettings>
#include <core/globals.h>
#include "viscintilla.h"

namespace KScope
{

namespace Editor
{

/**
 * An QScintilla editor widget used to view/edit files.
 * @author Elad Lahav
 */
class Editor : public ViScintilla
{
	Q_OBJECT

public:
	Editor(QWidget* parent = 0);
	~Editor();

	/**
	 * Options for text searches.
	 */
	struct SearchOptions
	{
		/**
		 * Whether the pattern represents a regular expression.
		 */
		bool regExp_;

		/**
		 * Whether the search is case sensitive.
		 */
		bool caseSensitive_;

		/**
		 * Whether to look for whole words only.
		 */
		bool wholeWordsOnly_;

		/**
		 * Whether the search should wrap at the end of the document.
		 */
		bool wrap_;

		/**
		 * Whether to search backward in the document.
		 */
		bool backward_;
	};

	bool load(const QString&, QsciLexer* lexer);
	bool save();
	bool canClose();
	void moveCursor(uint, uint);
	QString currentSymbol() const;
	void setFocus();
	void getCurrentLocation(Core::Location&);
	QString title() const;

	/**
	 * @return The path of the file loaded in the editor, an empty string for
	 *         a new file
	 */
	QString path() const { return path_; }

	/**
	 * @param index The unique index used to generate the title of the editor
	 */
	void setNewFileIndex(uint index) { newFileIndex_ = index; }

public slots:
	void search();
	void searchNext();
	void gotoLine();
	void gotoBlockBegin();

signals:
	/**
	 * Notifies the container that the editor with the given title is being
	 * closed.
	 * @param  title The unique title of the editor being closed
	 */
	void closed(const QString& title);

	/**
	 * Notifies the container of a change to the editor's title.
	 * @param  oldTitle
	 * @param  newTitle
	 */
	void titleChanged(const QString& oldTitle, const QString& newTitle);

protected:
	void closeEvent(QCloseEvent*);

private:
	/**
	 * The file being edited.
	 */
	QString path_;

	/**
	 * For new files only, stores the index used to create a unique title for
	 * this editor.
	 */
	uint newFileIndex_;

	/**
	 * Whether a file is currently being loaded.
	 */
	bool isLoading_;

	/**
	 * The line to go to when loading finishes.
	 */
	uint onLoadLine_;

	/**
	 * The column to go to when loading finishes.
	 */
	uint onLoadColumn_;

	/**
	 * Whether to set the keyboard focus when loading finishes.
	 */
	bool onLoadFocus_;

private slots:
	void loadDone(const QString&);
};

} // namespace Editor

} // namespace KScope

#endif  // __EDITOR_EDITOR_H__
