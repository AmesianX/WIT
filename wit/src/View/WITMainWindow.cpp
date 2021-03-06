#include "WITMainWindow.h"

#include "../WITApplication.h"

// - Qt
#include <QtGui>

WITMainWindow::WITMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	this->setWindowTitle("WIT");
	this->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
	this->centralLayout = new QVBoxLayout();
	this->setLayout(this->centralLayout);
	this->viewWidget = new WITViewWidget();
	this->setCentralWidget(this->viewWidget);

	resize(800,600);
	
	this->createMenus();
	this->createPanels();
}

WITMainWindow::~WITMainWindow(){}

void WITMainWindow::createMenus()
{
	// File Menu
	mnu_File = new QMenu(tr("&File"), this);
	mnu_File->addAction(tr("Load &Volume..."), this, SLOT(onLoadVolume()));
	mnu_File->addAction(tr("&Load PDB..."), this, SLOT(onLoadPDB()));

	this->menuBar()->addMenu(mnu_File);

	// Edit Menu
	mnu_Edit = new QMenu(tr("&Edit"), this);
	mnu_Edit->addAction(tr("Undo"), this, SLOT(onUndo()));
	mnu_Edit->addAction(tr("Redo"), this, SLOT(onRedo()));

	this->menuBar()->addMenu(mnu_Edit);

	// View Menu
	mnu_View = new QMenu(tr("&View"), this);
	QMenu *mnu_ViewCount = new QMenu(tr("Number of viewports"),this);
	mnu_View->addMenu(mnu_ViewCount);
	QActionGroup *viewGroup = new QActionGroup(mnu_ViewCount);
	QAction *viewSingle = new QAction("1", viewGroup);
    QAction *viewDual = new QAction("2", viewGroup);
    QAction *viewQuad = new QAction("4", viewGroup);

	viewSingle->setCheckable(true);
	viewDual->setCheckable(true);
	viewQuad->setCheckable(true);
	viewQuad->setChecked(true);

	mnu_ViewCount->addAction(viewSingle);
	mnu_ViewCount->addAction(viewDual);
	mnu_ViewCount->addAction(viewQuad);

	QSignalMapper *map = new QSignalMapper();
	map->setMapping(viewSingle,1);
	map->setMapping(viewDual,2);
	map->setMapping(viewQuad, 4);

	connect(viewSingle, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewDual, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewQuad, SIGNAL(triggered()), map, SLOT(map()));

	// Connect the view count selection menu to the viewWidget's setViewCount method
	connect(map, SIGNAL(mapped(int)), this->viewWidget, SLOT(onSetViewCount(int)));

	this->menuBar()->addMenu(mnu_View);
}

void WITMainWindow::onLoadPDB()
{
	// Show file dialog
	QString filename = QFileDialog::getOpenFileName(this, tr("Load PDB"), "",tr("PDB Files (*.pdb)"));

	// Fire signal
	emit this->loadPDB(filename.toStdString());
}

void WITMainWindow::onLoadVolume()
{
	// Show file dialog
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Volume"), "",tr("Volume Files (*.nii *.nii.gz)"));

	// Fire signal
	emit this->loadVolume(filename.toStdString());
}

void WITMainWindow::createPanels()
{
	/* 
		Tool Dock
	*/
	QDockWidget *toolDock = new QDockWidget("Tools", this);
    toolDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    toolDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toolDock->setTitleBarWidget(new QWidget(this));

	pnl_Tool = new WITToolPanel(toolDock);

	toolDock->setWidget(pnl_Tool);

	addDockWidget(Qt::TopDockWidgetArea, toolDock);

	/*
		Group Dock
	*/
	QDockWidget *groupDock = new QDockWidget("Pathways", this);
	groupDock->setAllowedAreas(Qt::BottomDockWidgetArea);
	groupDock->setFeatures(QDockWidget::NoDockWidgetFeatures);

	// We don't want a title bar, so we set an empty QWidget object to remove it
	groupDock->setTitleBarWidget(new QWidget(this));
	
	pnl_Group = new WITGroupPanel(groupDock);
	groupDock->setWidget(pnl_Group);

	this->addDockWidget(Qt::BottomDockWidgetArea, groupDock);

}
