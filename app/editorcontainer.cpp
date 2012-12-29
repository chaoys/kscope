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

#include <QFileDialog>
#include <QStatusBar>
#include <QDebug>
#include <editor/configdialog.h>
#include "application.h"
#include "editorcontainer.h"
#include "queryresultdialog.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  parent  Parent widget
 */
EditorContainer::EditorContainer(QMainWindow* parent)
	: QMdiArea(parent),
	  currentWindow_(NULL),
	  newFileIndex_(1),
	  windowActivationBlocked_(false)
{
	// Load editor configuration settings.
	Settings& settings = Application::settings();
	settings.beginGroup("Editor");
	config_.load(settings);
	settings.endGroup();

	// Notify when an active editor is available.
	connect(this, SIGNAL(subWindowActivated(QMdiSubWindow*)), this,
	        SLOT(windowActivated(QMdiSubWindow*)));

	// Display the edit mode for the current editor the status bar.
	editModeLabel_ = new QLabel(tr("N/A"), this);
	parent->statusBar()->addPermanentWidget(editModeLabel_);

	// Display the current cursor position in the status bar.
	cursorPositionLabel_ = new QLabel(tr("Line: N/A Column: N/A"), this);
	parent->statusBar()->addPermanentWidget(cursorPositionLabel_);
}

/**
 * Class destructor.
 */
EditorContainer::~EditorContainer()
{
}

/**
 * Fills the "Window" menu with a list of all open sub-window titles.
 * @param  wndMenu  The menu to populate
 */
void EditorContainer::populateWindowMenu(QMenu* wndMenu) const
{
	// Add an entry for each open sub-window.
	QMap<QString, QMdiSubWindow*>::ConstIterator itr;
	for (itr = fileMap_.begin(); itr != fileMap_.end(); ++itr)
		wndMenu->addAction(itr.key());

	// Activate a sub-window when its menu entry is selected.
	connect(wndMenu, SIGNAL(triggered(QAction*)), this,
	        SLOT(handleWindowAction(QAction*)));
}

/**
 * Checks for any unsaved-changes in the currently open editors.
 * @return true if the application can terminate, false if the user cancels
 *         the operation due to unsaved changes
 */
bool EditorContainer::canClose()
{
	// TODO: Saving a file may take too long (e.g., for NFS-mounted files).
	// In this case, the application should not terminate until the file has
	// been saved. The current behaviour may lead to data loss!

	// Iterate over all editor windows.
	foreach (QMdiSubWindow* window, fileMap_) {
		Editor::Editor* editor = editorFromWindow(window);
		if (!editor->canClose())
			return false;
	}

	return true;
}

/**
 * Stores the locations of all editors in a session object.
 * @param  session  The session object to use
 */
void EditorContainer::saveSession(Session& session)
{
	Core::LocationList locList;

	// Create a list of locations for the open editors.
	foreach (QMdiSubWindow* window, fileMap_) {
		Editor::Editor* editor = editorFromWindow(window);

		Core::Location loc;
		editor->getCurrentLocation(loc);
		locList.append(loc);
	}

	session.setEditorList(locList);

	// Store the path of the currently active editor.
	if (currentEditor())
		session.setActiveEditor(currentEditor()->path());

	// Store the state of the active window.
	QMdiSubWindow* window = currentSubWindow();
	session.setMaxActiveEditor(window ? window->isMaximized() : false);
}

/**
 * Opens editors based on the locations stored in a session object.
 * @param  session  The session object to use
 */
void EditorContainer::loadSession(Session& session)
{
	const Core::LocationList& locList = session.editorList();
	Core::LocationList::ConstIterator itr;

	// Do not handle changes to the active editor while loading.
	blockWindowActivation(true);

	// Open an editor for each location.
	for (itr = locList.begin(); itr != locList.end(); ++itr)
		(void)gotoLocationInternal(*itr);

	// Activate the previously-active editor.
	// We have to call windowActivated() explicitly, in the case the active
	// window is the last one to be loaded. In that case, the signal will not
	// be emitted.
	QString activeEditor = session.activeEditor();
	if (!activeEditor.isEmpty())
		(void)findEditor(activeEditor);

	// Re-enable handling of changes to active windows.
	blockWindowActivation(false);

	// Maximise the active window, if required.
	if (session.maxActiveEditor())
		currentSubWindow()->showMaximized();
}

