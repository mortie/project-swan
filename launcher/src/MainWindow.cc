#include "MainWindow.h"
#include "worlds.h"
#include "SwanLauncher.h"
#include <iostream>
#include <span>
#include <string>

static std::string getNewWorldName(std::span<World> worlds)
{
	constexpr const char *NAME = "Default";
	std::string name = NAME;
	bool collision = false;
	for (auto &world: worlds) {
		if (world.name == name) {
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
			if (world.name == name) {
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
	existingWorlds_->Bind(wxEVT_LISTBOX, &MainWindow::OnWorldSelect, this);
	existingWorlds_->Bind(wxEVT_LISTBOX_DCLICK, &MainWindow::OnWorldLaunch, this);
	existingWorlds_->Bind(wxEVT_LEFT_DOWN, &MainWindow::OnWorldListClick, this);
	box->Add(existingWorlds_, 1, wxEXPAND);

	auto *editRow = new wxBoxSizer(wxHORIZONTAL);
	box->Add(editRow, 0, wxEXPAND);

	selectedWorld_ = new wxTextCtrl(this, wxID_ANY, wxT(""));
	selectedWorld_->Enable(false);
	editRow->Add(selectedWorld_, 1, wxEXPAND | wxRIGHT, 5);

	deleteBtn_ = new wxButton(this, wxID_ANY, wxT("Delete"));
	deleteBtn_->Enable(false);
	deleteBtn_->Bind(wxEVT_BUTTON, &MainWindow::OnWorldDelete, this);
	editRow->Add(deleteBtn_, 0);

	renameBtn_ = new wxButton(this, wxID_ANY, wxT("Rename"));
	renameBtn_->Enable(false);
	renameBtn_->Bind(wxEVT_BUTTON, &MainWindow::OnWorldRename, this);
	editRow->Add(renameBtn_, 0);

	loadBtn_ = new wxButton(this, wxID_ANY, wxT("Launch"));
	loadBtn_->Enable(false);
	loadBtn_->Bind(wxEVT_BUTTON, &MainWindow::OnWorldLaunch, this);
	editRow->Add(loadBtn_, 0);

	auto *newWorldRow = new wxBoxSizer(wxHORIZONTAL);
	box->Add(newWorldRow, 0, wxEXPAND);

	newWorldName_ = new wxTextCtrl(this, wxID_ANY);
	newWorldName_->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnNewWorldClick, this);
	newWorldRow->Add(newWorldName_, 1, wxEXPAND | wxRIGHT, 5);

	newWorldBtn_ = new wxButton(this, wxID_ANY, wxT("Create New World"));
	newWorldBtn_->Bind(wxEVT_BUTTON, &MainWindow::OnNewWorldClick, this);
	newWorldRow->Add(newWorldBtn_, 0);

	reload();
}

void MainWindow::OnExit(wxCommandEvent &)
{
	Close(true);
}

void MainWindow::OnWorldListClick(wxMouseEvent &evt)
{
	wxPoint pos = evt.GetPosition();
	int index = existingWorlds_->HitTest(pos);
	existingWorlds_->SetSelection(index);
	updateSelection();
	evt.Skip();
}

void MainWindow::OnNewWorldClick(wxCommandEvent &)
{
	std::string name = newWorldName_->GetLineText(0).utf8_string();
	std::cerr << "Creating world: " << name << '\n';
	std::string id = makeWorld(name);
	launcher_->launch(id);
}

void MainWindow::OnWorldLaunch(wxCommandEvent &)
{
	int sel = existingWorlds_->GetSelection();
	if (sel == wxNOT_FOUND) {
		return;
	}

	World &world = worlds_[sel];
	if (!worldExists(world.id)) {
		std::cerr << "Tried to open '" << world.id << "', which doesn't exist\n";
		new wxDialog(this, wxID_ANY, "World doesn't exist");
		return;
	}

	std::cerr << "Loading world: " << world.name << '\n';
	launcher_->launch(world.id);
}

void MainWindow::OnWorldDelete(wxCommandEvent &)
{
	int sel = existingWorlds_->GetSelection();
	if (sel == wxNOT_FOUND) {
		return;
	}

	World &world = worlds_[sel];
	auto message = "Are you sure you want to delete '" + world.name + "'?";
	auto *dialog = new wxMessageDialog(this, message);
	dialog->SetMessageDialogStyle(wxYES_NO);
	int ret = dialog->ShowModal();
	if (ret != wxID_YES) {
		return;
	}

	deleteWorld(world.id);
	reload();
}

void MainWindow::OnWorldRename(wxCommandEvent &)
{
	int sel = existingWorlds_->GetSelection();
	if (sel == wxNOT_FOUND) {
		return;
	}

	World &world = worlds_[sel];
	auto message = "New name for '" + world.name +"':";
	auto newName = wxGetTextFromUser(message).utf8_string();
	if (newName == world.name || newName == "") {
		return;
	}

	renameWorld(world.id, newName);
	reload();
}

void MainWindow::OnWorldSelect(wxCommandEvent &)
{
	updateSelection();
}

void MainWindow::reload()
{
	worlds_ = listWorlds();
	wxArrayString worldNames;
	for (auto &world: worlds_) {
		worldNames.Add(world.name + " (" + world.creationTime + ")");
	}

	selectedWorld_->SetLabelText(wxT(""));
	existingWorlds_->Set(worldNames);
	newWorldName_->SetLabelText(getNewWorldName(worlds_));

	existingWorlds_->SetSelection(-1);
	updateSelection();
}

void MainWindow::disable()
{
	existingWorlds_->SetSelection(-1);
	updateSelection();
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

void MainWindow::updateSelection()
{
	int sel = existingWorlds_->GetSelection();
	if (sel == wxNOT_FOUND) {
		deleteBtn_->Enable(false);
		renameBtn_->Enable(false);
		loadBtn_->Enable(false);
		selectedWorld_->SetLabel("");
	} else {
		deleteBtn_->Enable(true);
		renameBtn_->Enable(true);
		loadBtn_->Enable(true);
		selectedWorld_->SetLabel(existingWorlds_->GetString(sel));
	}
}
