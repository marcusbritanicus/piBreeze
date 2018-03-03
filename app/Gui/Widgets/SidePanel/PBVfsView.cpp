/*
	*
	* PBVfsView.cpp - Mounted VirtualFS View class for PiBreeze
	*
*/

#include "PBVfsView.hpp"

inline QString getDevType( PBDeviceInfo info ) {

	QString dev = info.device();
	QString vfsType = info.fileSystemType();

	QStringList cdTypes = QStringList() << "cdfs" << "iso9660" << "udf";
	QString devType = QString( "unknown" );

	if ( cdTypes.contains( vfsType ) )
		return ":/icons/optical.png";

	if ( vfsType.contains( "encfs" ) )
		return ":/icons/encfs.png";

	if ( vfsType.contains( "archivemount" ) )
		return ":/icons/archive.png";

	QDir disks = QDir( "/dev/disk/by-path" );
	disks.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System );
	foreach( QString disk, disks.entryList() ) {
		QFileInfo info( disks.filePath( disk ) );
		if ( info.symLinkTarget() == dev ) {
			if ( info.absoluteFilePath().contains( "usb" ) )
				return QString( ":/icons/usb.png" );

			else {
				if ( vfsType.toLower().contains( "ntfs" ) )
					return QString( ":/icons/hdd-win.png" );

				else if ( vfsType.toLower().contains( "fuseblk" ) )
					return QString( ":/icons/hdd-win.png" );

				else if ( vfsType.toLower().contains( "ext" ) )
					return QString( ":/icons/hdd-linux.png" );

				else if ( vfsType.toLower().contains( "jfs" ) )
					return QString( ":/icons/hdd-linux.png" );

				else if ( vfsType.toLower().contains( "reiser" ) )
					return QString( ":/icons/hdd-linux.png" );

				else if ( vfsType.toLower().contains( "zfs" ) )
					return QString( ":/icons/hdd-linux.png" );

				else if ( vfsType.toLower().contains( "xfs" ) )
					return QString( ":/icons/hdd-linux.png" );

				else if ( vfsType.toLower().contains( "btrfs" ) )
					return QString( ":/icons/hdd-linux.png" );

				else
					return QString( ":/icons/hdd.png" );
			}
		}
	}

	if ( devType == "unknown" ) {
		/*
			*
			* Lets try /sys/block approach
			*
			* Take /dev/<dev> part of the /dev/<dev> and check if 'usb' ia part of
			* target of /sys/block/<dev>. Else check the starting of <dev> and determine the type
			*
		*/
		QString sysfsPath = QString( "/sys/block/%1" ).arg( baseName( dev ) );
		if ( readLink( sysfsPath ).contains( "usb" ) )
			return QString( ":/icons/usb.png" );

		else {
			if ( baseName( dev ).startsWith( "sd" ) )
			// We have a generic mass storage device
				return QString( ":/icons/hdd.png" );

			else if ( baseName( dev ).startsWith( "sr" ) )
				return QString( ":/icons/optical.png" );

			else if ( baseName( dev ).startsWith( "se" ) or baseName( dev ).startsWith( "ses" ) )
				return QString( ":/icons/enclosure.png" );

			else if ( baseName( dev ).startsWith( "st" ) )
				return QString( ":/icons/tape.png" );

			else
				return devType;
		}
	}

	return devType;
};

PBVfsIcon::PBVfsIcon( QWidget *parent ) : QWidget( parent ) {

	/* Default Pixmap */
	mPixmap = QPixmap( ":/icons/encfs.png" );

	/* Flash settings */
	alpha = 0;
	mAlphaDelta = 30;
	color = palette().color( QPalette::Window ).darker();
	color.setAlpha( alpha );

	currentStep = 0;
	flashSteps = 10;
	mFlashDuration = 200;

	flash = false;
	flashesCompleted = 0;
	maxFlashes = 2;

	timer.setInterval( mFlashDuration / flashSteps );
	timer.setSingleShot( false );

	connect( &timer, SIGNAL( timeout() ), this, SLOT( repaint() ) );

	/* To track mouse movements */
	setMouseTracking( true );

	/* Set the fixed size of 48 px */
	setFixedSize( 48, 48 );

	/* Drag and Drop */
	setAcceptDrops( true );
	dndEntry = false;

	/* DeviceView */
	devView = new PBVfsMenu( this );
	devView->setObjectName( "PBVfsMenu" );
	connect( devView, SIGNAL( triggered( QAction* ) ), this, SLOT( clickDrive( QAction* ) ) );

	/* ToolTip */
	setToolTip( "Virtual FS" );
};

