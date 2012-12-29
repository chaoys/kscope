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

#ifndef __APP_EDITORCONTAINER_H
#define __APP_EDITORCONTAINER_H

#include <QMdiArea>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMap>
#include <QMenu>
#include <core/globals.h>
#include <editor/editor.h>
#include <editor/config.h>
#include <editor/actions.h>
#include "locationhistory.h"
#include "session.h"

namespace KScope
{

namespace App
{

/**
 * Manages editor windows.
 * This is the central widget of the main window. It contains and manages all
 * open editor windows in an MDI-style.
 * @author Elad Lahav
 */
class EditorContainer : public QMdiArea
{
	Q_OBJECT

public:
	EditorContainer(QMainWindow* parent);
	~EditorContainer();

	void populateWindowMenu(QMenu*) const;
	bool canClose();
	void saveSession(Session&);
	void loadSession(Session&);
	void clearHistory();

	inline Editor::Editor* currentEditor() {
		return editorFromWindow(currentSubWindow());
	}

	const Editor::Actions& actions() const { return actions_; }

public slots:
	void newFile();
	void openFile();
	void configEditor();
	void gotoLocation(const Core::Location&);
	void gotoNextLocation();
	void gotoPrevLocation();
	void showLocalTags();
	void browseHistory();
	void closeAll();

signals:
	void hasActiveEditor(bool has);

private:
	QMdiSubWindow* currentWindow_;
	QMap<QString, QMdiSubWindow*> fileMap_;
	uint newFileIndex_;
	LocationHistory history_;
	bool windowActivationBlocked_;
	Editor::Config config_;
	Editor::Actions actions_;

	/**
	 * Displays the current cursor position in the status bar.
	 */
	QLabel* cursorPositionLabel_;

	/**
	 * Displays the edit mode for the current editor.
	 */
	QLabel* editModeLabel_;

	bool gotoLocationInternal(const Core::Location&);
	Editor::Editor* findEditor(const QString&);
	Editor::Editor* createEditor(const QString&);
	void blockWindowActivation(bool);

	static inline Editor::Editor* editorFromWindow(QMdiSubWindow* window) {
		if (!window)
			return NULL;

		return static_cast<Editor::Editor*>(window->widget());
	}

private slots:
	void handleWindowAction(QAction*);
	void windowActivated(QMdiSubWindow*);
	void removeEditor(const QString&);
	void remapEditor(const QString&, const QString&);
	void showCursorPosition(int, int);
	void showEditMode(Editor::ViScintilla::EditMode);
};

} // namespace App

} // namespace KScope

#endif // __APP_EDITORCONTAINER_H
