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

#include <QDockWidget>
#include <QCloseEvent>
#include <QStatusBar>
#include <QFileDialog>
#include <cscope/managedproject.h>
#include <editor/editor.h>
#include "mainwindow.h"
#include "editorcontainer.h"
#include "querydialog.h"
#include "queryresultdock.h"
#include "projectmanager.h"
#include "queryresultdialog.h"
#include "session.h"
#include "projectdialog.h"
#include "openprojectdialog.h"
#include "projectfilesdialog.h"
#include "configenginesdialog.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 */
MainWindow::MainWindow() : QMainWindow(), actions_(this)
{
	// Set the window title.
	// This changes whenever a project is opened/closed.
	setWindowTitle(false);

	// The main window icon.
	setWindowIcon(QIcon(":/images/kscope"));

	// Create the central widget (the editor manager).
	editCont_ = new EditorContainer(this);
	setCentralWidget(editCont_);

	// Create the query result dock.
	queryDock_ = new QueryResultDock(this);
	addDockWidget(Qt::RightDockWidgetArea, queryDock_);
	connect(queryDock_, SIGNAL(locationRequested(const Core::Location&)),
	        editCont_, SLOT(gotoLocation(const Core::Location&)));

	// Create the query dialogue.
	queryDlg_ = new QueryDialog(this);

	// Create a status bar.
	statusBar();

	// Initialise actions.
	// The order is important: make sure the child widgets are created BEFORE
	// calling setup().
	actions_.setup();

	// Apply saved window settings.
	readSettings();

	// Perform actions when a project is opened or closed.
	connect(ProjectManager::signalProxy(), SIGNAL(hasProject(bool)), this,
	        SLOT(projectOpenedClosed(bool)));

	// Rebuild the project when signalled by the project manager.
	connect(ProjectManager::signalProxy(), SIGNAL(buildProject()), this,
	        SLOT(buildProject()));
}

/**
 * Class destrutor.
 */
MainWindow::~MainWindow()
{
}

/**
 * Prompts the user for query information, and starts a query with the entered
 * parameters.
 * @param  type  The default query type to use
 */
void MainWindow::promptQuery(Core::Query::Type type)
{
	queryDlg_->setWindowTitle(tr("Query"));

	// Get the default pattern from the text under the cursor on the active
	// editor (if any).
	Editor::Editor* editor = editCont_->currentEditor();
	if (editor)
		queryDlg_->setPattern(editor->currentSymbol());

	// Prompt the user.
	if (queryDlg_->exec(type) != QDialog::Accepted)
		return;

	// Start a query with results shown in a view inside the query dock.
	queryDock_->query(Core::Query(queryDlg_->type(), queryDlg_->pattern()),
	                  false);
}

/**
 * Implements a definition query that does not display its results in the
 * query dock.
 * If the query results in a single location, this location is immediately
 * displayed in the editor container. Otherwise, the user is prompted for the
 * location, using a results dialogue.
 */
