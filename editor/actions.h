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

#ifndef __EDITOR_ACTIONS_H__
#define __EDITOR_ACTIONS_H__

#include <QObject>
#include <QAction>
#include <QMenu>
#include "editor.h"

namespace KScope
{

namespace Editor
{

/**
 * Manages actions for an editor.
 * This class provides a slot proxy for an editor object. This is beneficial in
 * a multi-editor environment, where the active window changes constantly. The
 * alternative would be to disconnect and connect editor slots directly, which
 * is inefficient.
 * @author Elad Lahav
 */
class Actions : public QObject
{
	Q_OBJECT

public:
	Actions(QObject* parent = 0);
	~Actions();

	void setupFileMenu(QMenu*) const;
	void setupEditMenu(QMenu*) const;
	void setEditor(Editor*);

public slots:
	/**
	 * Saves the current document.
	 */
	void save() { editor_->save(); }

	/**
	 * Copies the current selection to the clipboard.
	 */
	void copy() { editor_->copy(); }

	/**
	 * Copies the current selection to the clipboard and deletes it from the
	 * text.
	 */
	void cut() { editor_->cut(); }

	/**
	 * Paste clipboard contents in the current cursor position.
	 */
	void paste() { editor_->paste(); }

	/**
	 * Cancels the last operation.
	 */
	void undo() { editor_->undo(); }

	/**
	 * Re-executes a cancelled operation.
	 */
	void redo() { editor_->redo(); }

	/**
	 * Prompts the user for a pattern to search in the file.
	 */
	void find() { editor_->search(); }

	/**
	 * Repeats the last search, starting at the current potision.
	 */
	void findNext() { editor_->searchNext(); }

	/**
	 * Prompts the user for a line number, and moves the cursor to that line.
	 */
	void gotoLine() { editor_->gotoLine(); }

	/**
	 * Moves the cursor to the beginning of the current block.
	 */
	void gotoBlockBegin() { editor_->gotoBlockBegin(); }

signals:
	/**
	 * Emitted when a new editor is set.
	 * @param  valid true if the editor object pointer is valid, false otherwise
	 */
	void editorChanged(bool valid);

private:
	/**
	 * The managed editor object.
	 * May be NULL, in which case all actions should be disabled.
	 */
	Editor* editor_;

	/**
	 * Save document.
	 */
	QAction* actSave_;

	/**
	 * Copy text action.
	 */
	QAction* actCopy_;

	/**
	 * Cut text action.
	 */
	QAction* actCut_;

	/**
	 * Paste text action.
	 */
	QAction* actPaste_;

	/**
	 * Undo action.
	 */
	QAction* actUndo_;

	/**
	 * Redo action.
	 */
	QAction* actRedo_;

	/**
	 * Search text action.
	 */
	QAction* actFind_;

	/**
	 * Repeat search action.
	 */
	QAction* actFindNext_;

	/**
	 * Go to line action.
	 */
	QAction* actGotoLine_;

	QAction* actBlockBegin_;

	/**
	 * Allows all actions to be enabled when the editor pointer is valid, and
	 * disabled when it does not.
	 */
	QActionGroup* editorGroup_;
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_ACTIONS_H__
