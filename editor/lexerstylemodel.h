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

#ifndef __EDITOR_LEXERSTYLEMODEL_H__
#define __EDITOR_LEXERSTYLEMODEL_H__

#include <QAbstractItemModel>
#include <QSettings>
#include <qscilexer.h>
#include <core/treeitem.h>
#include "config.h"

namespace KScope
{

namespace Editor
{

/**
 * A model for displaying/editing lexer styles.
 * QScintilla uses QsciLexer-derived objects for drawing text. Each object
 * maintains a list of styles corresponding to syntactic entities in a
 * programming language. Unfortunately, the styles are not hierarchical, making
 * it quite difficult to express default values for certain properties (e.g.,
 * use a single font for all styles, except for comments).
 * This model attempts to overcome the limitations of QScintilla's style
 * management. It keeps styles as a tree, using the following levels:
 * (1) Common default style
 * (2) Per-language default style
 * (3) Syntactic styles
 * The root item, in level (1), corresponds to the default style of the common
 * lexer (@see CommonLexer). Each item in level (2) represents a lexer, and
 * manages the style marked as default for that lexer. Items in level (3)
 * correspond to all other (i.e., non-default) styles in a lexer.
 * Each style provides a set of properties that can be configured separately
 * (font, foreground colour, background colour). Additionally, each property can
 * be set to inherit the value of its parent style.
 * The model exposes two columns to views, with data for the first column
 * providing the name of the style, and the second providing a sample text
 * string that is formatted to use the styles properties. Additionally, a hidden
 * third column is used to expose another layer of the model, used for
 * manipulating properties. Thus, each model index that corresponds to a style
 * has two sets of children: its sub-styles, as described above, and its
 * properties. To access the properties, call index(0, 2, parent), where parent
 * is an index corresponding to a style. This will return a root index for
 * the set of properties. Using this index as a root of a tree view results in
 * a property editor.
 * Note that The model maintains style configuration separately from the actual
 * lexers, which are held by the editor Config object. Style configuration is
 * applied to the model directly. These changes can then be used to rebuild the
 * lexers.
 * @author Elad Lahav
 */
class LexerStyleModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	LexerStyleModel(const Config::LexerList&, QObject* parent = NULL);
	~LexerStyleModel();

	void load(QSettings&, bool);
	void store(QSettings&, bool) const;
	void copy(const LexerStyleModel&);
	void updateLexers() const;
	void applyInheritance(const QModelIndex&);

	// QAbstractItemModel implementation.
	QModelIndex index(int, int,
	                  const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex&) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex&, const QVariant&, int role);
	Qt::ItemFlags flags(const QModelIndex&) const;

	static QVariant inheritValue() { return inheritValue_; }

	static bool isInheritValue(const QVariant& var) {
		return var.toString() == inheritValue_;
	}

public slots:
	void resetStyles();

private:
	enum NodeType
	{
		StyleNode,
		PropertyNode
	};

	enum StyleProperty
	{
		Font = 0,
		Foreground,
		Background,
		_LastProperty
	};

	struct NodeData
	{
		virtual NodeType type() const = 0;
	};

	/**
	 * A node in the style tree.
	 */
	typedef Core::TreeItem<NodeData*> Node;

	/**
	 * Defines a property node.
	 * Property nodes are the children of a special property root node, which
	 * is embedded in every style node.
	 */
	struct PropertyData : public NodeData
	{
		/**
		 * Struct constructor.
		 * @param  prop The property represented by this node
		 */
		PropertyData(StyleProperty prop = _LastProperty)
			: prop_(prop), value_(), inherited_(false) {}

		/**
		 * @return The type of this node
		 */
		NodeType type() const { return PropertyNode; }

		/**
		 * The property ID.
		 */
		StyleProperty prop_;

		/**
		 * The current property value.
		 * This is always set to the actual value to use, regardless of whether
		 * the property is inherited or not.
		 */
		QVariant value_;

		/**
		 * Whether the property is inherited.
		 */
		bool inherited_;

		/**
		 * The style node that owns this property.
		 */
		Node* styleNode_;
	};

	/**
	 * Defines a node in the tree of styles.
	 */
	struct StyleData : public NodeData
	{
		/**
		 * Struct constructor.
		 * @param  lexer The lexer object to use
		 * @param  style The style ID, -1 to use the default style for this
		 *               lexer
		 */
		StyleData(QsciLexer* lexer = NULL, int style = -1)
			: lexer_(lexer), propRoot_(NULL) {
			// Determine the style ID.
			if ((lexer_ != NULL) && (style == -1))
				style_ = lexer->defaultStyle();
			else
				style_ = style;
		}

		/**
		 * @return The type of this node
		 */
		NodeType type() const { return StyleNode; }

		/**
		 * The lexer with which this style is associated.
		 */
		QsciLexer* lexer_;

		/**
		 * The ID of the style managed by this node.
		 */
		int style_;

		/**
		 * The root of a sub-tree holding property information.
		 */
		Node propRoot_;
	};

	/**
	 * The root of the style tree.
	 * Note that this is a fake node, whose only child is the common default
	 * style (i.e., the top-level style).
	 */
	Node root_;

	/**
	 * Converts a model index into a style tree node.
	 * @param  index The index to convert from
	 * @return The resulting node (may be NULL)
	 */
	Node* nodeFromIndex(const QModelIndex& index) {
		if (!index.isValid())
			return &root_;

		return static_cast<Node*>(index.internalPointer());
	}

	/**
	 * Converts a model index into a style tree node (const version).
	 * @param  index The index to convert from
	 * @return The resulting node (may be NULL)
	 */
	const Node* nodeFromIndex(const QModelIndex& index) const {
		if (!index.isValid())
			return &root_;

		return static_cast<const Node*>(index.internalPointer());
	}

	static inline PropertyData* propertyDataFromNode(const Node* node,
	                                         StyleProperty prop) {
		StyleData* style = static_cast<StyleData*>(node->data());
		Core::TreeItem<NodeData*>* propNode
			= static_cast<Core::TreeItem<NodeData*>*>
		      (style->propRoot_.child(prop));
		return static_cast<PropertyData*>(propNode->data());
	}

	/**
	 * Identifies the special property value, marking a property as inherited.
	 */
	static const QString inheritValue_;

	/**
	 * Used to verify that a settings object contains a valid style scheme.
	 */
	static const QString styleMagic_;

	Node* createStyleNode(Node*, QsciLexer*, int style = -1);
	void deleteStyleNode(Node*);
	void loadStyle(QSettings&, Node*);
	void storeStyle(QSettings&, const Node*) const;
	void copyStyle(Node*, const Node*);
	void updateLexerStyle(const Node*) const;
	void resetStyle(Node*);
	void setProperty(const QVariant&, Node*, StyleProperty, const QVariant&);
	void inheritProperty(const QVariant&, Node*, StyleProperty, bool);
	QVariant styleData(const Node*, int) const;
	QString propertyName(StyleProperty) const;
	QVariant::Type propertyType(StyleProperty) const;
	QVariant propertyData(PropertyData*, int role) const;
	QString propertyKey(StyleProperty) const;
	QVariant propertyDefaultValue(QsciLexer*, int, StyleProperty) const;
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_LEXERSTYLEMODEL_H__
