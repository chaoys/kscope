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

#include <QMenuBar>
#include <editor/actions.h>
#include "actions.h"
#include "mainwindow.h"
#include "editorcontainer.h"
#include "queryresultdock.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  Pointer to the main window
 */
Actions::Actions(MainWindow* parent) : QObject(parent)
{
}

/**
 * Class destructor.
 */
Actions::~Actions()
{
}

/**
 * Creates the actions.
 * Must be called after the main window's child widgets have been created.
 */
void Actions::setup()
{
	QMenu* menu;
	QAction* action;

	// File menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&File"));

	// Create a new file.
	action = new QAction(tr("&New"), this);
	action->setShortcut(tr("Ctrl+N"));
	action->setStatusTip(tr("New file"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(newFile()));
	menu->addAction(action);

	// Open an existing file.
	action = new QAction(tr("&Open"), this);
	action->setShortcut(tr("Ctrl+O"));
	action->setStatusTip(tr("Open existing file"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
			SLOT(openFile()));
	menu->addAction(action);

	// Add editor-specific actions.
	mainWnd()->editCont_->actions().setupFileMenu(menu);

	// Close all open files.
	action = new QAction(tr("Close &All"), this);
	action->setStatusTip(tr("Close all open files"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
			SLOT(closeAll()));
	action->setEnabled(false);
	connect(mainWnd()->editCont_, SIGNAL(hasActiveEditor(bool)), action,
	        SLOT(setEnabled(bool)));
	menu->addAction(action);

	menu->addSeparator();

	// Exit the application.
	action = new QAction(tr("&Quit"), this);
	action->setShortcut(tr("Alt+F4"));
	action->setStatusTip(tr("Exit the application"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(close()));
	menu->addAction(action);

	// Edit menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&Edit"));
	mainWnd()->editCont_->actions().setupEditMenu(menu);

	// View menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&View"));

	// Exclusive group for setting the editor view mode.
	QActionGroup* viewModeGroup = new QActionGroup(this);
	connect(viewModeGroup, SIGNAL(triggered(QAction*)), this,
	        SLOT(setEditorViewMode(QAction*)));

	// Display editors as sub-windows.
	action = new QAction(tr("&Sub-Window Mode"), this);
	action->setStatusTip(tr("Display editors as sub-windows"));
	action->setCheckable(true);
	action->setChecked(mainWnd()->editCont_->viewMode()
	                   == QMdiArea::SubWindowView);
	action->setData(QMdiArea::SubWindowView);
	menu->addAction(action);
	viewModeGroup->addAction(action);

	// Display editors as tabs.
	action = new QAction(tr("&Tabbed Mode"), this);
	action->setStatusTip(tr("Display editors as tabs"));
	action->setCheckable(true);
	action->setChecked(mainWnd()->editCont_->viewMode()
	                   == QMdiArea::TabbedView);
	action->setData(QMdiArea::TabbedView);
	menu->addAction(action);
	viewModeGroup->addAction(action);

	menu->addSeparator();

	// Show/hide the query dock.
	menu->addAction(mainWnd()->queryDock_->toggleViewAction());

	// A group for project actions requiring an open project.
	// Only enabled when there is an active project.
	QActionGroup* projectGroup = new QActionGroup(this);
	connect(ProjectManager::signalProxy(), SIGNAL(hasProject(bool)),
	        projectGroup, SLOT(setEnabled(bool)));
	projectGroup->setEnabled(false);

	// Navigate menu.
	// Enabled only when there is an active project.
	menu = mainWnd()->menuBar()->addMenu(tr("&Navigate"));

	// Go to the next location in the navigation history.
	action = new QAction(tr("Next &Location"), this);
	action->setShortcut(tr("Alt+Right"));
	action->setStatusTip(tr("Next location in navigation history"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(gotoNextLocation()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Go to the previous location in the navigation history.
	action = new QAction(tr("Previous L&ocation"), this);
	action->setShortcut(tr("Alt+Left"));
	action->setStatusTip(tr("Previous location in navigation history"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(gotoPrevLocation()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Select the next query result.
	action = new QAction(tr("Next &Result"), this);
	action->setShortcut(tr("Alt+Down"));
	action->setStatusTip(tr("Select the next query result"));
	connect(action, SIGNAL(triggered()), mainWnd()->queryDock_,
	        SLOT(selectNextResult()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Select the previous query result.
	action = new QAction(tr("Previous R&esult"), this);
	action->setShortcut(tr("Alt+Up"));
	action->setStatusTip(tr("Select the previous query result"));
	connect(action, SIGNAL(triggered()), mainWnd()->queryDock_,
	        SLOT(selectPrevResult()));
	menu->addAction(action);
	projectGroup->addAction(action);

	menu->addSeparator();

	// Browse location history.
	action = new QAction(tr("&History"), this);
	action->setShortcut(tr("Ctrl+H"));
	action->setStatusTip(tr("rowse location history"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(browseHistory()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Project menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&Project"));

	// Create a new project.
	action = new QAction(tr("&New..."), this);
	action->setStatusTip(tr("New project"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(newProject()));
	menu->addAction(action);

	// Open an existing project.
	action = new QAction(tr("&Open..."), this);
	action->setShortcut(tr("Ctrl+P"));
	action->setStatusTip(tr("Open project"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(openProject()));
	menu->addAction(action);

	menu->addSeparator();

	// Manage project files.
	action = new QAction(tr("&Files..."), this);
	action->setShortcut(tr("Ctrl+Shift+O"));
	action->setStatusTip(tr("Manage project files"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(projectFiles()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// View/edit project parameters.
	action = new QAction(tr("&Properties..."), this);
	action->setStatusTip(tr("View/edit project parameters"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(projectProperties()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Build the project's database.
	action = new QAction(tr("&Build Database"), this);
	action->setShortcut(tr("Ctrl+B"));
	action->setStatusTip(tr("Build the project database"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(buildProject()));
	menu->addAction(action);
	projectGroup->addAction(action);

	menu->addSeparator();

	// Close the active project.
	action = new QAction(tr("&Close"), this);
	action->setStatusTip(tr("Close the active project"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(closeProject()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Query menu.
	// Enabled only when there is an active project.
	menu = mainWnd()->menuBar()->addMenu(tr("&Query"));

	// Quick definition.
	// Needs an active editor to be enabled.
	action = new QAction(tr("&Quick Definition"), this);
	action->setShortcut(tr("Ctrl+]"));
	action->setStatusTip(tr("Find symbol definition"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(quickDefinition()));
	action->setEnabled(false);
	connect(mainWnd()->editCont_, SIGNAL(hasActiveEditor(bool)), action,
	        SLOT(setEnabled(bool)));
	menu->addAction(action);
	projectGroup->addAction(action);

	menu->addSeparator();

	// A group for all query actions.
	// Handles all of these actions with a single slot.
	QActionGroup* queryGroup = new QActionGroup(this);
	connect(queryGroup, SIGNAL(triggered(QAction*)), this,
	        SLOT(query(QAction*)));
	connect(ProjectManager::signalProxy(), SIGNAL(hasProject(bool)),
			queryGroup, SLOT(setEnabled(bool)));
	queryGroup->setEnabled(false);

	// Query references.
	action = new QAction(tr("&References"), queryGroup);
	action->setShortcut(tr("Ctrl+0"));
	action->setStatusTip(tr("Find all symbol references"));
	action->setData(Core::Query::References);
	menu->addAction(action);

	// Query definition.
	action = new QAction(tr("&Definition"), queryGroup);
	action->setShortcut(tr("Ctrl+1"));
	action->setStatusTip(tr("Find symbol definition"));
	action->setData(Core::Query::Definition);
	menu->addAction(action);

	// Query called functions.
	action = new QAction(tr("&Called Functions"), queryGroup);
	action->setShortcut(tr("Ctrl+2"));
	action->setStatusTip(tr("Show functions called from function"));
	action->setData(Core::Query::CalledFunctions);
	menu->addAction(action);

	// Query calling functions.
	action = new QAction(tr("C&alling Functions"), queryGroup);
	action->setShortcut(tr("Ctrl+3"));
	action->setStatusTip(tr("Find functions calling function"));
	action->setData(Core::Query::CallingFunctions);
	menu->addAction(action);

	// Query text.
	action = new QAction(tr("&Text"), queryGroup);
	action->setShortcut(tr("Ctrl+4"));
	action->setStatusTip(tr("Find text in files"));
	action->setData(Core::Query::Text);
	menu->addAction(action);

	// Find file.
	action = new QAction(tr("&File"), queryGroup);
	action->setShortcut(tr("Ctrl+7"));
	action->setStatusTip(tr("Find a file"));
	action->setData(Core::Query::FindFile);
	menu->addAction(action);

	// Find including files.
	action = new QAction(tr("&Including Files"), queryGroup);
	action->setShortcut(tr("Ctrl+8"));
	action->setStatusTip(tr("Find files #including a given file"));
	action->setData(Core::Query::IncludingFiles);
	menu->addAction(action);

	// Show local tags.
	action = new QAction(tr("Local &Tags"), this);
	action->setShortcut(tr("Ctrl+T"));
	action->setStatusTip(tr("List tags in the current file"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(showLocalTags()));
	connect(mainWnd()->editCont_, SIGNAL(hasActiveEditor(bool)), action,
	        SLOT(setEnabled(bool)));
	action->setEnabled(false);
	menu->addAction(action);
	projectGroup->addAction(action);

	menu->addSeparator();

	// Call tree.
	action = new QAction(tr("Call &Tree"), this);
	action->setShortcut(tr("Ctrl+\\"));
	action->setStatusTip(tr("Create a call tree"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(promptCallTree()));
	menu->addAction(action);
	projectGroup->addAction(action);

	// Settings menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&Settings"));

	// Editor configuration.
	action = new QAction(tr("Configure &Editor"), this);
	action->setStatusTip(tr("Modify editor settings"));
	connect(action, SIGNAL(triggered()), mainWnd()->editCont_,
	        SLOT(configEditor()));
	menu->addAction(action);

	// Engine configuration.
	action = new QAction(tr("Configure E&ngines"), this);
	action->setStatusTip(tr("Manage engine configuration"));
	connect(action, SIGNAL(triggered()), mainWnd(), SLOT(configEngines()));
	menu->addAction(action);

	// Dynamically-created Window menu.
	wndMenu_ = mainWnd()->menuBar()->addMenu(tr("&Window"));
	connect(wndMenu_, SIGNAL(aboutToShow()), this, SLOT(showWindowMenu()));

	// Help menu.
	menu = mainWnd()->menuBar()->addMenu(tr("&Help"));

	action = new QAction(tr("&About..."), this);
	action->setStatusTip(tr("Show application information"));
	connect(action, SIGNAL(triggered()), qApp, SLOT(about()));
	menu->addAction(action);

}

/**
 * A common handler for query menu actions.
 * @param  action  The triggered action
 */
void Actions::query(QAction* action)
{
	Core::Query::Type type;

	type = static_cast<Core::Query::Type>(action->data().toUInt());
	mainWnd()->promptQuery(type);
}

/**
 * Constructs a sub-menu showing all open editor windows.
 * Called before the "Window" menu is shown.
 */
void Actions::showWindowMenu()
{
	wndMenu_->clear();
	mainWnd()->editCont_->populateWindowMenu(wndMenu_);
}

/**
 * Determines the view mode of the editor container.
 * This is a common handler to the "View->Sub-Window Mode" and "View->Tabbed
 * Mode" actions.
 * @param  action The triggerred action
 */
void Actions::setEditorViewMode(QAction* action)
{
	QMdiArea::ViewMode mode =
		static_cast<QMdiArea::ViewMode>(action->data().toUInt());
	mainWnd()->editCont_->setViewMode(mode);
}

/**
 * @return Pointer to the main window
 */
MainWindow* Actions::mainWnd()
{
	return static_cast<MainWindow*>(parent());
}

} // namespace App

} // namespace KScope
