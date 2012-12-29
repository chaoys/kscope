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

#ifndef __KSCOPE_STACKWIDGET_H__
#define __KSCOPE_STACKWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>
#include <QLinkedList>
#include "ui_stackpage.h"

namespace KScope
{

namespace App
{

/**
 * A single page in a stack widget.
 * The page consists of a managed widget and a title bar. The title bar includes
 * a title for the page, and a "View" and "Close" buttons.
 */
class StackPage : public QWidget, public Ui::StackPage
{
	Q_OBJECT

public:
	StackPage(QWidget*, QWidget* parent = 0);
	~StackPage();

	/**
	 * @return The managed widget
	 */
	QWidget* widget() { return widget_; }

public slots:
	void showWidget();
	void hideWidget();

signals:
	/**
	 * Emitted when the widget is displayed.
	 * @param page A pointer to this page
	 */
	void activated(StackPage* page);

	/**
	 * Emitted when the page is closed.
	 * @param page A pointer to this page
	 */
	void removed(StackPage* page);

private:
	/**
	 * The managed widget.
	 */
	QWidget* widget_;

private slots:
	void remove();
};

/**
 * Manages multiple widgets in a vertical stack.
 * This widget is similar to QToolBox. Unlike QToolBox, however, each page has
 * a "Show" and "Close" buttons.
 * @author Elad Lahav
 */
class StackWidget : public QWidget
{
	Q_OBJECT

public:
	StackWidget(QWidget* parent = 0);
	~StackWidget();

	void addWidget(QWidget*);
	QList<QWidget*> widgets() const;
	void removeAll();

	/**
	 * @return The widget of the active page, NULL if no page exists
	 */
	QWidget* currentWidget() {
		if (activePage_)
			return activePage_->widget();
		return NULL;
	}

private:
	/**
	 * Vertical ayout for the pages.
	 */
	QVBoxLayout* layout_;

	/**
	 * Keeps a list of pages.
	 * This is used to determine the active page when the current one is closed.
	 */
	QLinkedList<StackPage*> pageList_;

	/**
	 * The currently active page.
	 * This is the only page whose widget is visible.
	 */
	StackPage* activePage_;

private slots:
	void setActivePage(StackPage*);
	void removePage(StackPage*);
};

}

}

#endif // __KSCOPE_STACKWIDGET_H__
