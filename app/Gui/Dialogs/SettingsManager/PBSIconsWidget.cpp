/*
	*
	* PBSGeneralWidget.cpp - General Settings Widget for PiBreeze
	*
*/

#include "PBSIconsWidget.hpp"
#include "PBMimeTypeModel.hpp"

PBIconThemeWidget::PBIconThemeWidget( QWidget *parent ) :QWidget( parent ) {

	createGUI();
};

void PBIconThemeWidget::createGUI() {

	iconThemesWidget = new PBIconThemeChooserWidget( this );
	folderViewWidget = new PBIconThemeViewerWidget( this );

	connect( iconThemesWidget, SIGNAL( reloadIcons() ), folderViewWidget, SIGNAL( setupModel() ) );

	QVBoxLayout *wLyt = new QVBoxLayout();
	wLyt->addWidget( iconThemesWidget );
	wLyt->addWidget( folderViewWidget );

	setLayout( wLyt );
};

PBIconThemeChooserWidget::PBIconThemeChooserWidget( QWidget *parent ) : QWidget( parent ) {

	QHBoxLayout *wLyt = new QHBoxLayout();

	prevBtn = new PBButton( QIcon::fromTheme( "arrow-left", QIcon( ":/icons/arrow-left.png" ) ), this );
	prevBtn->setFocusPolicy( Qt::NoFocus );
	prevBtn->setShortcut( tr( "Ctrl+P" ) );

	nextBtn = new PBButton( QIcon::fromTheme( "arrow-right", QIcon( ":/icons/arrow-right.png" ) ), this );
	nextBtn->setFocusPolicy( Qt::NoFocus );
	nextBtn->setShortcut( tr( "Ctrl+N" ) );

	themeCB = new QComboBox();
	themeCB->setFocusPolicy( Qt::NoFocus );

	loadThemes();

	connect( prevBtn, SIGNAL( clicked() ), this, SLOT( previousTheme() ) );
	connect( nextBtn, SIGNAL( clicked() ), this, SLOT( nextTheme() ) );
	connect( themeCB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( switchTheme( int ) ) );

	wLyt->addStretch( 0 );
	wLyt->addWidget( prevBtn );
	wLyt->addWidget( themeCB );
	wLyt->addWidget( nextBtn );
	wLyt->addStretch( 0 );

	setLayout( wLyt );
};

void PBIconThemeChooserWidget::loadThemes() {

	themesList.clear();

	QDir iconDir( "/usr/share/icons" );
	iconDir.setFilter( QDir::Dirs );

	foreach ( QString theme, iconDir.entryList() ) {
		if ( QFileInfo( iconDir.filePath( theme ) ).exists() ) {
			if ( QSettings( iconDir.filePath( theme + "/index.theme" ), QSettings::IniFormat ).value( "Icon Theme/Directories" ).toStringList().count() )
				themesList << theme;
		}
	}

	themeCB->addItems( themesList );
	current = themesList.indexOf( Settings->General.IconTheme );
	themeCB->setCurrentIndex( current );
};

void PBIconThemeChooserWidget::switchTheme( int idx ) {

	current = idx;

	QIcon::setThemeName( themesList[ current ] );
	Settings->setValue( "IconTheme", themesList[ current ] );

	emit reloadIcons();
};

void PBIconThemeChooserWidget::nextTheme() {

	if ( current + 1 == themesList.count() ) {
		current = 0;
		themeCB->setCurrentIndex( current );
	}
	else {
		current++;
		themeCB->setCurrentIndex( current );
	}

	QIcon::setThemeName( themesList[ current ] );
	Settings->setValue( "IconTheme", themesList[ current ] );
	emit reloadIcons();
};

void PBIconThemeChooserWidget::previousTheme() {

	if ( current == 0 ) {
		current = themesList.count() - 1;
		themeCB->setCurrentIndex( current );
	}
	else {
		current--;
		themeCB->setCurrentIndex( current );
	}

	QIcon::setThemeName( themesList[ current ] );
	Settings->setValue( "IconTheme", themesList[ current ] );
	emit reloadIcons();
};

PBIconThemeModel::PBIconThemeModel( QObject *parent ) : QAbstractListModel( parent ) {

	QTimer::singleShot( 10, this, SLOT( setupModel() ) );
};

int PBIconThemeModel::rowCount( const QModelIndex &parent ) const {

	if ( parent == QModelIndex() )
		return mimeNameList.count();

	else
		return 0;
};

QVariant PBIconThemeModel::data( const QModelIndex &index, int role ) const {

	int mimeIndex = index.row();

	switch( role ) {
		case Qt::DisplayRole:
			return mimeNameList.at( mimeIndex );

		case Qt::DecorationRole:
			return mimeIconList.at( mimeIndex );

		case Qt::ToolTipRole :
			return mimeTypeList.at( mimeIndex );

		default:
			return QVariant();
	}
};

void PBIconThemeModel::setupModel() {

	mimeNameList.clear();
	mimeIconList.clear();
	mimeTypeList.clear();

	beginResetModel();
	QSettings mdb1( PBXdg::userDir( PBXdg::XDG_CACHE_HOME ) + "PiBreeze/mimetypes.db", QSettings::NativeFormat );
	QSettings mdb2( ":/data/PBFSExtData.conf", QSettings::NativeFormat );
	Q_FOREACH( QString ext, mdb2.allKeys() ) {
		QString name = mdb2.value( ext ).toStringList().at( 0 );
		QString mime = mdb2.value( ext ).toStringList().at( 1 );
		if ( not mdb1.contains( mime ) )
			continue;

		QIcon ico;
		Q_FOREACH( QString path, mdb1.value( mime ).toStringList() )
			ico.addFile( path );

		mimeNameList << name;
		mimeIconList << ico;
		mimeTypeList << mime;

		qApp->processEvents();
	}
	endResetModel();

	emit layoutChanged();
};

PBIconThemeViewerWidget::PBIconThemeViewerWidget( QWidget *parent ) : QListView( parent ) {

	// View, Sizes and Resize Modes
	setViewMode( QListView::IconMode );
	setGridSize( QSize( 120, 96 ) );
	setIconSize( QSize( 48, 48 ) );
	setFlow( QListView::LeftToRight );

	setSpacing( 9 );
	setResizeMode( QListView::Adjust );
	setMovement( QListView::Static );

	// DragAndDrop
	setDragDropMode( QAbstractItemView::NoDragDrop );

	// Selection
	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setSelectionRectVisible( true );

	// Wrapping
	setWordWrap( true );
	setWrapping( true );

	// Uniform Item Sizes
	setUniformItemSizes( true );

	PBIconThemeModel *model = new PBIconThemeModel( this );
	connect( this, SIGNAL( setupModel() ), model, SLOT( setupModel() ) );
	connect( model, SIGNAL( layoutChanged() ), this, SLOT( update() ) );

	setModel( model );
};
