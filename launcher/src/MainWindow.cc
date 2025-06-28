#include "MainWindow.h"
#include "worlds.h"
#include "SwanLauncher.h"
#include <iostream>
#include <span>
#include <string>

static std::string getNewWorldName(std::span<std::string> worlds)
{
	constexpr const char *NAME = "Default";
	std::string name = NAME;
	bool collision = false;
	for (auto &world: worlds) {
		if (world == name) {
			collision = true;
			break;
		}
	}

	if (!collision) {
		return name;
	}

	int num = 2;
	while (true) {
		name = NAME;
		name += " ";
		name += std::to_string(num);

		bool collision = false;
		for (auto &world: worlds) {
			if (world == name) {
				collision = true;
				break;
			}
		}

		if (!collision) {
			return name;
		}

		num += 1;
	}
}

MainWindow::MainWindow(SwanLauncher *launcher):
	wxFrame(NULL, wxID_ANY, "SWAN Launcher"),
	launcher_(launcher)
{
	auto *menuBar = new wxMenuBar;
	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);

	auto *box = new wxBoxSizer(wxVERTICAL);
	SetSizer(box);

	existingWorlds_ = new wxListBox(this, wxID_ANY);
	existingWorlds_->Bind(wxEVT_LISTBOX_DCLICK, &MainWindow::LoadWorld, this);
	box->Add(existingWorlds_, 1, wxEXPAND);

	auto *newWorldRow = new wxBoxSizer(wxHORIZONTAL);
	box->Add(newWorldRow, 0, wxEXPAND);

	newWorldName_ = new wxTextCtrl(this, wxID_ANY);
	newWorldName_->Bind(wxEVT_TEXT_ENTER, &MainWindow::CreateNewWorld, this);
	newWorldRow->Add(newWorldName_, 1, wxEXPAND | wxRIGHT);

	newWorldBtn_ = new wxButton(this, wxID_ANY, wxT("New World"));
	newWorldBtn_->SetFocus();
	newWorldBtn_->Bind(wxEVT_BUTTON, &MainWindow::CreateNewWorld, this);
	newWorldRow->Add(newWorldBtn_, 0);

	reload();
}

void MainWindow::OnExit(wxCommandEvent &)
{
	Close(true);
}

void MainWindow::LoadWorld(wxCommandEvent &)
{
	int sel = existingWorlds_->GetSelection();
	if (sel == wxNOT_FOUND) {
		return;
	}

	std::string name = existingWorlds_->GetString(sel).utf8_string();
	if (!worldExists(name)) {
		std::cerr << "Tried to open '" << name << "', which doesn't exist\n";
		new wxDialog(this, wxID_ANY, "World doesn't exist");
		return;
	}

	std::cerr << "Loading world: " << name << '\n';
	launcher_->launch(name);
}

void MainWindow::CreateNewWorld(wxCommandEvent &)
{
	std::string name = newWorldName_->GetLineText(0).utf8_string();
	std::cerr << "Creating world: " << name << '\n';
	launcher_->launch(name);
}

void MainWindow::reload()
{
	auto worlds = listWorlds();
	wxArrayString worldNames;
	for (auto &world: worlds) {
		worldNames.Add(world);
	}

	existingWorlds_->Set(worldNames);
	newWorldName_->SetLabelText(getNewWorldName(worlds));
}

void MainWindow::disable()
{
	existingWorlds_->Enable(false);
	newWorldName_->Enable(false);
	newWorldBtn_->Enable(false);
}

void MainWindow::enable()
{
	existingWorlds_->Enable(true);
	newWorldName_->Enable(true);
	newWorldBtn_->Enable(true);
}
