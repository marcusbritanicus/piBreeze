/*
	*
	* PBBookmarkView.cpp - A highly customized QWidget for PiBreeze
	*
*/

#include "PBBookmarkView.hpp"
#include "PBGuiFunctions.hpp"

PBBookmarksIcon::PBBookmarksIcon( QWidget *parent ) : QWidget( parent ) {

	// Default Pixmap
	mPixmap = QPixmap( ":/icons/bookmark.png" );

	// Flash settings
	alpha = 0;
	mAlphaDelta = 30;
	color = QColor( 255, 255, 255, 0 );

	currentStep = 0;
	flashSteps = 10;
	mFlashDuration = 200;

	flash = false;
	flashesCompleted = 0;
	maxFlashes = 2;

	createGUI();
};

void PBBookmarksIcon::createGUI() {

	timer.setInterval( mFlashDuration / flashSteps );
	timer.setSingleShot( false );

	connect( &timer, SIGNAL( timeout() ), this, SLOT( repaint() ) );

	// To track mouse movements
	setMouseTracking( true );

	// Set the fixed size of 48 px
	setFixedSize( 48, 48 );

	// BookmarksView
	bmkView = new PBBookmarkMenu( this );
	bmkView->setObjectName( "PBBookmarksMenu" );
	connect( bmkView, SIGNAL( triggered( QAction* ) ), this, SLOT( clickDrive( QAction* ) ) );
};

/* Override the QLabel pixmap property handlers */
QPixmap PBBookmarksIcon::pixmap() {

	return mPixmap;
};

void PBBookmarksIcon::setPixmap( QPixmap newPixmap ) {

	mPixmap = newPixmap;

	repaint();
};

/* Color property handlers */
QColor PBBookmarksIcon::flashColor() {

	return color;
};

void PBBookmarksIcon::setFlashColor( QColor newColor ) {

	color = newColor;
	color.setAlpha( alpha );

	repaint();
};

/* Alpha property handlers */
qint64 PBBookmarksIcon::alphaDelta() {

	return mAlphaDelta;
};

void PBBookmarksIcon::setAlphaDelta( qint64 newAlphaDelta ) {

	if ( ( newAlphaDelta * 10 ) > 255 )
		mAlphaDelta = 25;

	else
		mAlphaDelta = newAlphaDelta;
};

/* Flash duration property handlers */
int PBBookmarksIcon::flashDuration() {

	return mFlashDuration;
};

void PBBookmarksIcon::setFlashDuration( int newFlashDuration ) {

	mFlashDuration = ( newFlashDuration >= 10 ? newFlashDuration : 10 );
	timer.setInterval( mFlashDuration / flashSteps );
};

/* Number of Flashes property handlers */
int PBBookmarksIcon::numberOfFlashes() {

	return maxFlashes;
};

void PBBookmarksIcon::setNumberOfFlashes( int newNumOfFlashes ) {

	maxFlashes = newNumOfFlashes;
};

/* flashFrames property handlers - Number of frames per flash */
int PBBookmarksIcon::flashFrames() {

	return flashSteps;
};

void PBBookmarksIcon::setFlashFrames( int newFlashSteps ) {

	/* Persistence of vision is 62.5 (~63) ms */

	flashSteps = ( newFlashSteps > mFlashDuration ? mFlashDuration / 63 : newFlashSteps );
	timer.setInterval( mFlashDuration / flashSteps );
};

/* Overriding of paint event for showing flashes */
void PBBookmarksIcon::paintEvent( QPaintEvent *pEvent ) {

	QPainter *painter = new QPainter( this );
	painter->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform );

	if ( flash ) {
		if ( flashesCompleted == maxFlashes ) {
			timer.stop();
			flash = false;
			currentStep = 0;
			alpha = 0;
			flashesCompleted = 0;
		}

		else if ( currentStep == flashSteps ) {
			flashesCompleted += 1;
			currentStep = 0;
		}

		else if ( currentStep < flashSteps / 2 ) {
			currentStep += 1;
			alpha += mAlphaDelta;
			color.setAlpha( alpha );
		}

		else if ( currentStep >= ( flashSteps / 2 ) ) {
			currentStep += 1;
			alpha -= mAlphaDelta;
			color.setAlpha( alpha );
		}
	}

	painter->save();

	painter->setPen( Qt::transparent );
	painter->setBrush( color );
	painter->drawRect( 0, 0, width(), height() );

	if ( not mPixmap.isNull() ) {
		painter->setBrush( Qt::transparent );
		painter->drawPixmap( 8, 8, 32, 32, mPixmap );
	}

	painter->restore();
	painter->end();

	pEvent->accept();
};

