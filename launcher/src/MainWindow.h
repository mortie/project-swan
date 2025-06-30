#pragma once

#include <string>
#include <vector>
#include <wx/wx.h>
#include "worlds.h"

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
	std::vector<World> worlds_;

	wxTextCtrl *selectedWorld_;
	wxButton *deleteBtn_;
	wxButton *renameBtn_;
	wxButton *loadBtn_;

	wxTextCtrl *newWorldName_;
	wxButton *newWorldBtn_;

	SwanLauncher *launcher_;
};