/**
 * Deletes all location items in the history list.
 */
void EditorContainer::clearHistory()
{
	history_.clear();
}

/**
 * Creates an editor window with an empty, unnamed file.
 */
void EditorContainer::newFile()
{
	(void)createEditor(QString());
}

/**
 * Prompts the user for a file name, and creates an editor window for editing
 * the selected file.
 */
void EditorContainer::openFile()
{
	QString path = QFileDialog::getOpenFileName(0, tr("Open File"));
	if (!path.isEmpty())
		(void)gotoLocationInternal(Core::Location(path));
}

/**
 * Displays the editor configuration dialogue.
 * Any changes to the configuration are then applied to all open editor windows.
 */
void EditorContainer::configEditor()
{
	// Show the "Configure Editor" dialogue.
	Editor::ConfigDialog dlg(config_, this);
	if (dlg.exec() == QDialog::Rejected)
		return;

	// Get the configuration parameters from the dialogue.
	dlg.getConfig(config_);

	// Store the editor configuration.
	Settings& settings = Application::settings();
	settings.beginGroup("Editor");
	config_.store(settings);
	settings.endGroup();

	// Apply new settings to all open editors.
	foreach (QMdiSubWindow* window, fileMap_)
		config_.apply(editorFromWindow(window));
}

/**
 * Sets the focus to a line in an editor window matching the given location.
 * @param  loc  The location to go to
 */
void EditorContainer::gotoLocation(const Core::Location& loc)
{
	// Get the current location.
	Core::Location curLoc;
	bool addCurrent = false;
	if (currentEditor()) {
		currentEditor()->getCurrentLocation(curLoc);
		addCurrent = true;
	}

	// Go to the new location.
	if (!gotoLocationInternal(loc))
		return;

	// Add both the previous and the new locations to the history list.
	if (addCurrent)
		history_.add(curLoc);
	history_.add(loc);
}

/**
 * Sets the focus to a line in an editor window matching the next location in
 * the history list.
 */
void EditorContainer::gotoNextLocation()
{
	Core::Location loc;
	if (history_.next(loc))
		(void)gotoLocationInternal(loc);
}

/**
 * Sets the focus to a line in an editor window matching the previous location
 * in the history list.
 */
void EditorContainer::gotoPrevLocation()
{
	Core::Location loc;
	if (history_.prev(loc))
		(void)gotoLocationInternal(loc);
}

/**
 * Shows a list of tags defined in the file of currently-active editor.
 */
