/**
 * Definitions for Qt4 based control panel
 */

#include "controlpanel.h"

#ifndef CONTROLPANEL_DISABLED

#include "view.h"
#include "screen.h"
#include "document.h"
#include <string>
#include <algorithm>

using namespace std;

namespace IPDF
{
	
	
ControlPanel::ControlPanel(RunArgs & args, QWidget * p) : QMainWindow(p), 
	m_view(args.view), m_doc(args.doc), m_screen(args.screen), m_width(300), m_height(300),
	m_state(ControlPanel::ABOUT), m_on_ok(NULL)
{
	// Size
	resize(m_width,m_height);

	
	// Main menues
	CreateMainMenu();
	CreateViewMenu();
	CreateDocumentMenu();
	CreateScreenMenu();
	
	CreateLayout();
	
	UpdateAll();
}

void ControlPanel::CreateLayout()
{
	m_text_edit = new QTextEdit(this);
	m_text_edit->setGeometry(10,35,m_width-20,m_height-100);
	
	m_ok_button = new QPushButton("OK", this);
	m_ok_button->setGeometry(10,35+m_height-90, m_width-20, 50);
	connect(m_ok_button, SIGNAL(clicked()), this, SLOT(PressOK()));
}

QMenu * ControlPanel::CreateMainMenu()
{
	QMenu * main = menuBar()->addMenu("&Main");
	
	QAction * about = new QAction("&About", this);
	main->addAction(about);
	connect(about, SIGNAL(triggered()), this, SLOT(StateAbout()));
	
	
	// Quit entry
	QAction * quit = new QAction("&Quit", this);
	main->addAction(quit);
	connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
	return main;
}

QMenu * ControlPanel::CreateDocumentMenu()
{
	QMenu * document = menuBar()->addMenu("&Document");
	
	m_document_set_font = new QAction("&Set Insertion Font", this);
	document->addAction(m_document_set_font);
	connect(m_document_set_font, SIGNAL(triggered()), this, SLOT(SetDocumentFont()));
	
	m_document_insert_text = new QAction("&Insert Text", this);
	document->addAction(m_document_insert_text);
	connect(m_document_insert_text, SIGNAL(triggered()), this, SLOT(StateInsertText()));
	
	m_document_load_svg = new QAction("&Load SVG From File", this);
	document->addAction(m_document_load_svg);
	connect(m_document_load_svg, SIGNAL(triggered()), this, SLOT(LoadSVGIntoDocument()));
	
	m_document_parse_svg = new QAction("&Input SVG Manually", this);
	document->addAction(m_document_parse_svg);
	connect(m_document_parse_svg, SIGNAL(triggered()), this, SLOT(StateParseSVG()));
	
	
	return document;
}

QMenu * ControlPanel::CreateViewMenu()
{
	QMenu * view = menuBar()->addMenu("&View");
	
	m_view_set_bounds = new QAction("&Set bounds", this);
	view->addAction(m_view_set_bounds);
	connect(m_view_set_bounds, SIGNAL(triggered()), this, SLOT(SetViewBounds()));
	
	m_view_show_bezier_bounds = new QAction("&Show Bezier Bounds", this);
	m_view_show_bezier_bounds->setCheckable(true);
	view->addAction(m_view_show_bezier_bounds);
	connect(m_view_show_bezier_bounds, SIGNAL(triggered()), this, SLOT(ToggleShowBezierBounds()));
	
	m_view_show_bezier_type = new QAction("&Show Bezier Type", this);
	m_view_show_bezier_type->setCheckable(true);
	view->addAction(m_view_show_bezier_type);
	connect(m_view_show_bezier_type, SIGNAL(triggered()), this, SLOT(ToggleShowBezierType()));
	
	m_view_show_fill_bounds = new QAction("&Show Fill Bounds", this);
	m_view_show_fill_bounds->setCheckable(true);
	view->addAction(m_view_show_fill_bounds);
	connect(m_view_show_fill_bounds, SIGNAL(triggered()), this, SLOT(ToggleShowFillBounds()));
	
	m_view_show_fill_points = new QAction("&Show Fill Points", this);
	m_view_show_fill_points->setCheckable(true);
	view->addAction(m_view_show_fill_points);
	connect(m_view_show_fill_bounds, SIGNAL(triggered()), this, SLOT(ToggleShowFillPoints()));
	
	m_view_enable_shading = new QAction("&Enable Shading", this);
	m_view_enable_shading->setCheckable(true);
	view->addAction(m_view_enable_shading);
	connect(m_view_enable_shading, SIGNAL(triggered()), this, SLOT(ToggleEnableShading()));
	
	return view;
}



QMenu * ControlPanel::CreateScreenMenu()
{
	QMenu * screen = menuBar()->addMenu("&Screen");
	
	m_screen_gpu_rendering = new QAction("&GPU Rendering", this);
	m_screen_gpu_rendering->setCheckable(true);
	m_screen_gpu_rendering->setToolTip("Uses the GPU for Rendering");
	
	m_screen_cpu_rendering = new QAction("&CPU Rendering", this);
	m_screen_cpu_rendering->setCheckable(true);
	m_screen_gpu_rendering->setToolTip("Uses the CPU for Rendering");
	
	m_screen_lazy_rendering = new QAction("&Lazy Rendering", this);
	m_screen_lazy_rendering->setCheckable(true);
		
	screen->addAction(m_screen_gpu_rendering);
	screen->addAction(m_screen_cpu_rendering);
	
	screen->addAction(m_screen_lazy_rendering);
	connect(m_screen_lazy_rendering, SIGNAL(triggered()), this, SLOT(ToggleLazyRendering()));
	
	connect(m_screen_gpu_rendering, SIGNAL(triggered()), this, SLOT(SetGPURendering()));
	connect(m_screen_cpu_rendering, SIGNAL(triggered()), this, SLOT(SetCPURendering()));
	
	m_screen_show_debug = new QAction("&Print Debug Info", this);
	m_screen_show_debug->setCheckable(true);
	
	screen->addAction(m_screen_show_debug);
	connect(m_screen_show_debug, SIGNAL(triggered()), this, SLOT(ToggleScreenDebugFont()));
	
	return screen;
}

void ControlPanel::paintEvent(QPaintEvent * e)
{
//	Debug("Called");
	
}

void ControlPanel::ChangeState(State next_state)
{
	m_state = next_state;
	UpdateAll();
}


void ControlPanel::UpdateAll()
{
	bool using_gpu_rendering = m_view.UsingGPURendering();
	m_screen_gpu_rendering->setChecked(using_gpu_rendering);
	m_screen_cpu_rendering->setChecked(!using_gpu_rendering);	
	m_screen_show_debug->setChecked(m_screen.DebugFontShown());
	m_screen_lazy_rendering->setChecked(m_view.UsingLazyRendering());
	
	m_view_show_bezier_bounds->setChecked(m_view.ShowingBezierBounds());
	m_view_show_bezier_type->setChecked(m_view.ShowingBezierType());
	m_view_show_fill_bounds->setChecked(m_view.ShowingFillBounds());
	m_view_show_fill_points->setChecked(m_view.ShowingFillPoints());
	m_view_enable_shading->setChecked(m_view.PerformingShading());
	
	
	// update things based on state
	const char * title;
	const char * tooltip;
	switch (m_state)
	{
		case INSERT_TEXT:
			title = "Insert Text";
			tooltip = "Type text to insert, press OK, simple.";
			m_text_edit->show();
			m_ok_button->show();
			m_on_ok = &ControlPanel::InsertTextIntoDocument;
			if (m_text_edit->toPlainText() == "")
				m_text_edit->setText("The quick brown\nfox jumps over\nthe lazy dog.");
			break;
		case PARSE_SVG:
			title = "Parse SVG";
			tooltip = "Enter valid SVG and press OK to insert.";
			m_text_edit->show();
			m_ok_button->show();
			m_on_ok = &ControlPanel::InsertSVGIntoDocument;
			if (m_text_edit->toPlainText() == "")
				m_text_edit->setText("<svg width=\"104\" height=\"186\">\n<path d = \"m 57,185\n\t c 0,0 57,-13 32,-43\n\t -25,-30 -53,2 -25, -30\n\t 28,-32 52,17 28,-32\n\t -24,-50 -16,44 -35,12\n\t-19,-32 13,-64 13,-64\n\t 0,0 40,-50 -0,-14\n\t -40,36 -94,68 -59,109\n\t 35,41 45,62 45,62 z\"/>\n</svg>");
			
			break;
		case ABOUT:
		default:
			title = "IPDF Control Panel";
			tooltip = "This is the IPDF Control Panel\nDo you feel in control?";
			m_text_edit->hide();
			m_ok_button->hide();
			m_on_ok = NULL;
			break;
	}
	
	// Title
	setWindowTitle(title);
	// Tooltip
	setToolTip(tooltip);
}

void ControlPanel::ToggleShowBezierBounds()
{
	bool state = m_view.ShowingBezierBounds();
	m_view.ShowBezierBounds(!state);
	UpdateAll();
}
void ControlPanel::ToggleShowBezierType()
{
	bool state = m_view.ShowingBezierType();
	m_view.ShowBezierType(!state);
	UpdateAll();
}
void ControlPanel::ToggleShowFillBounds()
{
	bool state = m_view.ShowingFillBounds();
	m_view.ShowFillBounds(!state);
	UpdateAll();
}

void ControlPanel::ToggleShowFillPoints()
{
	bool state = m_view.ShowingFillPoints();
	m_view.ShowFillPoints(!state);
	UpdateAll();
}

void ControlPanel::ToggleEnableShading()
{
	bool state = m_view.PerformingShading();
	m_view.PerformShading(!state);
	UpdateAll();
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

void ControlPanel::ToggleLazyRendering()
{
	bool state = m_view.UsingLazyRendering();
	m_view.SetLazyRendering(!state);
	UpdateAll();
}

void ControlPanel::ToggleScreenDebugFont()
{
	bool state = m_screen.DebugFontShown();
	m_screen.ShowDebugFont(!state);
	UpdateAll();
	
}

void ControlPanel::SetViewBounds()
{
	bool ok;
	Real xx = QInputDialog::getDouble(this, "View X Coordinate", "Enter X coordinate:", 0, -2e-30, 2e30,30,&ok);
	
	Real yy = QInputDialog::getDouble(this, "View Y Coordinate", "Enter Y coordinate:", 0, -2e-30, 2e30,30,&ok);
	
	Real w = QInputDialog::getDouble(this, "View Width", "Enter Width:", 1, -2e-30, 2e30,30,&ok);
	
	Real h = QInputDialog::getDouble(this, "View Height", "Enter Height:", 1, -2e-30, 2e30,30,&ok);
	m_view.SetBounds(Rect(xx,yy,w,h));
	
}

void ControlPanel::InsertTextIntoDocument()
{
	const Rect & bounds = m_view.GetBounds();
	Real xx = bounds.x;
	Real yy = bounds.y + bounds.h/Real(2);
	
	string msg = m_text_edit->toPlainText().toStdString();
	Real scale = bounds.h / Real(2);
	Debug("Insert \"%s\" at %f, %f, scale %f", msg.c_str(), Float(xx), Float(yy), Float(scale));
	m_doc.AddText(msg, scale, xx, yy);
	m_view.ForceRenderDirty();
	m_view.ForceBufferDirty();
	m_view.ForceBoundsDirty();
}
void ControlPanel::InsertSVGIntoDocument()
{
	Rect bounds(m_view.GetBounds());
	bounds.x += bounds.w/Real(2);
	bounds.y += bounds.h/Real(2);
	
	bounds.w /= Real(m_screen.ViewportWidth());
	bounds.h /= Real(m_screen.ViewportHeight());
	
	m_doc.ParseSVG(m_text_edit->toPlainText().toStdString(), bounds);
	m_view.ForceRenderDirty();
	m_view.ForceBufferDirty();
	m_view.ForceBoundsDirty();
}

void ControlPanel::LoadSVGIntoDocument()
{

	QString filename = QFileDialog::getOpenFileName(this, "Open SVG", "svg-tests", "Image Files (*.svg)");
	if (filename == "")
		return;
	
	#ifdef TRANSFORM_OBJECTS_NOT_VIEW
		Rect bounds(0,0,1,1);
	#else
	Rect bounds(m_view.GetBounds());
	#endif
	bounds.x += bounds.w/Real(2);
	bounds.y += bounds.h/Real(2);
	
	bounds.w /= Real(m_screen.ViewportWidth());
	bounds.h /= Real(m_screen.ViewportHeight());
	
	m_doc.LoadSVG(filename.toStdString(), bounds);
	m_view.ForceRenderDirty();
	m_view.ForceBufferDirty();
	m_view.ForceBoundsDirty();
}

void ControlPanel::SetDocumentFont()
{
	QString filename = QFileDialog::getOpenFileName(this, "Set Font", "fonts", "True Type Fonts (*.ttf)");
	if (filename != "")
		m_doc.SetFont(filename.toStdString());
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

