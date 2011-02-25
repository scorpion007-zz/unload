// Main plugin unloader functions.
// Copyright (c) 2009 Alex Budovski.

#include "common.h"
#include "resource.h"

#include <max.h>

#include <iparamb2.h>
#include <utilapi.h>
#include <windowsx.h>

// Handle to this plugin's module.
extern HINSTANCE hInstance;

// Unload a DLL given a descriptor.
void UnloadDLL(DllDesc *dll, Interface *ip) {
  assert(dll->loaded);

  // Remove all classes from the user-interface.
  for (int i = 0; i < dll->NumberOfClasses(); ++i) {
    ClassDesc *class_desc = (*dll)[i];
    ip->DeleteClass(class_desc);
  }

  // Unload the DLL.
  dll->Free();

  // Reset the loaded flag so that Max (and other plugin-manager utilities)
  // behaves correctly.
  dll->loaded = false;
}

// Clear our combo-box control.
void ClearPluginList(HWND hList) {
  ComboBox_ResetContent(hList);
}

// Populate a combo-box with a list of plugins given a DllDir.
void PopulatePluginList(HWND hList, DllDir *dll_dir) {
  TCHAR own_filepath[MAX_PATH] = {0};
  GetModuleFileName(hInstance, own_filepath, MAX_PATH - 1);
  // Just take the basename, not the entire path.
  const TCHAR *own_filename = _tcsrchr(own_filepath, _T('\\')) + 1;

  for (int i = 0; i < dll_dir->Count(); ++i) {
    DllDesc &dll_desc = (*dll_dir)[i];
    // Exclude unloaded plugins, and ourselves from the list.
    if (dll_desc.loaded && _tcsicmp(dll_desc.fname, own_filename) != 0) {
      // Max's TSTR class implicity converts to LPCTSTR.
      ComboBox_AddString(hList, dll_desc.fname);
    }
  }
  ComboBox_SetCurSel(hList, 0);
}

void RefreshPluginList(HWND hList, DllDir *dll_dir) {
  ClearPluginList(hList);
  PopulatePluginList(hList, dll_dir);
}

// Find a DllDesc by file name in a DllDir, performing a case-insensitive
// search.
// Returns NULL if not found.
DllDesc *FindDllDescByName(const TCHAR *file_name, DllDir *dll_dir) {
  for (int i = 0; i < dll_dir->Count(); ++i) {
    DllDesc *dll_desc = &(*dll_dir)[i];
    if (!_tcsicmp(dll_desc->fname, file_name)) {
      return dll_desc;
    }
  }
  return NULL;
}


// TODO: Compute actual plugin-class usage, and provide a warning to the user
// if they attempt to unload an in-use plugin.
// Not implemented.
BOOL IsDllInUse(DllDir *dll_dir, DllDesc *dll_desc) {
  assert(dll_desc->loaded);

  for (int i = 0; i < dll_desc->NumberOfClasses(); ++i) {
    ClassDesc *class_desc = (*dll_desc)[i];
    if (!class_desc) {
      continue;
    }
    ClassDirectory &class_dir = dll_dir->ClassDir();
    ClassEntry *class_entry = class_dir.FindClassEntry(
        class_desc->SuperClassID(),
        class_desc->ClassID());
    if (!class_entry) {
      continue;
    }
    DPrintf(_T("Use count: %d\n"), class_entry->UseCount());
    if (class_entry->UseCount() > 0) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL OnInitDialog(HWND hwnd, HWND /*hwndFocus*/, LPARAM lParam) {
  DllDir *dll_dir = (DllDir *)lParam;

  // Attach dll_dir to our dialog. Max wants us to use GWLP_USERDATA even
  // though we're dealing with a dialog. No idea why this is.
  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dll_dir);
  PopulatePluginList(GetDlgItem(hwnd, IDC_PLUG_LIST), dll_dir);
  return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
  // See above note about GWLP_USERDATA.
  DllDir *dll_dir = (DllDir *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (codeNotify) {
    case BN_CLICKED:
      switch (id) {
        case IDC_UNLOAD_BUT: {
          HWND hList = GetDlgItem(hwnd, IDC_PLUG_LIST);
          // Get current selection.
          int sel = ComboBox_GetCurSel(hList);
          int text_len = ComboBox_GetLBTextLen(hList, sel);
          TCHAR *plug_name = new TCHAR[text_len+1];
          ComboBox_GetLBText(hList, sel, plug_name);
          DllDesc *dll_desc = FindDllDescByName(plug_name, dll_dir);
          if (dll_desc) {
            // Remove it from the list.
            ComboBox_DeleteString(hList, sel);
            // Reset the selection.
            ComboBox_SetCurSel(hList, 0);
            // Unload it.
            UnloadDLL(dll_desc, GetCOREInterface());
          }
          break;
        }
        case IDC_REFRESH_BUT: {
          RefreshPluginList(GetDlgItem(hwnd, IDC_PLUG_LIST), dll_dir);
          break;
        }
      }
      break;

    // Prevent Max from stealing keyboard input from any combo-boxes while
    // they've got keyboard focus, so that their hotkeys work.
    case CBN_SETFOCUS:
      DisableAccelerators();
      break;
    case CBN_KILLFOCUS:
      EnableAccelerators();
      break;
  }
}


// Main dialog procedure for the utility.
INT_PTR CALLBACK UnloadPlugDlgProc(HWND hDlg,
                                   UINT msg,
                                   WPARAM wParam,
                                   LPARAM lParam) {
  switch (msg) {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
  }
  return FALSE;
}

class UnloadPlug2 : public UtilityObj {
  void BeginEditParams(Interface *ip, IUtil * /*iu*/) {
    unload_rollup_= ip->AddRollupPage(hInstance,
                                      MAKEINTRESOURCE(IDD_UNLOADPLUG),
                                      UnloadPlugDlgProc,
                                      _M("Unload Plugin"),
                                      (LPARAM)ip->GetDllDirectory());

  }
  void EndEditParams(Interface *ip, IUtil * /*iu*/) {
    ip->DeleteRollupPage(unload_rollup_);
  }
  // Implement this ourselves to ensure consistent heap usage.
  void DeleteThis() {
    DPrintf(_T("In DeleteThis()\n"));
    delete this;
  }
private:
  HWND unload_rollup_;
};

// Class descriptor for our plugin.
class UnloadPlug2Desc : public ClassDesc2 {
public:
  int IsPublic() {
    return TRUE;
  }
  void *Create(BOOL loading) {
    return new UnloadPlug2;
  }
  const TCHAR *ClassName() {
    return _T("Unload Plugin");
  }
  SClass_ID SuperClassID() {
    return UTILITY_CLASS_ID;
  }
  Class_ID ClassID() {
    return UNLOAD_PLUG2_CLASS_ID;
  }
  const TCHAR *Category() {
    return _T("");
  }
};

static UnloadPlug2Desc unload_plug2_desc;

// Returns a singleton instance of our class descriptor.
ClassDesc *GetUnloadPlug2Desc() {
  return &unload_plug2_desc;
}