/* Override the QLabel pixmap property handlers */
QPixmap PBVfsIcon::pixmap() {

	return mPixmap;
};

void PBVfsIcon::setPixmap( QPixmap newPixmap ) {

	mPixmap = newPixmap;

	repaint();
};

/* Color property handlers */
QColor PBVfsIcon::flashColor() {

	return color;
};

void PBVfsIcon::setFlashColor( QColor newColor ) {

	color = newColor;
	color.setAlpha( alpha );

	repaint();
};

/* Alpha property handlers */
qint64 PBVfsIcon::alphaDelta() {

	return mAlphaDelta;
};

void PBVfsIcon::setAlphaDelta( qint64 newAlphaDelta ) {

	if ( ( newAlphaDelta * 10 ) > 255 )
		mAlphaDelta = 25;

	else
		mAlphaDelta = newAlphaDelta;
};

/* Flash duration property handlers */
int PBVfsIcon::flashDuration() {

	return mFlashDuration;
};

void PBVfsIcon::setFlashDuration( int newFlashDuration ) {

	mFlashDuration = ( newFlashDuration >= 10 ? newFlashDuration : 10 );
	timer.setInterval( mFlashDuration / flashSteps );
};

/* Number of Flashes property handlers */
int PBVfsIcon::numberOfFlashes() {

	return maxFlashes;
};

void PBVfsIcon::setNumberOfFlashes( int newNumOfFlashes ) {

	maxFlashes = newNumOfFlashes;
};

/* flashFrames property handlers - Number of frames per flash */
int PBVfsIcon::flashFrames() {

	return flashSteps;
};

void PBVfsIcon::setFlashFrames( int newFlashSteps ) {

	/* Persistence of vision is 62.5 (~63) ms */

	flashSteps = ( newFlashSteps > mFlashDuration ? mFlashDuration / 63 : newFlashSteps );
	timer.setInterval( mFlashDuration / flashSteps );
};

