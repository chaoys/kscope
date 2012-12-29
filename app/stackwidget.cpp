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

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "stackwidget.h"

namespace KScope
{

namespace App
{

/**
 * Class constructor.
 * @param  widget  The widget to display
 * @param  parent  Parent widget
 */
StackPage::StackPage(QWidget* widget, QWidget* parent)
	: QWidget(parent), Ui::StackPage(), widget_(widget)
{
	// Create the UI.
	setupUi(this);
	label_->setText(widget->windowTitle());
	layout()->addWidget(widget);

	// Handle title bar button clicks.
	connect(showButton_, SIGNAL(clicked()), this, SLOT(showWidget()));
	connect(closeButton_, SIGNAL(clicked()), this, SLOT(remove()));
}

/**
 * Class destructor.
 */
StackPage::~StackPage()
{
}

/**
 * Displays the widget.
 */
void StackPage::showWidget()
{
	widget_->show();
	emit activated(this);
}

/**
 * Hides the widget, leaving only the title bar visible.
 */
void StackPage::hideWidget()
{
	widget_->hide();
}

/**
 * Destroys the page.
 */
void StackPage::remove()
{
	emit removed(this);
	deleteLater();
}

/**
 * Class constructor.
 * @param  parent Parent widget
 */
StackWidget::StackWidget(QWidget* parent) : QWidget(parent), activePage_(NULL)
{
	layout_ = new QVBoxLayout;
	layout_->setSpacing(0);
	layout_->setContentsMargins(0, 0, 0, 0);
	setLayout(layout_);
}

/**
 * Class destructor.
 */
StackWidget::~StackWidget()
{
}

/**
 * Creates a new stack page that holds the given widget.
 * @param  widget The widget to add
 */
void StackWidget::addWidget(QWidget* widget)
{
	// Create a new page.
	StackPage* page = new StackPage(widget, this);
	pageList_.append(page);
	connect(page, SIGNAL(activated(StackPage*)), this,
	        SLOT(setActivePage(StackPage*)));
	connect(page, SIGNAL(removed(StackPage*)), this,
	        SLOT(removePage(StackPage*)));

	// Show the page and make it the active one.
	layout_->addWidget(page);
	page->show();
	setActivePage(page);
}

/**
 * Creates a list of all widgets managed by the stack.
 * @return A list containing all widgets in the stack
 */
QList<QWidget*> StackWidget::widgets() const
{
	QList<QWidget*> widgetList;
	QLinkedList<StackPage*>::ConstIterator itr;

	for (itr = pageList_.begin(); itr != pageList_.end(); ++itr)
		widgetList.append((*itr)->widget());

	return widgetList;
}

/**
 * Deletes all pages from the stack.
 */
void StackWidget::removeAll()
{
	while (!pageList_.isEmpty())
		delete pageList_.takeFirst();

	activePage_ = NULL;
}

/**
 * Changes the active page in the stack.
 * This slot is called when a page shows its widget and emits the activated()
 * signal. The method needs to hide the widget on the previously active page.
 * @param  page The newly activated page
 */
void StackWidget::setActivePage(StackPage* page)
{
	// Nothing to do if this is already the active page.
	if (activePage_ == page)
		return;

	// Hide the widget of the previously active page.
	if (activePage_)
		activePage_->hideWidget();

	// Set a new active page.
	activePage_ = page;
}

/**
 * Removes a page from the stack.
 * This slot is called when a page is closed and emits the removed() signal. If
 * it is the currently active page, a new active page is selected.
 * @param  page The page to remove
 */
void StackWidget::removePage(StackPage* page)
{
	// Remove from the list of pages.
	QLinkedList<StackPage*>::Iterator itr;
	for (itr = pageList_.begin(); itr != pageList_.end(); ++itr) {
		if (*itr == page) {
			itr = pageList_.erase(itr);
			break;
		}
	}

	// Handle the case that the removed page was the active one.
	if (page == activePage_) {
		activePage_ = NULL;

		// Determine the new page to show:
		// 1. The page immediately above, if any
		// 2. The page immediately below, if any
		// 3. The list is empty, don't show any page
		if (!pageList_.isEmpty()) {
			if (itr == pageList_.begin())
				(*itr)->showWidget();
			else
				(*(--itr))->showWidget();
		}
	}
}

}

}