/* Overriding QLabel::mousePressEvent to emit clicked signal */
void PBBookmarksIcon::mousePressEvent( QMouseEvent *mEvent ) {

	mEvent->accept();
};

/* Overriding QLabel::mouseMoveEvent to expand the view */
void PBBookmarksIcon::mouseMoveEvent( QMouseEvent *mEvent ) {

	mEvent->accept();
};

/* Overriding QLabel::enterEvent to emit entered signal */
void PBBookmarksIcon::enterEvent( QEvent *eEvent ) {

	if ( not QRect( 0, 0, 48, 48 ).contains( mapFromGlobal( QCursor::pos() ) ) )
		return;

	/* Start the delay timer */
	delayTimer.stop();
	delayTimer.start( 250, this );

	eEvent->accept();
};

/* Overriding QLabel::leaveEvent to emit exited signal */
void PBBookmarksIcon::leaveEvent( QEvent *lEvent ) {

	/* If the mouse is in the bmkView, don't contract */
	if ( bmkView->isVisible() ) {
		if ( bmkView->rect().contains( bmkView->mapFromGlobal( QCursor::pos() ) ) )
			return;
	}

	/* Start the delay timer */
	delayTimer.stop();
	delayTimer.start( 250, this );

	lEvent->accept();
};

void PBBookmarksIcon::timerEvent( QTimerEvent *tEvent ) {

	if ( tEvent->timerId() == delayTimer.timerId() ) {

		delayTimer.stop();
		if ( not bmkView->isVisible() and QRect( 0, 0, 48, 48 ).contains( mapFromGlobal( QCursor::pos() ) ) ) {

			showBookmarks();
		}
	}

	else if ( tEvent->timerId() == closeTimer.timerId() ) {

		closeTimer.stop();
		if ( bmkView->isVisible() )
			bmkView->close();
	}

	else
		QWidget::timerEvent( tEvent );

	tEvent->accept();
};

/* Slot to access the flashing */
void PBBookmarksIcon::flashLabel() {

	if ( colorFlash ) {
		colorFlash = false;
		color = QColor( 255, 255, 255, 0 );
	}

	if ( timer.isActive() )
		timer.stop();

	timer.start();
	flash = true;
};

/* Slot to access the flashing with a given color */
void PBBookmarksIcon::flashLabel( QColor newColor ) {

	setFlashColor( newColor );

	if ( timer.isActive() )
		timer.stop();

	timer.start();
	flash = true;
};

void PBBookmarksIcon::showBookmarks() {

	bmkView->clear();

	// Spacer Label
	QLabel *lbl = new QLabel( "<h4>&nbsp;&nbsp;&nbsp;&nbsp;Bookmarks</h4>" );
	lbl->setFixedHeight( 48 );
	lbl->setMinimumWidth( 150 );

	QWidgetAction *lblAct = new QWidgetAction( bmkView );
	lblAct->setIcon( QIcon() );
	lblAct->setDefaultWidget( lbl );
	lblAct->setDisabled( true );

	bmkView->addAction( lblAct );

	Q_FOREACH( PBBookmarkInfo info, PBBookmarkInfo::allBookmarks() ) {
		QAction *act = new QAction( icon( PBIconManager::instance()->icon( "bookmarks" ) ), info.displayLabel, bmkView );
		act->setData( info.mountPoint );

		bmkView->addAction( act );
	}

	closeTimer.start( 1500, this );
	bmkView->exec( mapToGlobal( QPoint( 49, 0 ) ) );
};

void PBBookmarksIcon::clickDrive( QAction *act ) {

	QString mtpt = act->data().toString();
	emit driveClicked( mtpt );
};

PBBookmarkMenu::PBBookmarkMenu( QWidget *parent ) : QMenu( parent ) {

	entered = false;
};

void PBBookmarkMenu::enterEvent( QEvent *eEvent ) {

	entered = true;
	eEvent->accept();
};

void PBBookmarkMenu::close() {

	if ( not entered )
		QMenu::close();
};
