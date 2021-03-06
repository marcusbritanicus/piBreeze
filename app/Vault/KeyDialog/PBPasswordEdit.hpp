/*
	*
	* PBPasswordEdit.hpp - Password Input Dialog Class header
	*
*/

#pragma once

#ifdef STANDALONE
	#include "Global2.hpp"
#else
	#include "Global.hpp"
#endif
#include "PBPasswordInput.hpp"
#include "PBTools.hpp"

class PBPasswordEdit : public QLabel {
	Q_OBJECT

	public:
		PBPasswordEdit( QWidget *parent );

		/* Get out password */
		QString text();

		/* Set the password internally and display 10 hexagons */
		void setText( QString );

		/* Disable the widget, but set the text as 'Validated' */
		void setOkay();

		/* Reset the widget to its original state */
		void clear();

	private:
		QString mText;

	protected:
		void mousePressEvent( QMouseEvent *mEvent );
		void mouseReleaseEvent( QMouseEvent *mEvent );

	Q_SIGNALS:
		void passwordSet();
};
