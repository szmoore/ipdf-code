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
			static void Update() {if (g_panel != NULL) g_panel->UpdateAll();};
			
			ControlPanel(RunArgs & a, QWidget * p = NULL);
			virtual ~ControlPanel() {}
			
		private slots:
			void SetGPURendering();
			void SetCPURendering();


		private:
			static ControlPanel * g_panel;

			
			void UpdateAll();
					
			View & m_view;
			Document & m_doc;
			Screen & m_screen;
			
			QMenu * CreateMainMenu();
			QMenu * CreateViewMenu();
			QMenu * CreateDocumentMenu();
			QMenu * CreateScreenMenu();
			
			QAction * m_screen_gpu_rendering;
			QAction * m_screen_cpu_rendering;
			

	};

}

#endif //CONTROLPANEL_DISABLED

#endif //_CONTROLPANEL_H