void EditorContainer::showLocalTags()
{
	if (!currentEditor())
		return;

	// Create a query view dialogue.
	QueryResultDialog* dlg = new QueryResultDialog(this);
	dlg->setModal(true);

	// Go to selected locations.
	Core::QueryView* view = dlg->view();
	connect(view, SIGNAL(locationRequested(const Core::Location&)),
	        this, SLOT(gotoLocation(const Core::Location&)));

	dlg->setWindowTitle(tr("Local Tags"));
	dlg->show();

	try {
		// Run the query.
		Core::LocationModel* model = view->locationModel();
		model->setRootPath(ProjectManager::project()->rootPath());
		model->setColumns(ProjectManager::engine()
		                  .queryFields(Core::Query::LocalTags));
		ProjectManager::engine().query(view,
		                               Core::Query(Core::Query::LocalTags,
		                                           currentEditor()->path()));
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Shows a dialogue with the list of recently visited locations.
 */
void EditorContainer::browseHistory()
{
	// Construct the dialogue.
	QueryResultDialog dlg(this);
	dlg.setWindowTitle(tr("Location History"));

	// Add location history entries to the model.
	Core::QueryView* view = dlg.view();
	Core::LocationModel* model = view->locationModel();
	model->add(history_.list(), QModelIndex());

	// Setup the model's displayed columns.
	QList<Core::Location::Fields> columns;
	columns << Core::Location::File << Core::Location::Line
	        << Core::Location::Text;
	model->setColumns(columns);

	try {
		// Set the root path.
		model->setRootPath(ProjectManager::project()->rootPath());
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}

	view->resizeColumns();

	// Go to selected locations.
	connect(view, SIGNAL(locationRequested(const Core::Location&)),
	        this, SLOT(gotoLocation(const Core::Location&)));

	// Display the dialogue.
	dlg.exec();
}

/**
 * Changes the current editor and cursor position to the given location.
 * Creates a new editor window if one is not currently open for the location's
 * file.
 * @param  loc The location to go to
 * @return true if successful, false otherwise
 */
bool EditorContainer::gotoLocationInternal(const Core::Location& loc)
{
	// Get an editor for the given file.
	// If one does not exists, create a new one.
	Editor::Editor* editor = findEditor(loc.file_);
	if (editor == NULL) {
		editor = createEditor(loc.file_);
		if (editor == NULL)
			return false;
	}

	// Set the cursor position for the editor.
	editor->moveCursor(loc.line_, loc.column_);
	return true;
}

/**
 * Finds an editor sub-window matching the file.
 * @param  path      The path of the file to edit
 * @return The found editor if successful, NULL otherwise
 */
Editor::Editor* EditorContainer::findEditor(const QString& path)
{
	// Try to find an existing editor window, based on the path.
	QMap<QString, QMdiSubWindow*>::Iterator itr = fileMap_.find(path);
	if (itr == fileMap_.end())
		return NULL;

	// Get the editor widget for the window.
	QMdiSubWindow* window = *itr;
	Editor::Editor* editor = editorFromWindow(window);

	// Activate the window.
	if (window != currentSubWindow())
		setActiveSubWindow(window);
	else
		editor->setFocus();

	return editor;
}

/**
 * Creates a new editor sub-window for the given file.
 * @param  path The path to the file to edit
 * @return The editor widget if successful, false otherwise
 */
Editor::Editor* EditorContainer::createEditor(const QString& path)
{
	Editor::Editor* editor = new Editor::Editor(this);

	// Open the given file in the editor.
	if (!path.isEmpty()) {
		if (!editor->load(path, config_.lexer(path))) {
			delete editor;
			return NULL;
		}
	}
	else {
		// No path supplied, treat as a new file.
		editor->setNewFileIndex(newFileIndex_++);
	}

	// Set configuration parameters.
	config_.apply(editor);

	// Handle editor closing/name changes.
	connect(editor, SIGNAL(closed(const QString&)), this,
			SLOT(removeEditor(const QString&)));
	connect(editor, SIGNAL(titleChanged(const QString&, const QString&)),
	        this, SLOT(remapEditor(const QString&, const QString&)));

	// Show editor messages in the status bar.
	connect(editor, SIGNAL(message(const QString&, int)),
	        static_cast<QMainWindow*>(parent())->statusBar(),
	        SLOT(showMessage(const QString&, int)));

	// Create a new sub window for the editor.
	QMdiSubWindow* window = addSubWindow(editor);
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->setWindowTitle(editor->title());
	window->show();
	fileMap_[editor->title()] = window;

	return editor;
}

/**
 * Enables/disables handling of changes to the active editor in
 * windowActivated().
 * This is useful when doing mass-update operations, such as loading a session
 * or closing all files, to prevent unnecessary actions during the operation.
 * When activation is re-enabled, windowActivated() is called directly for the
 * currently active sub-window.
 * @param  block true to disable handling, false to enable
 */
void EditorContainer::blockWindowActivation(bool block)
{
	if (block) {
		windowActivationBlocked_ = true;
	}
	else {
		windowActivationBlocked_ = false;
		windowActivated(currentSubWindow());
	}
}

/**
 * Closes all editor windows.
 */
void EditorContainer::closeAll()
{
	// Prompt the user for unsaved changes.
	if (!canClose())
		return;

	// Do not handle changes to the active editor while closing.
	blockWindowActivation(true);

	// Delete all editor windows.
	foreach (QMdiSubWindow* window, fileMap_)
		delete window;
	fileMap_.clear();

	// No current window.
	currentWindow_ = NULL;
	showCursorPosition(0, 0);
	showEditMode(Editor::ViScintilla::Disabled);
	emit hasActiveEditor(false);

	// Re-enable handling of changes to active windows.
	blockWindowActivation(false);
}

/**
 * Common handler for the file names in the "Window" menu.
 * Activates the window corresponding to the chosen file.
 * @param  action  The triggered action
 */
void EditorContainer::handleWindowAction(QAction* action)
{
	(void)gotoLocationInternal(Core::Location(action->text()));
}

/**
 * Handles changes to the active editor window.
 * When an editor becomes active, it needs to get the keyboard focus. Also,
 * the container forwards certain signals to the active editor, which need to
 * be connected.
 * Note that this method is also called when the MDI area gains or loses focus.
 * If the latter happens, the active sub window becomes NULL, even though the
 * current sub window stays the same. This method is only concerned with changes
 * to the current sub window.
 * @param  window  The new active editor
 */
void EditorContainer::windowActivated(QMdiSubWindow* window)
{
	// Do nothing if activation signals are blocked.
	if (windowActivationBlocked_)
		return;

	// Do nothing if the active window is not the current one (i.e., a NULL
	// active window due to the MDI area losing focus).
	if (window != currentSubWindow())
		return;

	// Do nothing if the new current window is the same as the old one.
	if (window == currentWindow_)
		return;

	// Stop forwarding signals to the active editor.
	if (currentWindow_)
		disconnect(editorFromWindow(currentWindow_));

	// Remember the current window.
	currentWindow_ = window;

	// Update the active editor.
	Editor::Editor* editor = currentEditor();

	// Route editor actions to the active editor.
	actions_.setEditor(editor);

	if (!editor) {
		qDebug() << "No current editor";
		showCursorPosition(-1, -1);
		showEditMode(Editor::ViScintilla::Disabled);
		emit hasActiveEditor(false);
		return;
	}

	qDebug() << "Current editor" << (editor ? editor->path() : "");

	// Acquire keyboard focus.
	editor->setFocus();

	// Show information in the status bar.
	connect(editor, SIGNAL(cursorPositionChanged(int, int)), this,
	        SLOT(showCursorPosition(int, int)));
	connect(editor, SIGNAL(editModeChanged(Editor::ViScintilla::EditMode)),
	        this, SLOT(showEditMode(Editor::ViScintilla::EditMode)));

	// Update the current cursor position.
	// TODO: We have to update here, in case windowActivated() was called due
	// to an explicit user action. However, if called through
	// gotoLocationInternal(), the cursor position will change immediately after
	// returning. A better mechanism for updating the cursor position is
	// required.
	int line, column;
	editor->getCursorPosition(&line, &column);
	showCursorPosition(line, column);
	showEditMode(editor->editMode());

	emit hasActiveEditor(true);
}

/**
 * Removes an editor from the file map when its window is closed.
 * @param  title  The unique title of the editor being closed
 */
void EditorContainer::removeEditor(const QString& title)
{
	fileMap_.remove(title);
	qDebug() << title << "removed";
}

/**
 * Changes the map key for an editor window.
 * This slot is called when an editor changes its title (e.g., following a
 * "Save As" operation).
 * @param  oldTitle The previous title of the editor
 * @param  newTitle The new title of the editor
 */
void EditorContainer::remapEditor(const QString& oldTitle,
                                  const QString& newTitle)
{
	QMap<QString, QMdiSubWindow*>::Iterator itr = fileMap_.find(oldTitle);
	if (itr != fileMap_.end()) {
		QMdiSubWindow* window = *itr;
		fileMap_.remove(oldTitle);
		fileMap_[newTitle] = window;
	}
}

/**
 * Updates the current line and column numbers displayed in the status bar.
 * @param  line    The line number
 * @param  column  The column number
 */
void EditorContainer::showCursorPosition(int line, int column)
{
	QString text;
	if (line >= 0)
		text = QString(tr("Line: %1 ")).arg(line + 1);
	else
		text = tr("Line: N/A ");

	if (column >= 0)
		text += QString(tr("Column: %1 ")).arg(column + 1);
	else
		text += tr("Column: N/A ");

	cursorPositionLabel_->setText(text);
}

/**
 * Displays the edit mode of the current editor in the status bar.
 * @param  mode The mode to show
 */
void EditorContainer::showEditMode(Editor::ViScintilla::EditMode mode)
{
	switch (mode) {
	case Editor::ViScintilla::InsertMode:
		editModeLabel_->setText(tr("INSERT"));
		break;

	case Editor::ViScintilla::NormalMode:
		editModeLabel_->setText(tr("VI:NORMAL"));
		break;

	case Editor::ViScintilla::VisualMode:
		editModeLabel_->setText(tr("VI:VISUAL"));
		break;

	case Editor::ViScintilla::Disabled:
		editModeLabel_->setText(tr("N/A"));
		break;
	}
}

} // namespace App

} // namespace KScope
