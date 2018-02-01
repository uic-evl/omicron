#include "oinputserverGUI.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
void Main(array<String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	oinputserverGUI::oinputserverGUI form;
	Application::Run(%form);
}