/*
	*
	* PBTrashManager.cpp - PiBreeze FolderView Class
	*
*/

#include "PBTrashManager.hpp"

PBTrashManager::PBTrashManager( QWidget *parent ) : PBDialog( parent ) {

	ClipBoard = qApp->clipboard();

	setupGui();
	setDialogProperties();
};

void PBTrashManager::setupGui() {

	// Set Data Model
	tModel = new PBTrashModel();

	// Setup the view
	TrashView = new PBTrashView( tModel );

	// Buttons
	restoreButtons = new PBSegmentButton( QStringList() << "&Restore" << "Restore &All", this );
	connect( restoreButtons->segment( 0 ), SIGNAL( clicked() ), this, SLOT( restoreSelected() ) );
	connect( restoreButtons->segment( 1 ), SIGNAL( clicked() ), this, SLOT( restoreAll() ) );

	restoreButtons->segment( 0 )->setObjectName( "okBtn" );
	restoreButtons->segment( 1 )->setObjectName( "okBtn" );

	QList<QIcon> icons = QList<QIcon>() << QIcon::fromTheme( "edit-delete" ) << QIcon::fromTheme( "trash-empty" );

	deleteButtons = new PBSegmentButton( QStringList() << "&Delete" << "&Empty Trash", icons, this );
	connect( deleteButtons->segment( 0 ), SIGNAL( clicked() ), this, SLOT( deleteSelected() ) );
	connect( deleteButtons->segment( 1 ), SIGNAL( clicked() ), this, SLOT( emptyTrash() ) );

	deleteButtons->segment( 0 )->setObjectName( "abortBtn" );
	deleteButtons->segment( 1 )->setObjectName( "abortBtn" );

	// Layouts
	QHBoxLayout *btnLyt = new QHBoxLayout();
	btnLyt->setContentsMargins( QMargins() );
	btnLyt->setSpacing( 0 );

	btnLyt->addWidget( restoreButtons );
	btnLyt->addStretch();
	btnLyt->addWidget( deleteButtons );

	QVBoxLayout *widgetLyt = new QVBoxLayout();
	widgetLyt->setContentsMargins( QMargins() );
	widgetLyt->setSpacing( 0 );

	widgetLyt->addLayout( btnLyt );
	widgetLyt->addWidget( TrashView );

	QWidget *widget = new QWidget( this );
	widget->setLayout( widgetLyt );

	QHBoxLayout *baseLyt = new QHBoxLayout();
	baseLyt->addWidget( widget );
	setLayout( baseLyt );

	// Create and setup actions like copy, move, rename, etc, etc
	createAndSetupActions();

	// Do not accept focus
	setFocusPolicy( Qt::NoFocus );

	updateButtons();
};

void PBTrashManager::setDialogProperties() {

	setDialogTitle( "PiBreeze Trash Manager" );
	setDialogIcon( QIcon( ":/icons/pibreeze.png" ) );

	setMinimumSize( QSize( 900, 600 ) );
};

void PBTrashManager::createAndSetupActions() {

	connect( tModel, SIGNAL( restored( QModelIndexList ) ), this, SLOT( handleFailedRestore( QModelIndexList ) ) );

	reloadAct = new QAction( "Re&load", this );
	reloadAct->setShortcuts( PBSettings::instance()->Shortcuts.Reload );
	connect( reloadAct, SIGNAL( triggered() ), this, SLOT( doReload() ) );

	restoreAct = new QAction( "&Restore Selected", this );
	connect( restoreAct, SIGNAL( triggered() ), this, SLOT( restoreSelected() ) );

	restoreAllAct = new QAction( "Res&tore All", this );
	connect( restoreAllAct, SIGNAL( triggered() ), this, SLOT( restoreAll() ) );

	deleteAct = new QAction( "&Delete Selected", this );
	connect( deleteAct, SIGNAL( triggered() ), this, SLOT( deleteSelected() ) );

	emptyAct = new QAction( "&Empty Trash", this );
	connect( emptyAct, SIGNAL( triggered() ), this, SLOT( emptyTrash() ) );

	addAction( reloadAct );
	addAction( restoreAct );
	addAction( restoreAllAct );
	addAction( deleteAct );
	addAction( emptyAct );
};

void PBTrashManager::updateButtons() {

	if ( tModel->rowCount() ) {
		restoreButtons->setSegmentEnabled( 0 );
		restoreButtons->setSegmentEnabled( 1 );
		deleteButtons->setSegmentEnabled( 0 );
		deleteButtons->setSegmentEnabled( 1 );
	}

	else {
		restoreButtons->setSegmentDisabled( 0 );
		restoreButtons->setSegmentDisabled( 1 );
		deleteButtons->setSegmentDisabled( 0 );
		deleteButtons->setSegmentDisabled( 1 );
	}
};

