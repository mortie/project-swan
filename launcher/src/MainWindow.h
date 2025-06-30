#pragma once

#include <wx/wx.h>

class SwanLauncher;

class MainWindow: public wxFrame {
public:
	MainWindow(SwanLauncher *launcher);

	void OnExit(wxCommandEvent &);
	void OnWorldListClick(wxMouseEvent &event);
	void OnNewWorldClick(wxCommandEvent &);
	void OnWorldLaunch(wxCommandEvent &);
	void OnWorldDelete(wxCommandEvent &);
	void OnWorldRename(wxCommandEvent &);
	void OnWorldSelect(wxCommandEvent &);

	void reload();
	void disable();
	void enable();
	void updateSelection();

private:
	wxListBox *existingWorlds_;

	wxTextCtrl *selectedWorld_;
	wxButton *deleteBtn_;
	wxButton *renameBtn_;
	wxButton *loadBtn_;

	wxTextCtrl *newWorldName_;
	wxButton *newWorldBtn_;

	SwanLauncher *launcher_;
};

