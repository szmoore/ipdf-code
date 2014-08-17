/**
 * Definitions for Qt4 based control panel
 */

#include "controlpanel.h"
#include "view.h"
#include "screen.h"
#include "document.h"

#ifndef CONTROLPANEL_DISABLED

namespace IPDF
{
	
	
ControlPanel::ControlPanel(RunArgs & args, QWidget * p) : QMainWindow(p), 
	m_view(args.view), m_doc(args.doc), m_screen(args.screen)
{
	// Size
	resize(300,300);
	// Title
	setWindowTitle("IPDF Control Panel");
	// Tooltip
	setToolTip("This is the IPDF Control Panel.\nDo you feel in control?");
	
	// Main menues
	CreateMainMenu();
	CreateViewMenu();
	CreateDocumentMenu();
	CreateScreenMenu();
	
	UpdateAll();

}

QMenu * ControlPanel::CreateMainMenu()
{
	QMenu * main = menuBar()->addMenu("&Main");
	
	// Quit entry
	QAction * quit = new QAction("&Quit", this);
	main->addAction(quit);
	connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
	return main;
}

QMenu * ControlPanel::CreateDocumentMenu()
{
	QMenu * document = menuBar()->addMenu("&Document");
	return document;
}

QMenu * ControlPanel::CreateViewMenu()
{
	QMenu * view = menuBar()->addMenu("&View");
	

	
	return view;
}



QMenu * ControlPanel::CreateScreenMenu()
{
	QMenu * screen = menuBar()->addMenu("&Screen");
	
	m_screen_gpu_rendering = new QAction("&GPU Rendering", this);
	m_screen_gpu_rendering->setCheckable(true);
	
	m_screen_cpu_rendering = new QAction("&CPU Rendering", this);
	m_screen_cpu_rendering->setCheckable(true);
		
	screen->addAction(m_screen_gpu_rendering);
	screen->addAction(m_screen_cpu_rendering);
	
	connect(m_screen_gpu_rendering, SIGNAL(triggered()), this, SLOT(SetGPURendering()));
	connect(m_screen_cpu_rendering, SIGNAL(triggered()), this, SLOT(SetCPURendering()));
	
	return screen;
}

void ControlPanel::UpdateAll()
{
	bool using_gpu_rendering = m_view.UsingGPURendering();
	m_screen_gpu_rendering->setChecked(using_gpu_rendering);
	m_screen_cpu_rendering->setChecked(!using_gpu_rendering);	
}

void ControlPanel::SetGPURendering()
{
	m_view.SetGPURendering(true);
	UpdateAll();
}

void ControlPanel::SetCPURendering()
{
	m_view.SetGPURendering(false);
	UpdateAll();
}

ControlPanel * ControlPanel::g_panel = NULL;

int ControlPanel::Run(void * args)
{
	ControlPanel::RunArgs * a = (ControlPanel::RunArgs*)args;
	QApplication app(a->argc, a->argv);
	g_panel = new ControlPanel(*a);
	g_panel->show();
	int result = app.exec();
	a->screen.RequestQuit();
	delete g_panel;
	return result;
}
	
	
}

#endif //CONTROLPANEL_ENABLED

