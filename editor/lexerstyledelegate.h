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

#ifndef __EDITOR_LEXERSTYLEDELEGATE_H__
#define __EDITOR_LEXERSTYLEDELEGATE_H__

#include <QItemDelegate>
#include <QComboBox>
#include <QDebug>
#include "lexerstylemodel.h"

namespace KScope
{

namespace Editor
{

class StyleMenu : public QComboBox
{
	Q_OBJECT

public:
	StyleMenu(QWidget* parent) : QComboBox(parent) {
		connect(this, SIGNAL(currentIndexChanged(int)), this,
		        SLOT(notifyOnItemSelection()));

		insertItem(0, tr("<Select>"));
		insertItem(1, tr("Inherit"), LexerStyleModel::inheritValue());
		insertItem(2, tr("Custom..."));
	}

	void setCustomData(const QVariant& data) {
		setItemData(2, data);
	}

	QVariant currentData() const {
		return itemData(currentIndex());
	}

signals:
	void itemSelected(QWidget*);

private slots:
	void notifyOnItemSelection() {
		emit itemSelected(this);
	}
};

/**
 * Provides a mechanism for editing properties held in a LexerStyleModel model.
 * @author Elad Lahav
 */
class LexerStyleDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	LexerStyleDelegate(QObject* parent) : QItemDelegate(parent) {

	}

	~LexerStyleDelegate() {}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
	                      const QModelIndex& index) const {
		(void)option;
		(void)index;

		StyleMenu* menu = new StyleMenu(parent);
		connect(menu, SIGNAL(itemSelected(QWidget*)), this,
		        SIGNAL(commitData(QWidget*)));
		connect(menu, SIGNAL(itemSelected(QWidget*)), this,
		        SIGNAL(closeEditor(QWidget*)));
		return menu;
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const {
		StyleMenu* menu = static_cast<StyleMenu*>(editor);
		menu->setCustomData(index.model()->data(index, Qt::EditRole));
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model,
	                  const QModelIndex& index) const {
		// Get the selected combo-box item.
		StyleMenu* menu = static_cast<StyleMenu*>(editor);
		QVariant var = menu->currentData();

		// Do nothing for the "<Select>" item.
		if (!var.isValid())
			return;

		// The "Inherit" item, adjust the model.
		if (LexerStyleModel::isInheritValue(var)) {
			model->setData(index, var, Qt::EditRole);
			return;
		}

		// The "Custom..." item, signal the view to show an editor dialogue.
		emit editProperty(index, var);
	}

	void updateEditorGeometry(QWidget* editor,
	                          const QStyleOptionViewItem& option,
	                          const QModelIndex& index) const {
		(void)index;

		editor->setGeometry(option.rect);
	}

signals:
	void editProperty(const QModelIndex&, const QVariant&) const;
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_LEXERSTYLEDELEGATE_H__