void MainWindow::quickDefinition()
{
	QString symbol;

	// Get the default pattern from the text under the cursor on the active
	// editor (if any).
	Editor::Editor* editor = editCont_->currentEditor();
	if (editor)
		symbol = editor->currentSymbol();

	// Prompt for a symbol, if none is selected in the current editor.
	if (symbol.isEmpty()) {
		// Prompt for a symbol.
		queryDlg_->setWindowTitle(tr("Quick Definition"));
		QueryDialog::TypeList typeList;
		typeList << Core::Query::Definition;
		if (queryDlg_->exec(Core::Query::Definition, typeList)
		    != QDialog::Accepted) {
			return;
		}

		// Get the symbol from the dialogue.
		symbol = queryDlg_->pattern();
		if (symbol.isEmpty())
			return;
	}

	// Create a query view dialogue.
	QueryResultDialog* dlg = new QueryResultDialog(this);
	dlg->setModal(true);

	// Automatically select a single result.
	Core::QueryView* view = dlg->view();
	view->setAutoSelectSingleResult(true);

	// Forward selected locations to the editor container.
	connect(view, SIGNAL(locationRequested(const Core::Location&)),
	        editCont_, SLOT(gotoLocation(const Core::Location&)));

	// Only show the dialogue if needed.
	connect(view, SIGNAL(needToShow()), dlg, SLOT(show()));

	try {
		// Run the query.
		Core::LocationModel* model = view->locationModel();
		model->setRootPath(ProjectManager::project()->rootPath());
		model->setColumns(ProjectManager::engine()
		                  .queryFields(Core::Query::Definition));
		ProjectManager::engine().query(view,
		                               Core::Query(Core::Query::Definition,
		                                           symbol));
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Prompts the user for a symbol to create a call-tree for.
 */
void MainWindow::promptCallTree()
{
	queryDlg_->setWindowTitle(tr("Call Tree"));

	// Get the default pattern from the text under the cursor on the active
	// editor (if any).
	Editor::Editor* editor = editCont_->currentEditor();
	if (editor)
		queryDlg_->setPattern(editor->currentSymbol());

	// Prompt the user.
	QueryDialog::TypeList typeList;
	typeList << Core::Query::CalledFunctions << Core::Query::CallingFunctions;
	if (queryDlg_->exec(Core::Query::CalledFunctions, typeList)
	    != QDialog::Accepted) {
		return;
	}

	// Start a query with results shown in a view inside the query dock.
	queryDock_->query(Core::Query(queryDlg_->type(), queryDlg_->pattern()),
	                  true);
}

/**
 * Starts a build process for the current project's engine.
 * Provides progress information in either a modal dialogue or a progress-bar
 * in the window's status bar. The modal dialogue is used for initial builds,
 * while the progress-bar is used for rebuilds.
 */
void MainWindow::buildProject()
{
	try {
		// Create a build progress widget.
		if (ProjectManager::engine().status() == Core::Engine::Build) {
			buildProgress_.init(true, this);
		}
		else {
			QWidget* widget = buildProgress_.init(false, this);
			widget->setMaximumHeight(statusBar()->height() - 4);
			statusBar()->addWidget(widget);
		}

		// Start the build process.
		ProjectManager::engine().build(&buildProgress_);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

/**
 * Opens the given file for editing.
 * @param  path  The path of the requested file
 */
void MainWindow::openFile(const QString& path)
{
	editCont_->gotoLocation(Core::Location(path));
}

/**
 * Handles the "Project->New..." action.
 * Closes the current project, and displays the "New Project" dialogue.
 */
void MainWindow::newProject()
{
	// If an active project exists, it needs to be closed first.
	if (ProjectManager::hasProject()) {
		QString msg = tr("The active project needs to be closed.\n"
                         "Would you like to close it now?");
		int result = QMessageBox::question(this,
		                                   tr("Close Project"),
		                                   msg,
		                                   QMessageBox::Yes | QMessageBox::No);
		if (result == QMessageBox::No || !closeProject())
			return;
	}

	// Show the "New Project" dialogue.
	ProjectDialog dlg(this);
	dlg.setParamsForProject<Cscope::ManagedProject>(NULL);
	if (dlg.exec() == QDialog::Rejected)
		return;

	// Get the new parameters from the dialogue.
	Core::ProjectBase::Params params;
	dlg.getParams<Cscope::ManagedProject>(params);

	try {
		// Create a project.
		Cscope::ManagedProject proj;
		proj.create(params);

		// Load the new project.
		ProjectManager::load<Cscope::ManagedProject>(params.projPath_);
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
		return;
	}
}

/**
 * Handles the "Project->Open" action.
 * Displays the "Open Project" dialogue.
 */
void MainWindow::openProject()
{
	// If an active project exists, it needs to be closed first.
	if (ProjectManager::hasProject()) {
		QString msg = tr("The active project needs to be closed.\n"
                         "Would you like to close it now?");
		int result = QMessageBox::question(this,
		                                   tr("Close Project"),
		                                   msg,
		                                   QMessageBox::Yes | QMessageBox::No);
		if (result == QMessageBox::No || !closeProject())
			return;
	}

	// Show the "Open Project" dialogue.
	OpenProjectDialog dlg(this);
	switch (dlg.exec()) {
	case OpenProjectDialog::Open:
		try {
			ProjectManager::load<Cscope::ManagedProject>(dlg.path());
		}
		catch (Core::Exception* e) {
			e->showMessage();
			delete e;
		}
		break;

	case OpenProjectDialog::New:
		newProject();
		break;

	default:
		;
	}
}

/**
 * Handles the "Project->Close" action.
 * Closes all editor windows, and saves the session (if it is part of a
 * project).
 * @return true if the session was closed, false if the user aborts the
 *         operation (e.g., chooses to cancel when prompted to save a modified
 *         editor)
 */
bool MainWindow::closeProject()
{
	// Check all editors for unsaved changes.
	if (!editCont_->canClose())
		return false;

	if (ProjectManager::hasProject()) {
		// Store session information.
		Session session(ProjectManager::project()->path());
		editCont_->saveSession(session);
		queryDock_->saveSession(session);
		queryDock_->closeAll();
		session.save();
	}

	// Close open editor windows.
	editCont_->closeAll();

	// Reset histories.
	queryDlg_->clear();
	editCont_->clearHistory();

	// Close the project.
	ProjectManager::close();
	return true;
}

/**
 * Handles the "Project->Files..." action.
 * Shows the "Project Files" dialogue.
 */
void MainWindow::projectFiles()
{
	ProjectFilesDialog dlg(this);
	dlg.exec();
}

/**
 * Handles the "Project->Properties..." action.
 * Shows the "Project Properties" dialogue.
 */
void MainWindow::projectProperties()
{
	// Get the active project.
	const Cscope::ManagedProject* project
		= dynamic_cast<const Cscope::ManagedProject*>
			(ProjectManager::project());
	if (project == NULL)
		return;

	// Create the project properties dialogue.
	ProjectDialog dlg(this);
	dlg.setParamsForProject(project);
	if (dlg.exec() == QDialog::Rejected)
		return;

	// Get the new parameters from the dialogue.
	Core::ProjectBase::Params params;
	dlg.getParams<Cscope::ManagedProject>(params);

	bool rebuild = false;
	try {
		// Update project properties.
		ProjectManager::updateConfig(params);

		// Check if the project needs to be rebuilt.
		if (ProjectManager::engine().status() == Core::Engine::Rebuild)
			rebuild = true;
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}

	// Prompt the user for rebuilding the project.
	if (rebuild) {
		QString msg = tr("This project needs to be rebuilt.\nWould you like to"
		                 " build it now?");
		if (QMessageBox::question(this, tr("Rebuild Prject"), msg,
		                          QMessageBox::Yes | QMessageBox::No)
		    == QMessageBox::Yes) {
			buildProject();
		}
	}
}

/**
 * Handles the "Settings->Configure Engines" action.
 */
void MainWindow::configEngines()
{
	ConfigEnginesDialog dlg(this);
	dlg.exec();
}

/**
 * Called before the main window closes.
 * @param  event  Information on the closing event
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!closeProject()) {
		event->ignore();
		return;
	}

	// Save main-window geometry.
	writeSettings();

	// Close the main window and terminate the application.
	event->accept();
}

/**
 * Stores configuration in the settings file.
 */
void MainWindow::writeSettings()
{
	// Store main window position and size.
	Settings& settings = Application::settings();
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("state", saveState());
	settings.endGroup();
}

/**
 * Loads configuration from the settings file.
 */
void MainWindow::readSettings()
{
	// Restore main window position and size.
	Settings& settings = Application::settings();
	settings.beginGroup("MainWindow");
	resize(settings.value("size", QSize(1000, 600)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}

/**
 * Changes the window title to reflect the availability of a project.
 * @param  hasProject  true if a project is currently open, false otherwise
 */
void MainWindow::setWindowTitle(bool hasProject)
{
	QString title = qApp->applicationName();
	if (hasProject)
		title += " - " + ProjectManager::project()->name();

	QMainWindow::setWindowTitle(title);
}

/**
 * Performs actions following the opening or closing of a project.
 * This slot is connected to the hasProject() signal emitted by the project
 * manager.
 * @param  opened  true if a project was opened, false if a project was closed
 */
void MainWindow::projectOpenedClosed(bool opened)
{
	// Adjust the window title.
	setWindowTitle(opened);

	// Nothing else to to if a project was closed.
	if (!opened)
		return;

	// Restore the session.
	Session session(ProjectManager::project()->path());
	session.load();
	editCont_->loadSession(session);
	queryDock_->loadSession(session);

	try {
		// Show the project files dialogue if files need to be added to the
		// project.
		if (ProjectManager::codebase().needFiles())
			projectFiles();
	}
	catch (Core::Exception* e) {
		e->showMessage();
		delete e;
	}
}

} // namespace App

} // namespace KScope
