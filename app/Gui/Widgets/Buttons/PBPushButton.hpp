/*
	*
	* PBPushButton.cpp - An improved QCommandLinkButton for PiBreeze
	*
*/

#pragma once

#include <QtCore>
#include <QtGui>

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#endif

class PBPushButton : public QPushButton {
	Q_OBJECT

	public:
		PBPushButton( QWidget *parent = 0 );

		void setFont( QFont );

		QString text();
		void setText( QString );

		QString description();
		void setDescription( QString );

		QIcon icon();
		void setIcon( QIcon );

		QSize iconSize();
		void setIconSize( QSize );

		QSize minimumSizeHint();
		QSize sizeHint();

		bool isCheckable();
		void setCheckable( bool );

		bool isChecked();
		void setChecked( bool );

	private:
		void resetSize();

		QIcon mIcon;

		// Text
		QString mText;
		QString mDesc;

		// Button, Icon, Text and Description Size
		QSize mIdealSize;
		QSize mIconSize;
		QSize mTextSize;
		QSize mDescSize;

		int mWidth;

		// Button pressed and checked flags
		bool mCheckable;
		bool mBtnDown;
		bool mChecked;

		// Font and Font Metrics
		QFont textFont;
		QFontMetrics *fmb;
		QFontMetrics *fmn;

	protected:
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );
		void resizeEvent( QResizeEvent* );
		void paintEvent( QPaintEvent* );
};