QModelIndexList PBTrashManager::getSelection() {

	QModelIndexList selectedList;
	selectedList << TrashView->selectionModel()->selectedIndexes();
	Q_FOREACH( QModelIndex idx, selectedList )
		if ( idx.column() )
			selectedList.removeAt( selectedList.indexOf( idx ) );

	return selectedList;
};

bool PBTrashManager::hasSelection() {

	return TrashView->selectionModel()->hasSelection();
};

void PBTrashManager::doReload() {

	tModel->reload();
};

void PBTrashManager::handleFailedRestore( QModelIndexList failedIndexes ) {

	if ( not failedIndexes.count() )
		return;

	QTableWidget *table = new QTableWidget( 0, 2 );
	table->setFocusPolicy( Qt::NoFocus );

	table->verticalHeader()->hide();
	table->setHorizontalHeaderLabels( QStringList() << "File Name" << "Deleteion Date" );

	table->setShowGrid( false );
	table->setSelectionBehavior( QAbstractItemView::SelectRows );

	QHeaderView *headerView = new QHeaderView( Qt::Horizontal, table );
	table->setHorizontalHeader( headerView );
	#if QT_VERSION >= 0x050000
		headerView->setSectionResizeMode( 0, QHeaderView::Stretch );
		headerView->setSectionResizeMode( 1, QHeaderView::Fixed );
		table->verticalHeader()->setSectionResizeMode( QHeaderView::Fixed );
	#else
		headerView->setResizeMode( 0, QHeaderView::Stretch );
		headerView->setResizeMode( 1, QHeaderView::Fixed );
		table->verticalHeader()->setResizeMode( QHeaderView::Fixed );
	#endif

	table->setColumnWidth( 1, 200 );

	QStringList dirs, files;
	Q_FOREACH( QModelIndex idx, failedIndexes ) {
		PBTrashNode *node = static_cast<PBTrashNode*>( idx.internalPointer() );
		QTableWidgetItem *itm1 = new QTableWidgetItem( node->icon(), node->name() );
		QTableWidgetItem *itm2 = new QTableWidgetItem( node->deletionDate().toString( "MMM dd, yyyy hh:mm:ss" ) );

		itm1->setFlags( itm1->flags() & ~Qt::ItemIsEditable );
		itm2->setFlags( itm2->flags() & ~Qt::ItemIsEditable );

		table->insertRow( table->rowCount() );

		table->setItem( table->rowCount() - 1, 0, itm1 );
		table->setItem( table->rowCount() - 1, 1, itm2 );
	}

	PBMessageDialog::error( this,
		"PiBreeze - Error while restoring",
		"Some errors were encountered while restoring the files and folders from the trash. "			\
		"As a result, some of the files and folders have not have been restored. For the "				\
		"list for files and folders not restored click <tt><u>M</u>ore</tt>."
	);
};

void PBTrashManager::restoreSelected() {

	if ( hasSelection() )
		tModel->restore( getSelection() );

	updateButtons();
};

void PBTrashManager::restoreAll() {

	QModelIndexList selection;
	for( int row = 0; row < tModel->rowCount(); row++ )
		selection << tModel->index( row, 0 , TrashView->rootIndex() );

	tModel->restore( selection );

	updateButtons();
};

void PBTrashManager::deleteSelected() {

	if ( hasSelection() ) {
		int reply = PBMessageDialog::question( this,
			"Empty Trash?",
			"<p>You are about to remove all the selected files and folders in trash, from the disk. "
			"This operation cannot be undone, and the deleted data cannot be recoverd.</p>"
			"<p>Do you want to proceed?</p>"
		);

		if ( reply == PBMessageDialog::Yes )
			tModel->removeFromDisk( getSelection() );
	}

	updateButtons();
};

void PBTrashManager::emptyTrash() {

	int reply = PBMessageDialog::question( this,
		"Empty Trash?",
		"<p>You are about to remove all the files and folders in trash, from the disk. "
		"This operation cannot be undone, and the deleted data cannot be recoverd.</p>"
		"<p>Do you want to proceed?</p>"
	);

	if ( reply == PBMessageDialog::Yes ) {
		QModelIndexList selection;
		for( int row = 0; row < tModel->rowCount(); row++ )
			selection << tModel->index( row, 0 , TrashView->rootIndex() );

		tModel->removeFromDisk( selection );
	}

	updateButtons();
};