/* Overriding of paint event for showing flashes */
void PBVfsIcon::paintEvent( QPaintEvent *pEvent ) {

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
void PBVfsIcon::mousePressEvent( QMouseEvent *mEvent ) {

	if ( devView->isVisible() )
		devView->close();

	mEvent->accept();
};

/* Overriding QLabel::mouseMoveEvent to expand the view */
void PBVfsIcon::mouseMoveEvent( QMouseEvent *mEvent ) {

	mEvent->accept();
};

void PBVfsIcon::enterEvent( QEvent *eEvent ) {

	if ( dndEntry ) {
		eEvent->accept();
		return;
	}

	/* Start the delay timer */
	delayTimer.stop();
	delayTimer.start( 250, this );

	eEvent->accept();
};

void PBVfsIcon::leaveEvent( QEvent *eEvent ) {

	dndEntry = false;
	eEvent->accept();
};

void PBVfsIcon::dragEnterEvent( QDragEnterEvent *deEvent ) {

	dndEntry = true;

	/* Start the delay timer */
	delayTimer.stop();
	delayTimer.start( 250, this );

	deEvent->accept();
};

void PBVfsIcon::dragLeaveEvent( QDragLeaveEvent *dlEvent ) {

	dndEntry = false;
	dlEvent->accept();
};

void PBVfsIcon::timerEvent( QTimerEvent *tEvent ) {

	if ( tEvent->timerId() == delayTimer.timerId() ) {

		delayTimer.stop();
		if ( not devView->isVisible() and QRect( 0, 0, 48, 48 ).contains( mapFromGlobal( QCursor::pos() ) ) ) {

			showDevices();
		}
	}

	else if ( tEvent->timerId() == closeTimer.timerId() ) {

		closeTimer.stop();
		if ( devView->isVisible() )
			devView->close();
	}

	else
		QWidget::timerEvent( tEvent );

	tEvent->accept();
};

/* Slot to access the flashing */
void PBVfsIcon::flashLabel() {

	color = palette().color( QPalette::Window ).darker();
	color.setAlpha( alpha );

	if ( timer.isActive() )
		timer.stop();

	timer.start();
	flash = true;
};

/* Slot to access the flashing with a given color */
void PBVfsIcon::flashLabel( QColor newColor ) {

	setFlashColor( newColor );

	if ( timer.isActive() )
		timer.stop();

	timer.start();
	flash = true;
};

void PBVfsIcon::showDevices() {

	devView->clear();

	// Spacer Label
	QLabel *lbl = new QLabel( "<h4>&nbsp;&nbsp;&nbsp;&nbsp;VFS</h4>" );
	lbl->setFixedHeight( 48 );
	lbl->setMinimumWidth( 150 );

	QWidgetAction *lblAct = new QWidgetAction( devView );
	lblAct->setIcon( QIcon() );
	lblAct->setDefaultWidget( lbl );
	lblAct->setDisabled( true );

	devView->addAction( lblAct );

	Q_FOREACH( PBDeviceInfo info, PBDeviceManager::allVirtualMounts() ) {

		QWidgetAction *wa = new QWidgetAction( devView );
		wa->setData( info.mountPoint() );

		PBVfsAction *actWdgt = new PBVfsAction( info, devView );
		wa->setDefaultWidget( actWdgt );

		devView->addAction( wa );
	};

	closeTimer.start( 1500, this );
	devView->exec( mapToGlobal( QPoint( 49, 0 ) ) );
};

void PBVfsIcon::clickDrive( QAction *act ) {

	QWidgetAction *wAct = qobject_cast<QWidgetAction*>( act );
	PBVfsAction *devAct = qobject_cast<PBVfsAction*>( wAct->defaultWidget() );

	if ( devAct )
		emit driveClicked( devAct->mountPoint() );
};

PBVfsAction::PBVfsAction( PBDeviceInfo info, QWidget *parent ) : QWidget( parent ) {

	/* Mouse Tracking */
	setMouseTracking( true );

	/* Selection background */
	select = false;

	/* Fixed Item Heights */
	setFixedHeight( 32 );

	/* Width computation */
	QFontMetrics fm( font() );
	setMinimumWidth( qMax( 32 + fm.width( info.displayName() ) + 10, 150 ) );

	/*Data for Display  */
	mDeviceLabel = info.displayName();
	icon = QIcon( getDevType( info ) );
	mMountPoint = info.mountPoint();
	if ( not mMountPoint.endsWith( "/" ) )
		mMountPoint += "/";

	/* Disk Usage */
	percentUsed = 1.0 * info.bytesUsed() / info.bytesTotal();

	/* Enable drag and drop */
	setAcceptDrops( true );

	/* Set tooltip */
	setToolTip( mMountPoint );
};

QString PBVfsAction::mountPoint() {

	return mMountPoint;
};

void PBVfsAction::highlight( bool high ) {

	select = high;
	repaint();
}

void PBVfsAction::paintEvent( QPaintEvent *pEvent ) {

	QPainter painter( this );

	painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform );

	// Selection Background
	if ( select or hasFocus() ) {
		painter.save();
		QPalette pltt = palette();
		painter.setPen( Qt::NoPen );
		painter.setBrush( pltt.color( QPalette::Highlight ) );
		painter.drawRect( rect() );
		painter.restore();
	}

	// Label
	painter.save();
	painter.drawText( QRect( 32, 0, width() - 32 - 10, 20 ), Qt::AlignLeft | Qt::AlignVCenter, mDeviceLabel );
	painter.restore();

	// Icon
	painter.save();

	painter.setPen( Qt::NoPen );
	painter.drawPixmap( QRect( 4, 4, 24, 24 ), icon.pixmap( 24 ) );

	painter.restore();

	// Used/Free Size
	painter.save();

	int red = 0, green = 0;
	// Lots of free space
	if ( percentUsed <= 0.4 ) {
		green = ( int )( 255 );
		red = ( int )( percentUsed * 638 );
	}

	// Around 50% free space remains
	else if ( percentUsed <= 0.6 ) {
		red = 255;
		green = 255;
	}

	// Very less free space remaining
	else {
		green = ( int )( ( 1 - percentUsed ) * 638 );
		red = ( int )( 255 );
	}

	painter.setPen( Qt::NoPen );
	painter.setBrush( QColor( red, green, 0 ) );
	painter.drawRoundedRect( QRect( 32, 20, ( width() - 32 - 10 ) * percentUsed, 6 ), 4.0, 4.0 );

	painter.restore();

	painter.end();
};

