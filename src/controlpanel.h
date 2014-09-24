/**
 * Declarations for Qt4 based control panel
 */
#ifndef _CONTROLPANEL_H
#define _CONTROLPANEL_H

//#define CONTROLPANEL_DISABLED // To turn off the control panel

#ifndef CONTROLPANEL_DISABLED


#include <QPushButton>
#include <QMainWindow>
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QTextEdit>
#include <QInputDialog>
#include <QFileDialog>


namespace IPDF
{
	class View;
	class Document;
	class Screen;

	/**
	 * Class to manage Qt control panel
	 * Should be a singleton from the point of view of View, Document, Screen
	 *  - Just call "Update"
	 */
	class ControlPanel : public QMainWindow
	{	
		Q_OBJECT // Having this causes shit about undefined vtables
				// Not having it means things break
				// Apparently you need "qmake" to build qt applications
				// Or some bullshit -_-
		public:
			struct RunArgs
			{
				int argc;
				char ** argv;
				View & view;
				Document & doc;
				Screen & screen;
			};
			
			static int Run(void * args);
			static void Update() {if (g_panel != NULL) g_panel->UpdateAll();}			
	
		private:
			typedef enum {
				ABOUT,
				INSERT_TEXT,
				PARSE_SVG
			} State;
			
		private slots:
			void SetGPURendering();
			void SetCPURendering();
			void ToggleLazyRendering();
			void ToggleShowBezierBounds();
			void ToggleShowBezierType();
			void ToggleShowFillBounds();
			void ToggleShowFillPoints();
			void ToggleScreenDebugFont();
			void ToggleEnableShading();
			void SetViewBounds();
			void LoadSVGIntoDocument();
			void SetDocumentFont();
			void StateInsertText() {ChangeState(INSERT_TEXT);}
			void StateAbout() {ChangeState(ABOUT);}
			void StateParseSVG() {ChangeState(PARSE_SVG);}
			void PressOK() {if (m_on_ok != NULL) (this->*m_on_ok)();}

		private:
			static ControlPanel * g_panel;
			void paintEvent(QPaintEvent * e);
			ControlPanel(RunArgs & a, QWidget * p = NULL);
			virtual ~ControlPanel() {}
			void UpdateAll();
			void ChangeState(State next_state);
			View & m_view;
			Document & m_doc;
			Screen & m_screen;
			
			int m_width;
			int m_height;
			
			
			State m_state;
			
			QMenu * CreateMainMenu();
			QMenu * CreateViewMenu();
			QMenu * CreateDocumentMenu();
			QMenu * CreateScreenMenu();
			void CreateLayout();
			
			void InsertTextIntoDocument();
			void InsertSVGIntoDocument();
			
			QAction * m_screen_gpu_rendering;
			QAction * m_screen_cpu_rendering;
			QAction * m_screen_show_debug;
			QAction * m_screen_lazy_rendering;
			
			QAction * m_document_set_font;
			QAction * m_document_insert_text;
			QAction * m_document_parse_svg;
			QAction * m_document_load_svg;
			QAction * m_view_set_bounds;
			QAction * m_view_show_bezier_bounds;
			QAction * m_view_show_bezier_type;
			QAction * m_view_show_fill_bounds;
			QAction * m_view_show_fill_points;
			QAction * m_view_enable_shading;
			
		
			QTextEdit * m_text_edit;
			QPushButton * m_ok_button;
			
			void (ControlPanel::* m_on_ok)();


	};

}

#endif //CONTROLPANEL_DISABLED

#endif //_CONTROLPANEL_H
