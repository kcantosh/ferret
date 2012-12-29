#include "selectfiles.h"

BEGIN_EVENT_TABLE (MyListCtrl, wxListCtrl)
	EVT_MOUSE_EVENTS (MyListCtrl::OnMouseEvent)
	EVT_CHAR (MyListCtrl::OnKeyEvent)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE (SelectFiles, wxFrame)
	EVT_BUTTON (ID_ADD_FILES, SelectFiles::OnAdd)
	EVT_BUTTON (ID_CLEAR_FILES, SelectFiles::OnClear)
	EVT_BUTTON (ID_SETTINGS, SelectFiles::OnOptions)
	EVT_BUTTON (wxID_HELP, SelectFiles::OnHelp)
	EVT_CLOSE (SelectFiles::OnClose)
	EVT_BUTTON (ID_RUN_FERRET, SelectFiles::OnRun)
	EVT_CHECKBOX (ID_WEBFERRET, SelectFiles::OnSearchClicked)
END_EVENT_TABLE()
	
SelectFiles::SelectFiles ()
	: wxFrame (NULL, wxID_ANY, wxT("Ferret: Select document filenames for comparison"),
			wxDefaultPosition, wxSize (600, 550))
{
	CentreOnScreen ();
	_document_list = new DocumentList ();
	// set up internal widgets
	wxBoxSizer * frame_sizer = new wxBoxSizer (wxVERTICAL);
	// -- heading
	wxFont heading_font (15, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD, false);
	wxFont subheading_font (13, wxFONTFAMILY_SWISS, wxITALIC, wxNORMAL, false);
	wxStaticText * heading_1 = new wxStaticText (this, wxID_ANY,
			wxGetApp().GetVersionString ());
	heading_1->SetFont (heading_font);
	wxStaticText * heading_2 = new wxStaticText (this, wxID_ANY,
			wxT ("A Copy-Detection Tool"));
	heading_2->SetFont (subheading_font);
	wxStaticText * heading_3 = new wxStaticText (this, wxID_ANY,
			wxT("Developed by the Plagiarism Detection Group, University of Hertfordshire"));

	frame_sizer->Add (heading_1, 0, wxALIGN_CENTER | wxALL, 5);
	frame_sizer->Add (heading_2, 0, wxALIGN_CENTER | wxALL, 5);
	frame_sizer->Add (heading_3, 0, wxALIGN_CENTER | wxALL, 5);
	frame_sizer->Add (new wxStaticLine (this, wxID_ANY), 0, wxGROW | wxTOP | wxLEFT | wxRIGHT, 5);

	// -- buttons to handle selection of files, includes
	//    choice of selecting/adding files to the list, or 
	//    clearing the list to start again
	wxBoxSizer * selection_buttons_sizer = new wxBoxSizer (wxHORIZONTAL);
	
	selection_buttons_sizer->Add (MakeButton (this, ID_ADD_FILES,  wxT("Add Documents ..."),
				wxT("Use a file selector to identify files containing documents for comparison")),
		       	0, wxGROW | wxALL, 5);
	selection_buttons_sizer->Add (MakeButton (this, ID_CLEAR_FILES, wxT("Clear Documents"),
			       wxT("Remove every document filename from the list"), false),
			0, wxGROW | wxALL, 5);
	
	frame_sizer->Add (selection_buttons_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	// -- list control, to hold names of selected files
	frame_sizer->Add (MakeStaticText (this, wxT ("List of document filenames, to analyse for signs of copying:")),
		       	0, wxTOP | wxLEFT, 5);

	MyListCtrl * file_list = new MyListCtrl (this, ID_FILE_LIST);
	frame_sizer->Add (file_list, 1, wxGROW | wxALL, 5);
	
	// -- buttons at bottom
	wxBoxSizer * button_sizer = new wxBoxSizer (wxHORIZONTAL);

	button_sizer->Add (new wxButton (this, wxID_HELP), 0, wxGROW | wxLEFT | wxRIGHT, 5);	
	button_sizer->Add (MakeButton (this, ID_SETTINGS, wxT("Settings ..."),
				wxT("Advanced users: Control how Ferret converts your files")), 
			0, wxGROW | wxLEFT | wxRIGHT, 5);
	button_sizer->Add (MakeCheckBox (this, ID_WEBFERRET, wxT("Search internet"),
			wxT("Check this box if you want Ferret to search the internet")), 
			0, wxGROW | wxLEFT | wxRIGHT, 5);
	button_sizer->AddStretchSpacer (1);
	button_sizer->Add (MakeButton (this, ID_RUN_FERRET, wxT("Run Ferret"),
			wxT("Finish selecting documents, and perform comparison"), false),
			0, wxGROW | wxLEFT | wxRIGHT, 5);
#if __WXMAC__
	button_sizer->Add (new wxStaticText (this, wxID_ANY, wxT("")), 0, wxRIGHT, 10);
#endif

	frame_sizer->Add (button_sizer, 0, wxGROW | wxBOTTOM, 5);

	// -- end of widgets
	// compute best minimum height, and constrain window
	int best_height = 100; // allow for space between widgets
	best_height += heading_1->GetBestSize().GetHeight () * 5; // five pieces of similar text
	best_height += selection_buttons_sizer->GetMinSize().GetHeight ();
	int best_width = std::max (
			heading_3->GetBestSize().GetWidth (),
			selection_buttons_sizer->GetMinSize().GetWidth () + 30);
	SetSizeHints (std::max (best_height, best_width), best_height);
	
	SetSizer (frame_sizer);
#if __WXMSW__
	SetBackgroundColour (wxNullColour); // ensure background coloured, on Windows
#endif
}

void SelectFiles::OnAdd (wxCommandEvent & WXUNUSED(event))
{
	wxFileDialog dialog (NULL, wxT("Select file(s) to compare"), 
			wxEmptyString, wxEmptyString,
			wxT("All Files|*|Text (*.txt)|*.txt|Word (*.doc)|*.doc|Rich Text Format (*.rtf)|*.rtf|pdf (*.pdf)|*.pdf|C++ (*.cpp)|*.cpp|C (*.c)|*.c|Header (*.h)|*.h|Java (*.java)|*.java"),
			wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST );
	if (dialog.ShowModal () == wxID_OK)
	{
		wxArrayString paths;
		dialog.GetPaths (paths);
		AddDocuments (paths);
		if (paths.IsEmpty ())
		{
			((wxButton *) FindWindow (ID_CLEAR_FILES))->Enable (false);
		}
		else
		{
			((wxButton *) FindWindow (ID_CLEAR_FILES))->Enable (true);
			if ((*_document_list)[0]->IsCodeType ())
			{
				wxGetApp().SetTextType (false);
			}
			else
			{
				wxGetApp().SetTextType (true);
			}
		}
		UpdateButtons ();
	}
}

void SelectFiles::OnSearchClicked (wxCommandEvent & WXUNUSED(event))
{
	UpdateButtons ();
}

void SelectFiles::UpdateButtons ()
{
	// -- only allow 'runferret' if at least two files, or 1 file and can search internet
	if ((_document_list->Size () >= 2) ||
			((_document_list->Size () == 1) && 
			 (((wxCheckBox *) FindWindow (ID_WEBFERRET))->IsChecked ()) &&
			 (wxGetApp().IsTextType ())))
	{
		((wxButton *) FindWindow (ID_RUN_FERRET))->Enable (true);
	}
	else
	{
		((wxButton *) FindWindow (ID_RUN_FERRET))->Enable (false);
	}
	// -- only enable 'search internet' if text type is Text
	((wxCheckBox *) FindWindow (ID_WEBFERRET))->Enable (wxGetApp().IsTextType ());
}

void SelectFiles::OnClear (wxCommandEvent & WXUNUSED(event))
{
	((wxListCtrl *) FindWindow (ID_FILE_LIST))->DeleteAllItems ();
	_document_list->Clear ();
	((wxButton *) FindWindow (ID_CLEAR_FILES))->Enable (false);
	((wxButton *) FindWindow (ID_RUN_FERRET))->Enable (false);
}

void SelectFiles::OnOptions (wxCommandEvent & WXUNUSED(event))
{
	OptionSettings * settings = new OptionSettings (this);
	settings->ShowModal ();
	UpdateButtons ();
}

void SelectFiles::OnHelp (wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().ShowSelectionHelp ();
}

void SelectFiles::OnRun (wxCommandEvent & WXUNUSED(event))
{
	_document_list->ResetReading ();
	// perform text extraction
	if (!ExtractFiles ()) return; // abort, if cancel clicked in conversion
	ReadDocuments ();

	// download files and add to _document_list
	if (((wxCheckBox *) FindWindow (ID_WEBFERRET))->IsChecked () &&
			wxGetApp().IsTextType ())
	{
		wxGetApp().SetIgnoreUnknown (false); // retain unknown file types
		int num_documents = _document_list->Size ();
		DownloadFiles ();
		// perform text extraction on downloaded files
		if (!ExtractFiles (num_documents)) return; // abort, if cancel clicked in conversion
		ReadDocuments (num_documents);
	}
	if (_document_list->Size () >= 2)
	{
		WarnOfProblemFiles ();
		CreateComparisonView ();
	}
	else // warn that there are not enough documents and allow user to continue
	{
		wxMessageBox (wxT("After removing invalid document filetypes, there are not \nenough remaining to analyse.\nPlease add some more document filenames to analyse.\nIn some cases, you may want to alter the Settings\nto force Ferret to analyse different types of file."), 
			wxT("Warning: not enough valid documents to analyse"),
			wxOK | wxICON_INFORMATION, this);
	}
}

void SelectFiles::OnClose (wxCloseEvent & WXUNUSED(event))
{
	wxGetApp().CloseHelp ();
	Destroy ();
}

void SelectFiles::AddDocuments (wxArrayString & paths)
{
	wxListCtrl * file_list = (wxListCtrl *) FindWindow (ID_FILE_LIST);
	for (int i = 0, n = paths.GetCount (); i < n; ++i)
	{
		wxFileName filename (paths[i]);
		file_list->InsertItem (file_list->GetItemCount (), filename.GetFullName ());
		_document_list->AddDocument (paths[i]);
	}
}

// warn of any ignored or problem files in a dialog
void SelectFiles::WarnOfProblemFiles ()
{
	if (!wxGetApp().GetProblemFiles().IsEmpty () || !wxGetApp().GetIgnoredFiles().IsEmpty ())
	{
		wxString msg = wxT("Some files will not be analysed, as follows:\n\n");
		if (wxGetApp().GetProblemFiles().size () == 1)
		{
			msg += wxT("1 file was not converted\n");
		}
		else if (wxGetApp().GetProblemFiles().size () > 1)
		{
			msg += wxString::Format(wxT("%d files were not converted\n"), 
					wxGetApp().GetProblemFiles().size ());
		}
		
		if (wxGetApp().GetIgnoredFiles().size () == 1)
		{
			msg += wxT("1 file was ignored as its filetype is unknown\n");
		}
		else if (wxGetApp().GetIgnoredFiles().size () > 1)
		{
			msg += wxString::Format(wxT("%d files were ignored, as their filetypes were unknown\n"), 
					wxGetApp().GetIgnoredFiles().size ());
		}

		msg += wxT("\nMore details of the files affected can be found in the report, which you can create from the 'Comparison' display.");
		wxMessageBox (msg, wxT("Warning: Some files not analysed"),
				wxOK | wxICON_INFORMATION, this);
	}
}

void SelectFiles::CreateComparisonView ()
{
	// add documents to Ferret comparison window
	ComparisonTableView * frame = new ComparisonTableView ();
	{
		wxBusyCursor wait;
		wxBusyInfo info (wxT("Please wait: computing similarities ..."), this);
		frame->SetDocumentList (*_document_list);
	}
	// tidy up, and show the Ferret table of comparisons
	this->Destroy (); // don't close, as not exiting application

	frame->Show (true);	
}

// -- begins from document start_from, so list can be built up incrementally
void SelectFiles::ReadDocuments (int start_from)
{
	// read the documents, using a progress dialog
	wxProgressDialog dialog (wxT("Running Ferret"),
			wxT("Please wait whilst documents are being analysed ..."),
			_document_list->Size () - start_from,
			this,
			wxPD_CAN_ABORT |
			wxPD_APP_MODAL | 
			wxPD_ELAPSED_TIME | 
			wxPD_ESTIMATED_TIME | 
			wxPD_REMAINING_TIME);
#if __WXGTK__
	dialog.SetSize (150, 100);
#endif
	dialog.CentreOnParent ();

	for (int i = start_from, n = _document_list->Size (); i < n; ++i)
	{
		if (!dialog.Update (i - start_from))
		{
			if ( wxMessageBox (wxT("Do you really want to cancel, and return to selecting files?"),
						wxT("Cancelling the Progress Dialog"),
						wxYES_NO | wxICON_QUESTION) == wxYES )
			{
				return; // abort the run
			}
			else
			{
				dialog.Resume ();
			}
		}
		(*_document_list)[i]->SetType (GetDocumentType ());
		_document_list->ReadDocument (i);
	}
}

Document::DocumentType SelectFiles::GetDocumentType () const
{
	if (wxGetApp().IsTextType ()) 
	{
		return Document::typeText;
	}
	else
	{
		return Document::typeCode;
	}
}

// return true if extraction successful, false if cancelled
// -- begins from document start_from, so list can be built up incrementally
bool SelectFiles::ExtractFiles (int start_from)
{
	wxString extract_folder = wxGetApp().GetExtractFolder ();
	// maintain a progress dialog
	wxProgressDialog dialog (wxT("Preparing Documents"),
			wxT("Please wait while extracting text and copying documents ..."),
			_document_list->Size () - start_from,
			this,
			wxPD_CAN_ABORT |
			wxPD_APP_MODAL | 
			wxPD_ELAPSED_TIME | 
			wxPD_ESTIMATED_TIME | 
			wxPD_REMAINING_TIME);

	std::vector<Document *> to_remove; // keep a list of documents to remove
	// work through the documents in document list, performing copy/extraction where
	// appropriate
	for (int i = start_from, n = _document_list->Size (); i < n; ++i)
	{
		if (!dialog.Update (i - start_from, wxT("Please wait, processing file ...")))
		{
			if ( wxMessageBox (wxT("Do you really want to cancel, and return to extracting files?"),
						wxT("Cancelling the Progress Dialog"),
						wxYES_NO | wxICON_QUESTION) == wxYES )
			{
				return false; // abort the run
			}
			else
			{
				dialog.Resume ();
			}
		}

		if (!(*_document_list)[i]->ExtractDocument (extract_folder))
		{
			to_remove.push_back ((*_document_list)[i]);
		}
	}

	// remove unwanted or failed documents
	for (int i = 0, n = to_remove.size(); i < n; ++i)
	{
		_document_list->RemoveDocument (to_remove[i]);
	}

	return true;
}

// retrieve relevant documents from web, place into download folder, and add to _document_list
void SelectFiles::DownloadFiles ()
{
	wxBusyCursor wait; 
	wxBusyInfo info (wxT("Searching for and downloading documents, please wait ..."), this);
	wxGetApp().Yield ();

	_document_list->DownloadFiles ();
}

// *** Options Dialog ***

BEGIN_EVENT_TABLE (OptionSettings, wxDialog)
	EVT_BUTTON (ID_DOWNLOAD_BROWSE, OptionSettings::OnDownloadBrowse)
	EVT_BUTTON (ID_EXTRACT_BROWSE, OptionSettings::OnExtractBrowse)
	EVT_BUTTON (wxID_OK, OptionSettings::OnOk)
END_EVENT_TABLE ()

OptionSettings::OptionSettings (wxWindow * parent)
	: wxDialog (parent, wxID_ANY, wxT("Ferret: Settings"), 
			wxDefaultPosition, wxDefaultSize, 
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER, wxT(""))
{
	wxBoxSizer * sizer = new wxBoxSizer (wxVERTICAL);

	// -- indicate type of documents
	static wxString choices[] = { 
		wxT("Natural language (e.g. English)"), 
		wxT("Computer programs (e.g. Java, C++, C)") 
	};
	wxRadioBox * type_selection = new wxRadioBox (this, 
			ID_TYPE_SELECTION,
			wxT("Document type (change after selecting documents)"),
			wxDefaultPosition, wxDefaultSize,
			WXSIZEOF(choices), choices,
			1, wxRA_SPECIFY_COLS);
	type_selection->SetSelection ((wxGetApp().IsTextType()) ? 0 : 1);
	
	sizer->Add (type_selection, 0, wxGROW | wxALL, 5);

	// -- browser for folder name, and how files will be converted
	sizer->Add (new wxStaticText (this, wxID_ANY, wxT("Destination folder for files containing extracted text:")),
			0, wxALIGN_LEFT | wxALL, 5);

	// ---- horizontal panel
	wxBoxSizer * browser_sizer = new wxBoxSizer (wxHORIZONTAL);
	
	browser_sizer->Add (new wxStaticText (this, ID_EXTRACT_DIR_NAME, wxGetApp().GetExtractFolder ()), 
			1, wxGROW | wxALL, 5);
	browser_sizer->Add (MakeButton (this, ID_EXTRACT_BROWSE, wxT("Browse ..."),
				wxT("Use directory selector to choose a folder to save new files")),
		       	0, wxALIGN_RIGHT | wxRIGHT, 10);
	
	sizer->Add (browser_sizer, 0, wxGROW);
	
	// -- options, for what to do, and whether to place all files
	sizer->Add (MakeCheckBox (this, ID_COPY_ALL,
				wxT("Copy all files to destination folder, even if not extracting text"),
				wxT("Use when extracting to your own folder, to keep all analysed documents together"),
				wxGetApp().GetCopyAll ()), 
			0, wxALIGN_LEFT | wxLEFT | wxTOP, 5);
	sizer->Add (MakeCheckBox (this, ID_EXTRACT_ALL, 
				wxT("Treat all files as if they were word or rich text (rtf) documents"),
			wxEmptyString, wxGetApp().GetConvertAll()),
		       	0, wxALIGN_LEFT | wxLEFT, 5);
	sizer->Add (MakeCheckBox (this, ID_IGNORE_UNKNOWN, wxT("Ignore unknown file types"),
				wxT("Only analyse documents of known file type (see Help for details)"),
				wxGetApp().GetIgnoreUnknown ()),
			0, wxALIGN_LEFT | wxLEFT | wxBOTTOM, 5);

	sizer->Add (new wxStaticLine (this, wxID_ANY), 0, wxGROW | wxALL, 5);
	// -- browser for downloaded files folder name
	sizer->Add (MakeStaticText (this, wxT("Destination folder for downloaded files:")),
			0, wxALIGN_LEFT | wxALL, 5);

	// ---- horizontal panel
	wxBoxSizer * download_browser_sizer = new wxBoxSizer (wxHORIZONTAL);
	
	download_browser_sizer->Add (new wxStaticText (this, ID_DOWNLOAD_DIR_NAME, wxGetApp().GetDownloadFolder ()), 
			1, wxGROW | wxALL, 5);
	download_browser_sizer->Add (MakeButton (this, ID_DOWNLOAD_BROWSE, wxT("Browse ..."),
			wxT("Use directory selector to choose a folder to download files")),
		       	0, wxALIGN_RIGHT | wxRIGHT, 10);
	sizer->Add (download_browser_sizer, 0, wxGROW);
	
	wxGridSizer * download_choices_sizer = new wxGridSizer (3, 2, 2, 5);

	download_choices_sizer->Add (new wxStaticText (this, wxID_ANY, 
				wxT("Number of documents to download:")));
	download_choices_sizer->Add (MakeSpinCtrl (this, ID_MAX_DOWNLOADS,
				wxT("Maximum number of documents to download from internet"),
				1, 1000, wxGetApp().GetMaxDownloadedDocuments ()));

	download_choices_sizer->Add (new wxStaticText (this, wxID_ANY,
				wxT("Number of tuple search results:")));
	download_choices_sizer->Add (MakeSpinCtrl (this, ID_MAX_RESULTS,
				wxT("Maximum number of search results per tuple searched"),
				1, 1000, wxGetApp().GetMaxResultsPerTuple ()));

	download_choices_sizer->Add (new wxStaticText (this, wxID_ANY, 
				wxT("Number of tuples to search:")));
	download_choices_sizer->Add (MakeSpinCtrl (this, ID_MAX_TUPLE_SEARCHES,
				wxT("Maximum number of tuples to use for searching"),
				1, 1000, wxGetApp().GetMaxTupleSearches ()));

	sizer->Add (download_choices_sizer, 0, wxALL, 5);
	sizer->Add (new wxStaticLine (this, wxID_ANY), 0, wxGROW | wxALL, 5);
	SetSizer (sizer);
	sizer->Add (CreateButtonSizer (wxOK | wxCANCEL), 0, wxGROW | wxALL, 5);
	sizer->Fit (this);
	sizer->SetSizeHints (this);
}

void OptionSettings::OnDownloadBrowse (wxCommandEvent & WXUNUSED(event))
{
	OnBrowse (ID_DOWNLOAD_DIR_NAME);
}

void OptionSettings::OnExtractBrowse (wxCommandEvent & WXUNUSED(event))
{
	OnBrowse (ID_EXTRACT_DIR_NAME);
}

// given the id of the label holding the current directory name, 
// request user for a new directory, and update label if that new 
// directory exists
void OptionSettings::OnBrowse (int dir_name_id)
{
	wxStaticText * path = (wxStaticText *) FindWindow (dir_name_id);
	wxDirDialog dialog (this, wxT("Select a folder in which to place files"),
			path->GetLabel (), wxDD_NEW_DIR_BUTTON);
	if ((dialog.ShowModal () == wxID_OK) && wxFileName::DirExists (dialog.GetPath ()))
	{
		path->SetLabel (dialog.GetPath ());
	}
}

// copy all necessary settings into the application
void OptionSettings::OnOk (wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().SetTextType (((wxRadioBox *) FindWindow (ID_TYPE_SELECTION))->GetSelection () == 0);
	wxGetApp().SetDownloadFolder (((wxStaticText *) FindWindow (ID_DOWNLOAD_DIR_NAME))->GetLabel ());
	wxGetApp().SetExtractFolder (((wxStaticText *) FindWindow (ID_EXTRACT_DIR_NAME))->GetLabel ());
	wxGetApp().SetCopyAll (((wxCheckBox *) FindWindow (ID_COPY_ALL))->GetValue ());
	wxGetApp().SetConvertAll (((wxCheckBox *) FindWindow (ID_EXTRACT_ALL))->GetValue ());
	wxGetApp().SetIgnoreUnknown (((wxCheckBox *) FindWindow (ID_IGNORE_UNKNOWN))->GetValue ());
	wxGetApp().SetMaxDownloadedDocuments (((wxSpinCtrl *) FindWindow (ID_MAX_DOWNLOADS))->GetValue ());
	wxGetApp().SetMaxResultsPerTuple (((wxSpinCtrl *) FindWindow (ID_MAX_RESULTS))->GetValue ());
	wxGetApp().SetMaxTupleSearches (((wxSpinCtrl *) FindWindow (ID_MAX_TUPLE_SEARCHES))->GetValue ());

	EndModal (0);
}