/* Overriding QLabel::enterEvent to emit entered signal */
void PBVfsAction::enterEvent( QEvent *eEvent ) {

	select = true;
	repaint();
};

/* Overriding QLabel::leaveEvent to emit exited signal */
void PBVfsAction::leaveEvent( QEvent *lEvent ) {

	select = false;
	repaint();
};

void PBVfsAction::dragEnterEvent( QDragEnterEvent *deEvent ) {

	deEvent->acceptProposedAction();
};

void PBVfsAction::dragMoveEvent( QDragMoveEvent *dmEvent ) {

	const QMimeData *mData = dmEvent->mimeData();
	if ( not mData->hasUrls() ) {
		dmEvent->ignore();
		return;
	}

	if ( not isWritable( mMountPoint ) ) {
		dmEvent->ignore();
		return;
	}

	else if ( dirName( dmEvent->mimeData()->urls().at( 0 ).toLocalFile() ) == mMountPoint ) {
		dmEvent->ignore();
		return;
	}

	else {
		dmEvent->setDropAction( Qt::CopyAction );
		dmEvent->accept();
	}
};

void PBVfsAction::dropEvent( QDropEvent *dpEvent ) {

	if ( not dpEvent->mimeData()->hasUrls() ) {
		dpEvent->ignore();
		return;
	}

	if ( not isWritable( mMountPoint ) ) {
		dpEvent->ignore();
		return;
	}

	if ( dirName( dpEvent->mimeData()->urls().at( 0 ).toLocalFile() ) == mMountPoint ) {
		dpEvent->ignore();
		return;
	}

	PBProcess::Progress *progress = new PBProcess::Progress;
	progress->sourceDir = dirName( dpEvent->mimeData()->urls().at( 0 ).toLocalFile() );
	progress->targetDir = mMountPoint;

	QStringList srcList;
	Q_FOREACH( QUrl url, dpEvent->mimeData()->urls() )
		srcList << url.toLocalFile().replace( progress->sourceDir, "" );

	progress->type = PBProcess::Copy;

	PBIOProcess *proc = new PBIOProcess( srcList, progress );
	PBProcessManager::instance()->addProcess( progress, proc );

	progress->startTime = QTime::currentTime();

	proc->start();

	dpEvent->accept();
};

PBVfsMenu::PBVfsMenu( QWidget *parent ) : QMenu( parent ) {

	entered = false;
	connect( this, SIGNAL( hovered( QAction* ) ), this, SLOT( highlightAction( QAction* ) ) );
};

void PBVfsMenu::clear() {

	QMenu::clear();
	actionList.clear();
};

void PBVfsMenu::addAction( QWidgetAction *act ) {

	actionList << act;
	QMenu::addAction( act );
};

void PBVfsMenu::highlightAction( QAction *act ) {

	/* Remove highlighting from all QWidgetActions */
	Q_FOREACH( QWidgetAction *wa, actionList ) {
		PBVfsAction *devAct = qobject_cast<PBVfsAction*>( wa->defaultWidget() );
			if ( devAct )
				devAct->highlight( false );
	}

	/* Get the corresponding QWidgetAction */
	QWidgetAction *wAct = qobject_cast<QWidgetAction*>( act );
	if ( not wAct )
		return;

	/* Highlight only the one which is hovered */
	PBVfsAction *devAct = qobject_cast<PBVfsAction*>( wAct->defaultWidget() );
	if ( devAct )
		devAct->highlight( true );
};

void PBVfsMenu::enterEvent( QEvent *eEvent ) {

	entered = true;
	eEvent->accept();
};

void PBVfsMenu::close() {

	if ( not entered )
		QMenu::close();
};