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
#include <core/exception.h>
#include "lexerstylemodel.h"
#include "config.h"

namespace KScope
{

namespace Editor
{

const QString LexerStyleModel::inheritValue_ = "<Inherit>";

const QString LexerStyleModel::styleMagic_ = "Grqt237gva5FA8A3";

/**
 * Class constructor.
 * @param  parent Parent object
 */
LexerStyleModel::LexerStyleModel(const Config::LexerList& lexers,
                                 QObject* parent)
	: QAbstractItemModel(parent), root_(NULL)
{
	// Build the style tree from the list of lexers.
	// The first object is assumed to be the common defaults lexer.
	Node* defNode = NULL;
	foreach (QsciLexer* lexer, lexers) {
		if (defNode == NULL) {
			// Create the root node.
			defNode = createStyleNode(&root_, lexer);
		}
		else {
			// Create the default style node for this lexer.
			Node* lexNode = createStyleNode(defNode, lexer);

			// Create per-style nodes for this lexer.
			for (int i = 0; !lexer->description(i).isEmpty(); i++) {
				// Skip the default style.
				if (i == lexer->defaultStyle())
					continue;

				createStyleNode(lexNode, lexer, i);
			}
		}
	}
}

/**
 * Class destructor.
 */
LexerStyleModel::~LexerStyleModel()
{
	deleteStyleNode(root_.child(0));
}

/**
 * Reads style data from a QSettings object
 * @param  settings  The object to read from
 * @param  force     Trust the given settings file, and try to load styles even
 *                   if the magic key is not found
 * @throw  Exception If force is not set, thrown in case the magic key is not
 *                   found
 */
void LexerStyleModel::load(QSettings& settings, bool force)
{
	// Check for a magic key, identifying the settings object as a valid style
	// scheme.
	settings.beginGroup("EditorStyles");
	if (!force) {
		if ((settings.status() != QSettings::NoError)
		    || (settings.value("KScopeStyleSchemeMagic", "").toString()
                != styleMagic_)) {
			settings.endGroup();
			throw new Core::Exception(tr("Not a valid style scheme"));
		}
	}

	// Recursively load styles.
	loadStyle(settings, root_.child(0));
	settings.endGroup();
}

/**
 * Writes style data to a QSettings object
 * @param  settings  The object to write to
 * @param  force     Ignore errors
 * @throw  Exception
 */
void LexerStyleModel::store(QSettings& settings, bool force) const
{
	if (!force && (settings.status() != QSettings::NoError))
		throw new Core::Exception(tr("Failed to write style scheme"));

	settings.beginGroup("EditorStyles");
	settings.setValue("KScopeStyleSchemeMagic", styleMagic_);
	storeStyle(settings, root_.child(0));
	settings.endGroup();
}

/**
 * Copies style data from another model.
 * @param  model The model to copy from
 */
void LexerStyleModel::copy(const LexerStyleModel& model)
{
	copyStyle(root_.child(0), model.root_.child(0));
}

/**
 * Uses the data stored in the model to update the style properties of all
 * managed lexers.
 * This method should be called after any changes are applied to the model.
 */
void LexerStyleModel::updateLexers() const
{
	updateLexerStyle(root_.child(0));
}

/**
 * Forces all child styles to inherit the given property.
 * @param  index Corresponds to a property node in the parent style
 */
void LexerStyleModel::applyInheritance(const QModelIndex& index)
{
	// Ensure the index corresponds to a property node.
	Node* node = nodeFromIndex(index);
	if ((node == NULL) || (node->data() == NULL)
	    || (node->data()->type() != PropertyNode)) {
		return;
	}

	// Apply inheritance to child styles.
	PropertyData* data = static_cast<PropertyData*>(node->data());
	inheritProperty(data->value_, data->styleNode_, data->prop_, true);
}

/**
 * Restores the default styles.
 */
void LexerStyleModel::resetStyles()
{
	resetStyle(root_.child(0));
	reset();
}

/**
 * Creates an index.
 * @param  row    The row of the index, relative to the parent
 * @param  column The column of the index
 * @param  parent The parent index.
 * @return The resulting index
 */
QModelIndex LexerStyleModel::index(int row, int column,
                                   const QModelIndex& parent) const
{
	const Node* node = nodeFromIndex(parent);
	if (node == NULL)
		return QModelIndex();

	// Root nodes do not have data.
	if (node->data() == NULL)
		return createIndex(row, column, (void*)node->child(row));

	// Property nodes do not have children.
	if (node->data()->type() == PropertyNode)
		return QModelIndex();

	// For column 2 on a style node, return an index representing the root
	// property node.
	if (column == 2) {
		StyleData* style = static_cast<StyleData*>(node->data());
		return createIndex(row, column, (void*)&style->propRoot_);
	}

	// Return an index representing a child style.
	return createIndex(row, column, (void*)node->child(row));
}

/**
 * Finds the parent for the given index.
 * @param  index The index for which the parent is needed
 * @return The parent index
 */
QModelIndex LexerStyleModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	const Node* node = nodeFromIndex(index);
	if (node == NULL)
		return QModelIndex();

