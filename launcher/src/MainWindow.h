#pragma once

#include <wx/wx.h>

class SwanLauncher;

class MainWindow: public wxFrame {
public:
	MainWindow(SwanLauncher *launcher);

	void OnExit(wxCommandEvent &);
	void CreateNewWorld(wxCommandEvent &);
	void LoadWorld(wxCommandEvent &);

	void reload();
	void disable();
	void enable();

private:
	wxListBox *existingWorlds_;
	wxTextCtrl *newWorldName_;
	wxButton *newWorldBtn_;
	SwanLauncher *launcher_;
};

