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

#ifndef __APP_BUILDPROGRESS_H__
#define __APP_BUILDPROGRESS_H__

#include <QProgressDialog>
#include <QDebug>
#include <core/engine.h>
#include <core/progressbar.h>

namespace KScope
{

namespace App
{

/**
 * Provides progress information on a build process.
 * Information is displayed in either a modal dialogue, or a progress-bar
 * widget (which can be embedded in other widgets, such as a status bar).
 * @author Elad Lahav
 */
class BuildProgress : public QObject, public Core::Engine::Connection
{
	Q_OBJECT

public:
	/**
	 * Class constructor.
	 */
	BuildProgress() : Core::Engine::Connection(), dlg_(NULL), bar_(NULL) {}

	/**
	 * Class destructor.
	 */
	~BuildProgress() {}

	/**
	 * Creates a display widget.
	 * @param  useDialog  true to create a dialogue, false for a progress bar
	 * @param  parent     The parent widget to use
	 * @return The created widget
	 */
	QWidget* init(bool useDialog, QWidget* parent) {
		if (!useDialog) {
			bar_ = new Core::ProgressBar(parent);
			connect(bar_, SIGNAL(cancelled()), this, SLOT(stopBuild()));
			bar_->setFrameShape(QFrame::NoFrame);
			bar_->setProgress(0, 0);
			bar_->setLabel(tr("Initialising..."));
			bar_->show();
			return bar_;
		}

		dlg_ = new QProgressDialog(parent);
		connect(dlg_, SIGNAL(canceled()), this, SLOT(stopBuild()));
		dlg_->setWindowTitle(tr("Building Project"));
		dlg_->setLabelText(tr("Initialising..."));
		dlg_->setModal(true);
		dlg_->show();
		return dlg_;
	}

	/**
	 * Does nothing, as no data is expected from a build process.
	 * @param  locList  ignored
	 */
	void onDataReady(const Core::LocationList& locList) {
		(void)locList;
	}

	/**
	 * Deletes the widget when the build process ends.
	 */
	void onFinished() {
		qDebug() << __func__;
		if (dlg_)
			delete dlg_;
		if (bar_)
			delete bar_;

		dlg_ = NULL;
		bar_ = NULL;
	}

	/**
	 * Deletes the widget when the build process ends.
	 */
	void onAborted() {
		qDebug() << __func__;
		if (dlg_)
			delete dlg_;
		if (bar_)
			delete bar_;

		dlg_ = NULL;
		bar_ = NULL;
	}

	/**
	 * Updates progress information.
	 * @param  text  Progress message
	 * @param  cur   Current value
	 * @param  total Expected final value
	 */
	void onProgress(const QString& text, uint cur, uint total) {
		qDebug() << __func__ << text << cur << total;
		if (dlg_) {
			dlg_->setRange(0, total);
			dlg_->setValue(cur);
			dlg_->setLabelText(text);
		}
		if (bar_) {
			bar_->setProgress(cur, total);
			bar_->setLabel(text);
		}
	}

private:
	/**
	 * A progress dialogue.
	 */
	QProgressDialog* dlg_;

	/**
	 * A progress-bar.
	 */
	Core::ProgressBar* bar_;

private slots:
	/**
	 * Stops the build process when the "Cancel" button is clicked in either the
	 * dialogue or the progress-bar.
	 */
	void stopBuild() { stop(); }
};

} // namespace App

} // namespace KScope

#endif // __APP_BUILDPROGRESS_H__
