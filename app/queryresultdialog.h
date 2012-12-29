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

#ifndef __APP_QUERYRESULTDIALOG_H__
#define __APP_QUERYRESULTDIALOG_H__

#include <QDialog>
#include "ui_queryresultdialog.h"

namespace KScope
{

namespace App
{

/**
 * @author Elad Lahav <elad_lahav@users.sourceforge.net>
 */
class QueryResultDialog : public QDialog, private Ui::QueryResultDialog
{
	Q_OBJECT

public:
	QueryResultDialog(QWidget* parent = 0);
	~QueryResultDialog();

	Core::QueryView* view() { return view_; }
};

} // namespace App

} // namespace KScope

#endif // __APP_QUERYRESULTDIALOG_H__