	// Handle root nodes.
	if (node->data() == NULL)
		return QModelIndex();

	// Handle top-level style nodes.
	if (node->parent() == &root_)
		return QModelIndex();

	return createIndex(node->parent()->index(), 0, node->parent());
}

/**
 * Determines the number of child indices for the given parent.
 * @param  parent The parent index
 * @return The number of children for the index
 */
int LexerStyleModel::rowCount(const QModelIndex& parent) const
{
	const Node* node = nodeFromIndex(parent);
	if (node == NULL)
		return 0;

	// Handle root nodes.
	if (node->data() == NULL)
		return node->childCount();

	// Property nodes have no children.
	if (node->data()->type() == PropertyNode)
		return 0;

	// Column 2 is used for editing properties, so the number of children
	// is the number of available properties.
	if (parent.column() == 2) {
		StyleData* data = static_cast<StyleData*>(node->data());
		return data->propRoot_.childCount();
	}

	return node->childCount();
}

/**
 * Determines the number of columns in children of the given index.
 * This number is always 2.
 * @param  parent Ignored
 * @return The number of columns for children of the index
 */
int LexerStyleModel::columnCount(const QModelIndex& parent) const
{
	(void)parent;
	return 2;
}

/**
 * Provides the data to display/edit for a given index and role.
 * @param  index The index for which data is requested
 * @param  role  The requested role
 * @return The relevant data
 */
QVariant LexerStyleModel::data(const QModelIndex& index, int role) const
{
	const Node* node = nodeFromIndex(index);
	if (node == NULL || node->data() == NULL)
		return 0;

	if (node->data()->type() == StyleNode) {
		// Get the lexer and style ID for this node.
		StyleData* data = static_cast<StyleData*>(node->data());
		QsciLexer* lexer = data->lexer_;
		int style = data->style_;

		switch (index.column()) {
		case 0:
			// Show language name or style name in the first column.
			if (role == Qt::DisplayRole) {
				if (style == lexer->defaultStyle())
					return lexer->language();

				return lexer->description(style);
			}
			break;

		case 1:
			// Show a formatted text string in the second column, using the
			// style's properties.
			return styleData(node, role);
		}
	}
	else {
		// Get the lexer and style ID for this node.
		PropertyData* data = static_cast<PropertyData*>(node->data());

		switch (index.column()) {
		case 0:
			if (role == Qt::DisplayRole)
				return propertyName(data->prop_);
			break;

		case 1:
			return propertyData(data, role);
		}
	}

	return QVariant();
}

/**
 * Modifies a style's property.
 * The method updates the property to which the index corresponds, as well as
 * the same property for all child styles that inherit this property.
 * @param  index Corresponds to a property node
 * @param  value The new value to set
 * @param  role  Must be Qt::EditRole
 * @return true if the property was updated, false otherwise
 */
bool LexerStyleModel::setData(const QModelIndex& index, const QVariant& value,
                              int role)
{
	// Can only modify property nodes.
	Node* node = nodeFromIndex(index);
	if (!node || !node->data() || !node->data()->type() == PropertyNode)
		return false;

	if (role != Qt::EditRole)
		return false;

	// Set the property's value.
	PropertyData* data = static_cast<PropertyData*>(node->data());
	Node* styleNode = data->styleNode_;
	setProperty(value, styleNode, data->prop_, QVariant());

	// Update changes.
	emit dataChanged(index, index);
	QModelIndex styleIndex = createIndex(styleNode->index(), 1, styleNode);
	emit dataChanged(styleIndex, styleIndex);

	// Apply property to inheriting styles.
	inheritProperty(value, styleNode, data->prop_, false);

	return true;
}

Qt::ItemFlags LexerStyleModel::flags(const QModelIndex& index) const
{
	const Node* node = nodeFromIndex(index);
	if (!node || !node->data())
		return Qt::NoItemFlags;

	Qt::ItemFlags flags = Qt::NoItemFlags;

	switch (node->data()->type()) {
	case StyleNode:
		flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		break;

	case PropertyNode:
		flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		if (index.column() == 1)
			flags |= Qt::ItemIsEditable;
		break;
	}

	return flags;
}

/**
 * Allocates a new node for the given lexer and style.
 * Always use this method to create a new node, to ensure that the proeprties
 * for this style are also created.
 * @param  parent Pointer to the parent node
 * @param  lexer  The lexer to use
 * @param  style  The style ID
 * @return A pointer to the new node
 */
LexerStyleModel::Node* LexerStyleModel::createStyleNode(Node* parent,
                                                        QsciLexer* lexer,
                                                        int style)
{
	// Create the node.
	StyleData* data = new StyleData(lexer, style);
	Node* node = parent->addChild(data);

	// Create style properties.
	for (uint i = 0; i != _LastProperty; i++) {
		PropertyData* prop = new PropertyData(static_cast<StyleProperty>(i));
		prop->styleNode_ = node;
		data->propRoot_.addChild(prop);
	}

	return node;
}

/**
 * Recursively deteles style nodes.
 * @param  node The node to delete
 */
void LexerStyleModel::deleteStyleNode(Node* node)
{
	// Recursive call first.
	for (int i = 0; i < node->childCount(); i++)
		deleteStyleNode(node->child(i));

	// Delete all property data.
	StyleData* data = static_cast<StyleData*>(node->data());
	for (int i = 0; i < data->propRoot_.childCount(); i++) {
		Node* child = data->propRoot_.child(i);
		delete child->data();
		data->propRoot_.clear();
	}

	// Delete the node's data.
	delete data;
}

/**
 * Reads style data from a QSettings object for the given node and all its
 * children.
 * @param  settings The QSettings object to read from
 * @param  node     The style node
 */
void LexerStyleModel::loadStyle(QSettings& settings, Node* node)
{
	// Get the lexer and style ID from the node data.
	StyleData* data = static_cast<StyleData*>(node->data());
	QsciLexer* lexer = data->lexer_;
	int style = data->style_;

	// Create a key template for the settings object, of the form
	// LEXER\STYLE\%1, where %1 will be replaced by the property name.
	QString key = QString("Style/%1/%2/%3").arg(lexer->lexer()).arg(style);

	// Get the properties.
	for (uint i = 0; i != _LastProperty; i++) {
		StyleProperty prop = static_cast<StyleProperty>(i);
		setProperty(settings.value(key.arg(propertyKey(prop))), node, prop,
		            propertyDefaultValue(lexer, style, prop));
	}

	// Recursive call.
	for (int i = 0; i < node->childCount(); i++)
		loadStyle(settings, node->child(i));
}

/**
 * Writes style data to a QSettings object for the given node and all its
 * children.
 * @param  settings The QSettings object to write to
 * @param  node     The style node
 */
void LexerStyleModel::storeStyle(QSettings& settings, const Node* node) const
{
	// Get the lexer and style ID from the node data.
	StyleData* data = static_cast<StyleData*>(node->data());
	QsciLexer* lexer = data->lexer_;
	int style = data->style_;

	// Create a key template for the settings object, of the form
	// LEXER\STYLE\%1, where %1 will be replaced by the property name.
	QString key = QString("Style/%1/%2/%3").arg(lexer->lexer()).arg(style);

	// Get the properties.
	for (uint i = 0; i != _LastProperty; i++) {
		StyleProperty prop = static_cast<StyleProperty>(i);
		PropertyData* propData = propertyDataFromNode(node, prop);
		QVariant value = propData->inherited_ ? inheritValue_
		                                      : propData->value_;
		settings.setValue(key.arg(propertyKey(prop)), value);
	}

	// Recursive call.
	for (int i = 0; i < node->childCount(); i++)
		storeStyle(settings, node->child(i));
}

/**
 * Recursively copies style node data from another model.
 * @param  destNode The node to copy to
 * @param  srcNode  The node to copy from
 */
void LexerStyleModel::copyStyle(Node* destNode, const Node* srcNode)
{
	StyleData* destData = static_cast<StyleData*>(destNode->data());
	StyleData* srcData = static_cast<StyleData*>(srcNode->data());

	// Copy the style data.
	destData->lexer_ = srcData->lexer_;
	destData->style_ = srcData->style_;

	// Copy properties.
	for (uint i = 0; i != _LastProperty; i++) {
		StyleProperty prop = static_cast<StyleProperty>(i);
		PropertyData* destProp = propertyDataFromNode(destNode, prop);
		PropertyData* srcProp = propertyDataFromNode(srcNode, prop);

		destProp->value_ = srcProp->value_;
		destProp->inherited_ = srcProp->inherited_;
	}

	// Recursive call.
	for (int i = 0; i < destNode->childCount(); i++)
		copyStyle(destNode->child(i), srcNode->child(i));
}

/**
 * Recursively updates lexer style properties, using the data stored in the
 * model.
 * @param  node The style node holding the lexer to update
 */
void LexerStyleModel::updateLexerStyle(const Node* node) const
{
	StyleData* data = static_cast<StyleData*>(node->data());
	QsciLexer* lexer = data->lexer_;
	int style = data->style_;

	// Update lexer properties.
	QFont font = propertyDataFromNode(node, Font)->value_.value<QFont>();
	lexer->setFont(font, style);
	QColor foreground
		= propertyDataFromNode(node, Foreground)->value_.value<QColor>();
	lexer->setColor(foreground, style);
	QColor background
		= propertyDataFromNode(node, Background)->value_.value<QColor>();
	lexer->setPaper(background, style);

	// This is really nasty, but Scintilla leaves us no choice...
	// The EOL Fill flag needs to be set in order for whitespace past the end
	// of line to be drawn in the desired background colour. We apply this flag
	// to the default style, as well as any styles that have the same background
	// colour as the default.
	if ((style == lexer->defaultStyle())
	    || (lexer->paper(style) == lexer->paper(lexer->defaultStyle()))) {
		lexer->setEolFill(true, style);
	}

	// Recursive call.
	for (int i = 0; i < node->childCount(); i++)
		updateLexerStyle(node->child(i));
}

/**
 * Resets the style to use the default properties.
 * @param  node The style node to reset
 */
void LexerStyleModel::resetStyle(Node* node)
{
	StyleData* data = static_cast<StyleData*>(node->data());
	QsciLexer* lexer = data->lexer_;
	int style = data->style_;

	for (uint i = 0; i != _LastProperty; i++) {
		StyleProperty prop = static_cast<StyleProperty>(i);
		setProperty(QVariant(), node, prop,
		            propertyDefaultValue(lexer, style, prop));
	}

	// Recursive call.
	for (int i = 0; i < node->childCount(); i++)
		resetStyle(node->child(i));
}

/**
 * Assigns a value to a property.
 * The val parameter that is passed to this method can have one of three types
 * of values:
 * 1. A value that matches to the type of the property,
 * 2. The special string "<Inherit>",
 * 3. An invalid QVariant.
 * In the first case, the given value is set to the property. In the second, the
 * property value is set to that of the matching property as defined by the
 * parent style.
 * The third case happens when a property value is not defined (e.g., when
 * reading from an initial settings file). The property is set to the lexer's
 * value for that style, as passed in the defVal parameter. The method then
 * guesses whether the value is inherited, by comparing the value with that
 * of the parent style. This should give a good out-of-the-box default
 * behaviour.
 * @param  val    The value to assign
 * @param  node   The style node that owns the property
 * @param  prop   Property ID
 * @param  defVal A default value to use
 */
void LexerStyleModel::setProperty(const QVariant& val, Node* node,
                                  StyleProperty prop, const QVariant& defVal)
{
	PropertyData* data = propertyDataFromNode(node, prop);

	// Assign the value if its type matches the property type.
	if (val.type() == propertyType(prop)) {
		data->value_ = val;
		data->inherited_ = false;
		return;
	}

	// The root style cannot inherit values.
	// Use the default value, if it was properly defined. Otherwise, leave the
	// current value.
	if (node->parent() == &root_) {
		if (defVal.type() == propertyType(prop))
			data->value_ = defVal;
		data->inherited_ = false;
		return;
	}

	// Determine whether the value should be inherited.
	if (!isInheritValue(val)) {
		// No value was found in the settings object, and the property is
		// not marked as inherited.
		// Make an educated guess on whether the property should be
		// inherited, by checking if the parent value is the same as the
		// default one.
		if ((defVal.type() == propertyType(prop))
		    && (propertyDataFromNode(node->parent(), prop)->value_ != defVal)) {
			// Parent value differs, use the default one.
			data->value_ = defVal;
			data->inherited_ = false;
			return;
		}
	}

	// The value should be inherited.
	// Get the parent style's value for this property.
	data->value_ = propertyDataFromNode(node->parent(), prop)->value_;
	data->inherited_ = true;
}

/**
 * Recursively applies a property to all inheriting styles.
 * @param  val   The new property value
 * @param  node  The parent style node
 * @param  prop  The property to set
 * @param  force true to apply always inheritance, false to apply only if the
 *               property is currently marked as inherited
 */
void LexerStyleModel::inheritProperty(const QVariant& val, Node* node,
                                      StyleProperty prop, bool force)
{
	PropertyData* data = propertyDataFromNode(node, prop);
	for (int i = 0; i < node->childCount(); i++) {
		// Get the child node information.
		Node* child = node->child(i);
		PropertyData* childData = propertyDataFromNode(child, prop);

		// Force inheritance, if requested.
		if (force)
			childData->inherited_ = true;

		// Check if this property is inherited by the child.
		if (childData->inherited_) {
			// Set the new value.
			childData->value_ = data->value_;

			// Notify views of the change.
			QModelIndex index = createIndex(i, 1, (void*)child);
			emit dataChanged(index, index);

			// Recursive application.
			inheritProperty(val, child, prop, force);
		}
	}
}

/**
 * Creates a string with the style's font and colours to be displayed for the
 * second column of a style item.
 * @param  node The style node
 * @param  role The role to use
 * @return The data for the given style and role
 */
QVariant LexerStyleModel::styleData(const Node* node, int role) const
{
	switch (role) {
	case Qt::DisplayRole:
		return QString("Sample Text");

	case Qt::FontRole:
		return propertyDataFromNode(node, Font)->value_;

	case Qt::ForegroundRole:
		return propertyDataFromNode(node, Foreground)->value_;

	case Qt::BackgroundRole:
		return propertyDataFromNode(node, Background)->value_;

	default:
		;
	}

	return QVariant();
}

/**
 * @param  prop Property value
 * @return A display name for this property
 */
QString LexerStyleModel::propertyName(StyleProperty prop) const
{
	switch (prop) {
	case Font:
		return tr("Font");

	case Foreground:
		return tr("Text Colour");

	case Background:
		return tr("Background Colour");

	default:
		// Must not get here.
		Q_ASSERT(false);
	}

	return QString();
}

/**
 * @param  prop Property value
 * @return The QVariant type that is used to hold values for this property
 */
QVariant::Type LexerStyleModel::propertyType(StyleProperty prop) const
{
	switch (prop) {
	case Font:
		return QVariant::Font;

	case Foreground:
		return QVariant::Color;

	case Background:
		return QVariant::Color;

	default:
		// Must not get here.
		Q_ASSERT(false);
	}

	return QVariant::Invalid;
}

/**
 * @param  data Property data
 * @param  role The role to use
 * @return The value to return for this property
 */
QVariant LexerStyleModel::propertyData(PropertyData* data, int role) const
{
	switch (role) {
	case Qt::DisplayRole:
		if (data->inherited_)
			return tr("Inherit");
		if (propertyType(data->prop_) == QVariant::Font)
			return data->value_;
		break;

	case Qt::DecorationRole:
		if ((!data->inherited_)
			&& (propertyType(data->prop_) == QVariant::Color)) {
			return data->value_;
		}
		break;

	case Qt::EditRole:
		return data->value_;
	}

	return QVariant();
}

/**
 * @param  prop Property value
 * @return The key used to store this property in a QSettings object
 */
QString LexerStyleModel::propertyKey(StyleProperty prop) const
{
	switch (prop) {
	case Font:
		return "Font";

	case Foreground:
		return "Foreground";

	case Background:
		return "Background";

	default:
		// Must not get here.
		Q_ASSERT(false);
	}

	return QString();
}

/**
 * Determines the default value for a given style's property.
 * @param  lexer The lexer to which the style belong
 * @param  style The style to use
 * @param  prop  The property for which the value is requested
 * @return The default value of the property for this lexer and style
 */
QVariant LexerStyleModel::propertyDefaultValue(QsciLexer* lexer, int style,
                                               StyleProperty prop) const
{
	switch (prop) {
	case Font:
		return lexer->defaultFont(style);

	case Foreground:
		return lexer->defaultColor(style);

	case Background:
		return lexer->defaultPaper(style);

	default:
		// Must not get here.
		Q_ASSERT(false);
	}

	return QVariant();
}

} // namespace Editor

} // namespace KScope
