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
#include "actions.h"

namespace KScope
{

namespace Editor
{

/**
 * Class constructor.
 * Creates the actions.
 * @param  parent Owner object
 */
Actions::Actions(QObject* parent) : QObject(parent), editor_(NULL)
{
	// Disable all actions when the editor pointer is invalid.
	editorGroup_ = new QActionGroup(this);
	editorGroup_->setEnabled(false);
	connect(this, SIGNAL(editorChanged(bool)), editorGroup_,
	        SLOT(setEnabled(bool)));

	// Save document.
	actSave_ = new QAction(tr("&Save"), this);
	actSave_->setShortcut(QKeySequence("Ctrl+S"));
	actSave_->setStatusTip(tr("Save the document"));
	connect(actSave_, SIGNAL(triggered()), this, SLOT(save()));
	editorGroup_->addAction(actSave_);

	// Copy selected text.
	actCopy_ = new QAction(tr("&Copy"), this);
	actCopy_->setShortcut(QKeySequence("Ctrl+C"));
	actCopy_->setStatusTip(tr("Copy selected text"));
	connect(actCopy_, SIGNAL(triggered()), this, SLOT(copy()));
	editorGroup_->addAction(actCopy_);

	// Cut selected text.
	actCut_ = new QAction(tr("Cu&t"), this);
	actCut_->setShortcut(QKeySequence("Ctrl+X"));
	actCut_->setStatusTip(tr("Cut selected text"));
	connect(actCut_, SIGNAL(triggered()), this, SLOT(cut()));
	editorGroup_->addAction(actCut_);

	// Patse clipboard contents.
	actPaste_ = new QAction(tr("&Paste"), this);
	actPaste_->setShortcut(QKeySequence("Ctrl+V"));
	actPaste_->setStatusTip(tr("Paste clipboard contents"));
	connect(actPaste_, SIGNAL(triggered()), this, SLOT(paste()));
	editorGroup_->addAction(actPaste_);

	// Undo last action.
	actUndo_ = new QAction(tr("&Undo"), this);
	actUndo_->setShortcut(QKeySequence("Ctrl+Z"));
	actUndo_->setStatusTip(tr("Undo last action"));
	connect(actUndo_, SIGNAL(triggered()), this, SLOT(undo()));
	editorGroup_->addAction(actUndo_);

	// Repeat undone action.
	actRedo_ = new QAction(tr("&Redo"), this);
	actRedo_->setShortcut(QKeySequence("Ctrl+Y"));
	actRedo_->setStatusTip(tr("Repeat undone action"));
	connect(actRedo_, SIGNAL(triggered()), this, SLOT(redo()));
	editorGroup_->addAction(actRedo_);

	// Search text in file.
	actFind_ = new QAction(tr("&Find..."), this);
	actFind_->setShortcut(QKeySequence("Ctrl+F"));
	actFind_->setStatusTip(tr("Search text in file"));
	connect(actFind_, SIGNAL(triggered()), this, SLOT(find()));
	editorGroup_->addAction(actFind_);

	// Repeat last search.
	actFindNext_ = new QAction(tr("Find &Next"), this);
	actFindNext_->setShortcut(QKeySequence("F3"));
	actFindNext_->setStatusTip(tr("Repeat last search"));
	connect(actFindNext_, SIGNAL(triggered()), this, SLOT(findNext()));
	editorGroup_->addAction(actFindNext_);

	// Go to line.
	actGotoLine_ = new QAction(tr("&Go to Line..."), this);
	actGotoLine_->setShortcut(QKeySequence("Ctrl+G"));
	actGotoLine_->setStatusTip(tr("Move cursor to a different line"));
	connect(actGotoLine_, SIGNAL(triggered()), this, SLOT(gotoLine()));
	editorGroup_->addAction(actGotoLine_);

	actBlockBegin_ = new QAction(tr("&Block Beginning"), this);
	actBlockBegin_->setShortcut(QKeySequence("Ctrl+{"));
	actBlockBegin_->setStatusTip(tr("Go to the beginning of the current "
	                                "block"));
	connect(actBlockBegin_, SIGNAL(triggered()), this, SLOT(gotoBlockBegin()));
	editorGroup_->addAction(actBlockBegin_);
}

/**
 * Class destructor.
 */
Actions::~Actions()
{
}

/**
 * Fills a menu with file-related editor actions.
 * @param  menu A menu to which actions should be added
 */
void Actions::setupFileMenu(QMenu* menu) const
{
	menu->addAction(actSave_);
}

/**
 * Fills a menu with editing-related editor actions.
 * @param  menu A menu to which actions should be added
 */
void Actions::setupEditMenu(QMenu* menu) const
{
	menu->addAction(actCopy_);
	menu->addAction(actCut_);
	menu->addAction(actPaste_);

	menu->addSeparator();

	menu->addAction(actUndo_);
	menu->addAction(actRedo_);

	menu->addSeparator();

	menu->addAction(actFind_);
	menu->addAction(actFindNext_);

	menu->addSeparator();

	menu->addAction(actGotoLine_);
	menu->addAction(actBlockBegin_);
}

/**
 * Sets a new managed editor.
 * @param  editor The editor to manage, or NULL if no editor is available
 */
void Actions::setEditor(Editor* editor)
{
	editor_ = editor;
	emit editorChanged(editor_ != NULL);
}

} // namespace Editor

} // namespace KScope
