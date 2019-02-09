
// voidpad is a binary editor viewed in code page 437.

// TODO:
// *handle large paste commands (eg: copy all filenames from Everything - Ctrl + Shift + C and paste in voidpad currently does nothing.) -make sure clipboard is not open
// charmap - insert char - list of all dos chars.
// replacing \ to \\ in a 1million byte file should be instant.
// add an evaluate function, eg 301-1, select it and click evaluate and it will change to 300.
// < or > in search history is causing corruption in search history. (its removing the last character.) see: <em>
// * Please rename HKEY_LOCAL_MACHINE\SOFTWARE\voidpad\voidpad_tab_size to: HKEY_LOCAL_MACHINE\SOFTWARE\voidpad\tab_size
// Find should bring words fully into view, not partially. (vertically and horizontally)
// save the last find/replace original text
// watch the file (FindFirstChangeNotification) and warn if the current open document has changed outside the editor, allow user to reopen
// alt selecting
// associations getting unintentionally uninstalled.
// run as admin to change context menu options.

// networking
// lua scripts
// option - dont move cursor when selecting all.
// print?
// basic hex editing.
// high dpi support

// NOTES:

// * fixed search crash when opening a new file.
// * fix auto fill word select
// * fill out the find text with the last find text when starting up.
// [1.0.0.2a]
// * double click to select word, shift + click and release, shift + click and release loses word anchor. -this is the same as vs.
// * ctrl + click should select word.
// * when restoring from minimized with a mouse click, the top is brought into view, not the cursor. -dont WM_SIZE if iconic.
// * opening zero byte sized files now works.
// * always write \r\n for newlines.
// [1.0.0.1a]
// * replace text the same as find = lock.
// * custom copy/paste that supports nulls. -added (utf8) clipboard format with size: VOIDPAD 
// * ''doing'' needs to be a dialog model loop.
// * reset vscroll after opening new file.
// * we shouldnt be clipping cursor x pos, it can be longer than the text so we can move it up and down and keep the longer x position.
// * default save as to .txt
// * cycle clipboard from menu.
// * dynamic menus.
// * copy unicode to clipboard.
// * paste unicode convert to cp 437
// * empty search grey OK.
// * double click should select word and start selection doing
// * open utf8 / unicode from menu? -no this is an ANSI editor.
// * pressing down on last line, doesnt move cursor to last char. -and so it shouldnt.

// compiler options
#pragma warning(disable : 4311) // type cast void * to unsigned int
#pragma warning(disable : 4312) // type cast unsigned int to void *
#pragma warning(disable : 4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4313) // 'printf' : '%x' in format string conflicts with argument 2 of type 'line_t *'
#pragma warning(disable : 4996) // deprecation
//#pragma warning(disable : 4100) // unreferenced formal parameter

// #define VOIDPAD_REGISTRATION

#if _WIN64
	#define VERSION_MACHINE_TARGET L"x64"
#else
	#define VERSION_MACHINE_TARGET L"x86"
#endif

// REQUIRES IE 5.01
#define _WIN32_IE 0x0501

// WINNT required for some header definitions.
// minimum is really 0x0400
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

#define _INC_CTYPE

#include <windows.h>
#include <uxtheme.h>
#include "../resource.h"
#include <windowsx.h>
#include <Shellapi.h>
#include <commdlg.h> // openfilename
#include <shlwapi.h>
#include "version.h"

//DEBUG:
#ifdef _DEBUG
#include <stdio.h>
//#define printf(...)
#else
#define printf(...)
#endif

#define VOIDPAD_CHAR_TYPE_SPACE		0
#define VOIDPAD_CHAR_TYPE_SYMBOL	1
#define VOIDPAD_CHAR_TYPE_ALPHANUM	2

#define VOIDPAD_FIND_MAX		20
#define VOIDPAD_REPLACE_MAX		20

#define VOIDPAD_CAPTURE_SELECT	0x01
#define VOIDPAD_CAPTURE_SCROLL	0x02

// load unicode for windows 95/98
HMODULE LoadUnicowsProc(void);

extern FARPROC _PfnLoadUnicows = (FARPROC) &LoadUnicowsProc;

HMODULE LoadUnicowsProc(void)
{
    return(LoadLibraryA("unicows.dll"));
}

#define VOIDPAD_KEYSPEED_TIMER	1

#define VOIDPAD_KEY_FLAG_SHIFT		0x01
#define VOIDPAD_KEY_FLAG_CONTROL	0x02
#define VOIDPAD_KEY_FLAG_ALT		0x04

#define VOIDPAD_KEY_SCOPE_GLOBAL	0
#define VOIDPAD_KEY_SCOPE_EDIT		1

#define VOIDPAD_INSERT_MODE_OVERWRITE	0
#define VOIDPAD_INSERT_MODE_INSERT		1

#define VOIDPAD_OPTIONS_WIDE	420
#define VOIDPAD_OPTIONS_HIGH	360

enum
{
	ID_FILE_NEW = 1000,
	ID_FILE_OPEN,
	ID_FILE_SAVE,
	ID_FILE_SAVE_DIRECT,
	ID_FILE_SAVE_AS,
	ID_FILE_EXIT,

	ID_EDIT_UNDO,
	ID_EDIT_REDO,
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
	ID_EDIT_CYCLE_CLIPBOARD_RING,
	ID_EDIT_DELETE,
	ID_EDIT_DELETE_LINE,
	ID_EDIT_SELECT_ALL,
	ID_EDIT_FIND,
	ID_EDIT_REPLACE,
	ID_EDIT_FIND_PREV,
	ID_EDIT_FIND_NEXT,
	ID_EDIT_GOTO,

	ID_EDIT_MAKE_UPPERCASE,
	ID_EDIT_MAKE_LOWERCASE,
	ID_EDIT_INCREMENT_INTEGER,
	ID_EDIT_DECREMENT_INTEGER,
	ID_EDIT_VIEW_WHITE_SPACE,
	ID_EDIT_INCREASE_LINE_INDENT,
	ID_EDIT_DECREASE_LINE_INDENT,
	ID_EDIT_INSERT_HEX,
	ID_EDIT_INSERT_UTF8_SEQUENCE,
	ID_EDIT_INSERT_UTF16_SEQUENCE,
	
	ID_TOOLS_MACRO_RECORD,
	ID_TOOLS_MACRO_PLAY,
	ID_TOOLS_OPTIONS,
	ID_HELP_ABOUT,

	ID_FIND_STATIC,
	ID_FIND_COMBO,
	ID_FIND_CASE,
	ID_FIND_WHOLEWORD,

	ID_FIND_REPLACE_STATIC,
	ID_FIND_REPLACE_COMBO,
	ID_FIND_REPLACE_BUTTON,
	ID_FIND_REPLACEALL_BUTTON,

	ID_GOTO_WHAT_STATIC,
	ID_GOTO_WHAT_EDIT,

	ID_INSERT_HEX_WHAT_STATIC,
	ID_INSERT_HEX_WHAT_EDIT,

	ID_INSERT_UTF8_SEQUENCE_WHAT_STATIC,
	ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT,

	ID_INSERT_UTF16_SEQUENCE_WHAT_STATIC,
	ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT,

// not in menus:
	
	ID_EDIT_INCREASE_LINE_INDENT_OR_INSERT_TAB,
	ID_EDIT_DECREASE_LINE_INDENT_OR_INSERT_TAB,

	ID_EDIT_LEFT,
	ID_EDIT_EXTEND_LEFT,
	ID_EDIT_WORD_LEFT,
	ID_EDIT_EXTEND_WORD_LEFT,

	ID_EDIT_RIGHT,
	ID_EDIT_EXTEND_RIGHT,
	ID_EDIT_WORD_RIGHT,
	ID_EDIT_EXTEND_WORD_RIGHT,

	ID_EDIT_UP,
	ID_EDIT_EXTEND_UP,
	ID_EDIT_SCROLL_UP,

	ID_EDIT_DOWN,
	ID_EDIT_EXTEND_DOWN,
	ID_EDIT_SCROLL_DOWN,

	ID_EDIT_PAGEUP,
	ID_EDIT_EXTEND_PAGEUP,
	ID_EDIT_SCROLL_PAGEUP,
	ID_EDIT_EXTEND_SCROLL_PAGEUP,

	ID_EDIT_PAGEDOWN,
	ID_EDIT_EXTEND_PAGEDOWN,
	ID_EDIT_SCROLL_PAGEDOWN,
	ID_EDIT_EXTEND_SCROLL_PAGEDOWN,

	ID_EDIT_LINE_END,
	ID_EDIT_EXTEND_LINE_END,
	ID_EDIT_END,
	ID_EDIT_EXTEND_END,

	ID_EDIT_LINE_HOME,
	ID_EDIT_EXTEND_LINE_HOME,
	ID_EDIT_HOME,
	ID_EDIT_EXTEND_HOME,

	ID_EDIT_BACKSPACE,
	ID_EDIT_DELETE_PREVIOUS_WORD,
	ID_EDIT_INSERT_NEWLINE,
	ID_EDIT_INSERT_NEWLINE_ABOVE,
	ID_EDIT_INSERT_NEWLINE_BELOW,
	ID_EDIT_TOGGLE_INSERT_MODE,
	ID_EDIT_INSERT_DATE,
	
	ID_OPTIONS_APPLY,
	ID_OPTIONS_TREE,
	ID_OPTIONS_GENERAL_TAB,
	ID_OPTIONS_GENERAL_PAGE,
	ID_OPTIONS_EDITOR_TAB,
	ID_OPTIONS_EDITOR_PAGE,
	ID_OPTIONS_FONT_TAB,
	ID_OPTIONS_FONT_PAGE,
	ID_OPTIONS_ALL_CONTEXT_MENU,
	ID_OPTIONS_TXT_ASSOCIATION,
	ID_OPTIONS_INI_ASSOCIATION,
	ID_OPTIONS_NFO_ASSOCIATION,
	ID_OPTIONS_ALL_ASSOCATIONS,
	ID_OPTIONS_NONE_ASSOCATIONS,
	ID_OPTIONS_TABSIZE_STATIC,
	ID_OPTIONS_TABSIZE_EDIT,
	ID_OPTIONS_FOREGROUND_COLOR_STATIC,
	ID_OPTIONS_FOREGROUND_COLOR_BUTTON,
	ID_OPTIONS_BACKGROUND_COLOR_STATIC,
	ID_OPTIONS_BACKGROUND_COLOR_BUTTON,
	ID_OPTIONS_SELECTED_FOREGROUND_COLOR_STATIC,
	ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON,
	ID_OPTIONS_SELECTED_BACKGROUND_COLOR_STATIC,
	ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON,
	ID_OPTIONS_FONT_BITMAP_STATIC,
	ID_OPTIONS_FONT_BITMAP_EDIT,
	ID_OPTIONS_FONT_BITMAP_BROWSE,
	ID_OPTIONS_CP_STATIC,
	ID_OPTIONS_CP_EDIT,
	
};

enum
{
	VOIDPAD_COMMAND_NEW,
	VOIDPAD_COMMAND_OPEN, // wchar_t *filename
	VOIDPAD_COMMAND_SAVE,
	VOIDPAD_COMMAND_SAVE_DIRECT,
	VOIDPAD_COMMAND_SAVEAS, // wchar_t *filename
	VOIDPAD_COMMAND_SAVEAS_DIRECT, // wchar_t *filename
	
	VOIDPAD_COMMAND_UNDO,
	VOIDPAD_COMMAND_REDO,
	VOIDPAD_COMMAND_CUT,
	VOIDPAD_COMMAND_COPY,
	VOIDPAD_COMMAND_PASTE,
	VOIDPAD_COMMAND_CYCLE_CLIPBOARD_RING,
	VOIDPAD_COMMAND_DELETE,
	VOIDPAD_COMMAND_DELETE_LINE,
	VOIDPAD_COMMAND_SELECT_ALL,
	VOIDPAD_COMMAND_REPLACE, // unsigned char *find, unsigned char *replace, DWORD options.
	VOIDPAD_COMMAND_REPLACEALL, // unsigned char *find, unsigned char *replace, DWORD options.
	VOIDPAD_COMMAND_FIND_PREV,
	VOIDPAD_COMMAND_FIND_NEXT,
	VOIDPAD_COMMAND_GOTO, // DWORD line

	VOIDPAD_COMMAND_MAKE_UPPERCASE,
	VOIDPAD_COMMAND_MAKE_LOWERCASE,
	VOIDPAD_COMMAND_INCREMENT_INTEGER,
	VOIDPAD_COMMAND_DECREMENT_INTEGER,
	VOIDPAD_COMMAND_VIEW_WHITE_SPACE,
	VOIDPAD_COMMAND_INCREASE_LINE_INDENT_OR_INSERT_TAB,
	VOIDPAD_COMMAND_DECREASE_LINE_INDENT_OR_INSERT_TAB,
	VOIDPAD_COMMAND_INCREASE_LINE_INDENT,
	VOIDPAD_COMMAND_DECREASE_LINE_INDENT,

	VOIDPAD_COMMAND_CHAR,

	VOIDPAD_COMMAND_CURSOR_LEFT,
	VOIDPAD_COMMAND_CURSOR_EXTEND_LEFT,
	VOIDPAD_COMMAND_CURSOR_WORD_LEFT,
	VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_LEFT,

	VOIDPAD_COMMAND_CURSOR_RIGHT,
	VOIDPAD_COMMAND_CURSOR_EXTEND_RIGHT,
	VOIDPAD_COMMAND_CURSOR_WORD_RIGHT,
	VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_RIGHT,

	VOIDPAD_COMMAND_CURSOR_UP,
	VOIDPAD_COMMAND_CURSOR_EXTEND_UP,
	VOIDPAD_COMMAND_CURSOR_SCROLL_UP,

	VOIDPAD_COMMAND_CURSOR_DOWN,
	VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN,
	VOIDPAD_COMMAND_CURSOR_SCROLL_DOWN,

	VOIDPAD_COMMAND_CURSOR_PAGEUP,
	VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEUP,
	VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEUP,
	VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEUP,

	VOIDPAD_COMMAND_CURSOR_PAGEDOWN,
	VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEDOWN,
	VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEDOWN,
	VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEDOWN,

	VOIDPAD_COMMAND_CURSOR_END,
	VOIDPAD_COMMAND_CURSOR_EXTEND_END,
	VOIDPAD_COMMAND_CURSOR_LINE_END,
	VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_END,

	VOIDPAD_COMMAND_CURSOR_HOME,
	VOIDPAD_COMMAND_CURSOR_EXTEND_HOME,
	VOIDPAD_COMMAND_CURSOR_LINE_HOME,
	VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_HOME,
	
	VOIDPAD_COMMAND_DELETE_PREVIOUS_WORD,
	VOIDPAD_COMMAND_BACKSPACE,
	
	VOIDPAD_COMMAND_INSERT_NEWLINE,
	VOIDPAD_COMMAND_INSERT_NEWLINE_ABOVE,
	VOIDPAD_COMMAND_INSERT_NEWLINE_BELOW,
	
	VOIDPAD_COMMAND_TOGGLE_INSERT_MODE,

	VOIDPAD_COMMAND_FIND_SET_SEARCH,
	VOIDPAD_COMMAND_FIND_SET_REPLACE,
	
	VOIDPAD_COMMAND_FIND_CASE_ENABLE,
	VOIDPAD_COMMAND_FIND_CASE_DISABLE,

	VOIDPAD_COMMAND_FIND_WHOLEWORD_ENABLE,
	VOIDPAD_COMMAND_FIND_WHOLEWORD_DISABLE,
	
	VOIDPAD_COMMAND_INSERT_TEXT,
	VOIDPAD_COMMAND_INSERT_DATE,
};

// pages
enum
{
	VOIDPAD_OPTIONS_PAGE_GENERAL,
	VOIDPAD_OPTIONS_PAGE_EDITOR,
	VOIDPAD_OPTIONS_PAGE_FONT,
	VOIDPAD_OPTIONS_PAGE_COUNT,
};

typedef unsigned char utf8_t;

typedef struct voidpad_options_page_s
{
	int tab_id;
	int page_id;
	const wchar_t *name;
	
}voidpad_options_page_t;

voidpad_options_page_t voidpad_options_pages[] = 
{
	{ID_OPTIONS_GENERAL_TAB,ID_OPTIONS_GENERAL_PAGE,L"General"},
	{ID_OPTIONS_EDITOR_TAB,ID_OPTIONS_EDITOR_PAGE,L"Editor"},
	{ID_OPTIONS_FONT_TAB,ID_OPTIONS_FONT_PAGE,L"Font"},
};

typedef struct line_chunk_s
{
	struct line_chunk_s *next;
	
}line_chunk_t;

typedef struct free_line_s
{
	int count;
	struct free_line_s *next;
	
}free_line_t;

typedef struct line_s
{
	unsigned char *text;
	unsigned char *memory;
	struct line_s *next;
	int size;
	
	// length of the line in characters.
	// including newline character.
	int len;
	
}line_t;

typedef struct undo_s
{
	// text that was removed.
	unsigned char *deleted_text;
	int deleted_len;
	int del_x;
	int del_y;
	
	// save mark position
	// note that there might not be a selection.
	int mark_x;
	int mark_y;
	
	// the cursor position before text is added.
	int cursor_x;
	int cursor_y;
	
	// save the selection, it is possible there is no selection.
	int sel_x1;
	int sel_y1;
	int sel_x2;
	int sel_y2;
	
	// text that was added
	int inserted_len;
	int inserted_x;
	int inserted_y;
	
	// text that was added
	int delete_len;
	
	int is_more;
	int is_char;
	int is_clipboard_ring;

	// saved marker.
	int is_saved;
	
	struct undo_s *prev;
	struct undo_s *next;
	
}undo_t;

typedef struct redo_s
{
	// text that was removed.
	unsigned char *deleted_text;
	int deleted_len;
	int del_x;
	int del_y;
	
	// save mark position
	// note that there might not be a selection.
	int mark_x;
	int mark_y;
	
	// the cursor position before text is added.
	int cursor_x;
	int cursor_y;
	
	// save the selection, it is possible there is no selection.
	int sel_x1;
	int sel_y1;
	int sel_x2;
	int sel_y2;
	
	// text that was added
	int inserted_len;
	int inserted_x;
	int inserted_y;
	
	int is_more;
	
	// saved marker.
	int is_saved;
	
	struct redo_s *prev;
	struct redo_s *next;
	
}redo_t;

typedef struct replace_s
{
	unsigned char *text;
	
	struct replace_s *next;
	struct replace_s *prev;
	
}replace_t;

typedef struct find_s
{
	// find text in voidpad_cp.
	unsigned char *text;
	
	struct find_s *next;
	struct find_s *prev;
	
}find_t;

typedef struct macro_s
{
	int command;
	void *data;
	int size;
	struct macro_s *next;
	
}macro_t;

typedef struct voidpad_clipboard_ring_s
{
	unsigned char *text;
	int len;
	
	struct voidpad_clipboard_ring_s *next;
	struct voidpad_clipboard_ring_s *prev;
	
}voidpad_clipboard_ring_t;

typedef struct key_s
{
	int id;
	unsigned char vk;
	unsigned char scope;
	unsigned char flags;

}key_t;

// memory:
#ifdef _DEBUG

	typedef struct mem_s
	{
		char *file;
		int line;
		int size;
	
		struct mem_s *next;
		struct mem_s *prev;
		
		DWORD magic;
		
	}mem_t;

	static mem_t *mem_start = 0;
	static mem_t *mem_last = 0;

	#define mem_alloc(size) mem_alloc_FILE_LINE(__FILE__,__LINE__,size)
	#define mem_free(ptr) mem_free_FILE_LINE(__FILE__,__LINE__,ptr)

	static void *mem_alloc_FILE_LINE(char *file,int line,int size);
	static void mem_free_FILE_LINE(char *file,int line,void *ptr);
	static void mem_DEBUG(void);

#else

	static void *mem_alloc(int size);
	static void mem_free(void *ptr);

#endif

static void size_window(void);
static int get_vscroll(void);
static int get_vpage(void);
static int get_vmax(void);
static void new_file(int ask_save_changes);
static void voidpad_open_dialog(void);
static void voidpad_open(const wchar_t *filename);
static void build_line_list(void);
static void update_title(void);
static int get_nearest_cursor_colx(void);
static void get_cursor_rect(RECT *rect);
static void set_vscroll(int pos);
static int get_cursor_chx(void);
static int get_line_chend(int y);
static void set_cursor_col_position(int x,int y,int reset_col_selection,int reset_find,int bring_into_view);
static void set_cursor_ch_position(int x,int y,int reset_col_selection,int reset_find,int bring_into_view);
static int ch_to_col(int ch,int ln);
static int col_to_ch(int col,int ln);
static void update_ln_status(void);
static void update_col_status(void);
static void update_ch_status(void);
static void update_ins_status(void);
static wchar_t *wstring_alloc(const wchar_t *s);
static wchar_t *wstring_alloc_voidpad_cp(const utf8_t *s);
static wchar_t *wstring_alloc_utf8(const utf8_t *s);
static void wstring_copy(wchar_t *buf,int size,const wchar_t *s);
static void wstring_cat(wchar_t *buf,int size,const wchar_t *s);
static void wstring_cat_voidpad_cp(wchar_t *buf,int size,const unsigned char *s);
static int voidpad_saveas(void);
static int voidpad_save(void);
static int voidpad_save_direct(void);
static wchar_t *wstring_format_number(wchar_t *buf,int number);
static wchar_t *string_filepart(const wchar_t *s);
static int string_compare(const unsigned char *s1,const unsigned char *s2);
static int get_hscroll(void);
static int get_hpage(void);
static int get_hmax(void);
static void set_hscroll(int pos);
static void set_col_sel(int x1,int y1,int x2,int y2);
static int compare_ch_ln(int x1,int y1,int x2,int y2);
int string_length(const unsigned char *text);
void string_cat(unsigned char *buf,unsigned char *s);
int wstring_length(const wchar_t *text);
static int clip_ln(int y);
static int clip_col(int x,int clipped_y);
static void free_line(line_t *l);
static void clear(void);
static void delete_selection(int is_replace);
static int is_selection(void);
static void insert_text(const unsigned char *text,int len,int allow_undo,int select,int bring_into_view,int is_replace);
static void invalidate_range(int x1,int y1,int x2,int y2);
static void voidpad_cut(void);
static void voidpad_copy(void);
static void voidpad_paste(int select);
static int voidpad_exit(void);
static int save_changes(void);
static int next_ch(int *chx,int *y);
static int prev_ch(int *chx,int *y);
static void set_insert_mode(int mode);
static void voidpad_select_all(void);
static void voidpad_insert_hex_dialog(void);
static INT_PTR CALLBACK voidpad_insert_hex_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void voidpad_insert_utf8_sequence_dialog(void);
static INT_PTR CALLBACK voidpad_insert_utf8_sequence_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void voidpad_insert_utf16_sequence_dialog(void);
static INT_PTR CALLBACK voidpad_insert_utf16_sequence_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void voidpad_goto_dialog(void);
static void voidpad_goto_update_next_button(HWND hwnd);
INT_PTR CALLBACK voidpad_goto_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void voidpad_find_auto_fill(void);
static void voidpad_find_dialog(int replace);
static LRESULT CALLBACK voidpad_find_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static int voidpad_is_key(int vk,int flags,int scope);
static int voidpad_msg(MSG *msg);
static int voidpad_command(int command);
static int voidpad_command_data(int command,const void *data,int size);
static void select_word(int chx,int y,int *pleft,int *pright);
static int select_alphanum_word(int chx,int y,int *pleft,int *pright);
static int prev_word(int *chx,int *y);
static int next_word(int *chx,int *y);
static int can_read_from(int chx,int y);
static int char_type(int chx,int y);
static void os_set_default_font(HWND hwnd);
static void set_window_text(HWND hwnd,unsigned char *start,int len);
static int search(int direction,int is_replace,int is_replace_all);
static wchar_t *reg_alloc_list_item(wchar_t *value,unsigned char **item);
//static unsigned char *ini_alloc_list_item(unsigned char *value,unsigned char **item);
static int ini_is_key(const unsigned char *ini_key,const char *key);
static void write_ini_int(HANDLE h,const utf8_t *key,int value);
static void write_ini_string(HANDLE h,const utf8_t *key,wchar_t *value);
//static void file_write_ANSI(HANDLE h,const unsigned char *s,int escape);
static void file_write_utf8(HANDLE h,const utf8_t *s);
static void save_settings(void);
static LRESULT CALLBACK voidpad_edit_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void voidpad_macro_play(void);
static void voidpad_macro_record(void);
static void voidpad_macro_add(int command,const void *data,int size);
static void voidpad_macro_clear(void);
static void voidpad_clipboard_ring_add(const unsigned char *text);
static void voidpad_clipboard_ring_insert_at_start(voidpad_clipboard_ring_t *ccr);
static void voidpad_clipboard_ring_insert_at_end(voidpad_clipboard_ring_t *ccr);
static void voidpad_clipboard_ring_remove(voidpad_clipboard_ring_t *ccr);
static void voidpad_clipboard_ring_delete(voidpad_clipboard_ring_t *ccr);
static void voidpad_clipboard_ring_free(voidpad_clipboard_ring_t *cr);
static void voidpad_clipboard_ring_clear(void);
static int is_selected_pixel(int x,int y);
static void undo_add(int len,int is_more,int selection_deleted);
static void undo_add_selection(void);
static void undo_clear_selection(void);
static void undo(void);
static void redo(void);
static void undo_delete(undo_t *u);
static void redo_delete(redo_t *u); 
static void undo_free(undo_t *u);
static void redo_free(redo_t *u);
static int get_selection_len(void);
static unsigned char *alloc_selection(int *len);
static void col_ln_add_len(int x,int y,int len,int *out_x,int *out_y);
static void initmenupopup(HMENU menu,int id);
void os_remove_all_menu_items(HMENU hmenu);
static void get_col_ln_from_x_y(int x,int y,int *pcol,int *pln);
static void insert_new_line(int eol,int below);
static int get_integer(const unsigned char *p,int len);
static void draw_space(HDC hdc,int x,int y,int col,int ln);
static void fill_rect(HDC hdc,int x,int y,int wide,int high,COLORREF color);
static void redo(void);
static void redo_clear(void);
static void undo_clear(void);
static int voidpad_is_ws(wchar_t c);
static wchar_t *voidpad_skip_ws(wchar_t *p);
static wchar_t *voidpad_get_word(wchar_t *p,wchar_t *buf);
static void voidpad_process_command_line(wchar_t *cl);
static int voidpad_main(void);
static BOOL _os_emulate_IsAppThemed(void);
static BOOL _os_emulate_IsThemeActive(void);
static void _os_get_proc_address(HMODULE hmodule,const char *name,FARPROC *proc);
static void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect);
static void os_make_rect_completely_visible(RECT *prect);
static HMONITOR WINAPI _os_Emulate_MonitorFromRect(LPCRECT lprc,DWORD dwFlags);
static INT_PTR os_create_blank_dialog(HWND parent,DLGPROC proc,void *param);
static void os_center_dialog(HWND hwnd,int wide,int high);
static void ini_int(const unsigned char *ini_key,const unsigned char *ini_value,const char *key,int *var);
static void ini_string(const unsigned char *ini_key,const unsigned char *ini_value,const char *key,wchar_t **var);
static INT_PTR __stdcall _os_ds_setfont_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void os_load_default_hfont(void);
static int search_compare(unsigned char *p,unsigned char *e);
static int search_reverse_compare(unsigned char *p,unsigned char *start);
static INT_PTR __stdcall voidpad_multiline_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int is_alphanum(int c);
static int is_word(unsigned char *start,unsigned char *p,unsigned char *e,int len);
static void voidpad_find_position_widgets(int replace);
static void voidpad_find_delete(find_t *f);
static find_t *find_find(const unsigned char *text);
static find_t *find_add(const unsigned char *text);
static void voidpad_replace_delete(replace_t *f);
static replace_t *replace_find(const unsigned char *text);
static replace_t *replace_add(const unsigned char *text);
static void voidpad_combobox_insert_string(HWND hwnd,int id,int index,unsigned char *text);
static void voidpad_find_delete(find_t *f);
static void voidpad_find_update_search_combobox(void);
static void voidpad_find_update_replace_combobox(void);
static int is_indent(int y);
static void voidpad_bring_cursor_into_view(void);
static void voidpad_indent(int isshift);
static wchar_t *wstring_alloc_window_text(HWND hwnd);
static wchar_t *os_alloc_combo_list_text(HWND hwnd,int index);
static unsigned char *utf8_alloc_wchar(const wchar_t *wbuf);
static unsigned char *os_alloc_wchar_to_voidpad_cp(wchar_t *wbuf);
static void cursor_hide(void);
static void select_main(int word_left,int word_right,int word_y);
static int voidpad_tolower(int c);
static void voidpad_set_capture(int flag);
static void voidpad_release_capture(int flag);
static int voidpad_can_paste(void);
static void voidpad_find_search_changed(void);
static void voidpad_find_replace_changed(void);
static void combobox_set_position(HWND hwnd,int id,int x,int y,int wide,int high);
static void voidpad_options_main(void);
static void voidpad_help_about(void);
static HTREEITEM voidpad_options_add_treeitem(HWND hwnd,int id,HTREEITEM parent,int index,const wchar_t *name);
static INT_PTR CALLBACK voidpad_options_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void voidpad_options_treeview_changed(HWND hwnd);
static HWND voidpad_options_add_page(HWND hwnd,int tabid,int pageid,const wchar_t *name);
static INT_PTR CALLBACK voidpad_options_page_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void os_dlg_create_checkbox(HWND parent,int id,DWORD extra_style,int checked,const wchar_t *text,int x,int y,int wide);
static void os_dlg_create_button(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide);
static void os_dlg_create_static(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide);
static void os_dlg_create_edit(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide);
static void voidpad_install_all_context_menu(void);
static void voidpad_uninstall_all_context_menu(void);
static int voidpad_is_all_context_menu(void);
static void voidpad_install_association(const wchar_t *association,const wchar_t *description);
static void voidpad_uninstall_association(const wchar_t *assocation);
static int voidpad_is_association(const wchar_t *assocation);
static int wstring_compare(const wchar_t *s1,const wchar_t *s2);
static int voidpad_get_registry_string(HKEY hkey,const wchar_t *value,wchar_t *wbuf,int size_in_wchars);
static int voidpad_set_registry_string(HKEY hkey,const wchar_t *value,const wchar_t *wbuf);
static void voidpad_tab_size_changed(void);
static int voidpad_clip_tabsize(int tab_size);
static void voidpad_options_color_changed(HWND hwnd,int id);
static void free_line_add(free_line_t *fl);
static line_t *line_alloc(void);
static line_t *line_add(unsigned char *text,int len);
static int expand_wide(HWND hwnd,int wide,const wchar_t *text);
static void font_load(void);
static int os_rename_file(const wchar_t *old_name,const wchar_t *new_name,DWORD flags);
static utf8_t *ini_open(wchar_t *filename);
static int viv_reg_read_int(HKEY hkey,wchar_t *name,int *pvalue);
static int viv_reg_read_string(HKEY hkey,wchar_t *name,wchar_t **pvalue);
//static void voidpad_reg_write_int(HKEY hkey,wchar_t *name,int value);
//static void voidpad_reg_write_string(HKEY hkey,wchar_t *name,wchar_t *value);
static int wstring_get_escape_char(wchar_t c);
static wchar_t *wstring_copy_escaped_wstring(wchar_t *buf,const wchar_t *text);
static int _voidpad_hexchar(int n);
static int _voidpad_hexvalue(int h);
static void voidpad_load_settings(void);

static int voidpad_cp = 437;
static int voidpad_select_all_bring_into_view = 0;
static COLORREF voidpad_choose_color_cust_colors[16] = {RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255)};
HRESULT (WINAPI *os_EnableThemeDialogTexture)(__in HWND hwnd,__in DWORD dwFlags) = 0;
static UINT voidpad_clipboard_format = 0;
static POINT voidpad_scroll_pt;
static int voidpad_capture = 0;
static replace_t *replace_start = 0;
static replace_t *replace_last = 0;
static int replace_count = 0;
static find_t *find_start = 0;
static find_t *find_last = 0;
static int find_count = 0;
static HFONT os_default_hfont;
static HMONITOR (WINAPI *_os_MonitorFromRect)(LPCRECT lprc,DWORD dwFlags) = _os_Emulate_MonitorFromRect;
HMODULE _os_uxtheme_hdll = 0;
HMODULE _os_user32_hdll = 0;
BOOL (WINAPI *os_IsAppThemed)(VOID) = _os_emulate_IsAppThemed;
BOOL (WINAPI *os_IsThemeActive)(VOID) = _os_emulate_IsThemeActive;
static int view_white_space = 0;
static undo_t *undo_start = 0;
static undo_t *undo_last = 0;
int undo_last_char_x = 0;
int undo_last_char_y = 0;
static redo_t *redo_start = 0;
static redo_t *redo_last = 0;
static voidpad_clipboard_ring_t *voidpad_clipboard_ring_start = 0;
static voidpad_clipboard_ring_t *voidpad_clipboard_ring_last = 0;
static voidpad_clipboard_ring_t *voidpad_clipboard_ring_current = 0;
int voidpad_clipboard_ring_count = 0;
int voidpad_clipboard_ring_size = 20;
static macro_t *voidpad_macro_start = 0;
static macro_t *voidpad_macro_last = 0;
static HWND voidpad_hwnd = 0;
static HWND status_hwnd = 0;
static HWND edit_hwnd = 0;
static HWND find_hwnd = 0;
static HWND replace_hwnd = 0;
static unsigned char *file_data = 0;
//static HANDLE file_memory_map_handle = 0;
static free_line_t *free_line_start = 0;
static free_line_t *free_line_last = 0;
static line_chunk_t *line_chunk_start = 0;
static line_chunk_t *line_chunk_last = 0;
static line_t **line_list=0;
static line_t *line_start=0;
static line_t *line_last=0;
static int line_list_size=0;
static int line_count=0;
static int cursor_flash=1;
static int cursor_x = 0; // in columns, since we can position the cursor after a line of text chs.
static int cursor_y = 0;
static int cursor_blink_time=0;
static int font_wide;
static int font_high;
static int text_wide;
static wchar_t *voidpad_filename=0;
static int window_x = CW_USEDEFAULT;
static int window_y = CW_USEDEFAULT;
static int window_wide = CW_USEDEFAULT;
static int window_high = CW_USEDEFAULT;
static int find_x = CW_USEDEFAULT;
static int find_y = CW_USEDEFAULT;
static int find_case = 0;
static int find_wholeword = 0;
static int sel_x1 = 0;
static int sel_y1 = 0;
static int sel_x2 = 0;
static int sel_y2 = 0;
static int mark_x = 0;
static int mark_y = 0;
static int find_found_count = 0;
static int find_last_chx = -1;
static int find_last_y = -1;
static int find_start_direction = 0;
static int find_start_chx = -1;
static int find_start_y = 0;
static int voidpad_macro_is_recording = 0;
static WNDPROC voidpad_old_edit_proc;

static int background_r = 0;
static int background_g = 0;
static int background_b = 128;
static int foreground_r = 192;
static int foreground_g = 192;
static int foreground_b = 192;
static int selected_background_r = 255;
static int selected_background_g = 255;
static int selected_background_b = 255;
static int selected_foreground_r = 0;
static int selected_foreground_g = 0;
static int selected_foreground_b = 0;

static wchar_t *font_bitmap_filename = 0;
static HBITMAP font_hbitmap = 0;
static HGDIOBJ last_font_bitmap = 0;
static HDC font_hdc;
static int voidpad_tab_size = 4;
static int is_dirty = 0;
static int is_undo_saved = 0;
static int insert_mode = VOIDPAD_INSERT_MODE_INSERT;
//static unsigned char **clipboard_ring = 0;
//int clipboard_ring_pos = 0;
static int cursor_timer_set = 0;
static unsigned char *search_string = 0;
static int search_string_len = 0;
static unsigned char *replace_string = 0;
static int replace_string_len = 0;

static key_t keys[] = 
{
	{ID_EDIT_INCREASE_LINE_INDENT_OR_INSERT_TAB,VK_TAB,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_DECREASE_LINE_INDENT_OR_INSERT_TAB,VK_TAB,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},

	{ID_EDIT_LEFT,VK_LEFT,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_LEFT,VK_LEFT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_WORD_LEFT,VK_LEFT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_WORD_LEFT,VK_LEFT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT|VOIDPAD_KEY_FLAG_CONTROL},

	{ID_EDIT_RIGHT,VK_RIGHT,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_RIGHT,VK_RIGHT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_WORD_RIGHT,VK_RIGHT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_WORD_RIGHT,VK_RIGHT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT|VOIDPAD_KEY_FLAG_CONTROL},

	{ID_EDIT_UP,VK_UP,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_UP,VK_UP,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_SCROLL_UP,VK_UP,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},

	{ID_EDIT_DOWN,VK_DOWN,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_DOWN,VK_DOWN,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_SCROLL_DOWN,VK_DOWN,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},

	{ID_EDIT_PAGEUP,VK_PRIOR,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_PAGEUP,VK_PRIOR,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_SCROLL_PAGEUP,VK_PRIOR,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_SCROLL_PAGEUP,VK_PRIOR,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},

	{ID_EDIT_PAGEDOWN,VK_NEXT,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_PAGEDOWN,VK_NEXT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_SCROLL_PAGEDOWN,VK_NEXT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_SCROLL_PAGEDOWN,VK_NEXT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},

	{ID_EDIT_LINE_END,VK_END,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_LINE_END,VK_END,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_END,VK_END,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_END,VK_END,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},

	{ID_EDIT_LINE_HOME,VK_HOME,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_EXTEND_LINE_HOME,VK_HOME,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_HOME,VK_HOME,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_EXTEND_HOME,VK_HOME,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},

	{ID_EDIT_SELECT_ALL,'A',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_COPY,'C',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_GOTO,'G',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_INCREMENT_INTEGER,'I',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_DECREMENT_INTEGER,'I',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_MAKE_LOWERCASE,'U',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_MAKE_UPPERCASE,'U',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_PASTE,'V',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_CUT,'X',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_DELETE_LINE,'Y',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_UNDO,'Z',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_REDO,'Z',VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_BACKSPACE,VK_BACK,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_BACKSPACE,VK_BACK,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_UNDO,VK_BACK,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_ALT},
	{ID_EDIT_DELETE_PREVIOUS_WORD,VK_BACK,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_INSERT_NEWLINE,VK_RETURN,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_INSERT_NEWLINE,VK_RETURN,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_INSERT_NEWLINE_ABOVE,VK_RETURN,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_INSERT_NEWLINE_BELOW,VK_RETURN,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_DELETE,VK_DELETE,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_CUT,VK_DELETE,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_TOGGLE_INSERT_MODE,VK_INSERT,VOIDPAD_KEY_SCOPE_EDIT,0},
	{ID_EDIT_PASTE,VK_INSERT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_COPY,VK_INSERT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_CYCLE_CLIPBOARD_RING,VK_INSERT,VOIDPAD_KEY_SCOPE_EDIT,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_EDIT_INSERT_DATE,VK_F5,VOIDPAD_KEY_SCOPE_EDIT,0},
						
	{ID_EDIT_FIND,'F',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_REPLACE,'H',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_FILE_NEW,'N',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_FILE_OPEN,'O',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_TOOLS_MACRO_RECORD,'R',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_TOOLS_MACRO_PLAY,'P',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
//	{ID_TOOLS_OPTIONS,'T',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL|VOIDPAD_KEY_FLAG_SHIFT},
	{ID_HELP_ABOUT,VK_F1,VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_FILE_SAVE,'S',VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_CONTROL},
	{ID_EDIT_FIND_NEXT,VK_F3,VOIDPAD_KEY_SCOPE_GLOBAL,0},
	{ID_EDIT_FIND_PREV,VK_F3,VOIDPAD_KEY_SCOPE_GLOBAL,VOIDPAD_KEY_FLAG_SHIFT},
};

const DWORD crc_table[256] = 
{
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
  0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
  0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
  0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
  0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
  0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
  0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
  0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
  0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
  0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
  0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
  0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
  0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
  0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
  0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
  0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
  0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
  0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
  0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
  0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
  0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
  0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
  0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
  0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
  0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
  0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
  0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
  0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
  0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
  0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
  0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
  0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
  0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
  0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
  0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
  0x2d02ef8d
};

// the key lookup table  0123456789012345
const char *key_table = "347CDFGHJKMPRWXY"; 

#define KEY_COUNT (sizeof(keys) / sizeof(key_t))

#pragma function(memset)
void * __cdecl memset(void *p, int v, size_t s) {
    unsigned char *d;
    
    d = p;
    while (s) 
    {
        *d++ = v;
        
        s--;
    }
    
    return d;
}

// memset
// does not need to be fast, only used to initial small structs
// if you are initializing more than 4k your doing it wrong.
// dont use minicrt's very slow memset!
// crt_zero_memory(dst,256*1024*1024) results:
// 141ms DWORD set known aligned, with no alignment adjustment.
// 172 DWORD set unaligned.
// 156 DWORD set unaligned, with alignment adjustment.*
void crt_zero_memory(void *ptr,int size)
{
	unsigned char *d;
	
	d = ptr;
		
	{	
		int run;

		run = size / sizeof(uintptr_t);
		
		while(run)
		{
			*(uintptr_t *)d = 0;
			
			d += sizeof(uintptr_t);
			
			run--;
		}
	}

	{	
		int run;

		run = size & (sizeof(uintptr_t) - 1);
		
		while(run)
		{
			*d++ = 0;
			
			run--;
		}
	}
}

// copy (dst and src MUST NOT OVERLAP)
// MUST be fast!
// dont use minicrt's very slow memcpy!
// memcpy(dst,src,256*1024*1024) results:
// 328 ms (4byte copies debug)
// 250 ms (4byte copies release)*
// 266 ms (32byte copies release)
// 266 ms __movsd(dst,src,(256*1024*1024) / 4);
// 766 ms (minicrt)

void crt_copy_memory(void *dst,const void *src,int size)
{
	unsigned char *d;
	const unsigned char *s;
	
	d = dst;
	s = src;
		
	{	
		int run;

		run = size / sizeof(uintptr_t);
		
		while(run)
		{
			*(uintptr_t *)d = *(uintptr_t *)s;
			
			d += sizeof(uintptr_t);
			s += sizeof(uintptr_t);
			
			run--;
		}
	}

	{	
		int run;

		run = size & (sizeof(uintptr_t) - 1);
		
		while(run)
		{
			*d++ = *s++;
			
			run--;
		}
	}
}

// move with possible overlap
void crt_move_memory(void *dst,const void *src,int size)
{
	unsigned char *d;
	const unsigned char *s;
		
	// no overlap, just use normal copy.
	if ((dst <= src) || ((char *)dst >= ((char *)src) + size))
	{
		crt_copy_memory(dst,src,size);

		return;
	}
	
	d = ((unsigned char *)dst) + size;
	s = ((unsigned char *)src) + size;
		
	{	
		int run;

		run = size / sizeof(uintptr_t);
		
		while(run)
		{
			d -= sizeof(uintptr_t);
			s -= sizeof(uintptr_t);
			
			*(uintptr_t *)d = *(uintptr_t *)s;
			
			run--;
		}
	}

	{	
		int run;

		run = size & (sizeof(uintptr_t) - 1);
		
		while(run)
		{
			d--;
			s--;
			
			*d = *s;
			
			run--;
		}
	}
}

void string_cat(unsigned char *buf,unsigned char *s)
{
	unsigned char *d;
	
	d = buf;
	
	while(*d)
	{
		d++;
	}
	
	while(*s)
	{
		*d++ = *s;
		
		s++;
	}
	
	*d = 0;
}


int string_length(const unsigned char *text)
{
	register const unsigned char *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (int)(p - text);
}

int wstring_length(const wchar_t *text)
{
	register const wchar_t *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (int)(p - text);
}

// http://en.wikipedia.org/wiki/Power_of_two
// WARNING: not portable.
int os_ceil_power_2(int i)
{
	i = i - 1;
	i = i | (i >> 1);
	i = i | (i >> 2);
	i = i | (i >> 4);
	i = i | (i >> 8);
	i = i | (i >> 16);
	i = i + 1;
	
	return i;
}

static void free_line(line_t *l)
{
	if (l->memory) 
	{
		mem_free(l->memory);
	}
}

static void clear(void)
{
	SCROLLINFO si;
	
	if (line_list)
	{
		int i;
		
		for(i=0;i<line_count;i++)
		{
			free_line(line_list[i]);
		}

		mem_free(line_list);
	}
	
	if (file_data)
	{
		VirtualFree(file_data,0,MEM_RELEASE);
	
		file_data = 0;
	}
	
//	if (file_memory_map_handle)
//	{
//		CloseHandle(file_memory_map_handle);
//		
//		file_memory_map_handle = 0;
//	}

	{
		line_chunk_t *lc;
		
		lc = line_chunk_start;
		while(lc)
		{
			line_chunk_t *next_lc;
			
			next_lc = lc->next;
			
			mem_free(lc);
			
			lc = next_lc;
		}
	}
	
	line_chunk_start = 0;
	line_chunk_last = 0;
	free_line_start = 0;
	free_line_last = 0;
	
	cursor_x = 0;
	cursor_y = 0;
	cursor_flash = 1;
	line_count = 0;
	line_start = 0;
	line_last = 0;
	line_list = 0;
	text_wide = 0;
	line_list_size = 0;
	sel_x1 = 0;
	sel_y1 = 0;
	sel_x2 = 0;
	sel_y2 = 0;
	mark_x = 0;
	mark_y = 0;
	is_dirty = 0;
	is_undo_saved = 0;

	find_found_count = 0;
	find_last_chx = -1;
	find_last_y = -1;
	find_start_direction = 0;
	find_start_chx = -1;
	find_start_y = 0;
	
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = 0;
	
	SetScrollInfo(edit_hwnd,SB_VERT,&si,TRUE);
		
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = 0;
	
	SetScrollInfo(edit_hwnd,SB_HORZ,&si,TRUE);
	
	// clear undos.
	undo_clear();

	// clear redos.
	redo_clear();
}

static void free_line_add(free_line_t *fl)
{
	if (free_line_start)
	{
		free_line_last->next = fl;
	}
	else
	{
		free_line_start = fl;
	}

	fl->count = 1;
	fl->next = 0;
	
	free_line_last = fl;
}

static line_t *line_alloc(void)
{
	line_t *l;
	
	if (free_line_start)
	{
		l = (line_t *)free_line_start;
		
		if (free_line_start->count > 1)
		{
			((free_line_t *)(l + 1))->count = free_line_start->count - 1;
			((free_line_t *)(l + 1))->next = free_line_start->next;

			if (free_line_last == free_line_start)
			{
				free_line_last = (free_line_t *)(l + 1);
			}
			
			free_line_start = (free_line_t *)(l + 1);
		}
		else
		{
			free_line_start = free_line_start->next;
			if (!free_line_start)
			{
				free_line_last = 0;
			}
		}
	}
	else
	{
		line_chunk_t *lc;
		free_line_t *fl;
		
		lc = mem_alloc(65536);
		
		if (line_chunk_start)
		{
			line_chunk_last->next = lc;
		}
		else
		{
			line_chunk_start = lc;
		}
		
		line_chunk_last = lc;
		lc->next = 0;
		
		l = (line_t *)(lc + 1);
		fl = (free_line_t *)(l + 1);
		
		fl->count = ((65536 - sizeof(line_chunk_t)) / sizeof(line_t)) - 1;
		
		free_line_start = fl;
		free_line_last = fl;
		fl->next = 0;
	}
	
	return l;
}

static line_t *line_add(unsigned char *text,int len)
{
	line_t *l;

	l = line_alloc();	
	
	l->memory = 0;
	l->text = text;
	l->size = 0;
	l->len = len;

	if (line_start)
	{
		line_last->next = l;
	}
	else
	{
		line_start = l;
	}
	
	line_last = l;
	l->next = 0;
	line_count++;
	
	return l;
}

static void new_file(int ask_save_changes)
{
	if (ask_save_changes)
	{
		if (!save_changes()) return;
	}

	clear();

	line_add(0,0);
	
	build_line_list();
	
	if (voidpad_filename)
	{
		mem_free(voidpad_filename);
		
		voidpad_filename = 0;
	}
	
	update_title();
	update_ln_status();	
	update_col_status();	
	update_ch_status();	

	InvalidateRect(edit_hwnd,0,FALSE);

	size_window();
}

static void build_line_list(void)
{
	line_t **lp;
	line_t *l;
	
	// create line indexes
	line_list_size = line_count;
	line_list = (line_t **)mem_alloc(sizeof(line_t *) * line_list_size);
	lp = line_list;
	
	l = line_start;
	while(l)
	{
		*lp++ = l;
		
		l = l->next;
	}
}

static void update_title(void)
{
	wchar_t window_title[MAX_PATH];

	if (voidpad_filename)
	{
		wstring_copy(window_title,MAX_PATH,voidpad_filename);
	}
	else
	{
		wstring_copy(window_title,MAX_PATH,L"Untitled");
	}
	
	if (is_dirty)
	{
		wstring_cat(window_title,MAX_PATH,L"*");
	}

	wstring_cat(window_title,MAX_PATH,L" - voidPad");
	
	SetWindowTextW(voidpad_hwnd,window_title);
}

static int voidpad_saveas(void)
{
	OPENFILENAMEW ofn;
	wchar_t buf[MAX_PATH];
	int ok;
	
	ok = 0;
	
	crt_zero_memory(&ofn,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	// voidpad_filename will probably be empty...	
	wstring_copy(buf,MAX_PATH,voidpad_filename ? voidpad_filename : L"");
	
	ofn.hwndOwner = voidpad_hwnd;
	ofn.hInstance = GetModuleHandle(0);
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Save As";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"txt";
	
	if (GetSaveFileName(&ofn))
	{
		if (voidpad_command_data(VOIDPAD_COMMAND_SAVEAS,buf,(wstring_length(buf) + 1) * sizeof(wchar_t)))
		{
			ok = 1;
		}
	}
	
	return ok;
}

static int voidpad_saveas_direct(void)
{
	OPENFILENAMEW ofn;
	wchar_t buf[MAX_PATH];
	int ok;
	
	ok = 0;
	
	crt_zero_memory(&ofn,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	// voidpad_filename will probably be empty...	
	wstring_copy(buf,MAX_PATH,voidpad_filename ? voidpad_filename : L"");
	
	ofn.hwndOwner = voidpad_hwnd;
	ofn.hInstance = GetModuleHandle(0);
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Save As";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"txt";
	
	if (GetSaveFileName(&ofn))
	{
		if (voidpad_command_data(VOIDPAD_COMMAND_SAVEAS_DIRECT,buf,(wstring_length(buf) + 1) * sizeof(wchar_t)))
		{
			ok = 1;
		}
	}
	
	return ok;
}

// returns 1 if saved
//FIXME: write to another 64k-1MB buffer first. too many calls to WriteFile is bad.
// temp file is used followed by an atomic rename, powerloss means no data loss.
static int voidpad_save(void)
{
	HANDLE f;
	DWORD written;
	int i;
	int ok;
	DWORD le;
	wchar_t temp_filename[MAX_PATH];
	
	wstring_copy(temp_filename,MAX_PATH,voidpad_filename);
	wstring_cat(temp_filename,MAX_PATH,L".tmp");

	ok = 0;
	
	f = CreateFileW(temp_filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (f != INVALID_HANDLE_VALUE)
	{
		for(i=0;;i++)
		{
			if (i == line_count)
			{
				ok = 1;
				break;
			}
			
			if (!WriteFile(f,line_list[i]->text,line_list[i]->len,&written,0))
			{
				le = GetLastError();
				break;
			}
			
			if (written != (DWORD)line_list[i]->len)
			{
				le = GetLastError();
				break;
			}
		}

		CloseHandle(f);
	}
	else
	{
		le = GetLastError();
	}

	// rename (after handles are closed)
	if (ok)
	{
		ok = 0;
		
		if (os_rename_file(temp_filename,voidpad_filename,MOVEFILE_REPLACE_EXISTING))
		{
			ok = 1;
		}
		else
		{
			le = GetLastError();
		}
	}	
	
	if (ok)
	{
		if (is_dirty)
		{
			is_dirty = 0;
			
			update_title();
		}
		
		// clear last is_saved undo
		if (is_undo_saved)
		{
			undo_t *undo;
			redo_t *redo;
			
			undo = undo_start;
			while(undo)
			{
				undo->is_saved = 0;
			
				undo = undo->next;
			}	
					
			redo = redo_start;
			while(redo)
			{
				redo->is_saved = 0;
			
				redo = redo->next;
			}
		}
		
		// did we save undos?
		if (undo_last)
		{
			undo_last->is_saved = 1;
			
			is_undo_saved = 1;
		}
		else
		{
			// no undos
			is_undo_saved = 0;
		}
	}
	else
	{
		wchar_t msgbuf[MAX_PATH];
		wchar_t numberbuf[MAX_PATH];
		
		wstring_format_number(numberbuf,le);

		wstring_copy(msgbuf,MAX_PATH,L"Failed to save file: ");
		wstring_cat(msgbuf,MAX_PATH,string_filepart(voidpad_filename));
		wstring_cat(msgbuf,MAX_PATH,L" (");
		wstring_cat(msgbuf,MAX_PATH,numberbuf);
		wstring_cat(msgbuf,MAX_PATH,L")");
		
		MessageBoxW(voidpad_hwnd,msgbuf,L"Failed to save",MB_OK);
	}
	
	return ok;
}

// returns 1 if saved
//FIXME: write to another 64k-1MB buffer first. too many calls to WriteFile is bad.
// no temporary file is used, powerloss would mean data loss.
static int voidpad_save_direct(void)
{
	HANDLE f;
	DWORD written;
	int i;
	int ok;
	DWORD le;
	
	ok = 0;
	
	f = CreateFileW(voidpad_filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (f != INVALID_HANDLE_VALUE)
	{
		for(i=0;;i++)
		{
			if (i == line_count)
			{
				ok = 1;
				break;
			}
			
			if (!WriteFile(f,line_list[i]->text,line_list[i]->len,&written,0))
			{
				le = GetLastError();
				break;
			}
			
			if (written != (DWORD)line_list[i]->len)
			{
				le = GetLastError();
				break;
			}
		}

		CloseHandle(f);
	}
	else
	{
		le = GetLastError();
	}

	if (ok)
	{
		if (is_dirty)
		{
			is_dirty = 0;
			
			update_title();
		}
		
		// clear last is_saved undo
		if (is_undo_saved)
		{
			undo_t *undo;
			redo_t *redo;
			
			undo = undo_start;
			while(undo)
			{
				undo->is_saved = 0;
			
				undo = undo->next;
			}	
					
			redo = redo_start;
			while(redo)
			{
				redo->is_saved = 0;
			
				redo = redo->next;
			}
		}
		
		// did we save undos?
		if (undo_last)
		{
			undo_last->is_saved = 1;
			
			is_undo_saved = 1;
		}
		else
		{
			// no undos
			is_undo_saved = 0;
		}
	}
	else
	{
		wchar_t msgbuf[MAX_PATH];
		wchar_t numberbuf[MAX_PATH];
		
		wstring_format_number(numberbuf,le);

		wstring_copy(msgbuf,MAX_PATH,L"Failed to save file: ");
		wstring_cat(msgbuf,MAX_PATH,string_filepart(voidpad_filename));
		wstring_cat(msgbuf,MAX_PATH,L" (");
		wstring_cat(msgbuf,MAX_PATH,numberbuf);
		wstring_cat(msgbuf,MAX_PATH,L")");
		
		MessageBoxW(voidpad_hwnd,msgbuf,L"Failed to save",MB_OK);
	}
	
	return ok;
}

static int save_changes(void)
{
	if (is_dirty)
	{
		int ret;
		wchar_t msgbuf[MAX_PATH];
		
		wstring_copy(msgbuf,MAX_PATH,L"The text in the ");
		if (voidpad_filename)
		{
			wstring_cat(msgbuf,MAX_PATH,voidpad_filename);
		}
		else
		{
			wstring_cat(msgbuf,MAX_PATH,L"Untitled");
		}
		
		wstring_cat(msgbuf,MAX_PATH,L" file has changed.\n\nDo you want to save the changes?");

		ret = MessageBoxW(voidpad_hwnd,msgbuf,L"voidpad",MB_YESNOCANCEL|MB_ICONQUESTION);
		if (ret == IDYES) 
		{
			if (voidpad_filename)
			{
				if (!voidpad_save())
				{
					return 0;
				}
			}			
			else
			{
				if (!voidpad_saveas())
				{
					return 0;
				}
			}
			
			return 1;
		}
		
		if (ret == IDCANCEL)
		{
			return 0;
		}
	}
	
	// no
	return 1;
}

static void voidpad_open_dialog(void)
{
	OPENFILENAMEW ofn;
	wchar_t buf[MAX_PATH];

	if (!save_changes()) return;

	crt_zero_memory(&ofn,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	
	wstring_copy(buf,MAX_PATH,voidpad_filename ? voidpad_filename : L"");
	
	ofn.hInstance = GetModuleHandle(0);
	ofn.hwndOwner = voidpad_hwnd;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	
	if (GetOpenFileNameW(&ofn))
	{
		voidpad_command_data(VOIDPAD_COMMAND_OPEN,buf,(wstring_length(buf) + 1) * sizeof(wchar_t));
	}
}

// 1GB ReadFile() 1MB buffer, 2293ms, 436MB/s
// 1GB ReadFile() 64K buffer, 2308ms, 433MB/s
// 1GB ReadFile() 4K buffer, 2590ms, 386MB/s
// 1GB ReadFile() 64K buffer, 2340ms, 430MB/s (HeapAlloc)
// 1GB mmap copy 1MB buffer, 2247 ms 445 MB/s
// 1GB mmap, 2013 ms 496 MB/s
// we must use virtualalloc so we can allocate more than 4GB.
static void voidpad_open(const wchar_t *filename)
{
	HANDLE f;
	DWORD size_low;
	DWORD size_high;
	int ok;
	unsigned char *start;
	int col;

	clear();
	
	ok = 0;

	f = CreateFileW(filename,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
	if ((f == INVALID_HANDLE_VALUE) && (GetLastError() == 87))
	{
		// FILE_SHARE_DELETE is not valid on win9x.
		f = CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
	}

	if (f != INVALID_HANDLE_VALUE) 
	{
		__int64 size;
		
		size_low = GetFileSize(f,(LPDWORD)&size_high);
		
		size = size_low + ((__int64)size_high << 32);
	
		// allocate at least one byte...
		// even if we dont use it.
		if (size)
		{
			file_data = VirtualAlloc(0,size,MEM_COMMIT,PAGE_READWRITE);
		}
		
		if ((!size) || (file_data))
		{
			unsigned char *p;
			__int64 run;
			int was_line_return;
			ULARGE_INTEGER offset;
	
			p = file_data;
			run = size;
			was_line_return = 0;
			offset.QuadPart = 0;

			start = p;
			col=0;
			
			for(;;)			
			{
				DWORD numread;
				DWORD chunk_size;
				unsigned char *e;
				
				if (!run)
				{
					if (was_line_return)
					{
						col--;
					}
					
					if (col + 2 > text_wide)
					{
						text_wide = col + 2;
					}

					// this could be a zero lengthed line.
					line_add(start,p-start);

					build_line_list();

					if (voidpad_filename)
					{
						mem_free(voidpad_filename);
					}
			
					voidpad_filename = wstring_alloc(filename);

					update_title();
					update_ln_status();	
					update_col_status();	
					update_ch_status();	
					
					InvalidateRect(edit_hwnd,0,FALSE);

					size_window();

					ok = 1;
					break;
				}
				
				// use a smaller chunk, because we are spending too much time waiting on ram..
				// the smaller the chunk, the more we store in cpu cache to add our lines...
				if (run > 65536)
				{
					chunk_size = 65536;
				}
				else
				{
					chunk_size = run;
				}
			
				if (!ReadFile(f,p,chunk_size,&numread,0))
				{
					break;
				}
				
				if (numread != chunk_size)
				{
					break;
				}

				// parse lines..
				e = p + chunk_size;
				
				while(p < e)
				{
					if (*p == '\r')
					{
						was_line_return = 1;
						col++;
						p++;
					}
					else
					if (*p == '\n')
					{
						if (was_line_return)
						{
							col--;
						}
						
						if (col + 2 > text_wide)
						{
							text_wide = col + 2;
						}

						p++;

						line_add(start,p-start);
						
						start = p;
						col=0;
						was_line_return = 0;
					}
					else
					{
						was_line_return = 0;
						
						if (*p == '\t')
						{
							col = ((col / voidpad_tab_size) + 1) * voidpad_tab_size;
							p++;
						}
						else
						{
							col++;
							p++;
						}
					}
				}

				run -= chunk_size;
				offset.QuadPart += chunk_size;
			}
		}
		
		CloseHandle(f);
	}

	if (!ok)
	{
		wchar_t msgbuf[MAX_PATH];
		wchar_t numberbuf[MAX_PATH];

		wstring_format_number(numberbuf,GetLastError());
		
		wstring_copy(msgbuf,MAX_PATH,L"Failed to open file: ");
		wstring_cat(msgbuf,MAX_PATH,string_filepart(filename));
		wstring_cat(msgbuf,MAX_PATH,L" (");
		wstring_cat(msgbuf,MAX_PATH,numberbuf);
		wstring_cat(msgbuf,MAX_PATH,L")");
		
		new_file(0);

		MessageBoxW(voidpad_hwnd,msgbuf,L"Failed to open",MB_OK);
	}
}

void draw_char(HDC hdc,int x,int y,int col,int ln,int c)
{
	if ((compare_ch_ln(col,ln,sel_x1,sel_y1) >= 0) && (compare_ch_ln(col,ln,sel_x2,sel_y2) < 0))
	{
		SetBkColor(hdc,RGB(selected_foreground_r,selected_foreground_g,selected_foreground_b));
		SetTextColor(hdc,RGB(selected_background_r,selected_background_g,selected_background_b));
	}
	else
	{
		SetBkColor(hdc,RGB(foreground_r,foreground_g,foreground_b));
		SetTextColor(hdc,RGB(background_r,background_g,background_b));
	}
	
	BitBlt(hdc,x,y,font_wide,font_high,font_hdc,(c%32)*font_wide,(c / 32)*font_high,SRCCOPY);
}

static void draw_space(HDC hdc,int x,int y,int col,int ln)
{
	COLORREF fore_color;
	COLORREF back_color;
	
	if ((compare_ch_ln(col,ln,sel_x1,sel_y1) >= 0) && (compare_ch_ln(col,ln,sel_x2,sel_y2) < 0))
	{
		back_color = RGB(selected_background_r,selected_background_g,selected_background_b);

		fore_color = RGB(
			selected_background_r + ((selected_foreground_r - selected_background_r) / 2),
			selected_background_g + ((selected_foreground_g - selected_background_g) / 2),
			selected_background_b + ((selected_foreground_b - selected_background_b) / 2));
	}
	else
	{
		back_color = RGB(background_r,background_g,background_b);

		fore_color = RGB(
			background_r + ((foreground_r - background_r) / 2),
			background_g + ((foreground_g - background_g) / 2),
			background_b + ((foreground_b - background_b) / 2));
	}
	
	fill_rect(hdc,x,y,font_wide,font_high,back_color);

	fill_rect(hdc,x + (font_wide / 2)-1,y + (font_high / 2) - 1,2,2,fore_color);

}

static void draw_tab(HDC hdc,int x,int y,int col,int ln)
{
	COLORREF fore_color;
	COLORREF back_color;
	
	if ((compare_ch_ln(col,ln,sel_x1,sel_y1) >= 0) && (compare_ch_ln(col,ln,sel_x2,sel_y2) < 0))
	{
		back_color = RGB(selected_background_r,selected_background_g,selected_background_b);

		fore_color = RGB(
			selected_background_r + ((selected_foreground_r - selected_background_r) / 2),
			selected_background_g + ((selected_foreground_g - selected_background_g) / 2),
			selected_background_b + ((selected_foreground_b - selected_background_b) / 2));
	}
	else
	{
		back_color = RGB(background_r,background_g,background_b);

		fore_color = RGB(
			background_r + ((foreground_r - background_r) / 2),
			background_g + ((foreground_g - background_g) / 2),
			background_b + ((foreground_b - background_b) / 2));
	}
	
	fill_rect(hdc,x,y,font_wide,font_high,back_color);

	fill_rect(hdc,x+(font_wide / 2)-4,y + (font_high / 2),8,1,fore_color);

	{
		int i;
		
		for(i=0;i<3;i++)
		{
			SetPixel(hdc,x+6-i+(font_wide / 2)-4,y + (font_high / 2) - i - 1,fore_color);
			SetPixel(hdc,x+6-i+(font_wide / 2)-4,y + (font_high / 2) + i + 1,fore_color);
		}
	}
}

void fill_char(HDC hdc,int x,int y,int col,int ln)
{
	COLORREF color;
	
	if ((compare_ch_ln(col,ln,sel_x1,sel_y1) >= 0) && (compare_ch_ln(col,ln,sel_x2,sel_y2) < 0))
	{
		color = RGB(selected_background_r,selected_background_g,selected_background_b);
	}
	else
	{
		color = RGB(background_r,background_g,background_b);
	}
	
	fill_rect(hdc,x,y,font_wide,font_high,color);
}

static void fill_rect(HDC hdc,int x,int y,int wide,int high,COLORREF color)
{
	RECT rect;
	HBRUSH hbrush;
	
	hbrush = CreateSolidBrush(GetNearestColor(hdc,color));
	
	rect.left = x;
	rect.top = y;
	rect.right = x + wide;
	rect.bottom = y + high;
	
	FillRect(hdc,&rect,hbrush);
	
	DeleteObject(hbrush);
}

static wchar_t *wstring_format_number(wchar_t *buf,int number)
{
	wchar_t backbuf[MAX_PATH];
	wchar_t *n;
	wchar_t *d;
	
	d = buf;
	
	if (number < 0)
	{
		number = -number;
		*d++ = '-';
	}

	if (!number)
	{
		*d++ = '0';
		*d = 0;
		return d;
	}
	
	n = backbuf;
	
	while(number)
	{
		*n++ = (number % 10) + '0';
		number /= 10;
	}

	while(n > backbuf)
	{
		n--;
		
		*d = *n;
		
		d++;
	}
	*d = 0;
	
	return d;
}

static void update_ln_status(void)
{
	wchar_t buf[MAX_PATH];
	wchar_t *p;
	
	p = buf;
	*p++ = 'L';
	*p++ = 'n';
	*p++ = ' ';

	wstring_format_number(p,cursor_y + 1);
	SendMessage(status_hwnd,SB_SETTEXTW,1|SBT_NOBORDERS,(LPARAM)buf);
}

static void update_col_status(void)
{
	wchar_t buf[MAX_PATH];
	wchar_t *p;
	
	p = buf;
	*p++ = 'C';
	*p++ = 'o';
	*p++ = 'l';
	*p++ = ' ';

	wstring_format_number(p,get_nearest_cursor_colx()+1);
	SendMessage(status_hwnd,SB_SETTEXTW,2|SBT_NOBORDERS,(LPARAM)buf);
}

static void update_ch_status(void)
{
	wchar_t buf[MAX_PATH];
	wchar_t *p;
	
	p = buf;
	*p++ = 'C';
	*p++ = 'h';
	*p++ = ' ';

	wstring_format_number(p,get_cursor_chx()+1);
	SendMessage(status_hwnd,SB_SETTEXTW,3|SBT_NOBORDERS,(LPARAM)buf);
}

static void update_ins_status(void)
{
	wchar_t *modebuf;
	
	if (insert_mode == VOIDPAD_INSERT_MODE_INSERT)
	{
		modebuf = L"INS";
	}
	else
	{
		modebuf = L"OVR";
	}
	
	SendMessage(status_hwnd,SB_SETTEXTW,5|SBT_NOBORDERS,(LPARAM)modebuf);
}

void size_window(void)
{
	if (!IsIconic(voidpad_hwnd))
	{
		int wide;
		int high;
		int status_high;
		RECT rect;
		SCROLLINFO si;
		int parts[6];
		int cxvscroll;
		int cyhscroll;
		int last_vscroll_pos;
		int last_hscroll_pos;
		int vscroll_pos;
		int hscroll_pos;
		
		GetClientRect(voidpad_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top;

		parts[5] = wide - GetSystemMetrics(SM_CXVSCROLL);
		parts[4] = parts[5] - 30;
		parts[3] = parts[4] - 20;
		parts[2] = parts[3] - 70;
		parts[1] = parts[2] - 70;
		parts[0] = parts[1] - 70;
		
		SendMessage(status_hwnd,SB_SETPARTS,6,(LPARAM)parts);

		SendMessage(status_hwnd,WM_SIZE,0,0);
		
		GetWindowRect(status_hwnd,&rect);
		status_high = rect.bottom - rect.top;
		
		// position edit:
		SetWindowPos(edit_hwnd,0,0,0,wide,high-status_high,SWP_NOZORDER|SWP_NOACTIVATE);

		GetClientRect(edit_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top;
		wide -= 2;

		if (GetWindowLong(edit_hwnd,GWL_STYLE) & WS_VSCROLL) wide += GetSystemMetrics(SM_CXVSCROLL);
		if (GetWindowLong(edit_hwnd,GWL_STYLE) & WS_HSCROLL) high += GetSystemMetrics(SM_CYHSCROLL);
		
		if ((line_count * font_high <= high) && ((text_wide) * font_wide <= wide))
		{
			// no scroll bars.
		}
		else
		{
			cxvscroll = GetSystemMetrics(SM_CXVSCROLL);
			cyhscroll = GetSystemMetrics(SM_CYHSCROLL);
			
			if (line_count * font_high > high) 
			{
				wide -= cxvscroll; 
				cxvscroll = 0;
			}
			
			if ((text_wide) * font_wide > wide) 
			{
				high -= cyhscroll;
				cyhscroll = 0;
			}
			
			if (line_count * font_high > high) 
			{
				wide -= cxvscroll; 
			}
			
			if ((text_wide) * font_wide > wide) 
			{
				high -= cyhscroll;
			}
		}
		
		last_vscroll_pos = get_vscroll();
		last_hscroll_pos = get_hscroll();
			
		// vscroll	
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = (high) / font_high;
		si.nMin = 0;
		si.nMax = line_count - 1;
		
	printf("set vscroll\n")	;

		SetScrollInfo(edit_hwnd,SB_VERT,&si,TRUE);
		
		// hscroll.
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = (wide) / font_wide;
		si.nMin = 0;
		si.nMax = (text_wide) - 1;
		
		SetScrollInfo(edit_hwnd,SB_HORZ,&si,TRUE);

		vscroll_pos = get_vscroll();
		hscroll_pos = get_hscroll();

		// clip scroll pos;
		if ((vscroll_pos != last_vscroll_pos) || (hscroll_pos != last_hscroll_pos))
		{
			ScrollWindowEx(edit_hwnd,(last_hscroll_pos-hscroll_pos)*font_wide,(last_vscroll_pos-vscroll_pos)*font_high,0,0,0,0,SW_INVALIDATE);
		}
	}
}

void reset_cursor_blink(void)
{	
	if (cursor_timer_set)
	{
		if (!cursor_flash)
		{
			RECT rect;
			
			cursor_flash = 1;

			// invalidate
			get_cursor_rect(&rect);

			InvalidateRect(edit_hwnd,&rect,FALSE);
		}

		// reset timer.		
		SetTimer(edit_hwnd,VOIDPAD_KEYSPEED_TIMER,GetCaretBlinkTime(),0);
	}		
}

static int get_vscroll(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_VSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		
		if (GetScrollInfo(edit_hwnd,SB_VERT,&si))
		{
			return si.nPos;
		}
	}

	return 0;	
}

static int get_hscroll(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_HSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		if (GetScrollInfo(edit_hwnd,SB_HORZ,&si))
		{
			return si.nPos;
		}
	}
	
	return 0;
}

static int get_vpage(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_VSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		if (GetScrollInfo(edit_hwnd,SB_VERT,&si))
		{
			if (si.nPage > 0) 
			{
				return si.nPage;
			}
		}
	}
	
	return 1;
}

static int get_hpage(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_HSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		if (GetScrollInfo(edit_hwnd,SB_HORZ,&si))
		{
			if (si.nPage > 0) 
			{
				return si.nPage;
			}
		}
	}
	
	return 1;
}

static int get_vmax(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_VSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		if (GetScrollInfo(edit_hwnd,SB_VERT,&si))
		{
			return si.nMax;
		}
	}
	
	return 0;
}

static int get_hmax(void)
{
	if (GetWindowStyle(edit_hwnd) & WS_HSCROLL)
	{
		SCROLLINFO si;
		
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(edit_hwnd,SB_HORZ,&si);
		
		return si.nMax;
	}
	
	return 0;
}

static void set_vscroll(int pos)
{
	SCROLLINFO si;
	int lastpos;
	
	lastpos = get_vscroll();

	if (pos != lastpos)
	{
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = pos;
		
		SetScrollInfo(edit_hwnd,SB_VERT,&si,TRUE);

		// get clipped pos:
		pos = get_vscroll();
		
		if (pos != lastpos)
		{
			ScrollWindowEx(edit_hwnd,0,(lastpos-pos)*font_high,0,0,0,0,SW_INVALIDATE);
		}
	}
}

static void set_hscroll(int pos)
{
	SCROLLINFO si;
	int lastpos;
	
	lastpos = get_hscroll();

	if (pos != lastpos)
	{
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = pos;
		
		SetScrollInfo(edit_hwnd,SB_HORZ,&si,TRUE);

		// get clipped pos:
		pos = get_hscroll();
		
		if (pos != lastpos)
		{
			ScrollWindowEx(edit_hwnd,(lastpos-pos)*font_wide,0,0,0,0,0,SW_INVALIDATE);
		}
	}
}

static void get_cursor_rect(RECT *rect)
{
	int x;
	
	x = get_nearest_cursor_colx();
	
	rect->left = 2 + ((x - get_hscroll()) * font_wide);
	rect->top = (cursor_y - get_vscroll()) * font_high;
	
	if (insert_mode == VOIDPAD_INSERT_MODE_INSERT)
	{
		rect->right = rect->left + 1;
	}
	else
	{
		int chx;
		int right;
		
		chx = get_cursor_chx();
		
		right = ch_to_col(chx+1,cursor_y);
		if (right == x) right++;
		
		rect->right = 2 + ((right - get_hscroll()) * font_wide);
	}
	
	rect->bottom = rect->top + font_high;
}

static int get_line_colend(int y)
{
	unsigned char *p;
	int col;
	int len;
	
	len = line_list[y]->len;
	
	if ((len >= 2) && (line_list[y]->text[len-2] == '\r') && (line_list[y]->text[len-1] == '\n'))
	{
		len -= 2;
	}
	else
	if ((len >= 1) && (line_list[y]->text[len-1] == '\n'))
	{
		len -= 1;
	}

	p = line_list[y]->text;
	col = 0;
	while(len)
	{
		if (*p == '\t')
		{
			for(;;)
			{
				col++;
				if ((col % voidpad_tab_size) == 0) break;
			}
		}
		else
		{
			col++;
		}
		
		p++;
		len--;
	}

	return col;
}

static int get_text_wide(int y)
{
	return get_line_colend(y) + 1;
}

static int get_line_chend(int y)
{
	int len;
	
	len = line_list[y]->len;
	if ((len >= 2) && (line_list[y]->text[len-2] == '\r') && (line_list[y]->text[len-1] == '\n'))
	{
		len -= 2;
	}
	else
	if ((len >= 1) && (line_list[y]->text[len-1] == '\n'))
	{
		len -= 1;
	}

	return len;
}

// since cursor_x can be longer than the line it is on, this clips it to the length of text.
static int get_nearest_cursor_colx(void)
{
	unsigned char *p;
	int colx;
	int lastcolx;
	int len;
	
	len = line_list[cursor_y]->len;
	if ((len >= 2) && (line_list[cursor_y]->text[len-2] == '\r') && (line_list[cursor_y]->text[len-1] == '\n'))
	{
		len -= 2;
	}
	else
	if ((len >= 1) && (line_list[cursor_y]->text[len-1] == '\n'))
	{
		len -= 1;
	}

	p = line_list[cursor_y]->text;
	colx = 0;
	lastcolx = 0;
	while(len)
	{
		if (*p == '\t')
		{
			for(;;)
			{
				if (cursor_x == colx) return lastcolx;
				
				colx++;
				if (colx % voidpad_tab_size == 0) break;
			}
		}
		else
		{
			if (cursor_x == colx) return lastcolx;
			
			colx++;
		}
		
		p++;
		len--;
		lastcolx = colx;
	}

	return lastcolx;
}

static int get_cursor_chx(void)
{
	return col_to_ch(cursor_x,cursor_y);
}

// x doesnt have to be inside the len of the line
static void set_cursor_col_position(int x,int y,int reset_col_selection,int reset_find,int bring_into_view)
{
	RECT oldrect;
	RECT newrect;

	// clip y only
	// dont clip x so we can move the cursor up and down and keep the longer x position.
	y = clip_ln(y);
	
	if ((x != cursor_x) || (y != cursor_y))
	{
		get_cursor_rect(&oldrect);
		cursor_x = x;
		cursor_y = y;
		get_cursor_rect(&newrect);
		
		if ((oldrect.left != newrect.left) || (oldrect.top != newrect.top))
		{
			InvalidateRect(edit_hwnd,&oldrect,FALSE);
			InvalidateRect(edit_hwnd,&newrect,FALSE);
		}
		
		if (bring_into_view)
		{
			update_ln_status();
			update_col_status();
			update_ch_status();
			
			reset_cursor_blink();
		}
	}

	if (bring_into_view)
	{
		voidpad_bring_cursor_into_view();
	}
	
	if (reset_find)
	{
		find_last_chx = -1;
		find_last_y = -1;

		find_start_chx = -1;
		find_start_direction = 0;
	}

	if (reset_col_selection)
	{
		mark_x = x;
		mark_y = y;
		
		set_col_sel(0,0,0,0);
	}
	else
	{
		set_col_sel(mark_x,mark_y,x,y);
	}
}

// x doesnt have to be inside the len of the line
static void set_cursor_ch_position(int chx,int y,int reset_col_selection,int reset_find,int bring_into_view)
{
	int colx;
	
	// need to convert 
	colx = ch_to_col(chx,y);
	
	set_cursor_col_position(colx,y,reset_col_selection,reset_find,bring_into_view);
}

static int ch_to_col(int ch,int ln)
{
	unsigned char *p;
	int colx;
	int chx;
	int len;
	
	len = line_list[ln]->len;
	if ((len >= 2) && (line_list[ln]->text[len-2] == '\r') && (line_list[ln]->text[len-1] == '\n'))
	{
		len -= 2;
	}
	else
	if ((len >= 1) && (line_list[ln]->text[len-1] == '\n'))
	{
		len -= 1;
	}

	p = line_list[ln]->text;
	colx = 0;
	chx = 0;
	while(len)
	{
		if (chx == ch) return colx;
		
		if (*p == '\t')
		{
			for(;;)
			{
				colx++;
				if (colx % voidpad_tab_size == 0) break;
			}
		}
		else
		{
			colx++;
		}
		
		p++;
		len--;
		chx++;
	}

	return colx;
}

static int col_to_ch(int col,int ln)
{
	unsigned char *p;
	int colx;
	int chx;
	int len;
	
	len = line_list[ln]->len;
	if ((len >= 2) && (line_list[ln]->text[len-2] == '\r') && (line_list[ln]->text[len-1] == '\n'))
	{
		len -= 2;
	}
	else
	if ((len >= 1) && (line_list[ln]->text[len-1] == '\n'))
	{
		len -= 1;
	}

	p = line_list[ln]->text;
	colx = 0;
	chx = 0;
	while(len)
	{
		if (*p == '\t')
		{
			for(;;)
			{
				if (colx == col) return chx;
				
				colx++;
				if (colx % voidpad_tab_size == 0) break;
			}
		}
		else
		{
			if (colx == col) return chx;
			
			colx++;
		}
		
		p++;
		len--;
		chx++;
	}

	return chx;
}

static int clip_ln(int y)
{
	// clip y's first
	if (y < 0) 
	{
		return 0;
	}
	else
	if (y > line_count-1) 
	{
		return line_count - 1;
	}
	
	return y;
}

static int clip_col(int x,int clipped_y)
{
	int colend;
	
	if (x < 0)
	{
		return 0;
	}
	else
	{	
		colend = get_line_colend(clipped_y);
		
		if (x > colend)
		{
			return colend;
		}
	}
	
	return x;
}

// x2,y2 are inclusive.
// cols
static void invalidate_range(int x1,int y1,int x2,int y2)
{
	int y;
	int left;
	int right;
	RECT rect;
	RECT client_rect;
	int vs;
	int hs;
	int hwnd_right;
	int clip_top;
	int clip_high;
	
//printf("invalidate %d %d %d %d\n",x1,y1,x2,y2);	
	vs = get_vscroll();
	hs = get_hscroll();
	
	GetWindowRect(voidpad_hwnd,&rect);
	hwnd_right = rect.right;

	clip_top = get_vscroll();
	clip_high = rect.bottom - rect.top;
	
	y = y1;

	// clip.	
	if (y < clip_top)
	{
		y = clip_top;
	}

	while(y <= y2)
	{
		if (clip_high <= 0) break;
	
		if (y == y1)
		{
			// use start
			left = x1;
		}
		else
		{
			left = 0;
		}
		
		if (y == y2)
		{
			if (x2 == -1)
			{
				right = hwnd_right;
			}
			else
			{
				right = 2 + ((x2 - hs) * font_wide);
			}
		}
		else
		{
			right = 2 + ((get_line_colend(y) + 1) * font_wide);
		}
		
		rect.left = 2 + ((left - hs) * font_wide);
		rect.top = (y - vs) * font_high;
		rect.right = right;
		rect.bottom = rect.top + font_high;
		
		// clip, as really large rects fail.
		GetClientRect(edit_hwnd,&client_rect);
		
		if (rect.left < client_rect.left) rect.left = client_rect.left;
		if (rect.top < client_rect.top) rect.top = client_rect.top;
		if (rect.right > client_rect.right) rect.right = client_rect.right;
		if (rect.bottom > client_rect.bottom) rect.bottom = client_rect.bottom;
		
		InvalidateRect(edit_hwnd,&rect,FALSE);
		
		clip_high -= font_high;
		y++;
	}
}

// -1 if x1,y1 is less than x2,y2
// 0  if x1,y1 is equal to x2,y2
// 1  if x1,y1 is greater than x2,y2
static int compare_ch_ln(int x1,int y1,int x2,int y2)
{
	if (y1 < y2) return -1;
	if (y1 == y2)
	{
		if (x1 < x2) return -1;
		if (x1 == x2) return 0;
	}
	
	return 1;
}

// x1,y1 and x2,y2
static void set_col_sel(int x1,int y1,int x2,int y2)
{
	int i;

	// clip
	y1 = clip_ln(y1);
	x1 = clip_col(x1,y1);
	
	if (y2 > line_count-1)
	{
		y2 = line_count-1;
		x2 = get_line_colend(y2);
	}

	y2 = clip_ln(y2);
	x2 = clip_col(x2,y2);

	if ((x1 == x2) && (y1 == y2))
	{
		// null regions are all the same..
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
	}
	
	if (compare_ch_ln(x1,y1,x2,y2) > 0)
	{
		int temp;
		
		// sawp
		temp = x1;
		x1 = x2;
		x2 = temp;
		
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	
	if ((x1 != sel_x1) || (y1 != sel_y1) || (x2 != sel_x2) || (y2 != sel_y2))
	{
		if ((sel_x1 == sel_x2) && (sel_y1 == sel_y2))
		{
			// invalidate 
			invalidate_range(x1,y1,x2,y2);
		}
		else
		if ((x1 == x2) && (y1 == y2))
		{
			// invalidate 
			invalidate_range(sel_x1,sel_y1,sel_x2,sel_y2);
		}
		else
		{
			// left side
			i = compare_ch_ln(x1,y1,sel_x1,sel_y1);
			if (i < 0)
			{
				// invalidate until x2,y2, or start of sel_x1,sel_y1
				if (compare_ch_ln(x2,y2,sel_x1,sel_y1) < 0)
				{
					// |(x1,y1)      |(x2,y2)
					// **************      |(sel_x1,sel_y1)       |(sel_x2,sel_y2)
					// use x2
					invalidate_range(x1,y1,x2,y2);
				}
				else
				{
					// |(x1,y1)      |(x2,y2)
					// ********|(sel_x1,sel_y1)       |(sel_x2,sel_y2)
					
					// use sel_x1
					invalidate_range(x1,y1,sel_x1,sel_y1);
				}	
			}
			else
			if (i > 0)
			{
				// invalidate until sel_x2,sel_y2, or start of x1,y1
				if (compare_ch_ln(x2,y2,sel_x1,sel_y1) < 0)
				{
					// ***********************                |(x1,y1)      |(x2,y2)
					// |(sel_x1,sel_y1)       |(sel_x2,sel_y2)
					// use x2
					invalidate_range(sel_x1,sel_y1,sel_x2,sel_y2);
				}
				else
				{
					// *************|(x1,y1)      |(x2,y2)
					// |(sel_x1,sel_y1)       |(sel_x2,sel_y2)
					
					// use sel_x1
					invalidate_range(sel_x1,sel_y1,x1,y1);
				}	
			}
			
			// right side
			i = compare_ch_ln(x2,y2,sel_x2,sel_y2);
			if (i > 0)
			{
				if (compare_ch_ln(x1,y1,sel_x2,sel_y2) > 0)
				{
					//               (x1,y1)|           (x2,y2)|
					// (sel_x2,sel_y2)|      *******************
					invalidate_range(x1,y1,x2,y2);
				}
				else
				{
					// (x1,y1)|                       (x2,y2)|
					//         (sel_x2,sel_y2)|***************
					
					invalidate_range(sel_x2,sel_y2,x2,y2);
				}	
			}
			else
			if (i < 0)
			{
				if (compare_ch_ln(x2,y2,sel_x1,sel_y1) > 0)
				{
					//                         (x2,y2)|*************
					// (sel_x1,sel_y1)|             (sel_x2,sel_y2)|
					invalidate_range(x2,y2,sel_x2,sel_y2);
				}
				else
				{
					//     (x2,y2)|             *******************
					//          (sel_x1,sel_y1)|   (sel_x2,sel_y2)|
					
					invalidate_range(sel_x1,sel_y1,sel_x2,sel_y2);
				}	
			}
		}
		
		// save
		sel_x1 = x1;
		sel_y1 = y1;
		sel_x2 = x2;
		sel_y2 = y2;

//printf("sel %d %d %d %d\n",sel_x1,sel_y1,sel_x2,sel_y2);

	}
}

//
static LRESULT CALLBACK voidpad_edit_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_SETFOCUS:
		
			if (!cursor_timer_set)
			{
				RECT rect;
				
				cursor_flash = 1;
				
				get_cursor_rect(&rect);
				
				InvalidateRect(hwnd,&rect,FALSE);
				SetTimer(edit_hwnd,VOIDPAD_KEYSPEED_TIMER,GetCaretBlinkTime(),0);
				cursor_timer_set = 1;
			}
			
			break;
			
		case WM_KILLFOCUS:

			if (cursor_timer_set)
			{
				RECT rect;
				
				cursor_flash = !cursor_flash;
				
				get_cursor_rect(&rect);
				
				InvalidateRect(hwnd,&rect,FALSE);
				KillTimer(hwnd,VOIDPAD_KEYSPEED_TIMER);
				cursor_timer_set = 0;
			}
			
			break;
			
		case WM_TIMER:
		{
			switch(wParam)
			{
				case VOIDPAD_KEYSPEED_TIMER:
				{
					if (cursor_timer_set)
					{
						RECT rect;
						
						cursor_flash = !cursor_flash;
						
						get_cursor_rect(&rect);
						
						InvalidateRect(hwnd,&rect,FALSE);
					}
						
					break;
				}
			}
			
			break;
		}
		
		case WM_CHAR:
		{
			wchar_t wc;
			unsigned char text[MAX_PATH];
			int len;
			
			wc = wParam;
			
			if ((wParam >= ' ') || (wParam == '\t'))
			{
				len = WideCharToMultiByte(voidpad_cp,0,&wc,1,(LPSTR)text,MAX_PATH,0,0);
				
				voidpad_command_data(VOIDPAD_COMMAND_CHAR,text,len);

				return 0;
			}
			
			break;
		}
		
		case WM_LBUTTONDBLCLK:
		{
			if (!voidpad_macro_is_recording)
			{
				int col;
				int ln;
				int isctrl;
				int isalt;
				int isshift;
				int left;
				int right;
				
				isctrl = GetKeyState(VK_CONTROL) < 0;
				isalt = GetKeyState(VK_MENU) < 0;
				isshift = GetKeyState(VK_SHIFT) < 0;
			
				get_col_ln_from_x_y(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),&col,&ln);
			
				select_word(col_to_ch(col,ln),ln,&left,&right);

				if (left < right)
				{
					set_cursor_ch_position(left,ln,1,1,0);
					set_cursor_ch_position(right,ln,0,1,1);
				}
				else
				{
					ln = -1;
				}
				
				select_main(left,right,ln);
			}
			
			break;
		}
		
		case WM_RBUTTONDOWN:
		{
			if (!voidpad_macro_is_recording)
			{
				int x;
				int y;
			
				x = GET_X_LPARAM(lParam) - 2;
				y = GET_Y_LPARAM(lParam);

				y += get_vscroll() * font_high;
				x += get_hscroll() * font_wide;
			
				// is it selected?
				if (!is_selected_pixel(x,y))
				{
					get_col_ln_from_x_y(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),&x,&y);

					set_cursor_col_position(x,y,1,1,1);
				}
			}

			break;
		}
			
		case WM_LBUTTONDOWN:
		{
			if (!voidpad_macro_is_recording)
			{
				int x;
				int y;
				int isctrl;
				int isalt;
				int isshift;
				
				isctrl = GetKeyState(VK_CONTROL) < 0;
				isalt = GetKeyState(VK_MENU) < 0;
				isshift = GetKeyState(VK_SHIFT) < 0;
			
				if (isctrl)
				{
					SendMessage(hwnd,WM_LBUTTONDBLCLK,wParam,lParam);
				}
				else
				{
					get_col_ln_from_x_y(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),&x,&y);

					set_cursor_col_position(x,y,!isshift,1,1);

					select_main(-1,-1,-1);
				}
			}
			
			break;
		}
		
		case WM_MBUTTONDOWN:
		{
			GetCursorPos(&voidpad_scroll_pt);

			voidpad_set_capture(VOIDPAD_CAPTURE_SCROLL);

			break;
		}

		case WM_MBUTTONUP:

			voidpad_release_capture(VOIDPAD_CAPTURE_SCROLL);
			
			break;
			
		case WM_MOUSEMOVE:
		{
			if ((voidpad_capture & VOIDPAD_CAPTURE_SCROLL) && (GetCapture() == edit_hwnd))
			{
				POINT pt;
			
				GetCursorPos(&pt);
				
				if ((pt.x != voidpad_scroll_pt.x) || (pt.y != voidpad_scroll_pt.y))
				{
					SCROLLINFO si;
					int lasthpos;
					int hpos;
					int lastvpos;
					int vpos;
					
					lasthpos = get_hscroll();
					hpos = lasthpos + pt.x - voidpad_scroll_pt.x;

					lastvpos = get_vscroll();
					vpos = lastvpos + pt.y - voidpad_scroll_pt.y;

					if (hpos != lasthpos)
					{
						si.cbSize = sizeof(SCROLLINFO);
						si.fMask = SIF_POS;
						si.nPos = hpos;
						
						SetScrollInfo(edit_hwnd,SB_HORZ,&si,TRUE);

						// get clipped pos:
						hpos = get_hscroll();
					}

					if (vpos != lastvpos)
					{
						si.cbSize = sizeof(SCROLLINFO);
						si.fMask = SIF_POS;
						si.nPos = vpos;
						
						SetScrollInfo(edit_hwnd,SB_VERT,&si,TRUE);

						// get clipped pos:
						vpos = get_vscroll();
					}

					if ((hpos != lasthpos) || (vpos != lastvpos))
					{
						ScrollWindowEx(edit_hwnd,(lasthpos-hpos)*font_wide,(lastvpos-vpos)*font_high,0,0,0,0,SW_INVALIDATE);
					}
					
					SetCursorPos(voidpad_scroll_pt.x,voidpad_scroll_pt.y);
				}
			}

			break;
		}
		
		case WM_MOUSEWHEEL:
		{
			int delta;
			int wheelscrolllines;
			
			
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,(UINT *)&wheelscrolllines,0);

			// SPI_GETWHEELSCROLLLINES returns 0 on windows 9x.
			// default to 1 on 9x.
			if (wheelscrolllines < 1) wheelscrolllines = 3;
			if (wheelscrolllines > get_vpage()) wheelscrolllines = get_vpage();

			delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
//printf("delta %d, %d %d\n",(delta * wheelscrolllines) / WHEEL_DELTA,delta,wheelscrolllines);

			set_vscroll(get_vscroll() - (delta * wheelscrolllines) / WHEEL_DELTA);
			
			break;
		}	
		
//		case WM_MOUSEHWHEEL:
		case 0x020E:
		{
			int delta;
			int wheelscrolllines;
			
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,(UINT *)&wheelscrolllines,0);

			// SPI_GETWHEELSCROLLLINES returns 0 on windows 9x.
			// default to 1 on 9x.
			if (wheelscrolllines < 1) wheelscrolllines = 3;
			if (wheelscrolllines > get_hpage()) wheelscrolllines = get_hpage();

			delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
//printf("delta %d, %d %d\n",(delta * wheelscrolllines) / WHEEL_DELTA,delta,wheelscrolllines);

			set_hscroll(get_hscroll() + (delta * wheelscrolllines) / WHEEL_DELTA);
			
			break;
		}	
				
		case WM_VSCROLL:
		{
			int pos;
			int lastpos;
			
			pos = get_vscroll();
			lastpos = pos;
			
			switch(LOWORD(wParam))
			{
				case SB_LINEDOWN:
					pos++;
					break;
					
				case SB_LINEUP:
					pos--;
					break;

				case SB_PAGEDOWN:
					pos += get_vpage();
					break;

				case SB_PAGEUP:
					pos -= get_vpage();
					break;
					
				case SB_BOTTOM: 
					pos = get_vmax();
					break;
			
				case SB_TOP: 
					pos = 0;
					break;
			
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:
				{
					SCROLLINFO si;
					
					si.cbSize = sizeof(SCROLLINFO);
					si.fMask = SIF_TRACKPOS;
					GetScrollInfo(hwnd,SB_VERT,&si);
					
					pos = si.nTrackPos;
					
					break;
				}
			}
			
			set_vscroll(pos);
			
			break;
		}	
		
		case WM_HSCROLL:
		{
			int pos;
			int lastpos;
			
			pos = get_hscroll();
			lastpos = pos;
			
			switch(LOWORD(wParam))
			{
				case SB_LINEDOWN:
					pos++;
					break;
					
				case SB_LINEUP:
					pos--;
					break;

				case SB_PAGEDOWN:
					pos += get_hpage();
					break;

				case SB_PAGEUP:
					pos -= get_hpage();
					break;
					
				case SB_BOTTOM: 
					pos = get_hmax();
					break;
			
				case SB_TOP: 
					pos = 0;
					break;
			
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:
				{
					SCROLLINFO si;
					
					si.cbSize = sizeof(SCROLLINFO);
					si.fMask = SIF_TRACKPOS;
					GetScrollInfo(hwnd,SB_HORZ,&si);
					
					pos = si.nTrackPos;
					
					break;
				}
			}
			
			set_hscroll(pos);
			
			break;
		}	
					
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rect;
			HBRUSH back_brush;
			
			hdc = BeginPaint(hwnd, &ps);

			back_brush = CreateSolidBrush(GetNearestColor(hdc,RGB(background_r,background_g,background_b)));

			// fill left border
			rect.left = 0;
			rect.top = ps.rcPaint.top;
			rect.right = 2;
			rect.bottom = ps.rcPaint.bottom;
			FillRect(hdc,&rect,back_brush);

			{
				unsigned char *text;
				int len;
				int y;
				int x;
				int linei;
				int colx;
				
				linei = get_vscroll();
				y = 0;
				
				for(;;)
				{
					if (y >= ps.rcPaint.bottom) break;
					if (linei >= line_count) break;
					
					x = -(get_hscroll() * font_wide) + 2;
					colx = 0;
					
					text = line_list[linei]->text;
					len = line_list[linei]->len;
					
					if ((len >= 2) && (line_list[linei]->text[len-2] == '\r') && (line_list[linei]->text[len-1] == '\n'))
					{
						len -= 2;
					}
					else
					if ((len >= 1) && (line_list[linei]->text[len-1] == '\n'))
					{
						len -= 1;
					}
					
					while(len)
					{
						if (*text == '\t')
						{
							int drew_tab;
							
							drew_tab = 0;
							
							for(;;)
							{
								if (x >= ps.rcPaint.right) break;

								if (x + font_wide >= ps.rcPaint.left)
								{
									if (view_white_space)
									{
										if (!drew_tab)
										{
											draw_tab(hdc,x,y,colx,linei);

											drew_tab = 1;
										}
										else
										{
											fill_char(hdc,x,y,colx,linei);
										}
									}
									else
									{
										fill_char(hdc,x,y,colx,linei);
									}
								}

								x += font_wide;
								colx++;
								
								if (colx % voidpad_tab_size == 0) break;
							}

							text++;
							len--;
						}
						else
						{
							if (x >= ps.rcPaint.right) break;

							if (x + font_wide >= ps.rcPaint.left)
							{
								if (*text == ' ')
								{
									if (view_white_space)
									{
										draw_space(hdc,x,y,colx,linei);
									}
									else
									{
										fill_char(hdc,x,y,colx,linei);
									}
								}
								else
								{
									draw_char(hdc,x,y,colx,linei,*text);
								}
							}

							x += font_wide;
							colx++;
							text++;
							len--;
						}
					}

					// draw new lines
					if (linei + 1 < line_count)
					{
						if (x < ps.rcPaint.right)
						{
							if (x + font_wide >= ps.rcPaint.left)
							{
								fill_char(hdc,x,y,colx,linei);

								x += font_wide;
								colx++;
							}
						}
					}
										
					// fill to right of screen.
					rect.left = x;
					rect.top = y;
					rect.right = ps.rcPaint.right;
					rect.bottom = y + font_high;
					FillRect(hdc,&rect,back_brush);
					
					linei++;
					y += font_high;
				}

				// fill to bottom of screen.
				rect.left = ps.rcPaint.left;
				rect.top = y;
				rect.right = ps.rcPaint.right;
				rect.bottom = ps.rcPaint.bottom;
				FillRect(hdc,&rect,back_brush);
			}
			
			if ((cursor_flash) && (GetFocus() == edit_hwnd))
			{
				get_cursor_rect(&rect);
				
				InvertRect(hdc,&rect);
			}
			
			EndPaint(hwnd, &ps);
			
			DeleteObject(back_brush);
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
			
		case WM_CONTEXTMENU:
		{
			HMENU popup_menu;
			int x;
			int y;
			
			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);
			
			if ((x == -1) && (y == -1))
			{
				POINT pt;
				
				x = 2 + get_nearest_cursor_colx() * font_wide;
				y = (cursor_y+1) * font_high;
				
				pt.x = x;
				pt.y = y;
				
				ClientToScreen(voidpad_hwnd,&pt);
				
				x = pt.x;
				y = pt.y;
			}
			
			popup_menu = CreatePopupMenu();
			
			AppendMenuW(popup_menu,MF_STRING | (undo_last ? 0 : (MF_GRAYED|MF_DISABLED)),ID_EDIT_UNDO,L"&Undo");
			AppendMenuW(popup_menu,MF_SEPARATOR,0,0);
			AppendMenuW(popup_menu,MF_STRING,ID_EDIT_CUT,L"Cu&t");
			AppendMenuW(popup_menu,MF_STRING,ID_EDIT_COPY,L"&Copy");
			AppendMenuW(popup_menu,MF_STRING | (voidpad_can_paste() ? 0 : (MF_GRAYED|MF_DISABLED)),ID_EDIT_PASTE,L"&Paste");
			AppendMenuW(popup_menu,MF_STRING,ID_EDIT_DELETE,L"&Delete");
			AppendMenuW(popup_menu,MF_SEPARATOR,0,0);
			AppendMenuW(popup_menu,MF_STRING,ID_EDIT_SELECT_ALL,L"Select &All");

			TrackPopupMenu(popup_menu,0,x,y,0,voidpad_hwnd,0);
			
			DestroyMenu(popup_menu);
			
			return 0;
		}
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}


//
//  FUNCTION: voidpad_proc(hwnd, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK voidpad_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITMENUPOPUP:

			initmenupopup((HMENU)wParam,(int)LOWORD(lParam));
			
			break;

		case WM_SETFOCUS:
			SetFocus(edit_hwnd);
			break;
			
		case WM_CREATE:
		{
			DWORD ex_style;
			
			status_hwnd = CreateWindowExW(
				0,
				STATUSCLASSNAMEW,
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | SBARS_SIZEGRIP | WS_VISIBLE,
				0,0,0,0,
				hwnd,0,GetModuleHandle(0),0);
				
				
			{
				WNDCLASSEXW wcex;
				
				// Initialize global strings
				crt_zero_memory(&wcex,sizeof(wcex));
				wcex.cbSize = sizeof(wcex); 

				wcex.style			= CS_DBLCLKS;
				wcex.lpfnWndProc	= (WNDPROC)voidpad_edit_proc;
				wcex.cbClsExtra		= 0;
				wcex.cbWndExtra		= 0;
				wcex.hInstance		= GetModuleHandle(0);
				wcex.hIcon			= LoadIcon(GetModuleHandle(0),(LPCTSTR)IDI_ICON1);
				wcex.hCursor		= LoadCursor(NULL,IDC_IBEAM);
				wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
				wcex.lpszMenuName	= 0;
				wcex.lpszClassName	= L"VOIDPADEDIT";
				wcex.hIconSm		= LoadIcon(GetModuleHandle(0),(LPCTSTR)IDI_ICON1);

				RegisterClassExW(&wcex);
			}
			
			ex_style = 0;
			
			if (!((os_IsAppThemed()) && (os_IsThemeActive())))
			{
				ex_style |= WS_EX_CLIENTEDGE;
			}

			edit_hwnd = CreateWindowExW(
				ex_style,
				L"VOIDPADEDIT",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE,
				0,0,0,0,
				hwnd,0,GetModuleHandle(0),0);
				
			SetWindowLongPtr(edit_hwnd,GWLP_WNDPROC,(LONG_PTR)voidpad_edit_proc);
				
			SetFocus(edit_hwnd);
			
			DragAcceptFiles(hwnd,TRUE);
			
			break;
		}
		
		case WM_DROPFILES:
		{
			HDROP hdrop;	
			DWORD numfiles;
			wchar_t buf[MAX_PATH];
			
			hdrop = (HDROP)wParam;
			
			numfiles = DragQueryFile(hdrop,0xFFFFFFFF,0,0);
			if (numfiles > 0)
			{
				DragQueryFileW(hdrop,0,buf,MAX_PATH);
				
				if (save_changes())
				{
					voidpad_open(buf);
				}
			}
			
			break;
		}
		
		case WM_CLOSE:

			voidpad_exit();
			return 0;
		
		case WM_SIZE:
		
			size_window();

			if (!IsIconic(hwnd))
			{
				if (!IsMaximized(hwnd))
				{
					RECT rect;
					
					GetWindowRect(hwnd,&rect);
		
					window_wide = rect.right - rect.left;
					window_high = rect.bottom - rect.top;
				}
			}
			
			break;
			
		case WM_MOVE:
		{
			if (!IsIconic(hwnd))
			{
				if (!IsMaximized(hwnd))
				{
					RECT rect;
					
					GetWindowRect(hwnd,&rect);
		
					window_x = rect.left;
					window_y = rect.top;
				}
			}
			
			break;
		}
			
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case ID_FILE_NEW:
					voidpad_command(VOIDPAD_COMMAND_NEW);
					break;
					
				case ID_FILE_OPEN:
					voidpad_open_dialog();
					break;
				
				case ID_FILE_SAVE:

					if (voidpad_filename)
					{
						voidpad_command(VOIDPAD_COMMAND_SAVE);
					}
					else
					{
						voidpad_saveas();
					}
					break;
				
				case ID_FILE_SAVE_DIRECT:

					if (voidpad_filename)
					{
						voidpad_command(VOIDPAD_COMMAND_SAVE_DIRECT);
					}
					else
					{
						voidpad_saveas_direct();
					}
					break;
				
				case ID_FILE_SAVE_AS:
					voidpad_saveas();
					break;
					
				case ID_FILE_EXIT:
					voidpad_exit();
					break;
					
				case ID_EDIT_CUT:
					voidpad_command(VOIDPAD_COMMAND_CUT);
					break;

				case ID_EDIT_COPY:
					voidpad_command(VOIDPAD_COMMAND_COPY);
					break;

				case ID_EDIT_UNDO:
					voidpad_command(VOIDPAD_COMMAND_UNDO);
					break;

				case ID_EDIT_REDO:
					voidpad_command(VOIDPAD_COMMAND_REDO);
					break;

				case ID_EDIT_PASTE:
					voidpad_command(VOIDPAD_COMMAND_PASTE);
					break;

				case ID_EDIT_CYCLE_CLIPBOARD_RING:
					voidpad_command(VOIDPAD_COMMAND_CYCLE_CLIPBOARD_RING);
					break;

				case ID_EDIT_DELETE:
					voidpad_command(VOIDPAD_COMMAND_DELETE);
					break;
				
				case ID_EDIT_DELETE_LINE:
					voidpad_command(VOIDPAD_COMMAND_DELETE_LINE);
					break;
					
				case ID_EDIT_MAKE_UPPERCASE:
					voidpad_command(VOIDPAD_COMMAND_MAKE_UPPERCASE);
					break;
					
				case ID_EDIT_MAKE_LOWERCASE:
					voidpad_command(VOIDPAD_COMMAND_MAKE_LOWERCASE);
					break;
					
				case ID_EDIT_INCREMENT_INTEGER:
					voidpad_command(VOIDPAD_COMMAND_INCREMENT_INTEGER);
					break;

				case ID_EDIT_DECREMENT_INTEGER:
					voidpad_command(VOIDPAD_COMMAND_DECREMENT_INTEGER);
					break;
				
				case ID_EDIT_VIEW_WHITE_SPACE:
					voidpad_command(VOIDPAD_COMMAND_VIEW_WHITE_SPACE);
					break;

				case ID_EDIT_INCREASE_LINE_INDENT:
					voidpad_command(VOIDPAD_COMMAND_INCREASE_LINE_INDENT);
					break;

				case ID_EDIT_DECREASE_LINE_INDENT:
					voidpad_command(VOIDPAD_COMMAND_DECREASE_LINE_INDENT);
					break;
				
				case ID_EDIT_SELECT_ALL:
					voidpad_command(VOIDPAD_COMMAND_SELECT_ALL);
					break;
					
				case ID_EDIT_FIND:
					voidpad_find_dialog(0);
					break;
					
				case ID_EDIT_REPLACE:
					voidpad_find_dialog(1);
					break;
					
				case ID_EDIT_GOTO:
					voidpad_goto_dialog();
					break;
					
				case ID_TOOLS_MACRO_PLAY:
					voidpad_macro_play();
					break;
					
				case ID_TOOLS_MACRO_RECORD:
					voidpad_macro_record();
					break;
					
				case ID_TOOLS_OPTIONS:
					voidpad_options_main();
					break;
					
				case ID_HELP_ABOUT:
					voidpad_help_about();
					break;
					
				case ID_EDIT_INSERT_HEX:
					voidpad_insert_hex_dialog();
					break;
					
				case ID_EDIT_INSERT_UTF8_SEQUENCE:
					voidpad_insert_utf8_sequence_dialog();
					break;
					
				case ID_EDIT_INSERT_UTF16_SEQUENCE:
					voidpad_insert_utf16_sequence_dialog();
					break;
					
				case ID_EDIT_FIND_NEXT:
					voidpad_command(VOIDPAD_COMMAND_FIND_NEXT);
					break;

				case ID_EDIT_FIND_PREV:
					voidpad_command(VOIDPAD_COMMAND_FIND_PREV);
					break;
					
				case ID_EDIT_INCREASE_LINE_INDENT_OR_INSERT_TAB:
					voidpad_command(VOIDPAD_COMMAND_INCREASE_LINE_INDENT_OR_INSERT_TAB);
					break;

				case ID_EDIT_DECREASE_LINE_INDENT_OR_INSERT_TAB:
					voidpad_command(VOIDPAD_COMMAND_DECREASE_LINE_INDENT_OR_INSERT_TAB);
					break;

				case ID_EDIT_LEFT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_LEFT);
					break;
					
				case ID_EDIT_EXTEND_LEFT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_LEFT);
					break;
					
				case ID_EDIT_WORD_LEFT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_WORD_LEFT);
					break;

				case ID_EDIT_EXTEND_WORD_LEFT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_LEFT);
					break;

				case ID_EDIT_RIGHT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_RIGHT);
					break;

				case ID_EDIT_EXTEND_RIGHT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_RIGHT);
					break;

				case ID_EDIT_WORD_RIGHT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_WORD_RIGHT);
					break;

				case ID_EDIT_EXTEND_WORD_RIGHT:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_RIGHT);
					break;

				case ID_EDIT_UP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_UP);
					break;

				case ID_EDIT_EXTEND_UP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_UP);
					break;

				case ID_EDIT_SCROLL_UP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_SCROLL_UP);
					break;

				case ID_EDIT_DOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_DOWN);
					break;

				case ID_EDIT_EXTEND_DOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN);
					break;

				case ID_EDIT_SCROLL_DOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_SCROLL_DOWN);
					break;

				case ID_EDIT_PAGEUP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_PAGEUP);
					break;

				case ID_EDIT_EXTEND_PAGEUP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEUP);
					break;

				case ID_EDIT_SCROLL_PAGEUP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEUP);
					break;

				case ID_EDIT_EXTEND_SCROLL_PAGEUP:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEUP);
					break;

				case ID_EDIT_PAGEDOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_PAGEDOWN);
					break;

				case ID_EDIT_EXTEND_PAGEDOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEDOWN);
					break;

				case ID_EDIT_SCROLL_PAGEDOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEDOWN);
					break;

				case ID_EDIT_EXTEND_SCROLL_PAGEDOWN:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEDOWN);
					break;

				case ID_EDIT_LINE_END:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_LINE_END);
					break;

				case ID_EDIT_EXTEND_LINE_END:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_END);
					break;

				case ID_EDIT_END:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_END);
					break;

				case ID_EDIT_EXTEND_END:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_END);
					break;

				case ID_EDIT_LINE_HOME:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_LINE_HOME);
					break;

				case ID_EDIT_EXTEND_LINE_HOME:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_HOME);
					break;

				case ID_EDIT_HOME:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_HOME);
					break;

				case ID_EDIT_EXTEND_HOME:
					voidpad_command(VOIDPAD_COMMAND_CURSOR_EXTEND_HOME);
					break;

				case ID_EDIT_BACKSPACE:
					voidpad_command(VOIDPAD_COMMAND_BACKSPACE);
					break;

				case ID_EDIT_DELETE_PREVIOUS_WORD:
					voidpad_command(VOIDPAD_COMMAND_DELETE_PREVIOUS_WORD);
					break;

				case ID_EDIT_INSERT_NEWLINE:
					voidpad_command(VOIDPAD_COMMAND_INSERT_NEWLINE);
					break;

				case ID_EDIT_INSERT_NEWLINE_ABOVE:
					voidpad_command(VOIDPAD_COMMAND_INSERT_NEWLINE_ABOVE);
					break;

				case ID_EDIT_INSERT_NEWLINE_BELOW:
					voidpad_command(VOIDPAD_COMMAND_INSERT_NEWLINE_BELOW);
					break;

				case ID_EDIT_TOGGLE_INSERT_MODE:					
					voidpad_command(VOIDPAD_COMMAND_TOGGLE_INSERT_MODE);
					break;
					
				case ID_EDIT_INSERT_DATE:
					voidpad_command(VOIDPAD_COMMAND_INSERT_DATE);
					break;
			}
		
			break;
		}
			
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			
			hdc = BeginPaint(hwnd, &ps);
			
			EndPaint(hwnd, &ps);
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void create_window(int nCmdShow)
{
	WNDCLASSEXW wcex;
	HMENU hmenu;
	
	// Initialize global strings
	crt_zero_memory(&wcex,sizeof(wcex));
	wcex.cbSize = sizeof(wcex); 

	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC)voidpad_proc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(0);
	wcex.hIcon			= LoadIcon(GetModuleHandle(0),(LPCTSTR)IDI_ICON1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"VOIDPAD";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON1);

	RegisterClassExW(&wcex);


/*
	{
		HFONT hfont;
		
		hfont = CreateFont(16,0,0,0,0,0,0,0,0,0,0,0,0,L"Courier New");
		
		SelectObject(font_hdc,hfont );
		
		{
			int i;
			wchar_t letter[2];
			wchar_t lettera[2];
			RECT rect;
			
			rect.left = 0;
			rect.top = 0;
			rect.right = font_wide * 32;
			rect.bottom = font_high * 8;

			FillRect(font_hdc,&rect,GetStockObject(BLACK_BRUSH));
			
			SetTextColor(font_hdc,RGB(255,255,255));
			SetBkColor(font_hdc,RGB(0,0,0));
			
			lettera[1] = 0;
			
			for(i=0;i<256;i++)
			{
				lettera[0] = i;
				
				MultiByteToWideChar(437,0,lettera,-1,letter,2);
				TextOutW(font_hdc,(i % 32) *font_wide,(i / 32) * font_high,letter,1);
			}
		}
	}
*/

	hmenu = CreateMenu();

	AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)CreatePopupMenu(),L"&File");
	AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)CreatePopupMenu(),L"&Edit");
	AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)CreatePopupMenu(),L"&Tools");
	AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)CreatePopupMenu(),L"&Help");

	// Perform application initialization:
//printf("create window %d %d %d %d\n",window_x, window_y, window_wide, window_high);

	{
		RECT rect;
		
		rect.left = window_x;
		rect.top = window_y;
		rect.right = window_x + window_wide;
		rect.bottom = window_y + window_high;
		
		os_make_rect_completely_visible(&rect);
		
		voidpad_hwnd = CreateWindowExW(
			0,
			L"VOIDPAD", 
			L"voidPad", 
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
			NULL, hmenu, GetModuleHandle(0), NULL);
	}

	size_window();
	update_ins_status();
	update_title();
	update_ln_status();
	update_col_status();
	update_ch_status();

	// reset timer.		
	cursor_flash = 1;
	SetTimer(edit_hwnd,VOIDPAD_KEYSPEED_TIMER,GetCaretBlinkTime(),0);
	cursor_timer_set = 1;
			
	ShowWindow(voidpad_hwnd, nCmdShow);
	UpdateWindow(voidpad_hwnd);
}

static utf8_t *ini_open(wchar_t *filename)
{
	HANDLE h;
	unsigned char *data;
	int ok;
	
	ok = 0;
	data = 0;
	
	h = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if (h != INVALID_HANDLE_VALUE)
	{	
		DWORD size_high;
		DWORD size_low;
		
		size_low = GetFileSize(h,&size_high);
		
		if ((!size_high) && (size_low <= 2147483647))
		{
			DWORD numread;
		
			data = mem_alloc(size_low + 1);

			if (ReadFile(h,data,size_low,&numread,0))
			{
				if (numread == size_low)
				{
					data[size_low] = 0;
					ok = 1;
				}
			}
			
			if (!ok)
			{
				mem_free(data);
				
				data = 0;
			}
		}
		
		CloseHandle(h);
	}

	return data;	
}

static int viv_reg_read_int(HKEY hkey,wchar_t *name,int *pvalue)
{
	DWORD size;
	DWORD type;
	DWORD value;
	
	type = REG_DWORD;
	size = sizeof(DWORD);
	
	if (RegQueryValueEx(hkey,name,0,&type,(BYTE *)&value,&size) == ERROR_SUCCESS)
	{
		*pvalue = (int)value;
		
		return 1;
	}
	
	return 0;
}

static int viv_reg_read_string(HKEY hkey,wchar_t *name,wchar_t **pvalue)
{
	DWORD size;
	DWORD type;
	wchar_t *p;
	
	type = REG_SZ;
	size = 0;
	
	if (RegQueryValueEx(hkey,name,0,&type,0,&size) == ERROR_SUCCESS)
	{
		p = mem_alloc(size);
		
		if (RegQueryValueEx(hkey,name,0,&type,(BYTE *)p,&size) == ERROR_SUCCESS)
		{
			if (*pvalue)
			{
				mem_free(*pvalue);
			}
			
			*pvalue = p;
			
			return 1;
		}
		
		mem_free(p);
	}
	
	return 0;
}

static void voidpad_load_settings(void)
{
	HKEY hkey;
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"Software\\voidPad",0,KEY_READ,&hkey) == ERROR_SUCCESS)
	{
		int tab_size;

		viv_reg_read_int(hkey,L"x",&window_x);
		viv_reg_read_int(hkey,L"y",&window_y);
		viv_reg_read_int(hkey,L"wide",&window_wide);
		viv_reg_read_int(hkey,L"high",&window_high);
		
		viv_reg_read_int(hkey,L"background_r",&background_r);
		viv_reg_read_int(hkey,L"background_g",&background_g);
		viv_reg_read_int(hkey,L"background_b",&background_b);
		viv_reg_read_int(hkey,L"foreground_r",&foreground_r);
		viv_reg_read_int(hkey,L"foreground_g",&foreground_g);
		viv_reg_read_int(hkey,L"foreground_b",&foreground_b);

		viv_reg_read_int(hkey,L"selected_background_r",&selected_background_r);
		viv_reg_read_int(hkey,L"selected_background_g",&selected_background_g);
		viv_reg_read_int(hkey,L"selected_background_b",&selected_background_b);
		viv_reg_read_int(hkey,L"selected_foreground_r",&selected_foreground_r);
		viv_reg_read_int(hkey,L"selected_foreground_g",&selected_foreground_g);
		viv_reg_read_int(hkey,L"selected_foreground_b",&selected_foreground_b);

		if (viv_reg_read_int(hkey,L"tab_size",&tab_size))
		{
			voidpad_tab_size = voidpad_clip_tabsize(tab_size);
		}
		
		viv_reg_read_int(hkey,L"insert_mode",&insert_mode);
		viv_reg_read_int(hkey,L"clipboard_ring_size",&voidpad_clipboard_ring_size);
		viv_reg_read_int(hkey,L"view_white_space",&view_white_space);
		viv_reg_read_int(hkey,L"select_all_bring_into_view",&voidpad_select_all_bring_into_view);
		
		viv_reg_read_string(hkey,L"font_bitmap_filename",&font_bitmap_filename);
		
		viv_reg_read_int(hkey,L"font_cp",&voidpad_cp);
			
		{
			wchar_t *list;

			list = 0;

			viv_reg_read_string(hkey,L"find_history",&list);
			
			if (list)
			{
				wchar_t *listp;

				listp = list;

				for(;;)								
				{
					unsigned char *listitem;
				
					listp = reg_alloc_list_item(listp,&listitem);
					if (!listp) break;
			
					find_add(listitem);
												
					mem_free(listitem);
				}
				
				mem_free(list);
			}
		}		
				
		{
			wchar_t *list;

			list = 0;

			viv_reg_read_string(hkey,L"replace_history",&list);
			
			if (list)
			{
				wchar_t *listp;

				listp = list;

				for(;;)								
				{
					unsigned char *listitem;
				
					listp = reg_alloc_list_item(listp,&listitem);
					if (!listp) break;
			
					replace_add(listitem);
												
					mem_free(listitem);
				}
				
				mem_free(list);
			}
		}		

		RegCloseKey(hkey);
	}
}
/*
static unsigned char *ini_alloc_list_item(unsigned char *value,unsigned char **item)
{
	unsigned char *d;
	unsigned char *p;
	
	if (!*value)
	{
		return 0;
	}
	
	d = value;
	p = value;
	
	while(*p)
	{
		if (*p == '"')
		{
			p++;
			
			while(*p)
			{
				if (*p == '"') 
				{
					p++;
					
					break;
				}
				else
				if (*p == '\\')
				{
					p++;
					switch(*p)
					{
						case '"': *d++ = '"'; break;
						case 't': *d++ = 't'; break;
						case '\\': *d++ = '\\'; break;
						case 'n': *d++ = 'n'; break;
						case 'r': *d++ = 'r'; break;
					}
					p++;
				}
				else
				{
					*d++ = *p;
					
					p++;
				}
			}
		}
		else
		if ((*p == ';') || (*p == ','))
		{
			p++;
			break;
		}
		else
		{
			*d++ = *p++;
		}
	}
	
	*d = 0;
	
	{
		int wlen;
		wchar_t *wbuf;
		
		wlen = MultiByteToWideChar(CP_UTF8,0,(char *)value,d-value,0,0);
		
		wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
		
		MultiByteToWideChar(CP_UTF8,0,(char *)value,d-value,wbuf,wlen);
		
		wbuf[wlen] = 0;

		*item = os_alloc_wchar_to_voidpad_cp(wbuf);
		
		mem_free(wbuf);
	}
	
	return p;
}
*/

static wchar_t *reg_alloc_list_item(wchar_t *value,unsigned char **item)
{
	wchar_t *d;
	wchar_t *p;
	
	if (!*value)
	{
		return 0;
	}
	
	d = value;
	p = value;
	
	while(*p)
	{
		if (*p == '"')
		{
			p++;
			
			while(*p)
			{
				if (*p == '"') 
				{
					p++;
					
					break;
				}
				else
				if (*p == '\\')
				{
					p++;
					switch(*p)
					{
						case '"': *d++ = '"'; break;
						case 't': *d++ = 't'; break;
						case '\\': *d++ = '\\'; break;
						case 'n': *d++ = 'n'; break;
						case 'r': *d++ = 'r'; break;
					}
					p++;
				}
				else
				{
					*d++ = *p;
					
					p++;
				}
			}
		}
		else
		if ((*p == ';') || (*p == ','))
		{
			p++;
			break;
		}
		else
		{
			*d++ = *p++;
		}
	}
	
	*d = 0;
	
	*item = os_alloc_wchar_to_voidpad_cp(value);
	
	return p;
}

static int ini_is_key(const unsigned char *ini_key,const char *key)
{
	if (string_compare(ini_key,(unsigned char *)key) == 0)
	{
		return 1;
	}
	
	return 0;
}

static void ini_int(const unsigned char *ini_key,const unsigned char *ini_value,const char *key,int *var)
{
	if (ini_is_key(ini_key,key))
	{
		*var = get_integer(ini_value,string_length(ini_value));
	}
}

static void ini_string(const unsigned char *ini_key,const unsigned char *ini_value,const char *key,wchar_t **var)
{
	if (ini_is_key(ini_key,key))
	{
		if (*var)
		{
			mem_free(*var);
		}
		
		*var = wstring_alloc_utf8(ini_value);
	}
}

static int is_parent(HWND parent,HWND hwnd)
{
	while(hwnd)
	{
		if (hwnd == parent) return 1;
		
		hwnd = GetParent(hwnd);
	}
	
	return 0;
}

static int voidpad_is_key(int vk,int flags,int scope)
{
	int keyi;

	for(keyi = 0;keyi<KEY_COUNT;keyi++)
	{
		if (keys[keyi].vk == vk)
		{
			if (keys[keyi].flags == flags)
			{
				if (keys[keyi].scope == scope)
				{
					SendMessage(voidpad_hwnd,WM_COMMAND,MAKEWPARAM(keys[keyi].id,0),0);
					
					return 1;
				}
			}
		}
	}
	
	return 0;
}

static int voidpad_msg(MSG *msg)
{
	switch(msg->message)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			int flags;
			
			flags = 0;
			if (GetKeyState(VK_CONTROL) < 0) flags |= VOIDPAD_KEY_FLAG_CONTROL;
			if (GetKeyState(VK_MENU) < 0) flags |= VOIDPAD_KEY_FLAG_ALT;
			if (GetKeyState(VK_SHIFT) < 0) flags |= VOIDPAD_KEY_FLAG_SHIFT;

			if (GetFocus() == edit_hwnd)
			{
				// these commands MUST have focus.
				if (voidpad_is_key(msg->wParam,flags,VOIDPAD_KEY_SCOPE_EDIT))
				{
					return 1;
				}
			}

			// these commands dont need focus.
			if (voidpad_is_key(msg->wParam,flags,VOIDPAD_KEY_SCOPE_GLOBAL))
			{
				return 1;
			}

			break;
		}
	}

	if (is_parent(find_hwnd,msg->hwnd))
	{
		if (IsDialogMessage(find_hwnd,msg))
		{
			return 1;
		}
	}
	
	return 0;
}

static int voidpad_is_ws(wchar_t c)
{
	switch(c)
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return 1;
	}
	
	return 0;
}

static wchar_t *voidpad_skip_ws(wchar_t *p)
{
	while(*p)
	{
		if (!voidpad_is_ws(*p))
		{
			break;
		}
		
		p++;
	}
	
	return p;
}

static wchar_t *voidpad_get_word(wchar_t *p,wchar_t *buf)
{
	wchar_t *d;
	int is_quote;
	
	d = buf;
	is_quote = 0;

	while(*p)
	{
		if ((*p == '"') && (p[1] == '"'))
		{
			p += 2;
			*d++ = '"';
		}
		else
		if (*p == '"')
		{
			is_quote = !is_quote;
			p++;
		}
		else
		if ((!is_quote) && (voidpad_is_ws(*p)))
		{
			break;
		}
		else
		{
			*d++ = *p;
			p++;
		}
	}
	
	*d = 0;
	
	return p;
}

static void voidpad_process_command_line(wchar_t *cl)
{
	wchar_t *p;
	wchar_t buf[MAX_PATH];
	
	p = cl;
	
	p = voidpad_skip_ws(p);
	
	// skip filename
	p = voidpad_get_word(p,buf);
	p = voidpad_skip_ws(p);

	// skip first parameter.
	p = voidpad_get_word(p,buf);
	
	if (*buf)
	{
		HANDLE h;
		WIN32_FIND_DATA fd;
		
		h = FindFirstFile(buf,&fd);
		if (h != INVALID_HANDLE_VALUE)
		{
			*string_filepart(buf) = 0;
			
			wstring_cat(buf,MAX_PATH,fd.cFileName);

			// open 
			voidpad_open(buf);
		
			FindClose(h);
		}
		else
		{
			// open 
			voidpad_open(buf);
		}
	}
}

static int voidpad_main(void)
{
	MSG msg;
	HDC screen_hdc;
	
	printf("voidpad\n");
	
	// UxTheme.dll
	_os_uxtheme_hdll = LoadLibraryA("UxTheme.dll");
	if (_os_uxtheme_hdll)
	{
		_os_get_proc_address(_os_uxtheme_hdll,"IsAppThemed",(FARPROC *)&os_IsAppThemed);
		_os_get_proc_address(_os_uxtheme_hdll,"IsThemeActive",(FARPROC *)&os_IsThemeActive);
		_os_get_proc_address(_os_uxtheme_hdll,"EnableThemeDialogTexture",(FARPROC *)&os_EnableThemeDialogTexture);
	}
	
	_os_user32_hdll = LoadLibraryA("User32.dll");
	if (_os_user32_hdll)
	{
		_os_get_proc_address(_os_user32_hdll,"MonitorFromRect",(FARPROC *)&_os_MonitorFromRect);
	}
	
	OleInitialize(0);

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    DefWindowProc(NULL,0,0,0);

	// init common controls
	{
		INITCOMMONCONTROLSEX icex;

		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icex);
	}
	
	os_load_default_hfont();

	font_bitmap_filename = wstring_alloc(L"");

#ifdef VERSION_REGISTRATION
	voidpad_registered_name = wstring_alloc(L"");
	voidpad_registered_key = wstring_alloc(L"");
#endif

	printf("loadini\n");

#ifdef VERSION_REGISTRATION
	voidpad_load_settings(0);
#else
	voidpad_load_settings();
#endif

	// is registered?
#ifdef VOIDPAD_REGISTRATION
	voidpad_load_settings(1);
	voidpad_is_registered = help_register_is_valid(voidpad_registered_name,voidpad_registered_key);
#endif
	
	// set initial find text.
	if (!search_string)
	{
		if (find_last)
		{
			int len;
			
			len = string_length(find_last->text);
			search_string = mem_alloc(len + 1);
			search_string_len = len;
			crt_copy_memory(search_string,find_last->text,len);
			search_string[len] = 0;
		}
	}

	// make sure theres at least one line.	
	line_add(0,0);
	build_line_list();
	
	// create font.
	screen_hdc = GetDC(0);
	font_hdc = CreateCompatibleDC(screen_hdc);
	
	font_load();

	voidpad_clipboard_format = RegisterClipboardFormat(L"VOIDPAD");
	
	printf("create window\n");
	create_window(SW_SHOWNORMAL);

	printf("cl %S\n",GetCommandLineW());

	voidpad_process_command_line(GetCommandLineW());

#ifdef VERSION_REGISTRATION
	if (!voidpad_is_registered)
	{
		if (MessageBox(voidpad_hwnd,L"This copy of voidPad is not registered.\n\nWould you like to register now?",L"voidPad",MB_YESNO|MB_ICONWARNING) == IDYES)
		{	
			voidpad_help_register();
		}
	}
#endif

printf("enter loop\n");

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!voidpad_msg(&msg)) 
		{
 			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

printf("Exit loop\n");
	

	save_settings();

	DestroyWindow(voidpad_hwnd);
	
	if (line_list)
	{
		int i;
		
		for(i=0;i<line_count;i++)
		{
			free_line(line_list[i]);
		}
		
		mem_free(line_list);
	}	

	if (file_data)
	{
		VirtualFree(file_data,0,MEM_RELEASE);
	}
	
//	if (file_memory_map_handle)
//	{
//		CloseHandle(file_memory_map_handle);
//	}

	// free lines chunks.
	{
		line_chunk_t *lc;
		
		lc = line_chunk_start;
		while(lc)
		{
			line_chunk_t *next_lc;
			
			next_lc = lc->next;
			
			mem_free(lc);
			
			lc = next_lc;
		}
	}
	
	// clear undos.
	undo_clear();
	
	// clear redos.
	redo_clear();
	
	DeleteObject(os_default_hfont);
	
	// uninit COM
	OleUninitialize();
	
	if (search_string)
	{
		mem_free(search_string);
	}
	
	if (replace_string)
	{
		mem_free(replace_string);
	}
	
	// find history
	{
		find_t *f;
		
		f = find_start;
		while(f)
		{
			find_t *next_f;
			
			next_f = f->next;
			
			mem_free(f->text);
			mem_free(f);
			
			f = next_f;
		}
	}
	
	// replace history
	{
		replace_t *f;
		
		f = replace_start;
		while(f)
		{
			replace_t *next_f;
			
			next_f = f->next;
			
			mem_free(f->text);
			mem_free(f);
			
			f = next_f;
		}
	}
	
	if (voidpad_filename)
	{
		mem_free(voidpad_filename);
	}

#ifdef VERSION_REGISTRATION
	mem_free(voidpad_registered_name);
	mem_free(voidpad_registered_key);
#endif

	mem_free(font_bitmap_filename);

	// clear clipboard ring.
	voidpad_clipboard_ring_clear();
	
	// clear macros
	voidpad_macro_clear();
	
	if (font_hbitmap)
	{
		SelectObject(font_hdc,last_font_bitmap);
		DeleteObject(font_hbitmap);
	}
	
	DeleteDC(font_hdc);
	
	// DEBUG
	#ifdef _DEBUG
	
		mem_DEBUG();
		
	#endif
	
	if (_os_uxtheme_hdll)
	{
		FreeLibrary(_os_uxtheme_hdll);
	}
	
	if (_os_user32_hdll)
	{
		FreeLibrary(_os_user32_hdll);
	}
		
	
printf("cleanup done\n");

	return (int) msg.wParam;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	hInstance = hInstance;
	hPrevInstance = hPrevInstance;
	lpCmdLine = lpCmdLine;
	nCmdShow = nCmdShow;
	
printf("hInstance %d %d\n",hInstance,GetModuleHandle(0));

	return voidpad_main();
}

int __cdecl main(int argc,char **argv)
{
	argc = argc;
	argv = argv;
	
printf("hInstance %d\n",GetModuleHandle(0));
printf("hInstance %d\n",GetLastError());
	
	return voidpad_main();
}

static void wstring_copy(wchar_t *d,int size,const wchar_t *s)
{
	size--;
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s++;
	}
	
	*d = 0;
}

static wchar_t *string_filepart(const wchar_t *s)
{
	const wchar_t *last;
	
	last = s;
	
	while(*s)
	{
		if (*s == '\\') last = s + 1;
		
		s++;
	}
	
	return (wchar_t *)last;
}

static int is_selection(void)
{
	if ((sel_x1 != sel_x2) || (sel_y1 != sel_y2))
	{
		return 1;
	}
	
	return 0;
}

static void insert_new_line(int eol,int below)
{
	unsigned char *new_line_text;
	int new_line_len;
	unsigned char *p;
	unsigned char *e;
	int tab_count;
	
	tab_count = 0;
	
	if (eol)
	{
		int y;
		
		if (is_selection())
		{
			if (below)
			{
				y = sel_y2;
			}
			else
			{
				y = sel_y1;
			}
		}
		else
		{
			y = cursor_y;
		}
	
		undo_clear_selection();
		
		if (below)
		{
			set_cursor_col_position(get_line_colend(y),y,1,1,0);
		}
		else
		{
			if (y-1 < 0)
			{
				set_cursor_col_position(0,0,1,1,0);
			}
			else
			{
				set_cursor_col_position(get_line_colend(y - 1),y-1,1,1,0);
			}
		}
	}
	
	// count tabs
	p = line_list[cursor_y]->text;
	e = line_list[cursor_y]->text + line_list[cursor_y]->len;
	while(p < e)
	{
		if (*p != '\t') 
		{
			break;
		}
		
		tab_count++;
		p++;
	}

	// calculate size and allocate new line text buffer.
	new_line_len = 2 + tab_count;
	new_line_text = mem_alloc(new_line_len);
	p = new_line_text;
	
	// add new line.
	*p++ = '\r';
	*p++ = '\n';
	
	// fill in tabs.
	while(tab_count)
	{
		*p++ = '\t';
		
		tab_count--;
	}
	
	insert_text(new_line_text,new_line_len,1,0,1,0);
	
	mem_free(new_line_text);
}

// returns 1 if text is deleted, 0 if not.
// never brings the cursor into view
// delete selection MUST always be followed by a insert_text
// even if no text is inserted.
static void delete_selection(int is_replace)
{
	int y;
	int size;
	int ch1;
	int ch2;
	int new_text_wide;
	
	if (is_selection())
	{
		int backup_sel_x1;
		int backup_sel_y1;
		int backup_sel_x2;
		int backup_sel_y2;

		backup_sel_x1 = sel_x1;
		backup_sel_y1 = sel_y1;
		backup_sel_x2 = sel_x2;
		backup_sel_y2 = sel_y2;
	
		ch1 = col_to_ch(sel_x1,sel_y1);
		ch2 = col_to_ch(sel_x2,sel_y2);
		
		if (is_replace)
		{
			if (find_start_chx != -1)
			{
				if (compare_ch_ln(find_start_chx,find_start_y,ch1,sel_y1) > 0)
				{
					if (compare_ch_ln(find_start_chx,find_start_y,ch2,sel_y2) > 0)
					{
						// subtract the entire deletion from find_start.
						if (find_start_y == sel_y2)
						{
							if (sel_y1 == sel_y2)
							{
	printf("delsel: moved find start %d,%d -> %d,%d\n",find_start_chx,find_start_y,find_start_chx - (ch2 - ch1),find_start_y - (sel_y2 - sel_y1));
								find_start_chx -= ch2 - ch1;
							}
							else
							{
	printf("delsel: moved find start %d,%d -> %d,%d\n",find_start_chx,find_start_y,find_start_chx - (ch2),find_start_y - (sel_y2 - sel_y1));
								find_start_chx -= ch2;
							}
						}
						
						find_start_y -= sel_y2 - sel_y1;
					}
					else
					{	
						// start position was inside the deleted selection.
printf("delsel: moved find start %d,%d -> %d,%d\n",find_start_chx,find_start_y,ch1,sel_y1);
						find_start_chx = ch1;
						find_start_y = sel_y1;
					}
				}
			}
		}
		
		cursor_hide();
				
		// move the cursor before we delete the text,
		// otherwise the cursor position is pointing into invalid text
		// and would crash us.
		// move cursor to end
		set_cursor_col_position(sel_x1,sel_y1,1,!is_replace,0);

//	printf("ch1 %d, ch2 %d, len2 %d\n",ch1,ch2,line_list[sel_y2]->len);

		// delete text after right.
		// add text after sel_x2,sel_y2
		size = ch1 + line_list[backup_sel_y2]->len - ch2;
		
//	printf("size %d\n",size);
		size = os_ceil_power_2(size);
		
		// make sure we have enough room.
		if (size > line_list[backup_sel_y1]->size)
		{
			unsigned char *new_mem;
			
//	printf("newsize %d\n",size);
			new_mem = mem_alloc(size);
			
			// copy old over
			crt_copy_memory(new_mem,line_list[backup_sel_y1]->text,ch1);
			crt_copy_memory(new_mem+ch1,line_list[backup_sel_y2]->text+ch2,line_list[backup_sel_y2]->len - ch2);

			if (line_list[backup_sel_y1]->memory)
			{
				mem_free(line_list[backup_sel_y1]->memory);
			}

			line_list[backup_sel_y1]->memory = new_mem;
			line_list[backup_sel_y1]->size = size;
			line_list[backup_sel_y1]->text = new_mem;
		}
		else
		{
			crt_move_memory(line_list[backup_sel_y1]->text+ch1,line_list[backup_sel_y2]->text + ch2,line_list[backup_sel_y2]->len - ch2);
		}
		
		// inc len
		line_list[backup_sel_y1]->len = ch1 + line_list[backup_sel_y2]->len - ch2;
		
		// calculate text_wide.
		new_text_wide = get_text_wide(backup_sel_y1);
		if (new_text_wide > text_wide) 
		{
			text_wide = new_text_wide;
		}

		if (backup_sel_y2 - backup_sel_y1)
		{
			// delete left over lines.
			y = backup_sel_y1 + 1;
			while(y <= backup_sel_y2)
			{
				free_line(line_list[y]);
				free_line_add((free_line_t *)line_list[y]);
			
				y++;
			}
			
			// rebuild array.
//	printf("%d %d %d\n",backup_sel_y1+1,backup_sel_y2+1,(line_count - backup_sel_y2) );
			crt_move_memory(&line_list[backup_sel_y1+1],&line_list[backup_sel_y2+1],(line_count - (backup_sel_y2+1)) * sizeof(line_t *));
			
//	printf("line count %d, new %d\n",line_count,line_count - (backup_sel_y2 - backup_sel_y1));
			line_count -= backup_sel_y2 - backup_sel_y1;
		}

		// move the end of sel_x2,sel_y2 to sel_x1,sel_y1.
		// we can not scroll the window because if we have tabs, the scrolling is not always one pixel.
		// we MUST invalidate the entire row after sel_x2.
		{
			RECT rect;
			int wide;
			
			GetClientRect(edit_hwnd,&rect);
			wide = rect.right - rect.left;

			rect.left = 2 + (backup_sel_x1 * font_wide);
			rect.top = backup_sel_y1 * font_high;
			rect.right = (get_hscroll() * font_wide) + wide;
			rect.bottom = rect.top + font_high;
			
			OffsetRect(&rect,-get_hscroll()*font_wide,-get_vscroll()*font_high);
			
			InvalidateRect(edit_hwnd,&rect,FALSE);
		}
				
		// invalidate everything under backup_sel_y1.
		if (backup_sel_y1 - backup_sel_y2)
		{
			RECT rect;
			
			GetClientRect(edit_hwnd,&rect);

			rect.top = ((backup_sel_y1+1) * font_high) - (get_vscroll() * font_high);

			InvalidateRect(edit_hwnd,&rect,FALSE);
		}		
	}
}

// insert multi-line text.
static void insert_text(const unsigned char *text,int len,int allow_undo,int select,int bring_into_view,int is_replace)
{
	int size;
	int new_text_wide;
	const unsigned char *start;
	int ch;
	const unsigned char *p;
	unsigned char *line_p;
	const unsigned char *e;
	int new_line_count;
	int new_line_list_size;
	int isnl;
	line_t *first_line;
	int line_index;
	int endch;
	int find_start_cmp;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// count number of lines first
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	new_line_count = 0;
	
	p = text;
	e = text + len;
	while(p < e)
	{	
		if ((p + 2 <= e) && (*p == '\r') && (p[1] == '\n'))
		{
			p += 2;
			
			new_line_count++;
		}
		else
		if (*p == '\n')
		{
			p++;

			new_line_count++;
		}
		else
		{
			p++;
		}
	}
	
	if (allow_undo)
	{
		undo_add(len,0,1);
	}

	if (is_replace)
	{
		if (find_start_chx != -1)
		{
			if (is_selection())
			{
				find_start_cmp = compare_ch_ln(col_to_ch(sel_x1,sel_y1),sel_y1,find_start_chx,find_start_y);
			}
			else
			{
				find_start_cmp = compare_ch_ln(col_to_ch(cursor_x,cursor_y),cursor_y,find_start_chx,find_start_y);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// delete the selection	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	delete_selection(is_replace);
	
	cursor_hide();
	
	// ensure mark is at cursor for selection.
	mark_x = cursor_x;
	mark_y = cursor_y;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// save cursor position
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ch = get_cursor_chx();
	first_line = line_list[cursor_y];

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// pre alloc line list
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
//printf("new_line_count %d\n",new_line_count);

	new_line_list_size = line_count + new_line_count;
	
	if (new_line_list_size > line_list_size)
	{
		line_t **new_line_list;

printf("new line list size %d, old size %d\n",new_line_list_size,line_list_size);
		new_line_list_size = os_ceil_power_2(new_line_list_size);
		
		new_line_list = mem_alloc(new_line_list_size * sizeof(line_t *));

		// copy old list up to cursor_y (exclusive)
printf("old list before cursor %d %d\n",(cursor_y),line_count);
		crt_copy_memory(new_line_list,line_list,(cursor_y) * sizeof(line_t *));
		
		// copy old list after cursor_y + new_line_count + 1
printf("remainder %d / %d, %d\n",cursor_y + 1,line_count,line_count - (cursor_y+1));
		crt_copy_memory(&new_line_list[cursor_y + new_line_count + 1],&line_list[cursor_y + 1],(line_count - (cursor_y+1)) * sizeof(line_t *));

		// replace line list with the new list
		mem_free(line_list);
		line_list = new_line_list;
		line_list_size = new_line_list_size;
	}
	else
	{
		// make a gap
//printf("gap dst %d, src %d, len %d\n",cursor_y+new_line_count+1,cursor_y+1,line_count - (cursor_y + 1));
		crt_move_memory(&line_list[cursor_y+new_line_count + 1],&line_list[cursor_y+1],(line_count - (cursor_y + 1)) * sizeof(line_t *));
	}
	
	line_count += new_line_count;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// add lines
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	#error pasting 0 length text doesnot work.
	
	p = text;
	e = text + len;
	line_index = cursor_y;
	
	for(;;)
	{
		start = p;
		
		// skip to end	or EOL
		for(;;)
		{	
			if (p >= e)
			{
				isnl = 0;
				break;
			}
			else
			if ((p + 2 <= e) && (*p == '\r') && (p[1] == '\n'))
			{
				p += 2;
				isnl = 2;
				
				break;
			}
			else
			if (*p == '\n')
			{
				p++;
				isnl = 1;

				break;
			}
			else
			{
				p++;
			}
		}
		/*
		{
			unsigned char *_p;
			unsigned char *_e;
			
			_p = start;
			_e = start + (int)(p - start - isnl);
			
			printf("add line %d: ",line_index);
			
			while(_p < _e)
			{
				printf("%c",*_p);
				
				_p++;
			}
			printf(" (%d)\n",p - start - isnl);
		}
		*/	
		// we have a line
		size = 0;

		// is this the first line ? 
		// if so add the start of the text.
		
		if (start == text)
		{
			size += ch;
		}
		
		// add the length of the text being inserted.
		size += p - start;
		
		// add the end text if this is the last line.
		if (!isnl)
		{
			size += first_line->len - ch;
		}
		
		// create a new line
		line_list[line_index] = line_alloc();

		// init
		line_list[line_index]->memory = mem_alloc(size);
		line_list[line_index]->len = size;
		line_list[line_index]->text = line_list[line_index]->memory;
		line_list[line_index]->size = size;
		
		// fill in text
		line_p = line_list[line_index]->memory;
		
		// copy first line left
		if (start == text)
		{
			crt_copy_memory(line_p,first_line->text,ch);
			line_p += ch;
		}
		
		// copy inserted text
		crt_copy_memory(line_p,start,p-start);
		line_p += p-start;
		
		// copy first line right
		if (!isnl)
		{
			// save endch
			endch = line_p - line_list[line_index]->memory;

			crt_copy_memory(line_p,first_line->text + ch,first_line->len - ch);
		}
	
		new_text_wide = get_text_wide(line_index);
		if (new_text_wide > text_wide) 
		{
			text_wide = new_text_wide;
		}
		
		line_index++;
		
		if (!isnl) break;
	}
	
	// free the initial line we replaced.
	free_line(first_line);
	free_line_add((free_line_t *)first_line);

	// invalidate everything after first line.
	if (new_line_count)
	{
		RECT rect;
		
		GetClientRect(edit_hwnd,&rect);
		rect.top = ((cursor_y + 1) * font_high) - (get_vscroll() * font_high);
		
		InvalidateRect(edit_hwnd,&rect,FALSE);
	}

	// move everything to the right of the insertion
	// we MUST invalidate instead of scrolling
	// because tabs dont always move by one charactor. 

	{
		RECT rect;
		int wide;
		
		GetClientRect(edit_hwnd,&rect);
		wide = rect.right - rect.left;
		
		rect.left = 2 + (ch_to_col(ch,cursor_y) * font_wide);
		rect.top = cursor_y * font_high;
		rect.right = (get_hscroll()*font_wide) + wide;
		rect.bottom = rect.top + font_high;
		
		OffsetRect(&rect,-get_hscroll()*font_wide,-get_vscroll()*font_high);
		
		InvalidateRect(edit_hwnd,&rect,FALSE);
	}

	// resize the window.
	size_window();
	
	if (is_replace)
	{
		if (find_start_chx != -1)
		{
			if (find_start_cmp < 0)
			{
printf("insert: moved find start %d,%d -> ",find_start_chx,find_start_y);
				if (cursor_y == find_start_y)
				{
					find_start_chx -= col_to_ch(cursor_x,cursor_y);
					find_start_chx += endch;
				}
				else
				{
					// its been moved down.
					find_start_y += new_line_count;
				}
printf("%d,%d\n",find_start_chx,find_start_y);
			}
		}
	}

	// move the cursor.
	set_cursor_ch_position(endch,cursor_y + new_line_count,select?0:1,!is_replace,bring_into_view);
	
	if (!is_dirty)
	{
		is_dirty = 1;
		
		update_title();
	}
}

// returns 0 if no next ch.
static int next_ch(int *chx,int *y)
{
	int newx;
	int newy;
	
	newx = *chx;
	newy = *y;

	if ((newy == line_count - 1) && (newx == get_line_chend(newy)))
	{
		return 0;
	}
		
	if (newx + 1 > get_line_chend(newy))
	{
		if (newy + 1 >= line_count)
		{
			*chx = newx;
			*y = newy;
		
			return 1;
		}

		newx = 0;
		newy++;
	}
	else
	{
		newx++;
	}
	
	*chx = newx;
	*y = newy;
	
	return 1;
}

static int char_type(int chx,int y)
{
	int c;
	
	if ((y >= 0) && (y < line_count))
	{
		if ((chx >= 0) && (chx < line_list[y]->len))
		{
			c = line_list[y]->text[chx];

			if (c == ' ') return VOIDPAD_CHAR_TYPE_SPACE;
			if (c == '\t') return VOIDPAD_CHAR_TYPE_SPACE;
			if (c == '\r') return VOIDPAD_CHAR_TYPE_SPACE;
			if (c == '\n') return VOIDPAD_CHAR_TYPE_SPACE;
			
			if (is_alphanum(c))
			{
				return VOIDPAD_CHAR_TYPE_ALPHANUM;
			}
			
			return VOIDPAD_CHAR_TYPE_SYMBOL;
		}
	}

	// null and newlines
	return VOIDPAD_CHAR_TYPE_SPACE;
}

static int can_read_from(int chx,int y)
{
	// already at the end?
	if ((y >= 0) && (y < line_count))
	{
		// newlines are readable.
		if ((chx >= 0) && (chx < line_list[y]->len))
		{
			return 1;
		}
	}
	
	return 0;
}

// returns 0 if no next word
static int next_word(int *chx,int *y)
{
	int newx;
	int newy;
	int first;
	int firstctype;
	int ctype;
	
	newx = *chx;
	newy = *y;
	first = 1;
	
	if ((newy == line_count - 1) && (newx == get_line_chend(newy)))
	{
		return 0;
	}
	
	for(;;)
	{
		// is ws?
		if (first)
		{
			firstctype = char_type(newx,newy);
		}
		else
		{
			ctype = char_type(newx,newy);
			if (ctype == VOIDPAD_CHAR_TYPE_SPACE) firstctype = VOIDPAD_CHAR_TYPE_SPACE;
			
			if (ctype != firstctype)
			{
				*chx = newx;
				*y = newy;
				
				return 1;
			}
		}

		// find a whitespace, or letter that breaks the search.

		if (newx + 1 > get_line_chend(newy))
		{
			if ((!first) || (newy + 1 >= line_count))
			{
				*chx = newx;
				*y = newy;
				
				return 1;
			}

			newx = 0;
			newy++;
		}
		else
		{
			// unless this is a tab?
			newx++;
		}
		
		first = 0;
	}
}

// returns 0 if no next ch.
static int prev_ch(int *chx,int *y)
{
	int newx;
	int newy;
	
	newx = *chx;
	newy = *y;
	
	if ((newx == 0) && (newy == 0))
	{
		return 0;
	}

	if (newx - 1 < 0)
	{
		if (newy - 1 < 0)
		{
			*chx = newx;
			*y = newy;
			
			return 1;
		}

		newy--;
		newx = get_line_chend(newy);
	}
	else
	{
		newx--;
	}
	
	*chx = newx;
	*y = newy;
	
	return 1;
}

// returns 0 if no next ch.
static int prev_word(int *chx,int *y)
{
	int newx;
	int newy;
	int first;
	int firstctype;
	int ctype;
	int lastx;
	int lasty;
	
	newx = *chx;
	newy = *y;
	first = 1;

	if ((newx == 0) && (newy == 0))
	{	
		return 0;
	}
	
	for(;;)
	{
		lastx = newx;
		lasty = newy;
		
		if (newx - 1 < 0)
		{
			if ((!first) || (newy - 1 < 0))
			{
				*chx = newx;
				*y = newy;
				
				return 1;
			}

			newy--;
			newx = get_line_chend(newy);
		}
		else
		{
			newx--;
		}
		
		// is ws?
		if (first)
		{
			firstctype = char_type(newx,newy);
		}
		else
		{
			ctype = char_type(newx,newy);
			if (ctype != VOIDPAD_CHAR_TYPE_SPACE) 
			{
				if (firstctype == VOIDPAD_CHAR_TYPE_SPACE)
				{
					firstctype = ctype;
				}
			}
			
			if (ctype != firstctype)
			{
				*chx = lastx;
				*y = lasty;
				
				return 1;
			}
		}

		first = 0;
	}
}

static void select_word(int chx,int y,int *pleft,int *pright)
{
	int clicktype;
	int chend;
	int left;
	int right;

printf("select word %d %d\n",chx,y)	;
	
	chend = get_line_chend(y);
	
	// find the end of the word first.
	clicktype = char_type(chx,y);
	
printf("clicktype %d\n",clicktype);

	// only allow space selecting when more than one space exists.
	if (clicktype == VOIDPAD_CHAR_TYPE_SPACE)
	{
		if (chx + 1 < chend)
		{
			if (char_type(chx + 1,y) == VOIDPAD_CHAR_TYPE_SPACE)
			{
				goto start;
			}
		}
		
		// next ch is NOT a space.
		
		if (chx - 1 >= 0)
		{
			if (char_type(chx - 1,y) == VOIDPAD_CHAR_TYPE_SPACE)
			{
				goto start;
			}
		}
		
printf("single space\n",chx,y);

		// single space char, try prev with higher priority, then try next
		if (chx - 1 >= 0)
		{
			clicktype = char_type(chx - 1,y);
			chx--;
		}
		else
		if (chx + 1 < chend)
		{
			clicktype = char_type(chx + 1,y);
			chx++;
		}
		
		// fall through, and just select the single space..
	}
	
start:

printf("type %c\n",clicktype);

	// find the start of the word.
	left = chx;
	for(;;)
	{
		left--;
		
		if ((left < 0) || (char_type(left,y) != clicktype))
		{
			left++;
			break;
		}
	}
	
	// find right
	right = chx;
	for(;;)
	{
		right++;
		
		if ((right >= chend) || (char_type(right,y) != clicktype))
		{
			right--;
			break;
		}
	}
	
	*pleft = left;
	
	if (right < line_list[y]->len)
	{
		*pright = right + 1;
	}
	else
	{
		*pright = right;
	}
}

static int select_alphanum_word(int chx,int y,int *pleft,int *pright)
{
	int clicktype;
	int chend;
	int left;
	int right;

printf("select word %d %d\n",chx,y)	;
	
	chend = get_line_chend(y);
	
	// find the end of the word first.
	clicktype = char_type(chx,y);
	
printf("clicktype %d\n",clicktype);

	// only allow space selecting when more than one space exists.
	if (clicktype != VOIDPAD_CHAR_TYPE_ALPHANUM)
	{
		// single space char, try prev with higher priority, then try next
		if (chx - 1 >= 0)
		{
			clicktype = char_type(chx - 1,y);
			if (clicktype == VOIDPAD_CHAR_TYPE_ALPHANUM)
			{
				chx--;
				
				goto start;
			}
		}
		
		// fall through, and just select the single space..
		return 0;
	}
	
start:

printf("type %c\n",clicktype);

	// find the start of the word.
	left = chx;
	for(;;)
	{
		left--;
		
		if ((left < 0) || (char_type(left,y) != clicktype))
		{
			left++;
			break;
		}
	}
	
	// find right
	right = chx;
	for(;;)
	{
		right++;
		
		if ((right >= chend) || (char_type(right,y) != clicktype))
		{
			right--;
			break;
		}
	}
	
	*pleft = left;
	
	if (right < line_list[y]->len)
	{
		*pright = right + 1;
	}
	else
	{
		*pright = right;
	}
	
	return 1;
}

static void voidpad_cut(void)
{
	if (!is_selection())
	{
		undo_clear_selection();
	
		set_col_sel(0,cursor_y,0,cursor_y+1);
	}
	
	voidpad_copy();
	insert_text(0,0,1,0,1,0);
}

static void voidpad_copy(void)
{
	HGLOBAL hglobal;
	unsigned char *selected_text;
	int selected_len;

	if (OpenClipboard(voidpad_hwnd))
	{
		int wlen;

		EmptyClipboard();

		selected_text = alloc_selection(&selected_len);
		if (!selected_text)
		{
			// no selected? just copy current line.
			selected_len = line_list[cursor_y]->len;
			selected_text = mem_alloc(selected_len + 1);
			crt_copy_memory(selected_text,line_list[cursor_y]->text,selected_len + 1);

//review: add a new line if one does not exist?
		}
		
		wlen = MultiByteToWideChar(voidpad_cp,0,(char *)selected_text,selected_len,0,0);
		
		hglobal = GlobalAlloc(GMEM_MOVEABLE,(wlen + 1) * sizeof(wchar_t));
		if (hglobal)
		{
			wchar_t *wbuf;
			
			wbuf = GlobalLock(hglobal);
			if (wbuf)
			{
				MultiByteToWideChar(voidpad_cp,0,(char *)selected_text,selected_len,wbuf,wlen);
				wbuf[wlen] = 0;
				
				GlobalUnlock(hglobal);
						
				SetClipboardData(CF_UNICODETEXT,hglobal);
			}
		}		
		
		hglobal = GlobalAlloc(GMEM_MOVEABLE,sizeof(DWORD) + (selected_len));
		if (hglobal)
		{
			unsigned char *buf;
			
			buf = GlobalLock(hglobal);
			if (buf)
			{
				*(DWORD *)buf = selected_len;
				buf += sizeof(DWORD);
				
				crt_copy_memory(buf,selected_text,selected_len);
			
				GlobalUnlock(hglobal);
						
				SetClipboardData(voidpad_clipboard_format,hglobal);
			}
		}

		voidpad_clipboard_ring_add(selected_text);
		
		mem_free(selected_text);
				
		CloseClipboard();
	}
}

static void voidpad_paste(int select)
{
	if (OpenClipboard(voidpad_hwnd))
	{
		HGLOBAL hglobal;

		hglobal = GetClipboardData(voidpad_clipboard_format);
		if (hglobal)
		{
			unsigned char *buf;
			
			buf = (unsigned char *)GlobalLock(hglobal);
			if (buf)
			{
				DWORD len;
				
				len = *(DWORD *)buf;
				buf += sizeof(DWORD);
				
				insert_text(buf,len,1,select,1,0);
				
				GlobalUnlock(hglobal);
			}
		}
		else
		{
			hglobal = GetClipboardData(CF_UNICODETEXT);
			if (hglobal)
			{
				wchar_t *buf;
				
				buf = (wchar_t *)GlobalLock(hglobal);
				if (buf)
				{
					unsigned char *text;
					
					text = os_alloc_wchar_to_voidpad_cp(buf);
					
					insert_text(text,string_length(text),1,select,1,0);
					
					mem_free(text);
					
					GlobalUnlock(hglobal);
				}
			}
		}
	
		CloseClipboard();
	}
}

static int voidpad_exit(void)
{
	if (save_changes())
	{
		PostQuitMessage(0);
		
		return 1;
	}
	
	return 0;
}

static void set_insert_mode(int mode)
{
	if (mode != insert_mode)
	{
		RECT rect;
		
		// hide old cursor
		get_cursor_rect(&rect);

		InvalidateRect(edit_hwnd,&rect,FALSE);

		// change mode		
		insert_mode = mode;
		
		// redraw new cursor.
		get_cursor_rect(&rect);

		InvalidateRect(edit_hwnd,&rect,FALSE);
		
		reset_cursor_blink();
		
		// update statusbar
		update_ins_status();
	}
}


static void voidpad_select_all(void)
{
	// dont bring into view.
	set_cursor_col_position(0,0,1,1,0);
	set_cursor_col_position(get_line_colend(line_count-1),line_count-1,0,1,voidpad_select_all_bring_into_view);
}

// attempt to automatically fill in the search field with text from the editor.
static void voidpad_find_auto_fill(void)
{
	// fill the find box with the current selection or word
	if (GetFocus() == edit_hwnd)
	{
		SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),CB_SETCURSEL,(WPARAM)-1,0);
	
		if (is_selection())
		{
			if (sel_y1 == sel_y2)
			{
				int sel_ch1;
				int sel_ch2;
				
				sel_ch1 = col_to_ch(sel_x1,sel_y1);
				sel_ch2 = col_to_ch(sel_x2,sel_y2);
				
				set_window_text(GetDlgItem(find_hwnd,ID_FIND_COMBO),line_list[sel_y1]->text + sel_ch1,sel_ch2 - sel_ch1);
				
				goto set;
			}
		}
		else
		{
			int left;
			int right;
			
			// fill in with closest word.
			if (select_alphanum_word(col_to_ch(cursor_x,cursor_y),cursor_y,&left,&right))
			{
				set_window_text(GetDlgItem(find_hwnd,ID_FIND_COMBO),line_list[cursor_y]->text + left,right - left);
					
				goto set;
			}
		}

		if (search_string)
		{
			set_window_text(GetDlgItem(find_hwnd,ID_FIND_COMBO),search_string,search_string_len);
		}
		else
		{
			set_window_text(GetDlgItem(find_hwnd,ID_FIND_COMBO),0,0);
		}
	}
	
set:

	voidpad_find_search_changed();

	SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),CB_SETEDITSEL,0,MAKELPARAM(0,-1));
	SetFocus(GetDlgItem(find_hwnd,ID_FIND_COMBO));
}

static void voidpad_find_dialog(int replace)
{
	WNDCLASSEXW wcex;
	RECT rect;
	int wide;
	int high;
	
	rect.left = 0;
	rect.top = 0;
	rect.right = 384;
	
	if (replace)
	{
		rect.bottom = 132 + 45;
	}
	else
	{
		rect.bottom = 132;
	}

	AdjustWindowRectEx(&rect,WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,FALSE,WS_EX_TOOLWINDOW);
	
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;
	
	if ((find_x == CW_USEDEFAULT) || (find_y == CW_USEDEFAULT))
	{
		POINT p1;
		POINT p2;
		
		p1.x = 0;
		p1.y = 0;
		ClientToScreen(edit_hwnd,&p1);
		
		GetClientRect(edit_hwnd,&rect);
		p2.x = rect.right - rect.left;
		p2.y = rect.bottom - rect.top;

		ClientToScreen(edit_hwnd,&p2);
		
		find_x = p1.x + ((p2.x - p1.x) / 2) - (wide / 2);
		find_y = p1.y + ((p2.y - p1.y) / 2) - (high / 2);
	}
	
	rect.left = find_x;
	rect.top = find_y;
	rect.right = rect.left + wide;
	rect.bottom = rect.top + high;

	os_make_rect_completely_visible(&rect);

	if (find_hwnd)
	{
		SetWindowPos(find_hwnd,0,rect.left,rect.top,wide,high,SWP_NOZORDER|SWP_NOACTIVATE);
	
		voidpad_find_position_widgets(replace);
		
		voidpad_find_auto_fill();
		
		return;
	}

	// Initialize global strings
	crt_zero_memory(&wcex,sizeof(wcex));
	wcex.cbSize = sizeof(wcex); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)voidpad_find_proc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(0);
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL,IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"VOIDPADFIND";
	wcex.hIconSm		= 0;

	RegisterClassExW(&wcex);


	find_hwnd = CreateWindowExW(
		WS_EX_TOOLWINDOW,
		L"VOIDPADFIND", 
		L"", 
		WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		rect.left,rect.top,wide,high, 
		voidpad_hwnd,NULL,GetModuleHandle(0),NULL);
		
	voidpad_find_position_widgets(replace);
		
	voidpad_find_update_search_combobox();
	voidpad_find_update_replace_combobox();
	
	// fill the find box with the current selection or word
	voidpad_find_auto_fill();

	if (ComboBox_GetCount(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO)))
	{
		ComboBox_SetCurSel(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),0);
	}
	
	voidpad_find_replace_changed();

	ShowWindow(find_hwnd, SW_SHOWNORMAL);
	UpdateWindow(find_hwnd);
}

static void voidpad_goto_dialog(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_goto_proc,0);
}

static void voidpad_insert_hex_dialog(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_insert_hex_proc,0);
}

static void voidpad_insert_utf8_sequence_dialog(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_insert_utf8_sequence_proc,0);
}

static void voidpad_insert_utf16_sequence_dialog(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_insert_utf16_sequence_proc,0);
}

static void set_window_rect(HWND hwnd,int x,int y,int wide,int high)
{
	SetWindowPos(hwnd,0,x,y,wide,high,SWP_NOZORDER|SWP_NOACTIVATE);
}

static void os_set_default_font(HWND hwnd)
{
	SendMessage(hwnd,WM_SETFONT,(WPARAM)os_default_hfont,(LPARAM)FALSE);
}

static void voidpad_find_position_widgets(int replace)
{
	RECT rect;
	int wide;
	int high;
	int x;
	int y;

	GetClientRect(find_hwnd,&rect);
	
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;
			
	x = 12;
	y = 12;
	wide -= 24;
	high -= 24;

	SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_STATIC),0,x,y,wide,15,SWP_NOACTIVATE|SWP_NOZORDER);
	y += 15 + 3;
	
	combobox_set_position(find_hwnd,ID_FIND_COMBO,x,y,wide,21+96);
	y += 21 + 6;	
	
	if (replace)
	{
		SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_REPLACE_STATIC),0,x,y,wide,15,SWP_NOACTIVATE|SWP_NOZORDER);
		y += 15 + 3;
		
		combobox_set_position(find_hwnd,ID_FIND_REPLACE_COMBO,x,y,wide,21+96);
		y += 21 + 6;	

		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_STATIC),SW_SHOWNA);
		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),SW_SHOWNA);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_STATIC),TRUE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),TRUE);
	}
	else
	{
		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_STATIC),SW_HIDE);
		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),SW_HIDE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_STATIC),FALSE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),FALSE);
	}

	SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_CASE),0,x,y,wide,15,SWP_NOACTIVATE|SWP_NOZORDER);
	y += 15 + 6;	

	SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_WHOLEWORD),0,x,y,wide,15,SWP_NOACTIVATE|SWP_NOZORDER);
	y += 15 + 6;
	
	if (replace)
	{
		SetWindowPos(GetDlgItem(find_hwnd,IDOK),0,x+wide-75-6-75-6-75,y,75,23,SWP_NOACTIVATE|SWP_NOZORDER);
		SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),0,x+wide-75-6-75,y,75,23,SWP_NOACTIVATE|SWP_NOZORDER);
		SetWindowPos(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),0,x+wide-75,y,75,23,SWP_NOACTIVATE|SWP_NOZORDER);
		y += 23 + 6;	

		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),SW_SHOWNA);
		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),SW_SHOWNA);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),TRUE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),TRUE);
	}
	else
	{
		SetWindowPos(GetDlgItem(find_hwnd,IDOK),0,x+wide-75,y,75,23,SWP_NOACTIVATE|SWP_NOZORDER);
		y += 23 + 6;	

		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),SW_HIDE);
		ShowWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),SW_HIDE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),FALSE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),FALSE);
	}

	SetWindowText(find_hwnd,replace ? L"Replace" : L"Find");
}

static LRESULT CALLBACK voidpad_find_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_MOVE:
		{
			if (IsWindowVisible(hwnd))
			{
				RECT rect;
				GetWindowRect(find_hwnd,&rect);
				find_x = rect.left;
				find_y = rect.top;
			}
			break;
		}

		case WM_SETFOCUS:
			SetFocus(GetDlgItem(find_hwnd,ID_FIND_COMBO));
			break;
			
		case WM_DESTROY:
			find_hwnd = 0;
			break;

		case WM_CREATE:
		{
			// static find what:
			os_dlg_create_static(hwnd,ID_FIND_STATIC,0,L"Fi&nd what:",0,0,0);
			
			// edit
			CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"COMBOBOX",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_HASSTRINGS,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_COMBO,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_COMBO));
							
			// static find what:
			os_dlg_create_static(hwnd,ID_FIND_REPLACE_STATIC,0,L"Re&place with:",0,0,0);
			
			// edit
			CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"COMBOBOX",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_HASSTRINGS,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_REPLACE_COMBO,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_REPLACE_COMBO));
							
			// case
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Match &case",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_CASE,GetModuleHandle(0),0);

			CheckDlgButton(hwnd,ID_FIND_CASE,find_case);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_CASE));

			// wholeword
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Match &whole word",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_WHOLEWORD,GetModuleHandle(0),0);

			CheckDlgButton(hwnd,ID_FIND_WHOLEWORD,find_wholeword);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_WHOLEWORD));
			
			// find next button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"&Find Next",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				0,0,0,0,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));
			
			// replace
			CreateWindowExW(
				0,
				L"BUTTON",
				L"&Replace",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_REPLACE_BUTTON,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_REPLACE_BUTTON));	
					
			// replace all
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Replace &All",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				0,0,0,0,
				hwnd,(HMENU)ID_FIND_REPLACEALL_BUTTON,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_FIND_REPLACEALL_BUTTON));
			
			break;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case ID_FIND_COMBO:
				
					if ((HIWORD(wParam) == CBN_EDITCHANGE) || (HIWORD(wParam) == CBN_SELCHANGE))
					{
						voidpad_find_search_changed();
					}
					
					break;
				
				case ID_FIND_REPLACE_COMBO:
				
					if ((HIWORD(wParam) == CBN_EDITCHANGE) || (HIWORD(wParam) == CBN_SELCHANGE))
					{
						voidpad_find_replace_changed();
					}
					
					break;
				
				case ID_FIND_CASE:

					voidpad_command((IsDlgButtonChecked(find_hwnd,ID_FIND_CASE) == BST_CHECKED) ? VOIDPAD_COMMAND_FIND_CASE_ENABLE : VOIDPAD_COMMAND_FIND_CASE_DISABLE);

					break;

				case ID_FIND_WHOLEWORD:

					voidpad_command((IsDlgButtonChecked(find_hwnd,ID_FIND_WHOLEWORD) == BST_CHECKED) ? VOIDPAD_COMMAND_FIND_WHOLEWORD_ENABLE : VOIDPAD_COMMAND_FIND_WHOLEWORD_DISABLE);

					break;
					
				case ID_FIND_REPLACE_BUTTON:
					SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),CB_SETEDITSEL,0,MAKELPARAM(0,-1));
					SetFocus(GetDlgItem(find_hwnd,ID_FIND_COMBO));
					voidpad_command(VOIDPAD_COMMAND_REPLACE);
					break;
					
				case ID_FIND_REPLACEALL_BUTTON:
					SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),CB_SETEDITSEL,0,MAKELPARAM(0,-1));
					SetFocus(GetDlgItem(find_hwnd,ID_FIND_COMBO));
					voidpad_command(VOIDPAD_COMMAND_REPLACEALL);
					break;

				case IDOK:
					SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),CB_SETEDITSEL,0,MAKELPARAM(0,-1));
					SetFocus(GetDlgItem(find_hwnd,ID_FIND_COMBO));
					voidpad_command((GetKeyState(VK_SHIFT) < 0) ? VOIDPAD_COMMAND_FIND_PREV : VOIDPAD_COMMAND_FIND_NEXT);
					break;

				case IDCANCEL:
					DestroyWindow(find_hwnd);
					break;
			}
			
			break;
		}
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static void voidpad_goto_update_next_button(HWND hwnd)
{
	int enabled;
	
	enabled = GetWindowTextLength(GetDlgItem(hwnd,ID_GOTO_WHAT_EDIT));
	
	EnableWindow(GetDlgItem(hwnd,IDOK),enabled);
}

INT_PTR CALLBACK voidpad_goto_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwnd,ID_GOTO_WHAT_EDIT));
			return TRUE;
			
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,256,98);
			
			SetWindowText(hwnd,L"Go To Line");
				
			x = 12;
			y = 12;
			wide = 256 - 24;
			high = 98 - 24;
	
			// static goto what:
			{
				wchar_t wbuf[MAX_PATH];
				wchar_t number_buf[MAX_PATH];
				
				wstring_format_number(number_buf,line_count);

				wstring_copy(wbuf,MAX_PATH,L"&Line Number (1-");
				wstring_cat(wbuf,MAX_PATH,number_buf);
				wstring_cat(wbuf,MAX_PATH,L"):");
				
				os_dlg_create_static(hwnd,ID_GOTO_WHAT_STATIC,0,wbuf,x,y,wide);
			}
			
			y += 15 + 3;
					
			// edit
			CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"EDIT",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL,
				x,y,wide,21,
				hwnd,(HMENU)ID_GOTO_WHAT_EDIT,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_GOTO_WHAT_EDIT));

			y += 21 + 12;
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"OK",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75,y,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
			
			// fill the goto box with the current selection or word
			SetDlgItemInt(hwnd,ID_GOTO_WHAT_EDIT,cursor_y + 1,FALSE);

			SendMessage(GetDlgItem(hwnd,ID_GOTO_WHAT_EDIT),EM_SETSEL,0,(LPARAM)-1);
			SetFocus(GetDlgItem(hwnd,ID_GOTO_WHAT_EDIT));
					
			voidpad_goto_update_next_button(hwnd);
				
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case ID_GOTO_WHAT_EDIT:
				
					if (HIWORD(wParam) == EN_CHANGE)
					{
						voidpad_goto_update_next_button(hwnd);
					}
					
					break;
					
				case IDOK:
				{
					DWORD y;
					
					y = GetDlgItemInt(hwnd,ID_GOTO_WHAT_EDIT,0,FALSE)-1;

					voidpad_command_data(VOIDPAD_COMMAND_GOTO,&y,sizeof(DWORD));

					EndDialog(hwnd,1);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd,2);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}


static INT_PTR CALLBACK voidpad_insert_utf8_sequence_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwnd,IDOK));
			return TRUE;
			
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,512,384);
			
			SetWindowText(hwnd,L"Edit UTF-8");
				
			x = 12;
			y = 12;
			wide = 512 - 24;
			high = 384 - 24;
	
			// static goto what:
			os_dlg_create_static(hwnd,ID_INSERT_UTF8_SEQUENCE_WHAT_STATIC,0,L"&Text:",x,y,wide);
			
			y += 15 + 3;
					
			{
				unsigned char *buf;
				int len;
				int wlen;
				wchar_t *wbuf;
				
				buf = alloc_selection(&len);
				
				wlen = MultiByteToWideChar(CP_UTF8,0,(char *)buf,len,0,0);
				wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
				
				MultiByteToWideChar(CP_UTF8,0,(char *)buf,len,wbuf,wlen);
				wbuf[wlen] = 0;
				
				// edit
				CreateWindowExW(
					WS_EX_CLIENTEDGE,
					L"EDIT",
					wbuf,
					WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_WANTRETURN,
					x,y,wide,307,
					hwnd,(HMENU)ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT,GetModuleHandle(0),0);
					
				voidpad_old_edit_proc = SubclassWindow(GetDlgItem(hwnd,ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT),voidpad_multiline_edit_proc);
					
				mem_free(wbuf);
				if (buf)
				{
					mem_free(buf);
				}
			}

			os_set_default_font(GetDlgItem(hwnd,ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT));

			y += 307 + 12;
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Insert",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75,y,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
			
			SetFocus(GetDlgItem(hwnd,ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT));
					
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t *wbuf;
					unsigned char *buf;
					
					wbuf = wstring_alloc_window_text(GetDlgItem(hwnd,ID_INSERT_UTF8_SEQUENCE_WHAT_EDIT));
					
					buf = utf8_alloc_wchar(wbuf);
					
					voidpad_command_data(VOIDPAD_COMMAND_INSERT_TEXT,buf,string_length(buf));
					
					mem_free(buf);
					mem_free(wbuf);
					
					EndDialog(hwnd,1);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd,2);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static INT_PTR CALLBACK voidpad_insert_hex_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwnd,IDOK));
			return TRUE;
			
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,512,384);
			
			SetWindowText(hwnd,L"Edit Hex");
				
			x = 12;
			y = 12;
			wide = 512 - 24;
			high = 384 - 24;
	
			// static goto what:
			os_dlg_create_static(hwnd,ID_INSERT_HEX_WHAT_STATIC,0,L"&Text:",x,y,wide);
			
			y += 15 + 3;
					
			{
				unsigned char *buf;
				int len;
				int wlen;
				wchar_t *wbuf;
				
				buf = alloc_selection(&len);
				
				wlen = ((len * 2) + (((len + 15) / 16) * 2)) * sizeof(wchar_t);
				wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
				
				{
					unsigned char *p;
					wchar_t *d;
					DWORD run;
					DWORD i;

					p = buf;
					d = wbuf;
					
					run = len;
					i = 0;
					
					while(run)
					{
						if (i)
						{
							if ((i % 16) == 0)
							{
								*d++ = '\r';
								*d++ = '\n';
							}
						}
						
						*d++ = _voidpad_hexchar(*p / 16);
						*d++ = _voidpad_hexchar(*p % 16);
						
						p++;
						run--;
						i++;
					}
					*d = 0;
				}
				
				// edit
				CreateWindowExW(
					WS_EX_CLIENTEDGE,
					L"EDIT",
					wbuf,
					WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_WANTRETURN,
					x,y,wide,307,
					hwnd,(HMENU)ID_INSERT_HEX_WHAT_EDIT,GetModuleHandle(0),0);
					
				voidpad_old_edit_proc = SubclassWindow(GetDlgItem(hwnd,ID_INSERT_HEX_WHAT_EDIT),voidpad_multiline_edit_proc);
					
				mem_free(wbuf);
				if (buf)
				{
					mem_free(buf);
				}
			}

			os_set_default_font(GetDlgItem(hwnd,ID_INSERT_HEX_WHAT_EDIT));

			y += 307 + 12;
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Insert",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75,y,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
			
			SetFocus(GetDlgItem(hwnd,ID_INSERT_HEX_WHAT_EDIT));
					
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t *wbuf;
					const wchar_t *s;
					int wlen;
					int len;
					unsigned char *buf;
					unsigned char *d;
					
					wbuf = wstring_alloc_window_text(GetDlgItem(hwnd,ID_INSERT_HEX_WHAT_EDIT));
					wlen = wstring_length(wbuf);
					
					len = wlen / 2;
					buf = mem_alloc(len);

					s = wbuf;
					d = buf;
					
					for(;;)
					{
						if ((*s) && (s[1]))
						{
							*d++ = (_voidpad_hexvalue(*s) << 4) | _voidpad_hexvalue(s[1]);
							
							s += 2;
						}
						else
						{
							break;
						}
					}
					
					voidpad_command_data(VOIDPAD_COMMAND_INSERT_TEXT,buf,len);
					
					mem_free(buf);
					mem_free(wbuf);
					
					EndDialog(hwnd,1);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd,2);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static INT_PTR CALLBACK voidpad_insert_utf16_sequence_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwnd,IDOK));
			return TRUE;
			
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,512,384);
			
			SetWindowText(hwnd,L"Edit UTF-16");
				
			x = 12;
			y = 12;
			wide = 512 - 24;
			high = 384 - 24;
	
			// static goto what:
			os_dlg_create_static(hwnd,ID_INSERT_UTF16_SEQUENCE_WHAT_STATIC,0,L"&Text:",x,y,wide);
			
			y += 15 + 3;
					
			{
				unsigned char *buf;
				int len;
				int wlen;
				wchar_t *wbuf;
				
				buf = alloc_selection(&len);
				
				wlen = (len / sizeof(wchar_t));
				wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
				
				crt_copy_memory(wbuf,buf,wlen * sizeof(wchar_t));
				
				wbuf[wlen] = 0;

				// edit
				CreateWindowExW(
					WS_EX_CLIENTEDGE,
					L"EDIT",
					wbuf,
					WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_WANTRETURN,
					x,y,wide,307,
					hwnd,(HMENU)ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT,GetModuleHandle(0),0);
					
				voidpad_old_edit_proc = SubclassWindow(GetDlgItem(hwnd,ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT),voidpad_multiline_edit_proc);
					
				mem_free(wbuf);
				if (buf)
				{
					mem_free(buf);
				}
			}

			os_set_default_font(GetDlgItem(hwnd,ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT));

			y += 307 + 12;
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Insert",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75,y,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
			
			SetFocus(GetDlgItem(hwnd,ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT));
					
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t *wbuf;
					
					wbuf = wstring_alloc_window_text(GetDlgItem(hwnd,ID_INSERT_UTF16_SEQUENCE_WHAT_EDIT));
				
					voidpad_command_data(VOIDPAD_COMMAND_INSERT_TEXT,wbuf,wstring_length(wbuf) * sizeof(wchar_t));
					
					mem_free(wbuf);
					
					EndDialog(hwnd,1);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd,2);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static int voidpad_command(int command)
{
	return voidpad_command_data(command,0,0);
}

static int voidpad_command_data(int command,const void *data,int size)
{
printf("command %d (%d)\n",command,size);

	if (voidpad_macro_is_recording)
	{
		voidpad_macro_add(command,data,size);
	}
	
	switch(command)
	{
		case VOIDPAD_COMMAND_NEW:	
			new_file(1);
			break;

		case VOIDPAD_COMMAND_OPEN: // wchar_t *filename
			voidpad_open(data);
			break;
			
		case VOIDPAD_COMMAND_SAVE:

			if (voidpad_filename)
			{
				if (voidpad_save())
				{
					return 1;
				}
			}

			break;
		
		case VOIDPAD_COMMAND_SAVE_DIRECT:

			if (voidpad_filename)
			{
				if (voidpad_save_direct())
				{
					return 1;
				}
			}

			break;
			
		case VOIDPAD_COMMAND_SAVEAS:
		
			if (voidpad_filename)
			{
				mem_free(voidpad_filename);
			}
			
			voidpad_filename = wstring_alloc(data);
		
			update_title();
			
			if (voidpad_save())
			{
				return 1;
			}

			break;
		
		case VOIDPAD_COMMAND_UNDO:
			undo();
			break;
			
		case VOIDPAD_COMMAND_REDO:
			redo();
			break;
			
		case VOIDPAD_COMMAND_CUT:
			voidpad_cut();
			break;
			
		case VOIDPAD_COMMAND_COPY:
			voidpad_copy();
			break;
			
		case VOIDPAD_COMMAND_PASTE:
			voidpad_paste(0);
			break;
			
		case VOIDPAD_COMMAND_CYCLE_CLIPBOARD_RING:

			// paste cycle
			if (voidpad_clipboard_ring_last)
			{
				if (is_selection())
				{
					unsigned char *selected_text;
					int selected_len;
					
					selected_text = alloc_selection(&selected_len);
					
					if (selected_len == voidpad_clipboard_ring_last->len)
					{
						if (string_compare(selected_text,voidpad_clipboard_ring_last->text) == 0)
						{
							voidpad_clipboard_ring_t *cr;
							
							cr = voidpad_clipboard_ring_last;
						
							voidpad_clipboard_ring_remove(cr);
							voidpad_clipboard_ring_insert_at_start(cr);

				
							if (undo_last)
							{
								if (undo_last->is_clipboard_ring)
								{
									undo_last->is_more = 1;
								}
							}						
						}
					}
					
					mem_free(selected_text);
				}
				
				insert_text(voidpad_clipboard_ring_last->text,voidpad_clipboard_ring_last->len,1,1,1,0);
				
				undo_last->is_clipboard_ring = 1;
			}
			else
			{
				voidpad_paste(1);
			}

			break;
			
		case VOIDPAD_COMMAND_DELETE:
		{
			int x;
			int y;
								
			if (is_selection())
			{
				insert_text(0,0,1,0,1,0);
			}
			else
			{
				x = get_cursor_chx();
				y = cursor_y;
				
				if (next_ch(&x,&y))
				{
					// add an undo so we dont restore the temp selection.
					undo_add_selection();
				
					set_col_sel(get_nearest_cursor_colx(),cursor_y,ch_to_col(x,y),y);

					insert_text(0,0,1,0,1,0);
				}
			}

			break;
		}
			
		case VOIDPAD_COMMAND_DELETE_LINE:

			// make sure we only delete the line if it would actually delete something.
			// otherwise we would fill the undo queue with useless crap.
			if ((is_selection()) || (cursor_y + 1 < line_count) || (line_list[cursor_y]->len > 0))
			{
				// save selection.
				// remove the entire selection so we only have to deal with one line below.
				undo_add_selection();
		
				set_col_sel(0,cursor_y,0,cursor_y+1);
				
				insert_text(0,0,1,0,1,0);
			}
					
			break;
			
		case VOIDPAD_COMMAND_SELECT_ALL:
			voidpad_select_all();
			break;
			
		case VOIDPAD_COMMAND_REPLACE:
		case VOIDPAD_COMMAND_REPLACEALL:
		{
			int replace_count;
		
			replace_add(replace_string);

			// update search and params from find_hwnd.
			if (find_hwnd)
			{
				voidpad_find_update_replace_combobox();
			}
			
			replace_count = 0;
			
			for(;;)
			{
				if (is_selection())
				{
					if (sel_y1 == sel_y2)
					{
						int sel_ch1;
						int sel_ch2;
						
						sel_ch1 = col_to_ch(sel_x1,sel_y1);
						sel_ch2 = col_to_ch(sel_x2,sel_y2);
						
						if (sel_ch2 - sel_ch1 == search_string_len)
						{
							if (search_compare(line_list[sel_y1]->text + sel_ch1,line_list[sel_y1]->text + line_list[sel_y1]->len))
							{
								if ((!find_wholeword) || (is_word(line_list[sel_y1]->text,line_list[sel_y1]->text + sel_ch1,line_list[sel_y1]->text + line_list[sel_y1]->len,search_string_len)))
								{
									// merge into one big undo.
									if (replace_count)
									{
										undo_last->is_more = 1;
									}

									// if we find text before we even do a search
									// set the find start here, so we dont find the same text again..
									if (find_start_chx == -1)
									{
										find_start_chx = sel_ch1;
										find_start_y = sel_y1;
										find_start_direction = 1;
									}
									
									// mark this as the last found text
									// and increase the find count.
									if (find_last_chx == -1)
									{
										find_last_chx = sel_ch1;
										find_last_y = sel_y1;
										
										find_found_count = 1;
									}
									
									insert_text(replace_string,replace_string_len,1,1,0,1);
									replace_count++;
								}
							}
						}
					}
				}

				if (!search(1,1,(command == VOIDPAD_COMMAND_REPLACEALL) && (replace_count)))
				{
					// reset start, so the next find or replace will start with the replaced selected text.
					// otherwise we would replace the selected text and think we have done a full loop already.
					find_start_chx = -1;
				
					break;
				}
				
				if (command == VOIDPAD_COMMAND_REPLACE) break;
			}

			voidpad_bring_cursor_into_view();
			
			if ((command == VOIDPAD_COMMAND_REPLACEALL) && (replace_count))
			{
				wchar_t wbuf[MAX_PATH];
				
				wstring_format_number(wbuf,replace_count);
				wstring_cat(wbuf,MAX_PATH,L" occurrence(s) replaced.");
				
				MessageBox(find_hwnd ? find_hwnd : voidpad_hwnd,wbuf,L"voidPad",MB_OK|MB_ICONINFORMATION);
			}	
											
			break;
		}
			
		case VOIDPAD_COMMAND_FIND_PREV:
		case VOIDPAD_COMMAND_FIND_NEXT:

			if (search_string_len)
			{
				find_add(search_string);

				// update search and params from find_hwnd.
				if (find_hwnd)
				{
					voidpad_find_update_search_combobox();
				}
				
				// execute the search in the desired direction.
				search(command == VOIDPAD_COMMAND_FIND_NEXT ? 1 : -1,0,0);
			}
			
			break;
			
		case VOIDPAD_COMMAND_GOTO:
			set_cursor_col_position(0,*(DWORD *)data,1,1,1);
			break;

		case VOIDPAD_COMMAND_MAKE_UPPERCASE:
		case VOIDPAD_COMMAND_MAKE_LOWERCASE:
		case VOIDPAD_COMMAND_INCREMENT_INTEGER:
		case VOIDPAD_COMMAND_DECREMENT_INTEGER:
		{
			unsigned char *text;
			int len;
			
			text = alloc_selection(&len);
			if (text)
			{
				unsigned char *p;
				unsigned char *e;
				
				p = text;
				e = text + len;
					
				switch(command)
				{
					case VOIDPAD_COMMAND_MAKE_UPPERCASE:
					
						while(p != e)
						{
							if ((*p >= 'a') && (*p <= 'z'))
							{
								*p = *p + 'A' - 'a';
							}
							
							p++;
						}

						insert_text(text,len,1,1,1,0);
				
						break;

					case VOIDPAD_COMMAND_MAKE_LOWERCASE:
					
						while(p != e)
						{
							if ((*p >= 'A') && (*p <= 'Z'))
							{
								*p = *p + 'a' - 'A';
							}
							
							p++;
						}

						insert_text(text,len,1,1,1,0);
						
						break;

					case VOIDPAD_COMMAND_INCREMENT_INTEGER:
					case VOIDPAD_COMMAND_DECREMENT_INTEGER:				
					{
						int i;
						unsigned char buf[256];
						unsigned char *d;
						int sign;
						
						i = get_integer(p,len);
						
						switch(command)
						{
							case VOIDPAD_COMMAND_INCREMENT_INTEGER:
								i++;
								break;
								
							case VOIDPAD_COMMAND_DECREMENT_INTEGER:				
								i--;
								break;
						}
						
						d = buf + 256;
						*--d = 0;
						sign = 0;
						
						if (i < 0)
						{
							i = -i;
							sign = 1;
						}

						if (i)						
						{
							for(;;)
							{
								*--d = '0' + i % 10;
								
								i /= 10;
								if (!i) break;
							}
						}
						else
						{
							*--d = '0';
						}

						if (sign)
						{
							*--d = '-';
						}
							
						insert_text(d,string_length(d),1,1,1,0);
						
						break;
					}

				}

				mem_free(text);
			}

			break;
		}
			
		case VOIDPAD_COMMAND_VIEW_WHITE_SPACE:
			view_white_space = !view_white_space;
			InvalidateRect(edit_hwnd,0,FALSE);
			break;
			
		case VOIDPAD_COMMAND_INCREASE_LINE_INDENT_OR_INSERT_TAB:

			if (sel_y1 != sel_y2)
			{
				voidpad_indent(0);
			}
			else
			{
				// dont treat tabs as chars, allow tabs to break multiple undos.
				insert_text((const unsigned char *)"\t",1,1,0,1,0);
			}
			break;
			
		case VOIDPAD_COMMAND_DECREASE_LINE_INDENT_OR_INSERT_TAB:

			if (sel_y1 != sel_y2)
			{
				voidpad_indent(1);
			}
			else
			{
				// dont treat tabs as chars, allow tabs to break multiple undos.
				insert_text((const unsigned char *)"\t",1,1,0,1,0);
			}
			break;
			
		case VOIDPAD_COMMAND_INCREASE_LINE_INDENT:
			voidpad_indent(0);
			break;
			
		case VOIDPAD_COMMAND_DECREASE_LINE_INDENT:
			voidpad_indent(1);
			break;
			
		case VOIDPAD_COMMAND_CHAR:
		{
			// like insert_text, but insertion is a charactor (can be multi-byte)
			// multiple insert_chars are combined into one undo.
			int was_selection;
			int cursor_snap_colx;
				
			was_selection = is_selection();
			
			if (insert_mode == VOIDPAD_INSERT_MODE_OVERWRITE)
			{
				if (!was_selection)
				{
					// make sure we restore no selection.
					undo_add_selection();
							
					// will select nothing if the next char doesnt exist.
					set_col_sel(get_nearest_cursor_colx(),cursor_y,ch_to_col(col_to_ch(cursor_x,cursor_y) + 1,cursor_y),cursor_y);
				}
			}

			insert_text(data,size,1,0,1,0);
			
			undo_last->is_char = 1;

			cursor_snap_colx = get_nearest_cursor_colx();

			// there MUST be no selection for consecutive undos.
			if (!was_selection)				
			{
				// the previous undo MUST be a char.
				// and it's position MUST be before the current char.
				if (undo_last->prev)
				{
					if (undo_last->prev->is_char)
					{
						int cursor_x2;
						int cursor_y2;
						
						col_ln_add_len(undo_last_char_x,undo_last_char_y,1,&cursor_x2,&cursor_y2);
						
						if ((cursor_x2 == cursor_snap_colx) && (cursor_y2 == cursor_y))
						{
							undo_last->prev->is_more = 1;
						}
					}
				}
			}
			
			// save cursor pos.
			undo_last_char_x = cursor_snap_colx;
			undo_last_char_y = cursor_y;
		
			break;
		}
		
		case VOIDPAD_COMMAND_CURSOR_LEFT:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_LEFT:

			if ((command != VOIDPAD_COMMAND_CURSOR_EXTEND_LEFT) && (is_selection()))
			{
				set_cursor_col_position(sel_x1,sel_y1,1,1,1);
			}
			else
			{
				int newx;
				int newy;
				
				newx = get_cursor_chx();
				newy = cursor_y;

				prev_ch(&newx,&newy);

				set_cursor_ch_position(newx,newy,command != VOIDPAD_COMMAND_CURSOR_EXTEND_LEFT,1,1);
			}
			
			break;

		case VOIDPAD_COMMAND_CURSOR_WORD_LEFT:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_LEFT:
		{
			int newx;
			int newy;
			
			newx = get_cursor_chx();
			newy = cursor_y;

			prev_word(&newx,&newy);

			set_cursor_ch_position(newx,newy,command != VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_LEFT,1,1);
			
			break;
		}
			
		case VOIDPAD_COMMAND_CURSOR_RIGHT:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_RIGHT:

			if ((command != VOIDPAD_COMMAND_CURSOR_EXTEND_RIGHT) && (is_selection()))
			{
				set_cursor_col_position(sel_x2,sel_y2,1,1,1);
			}
			else
			{
				int newx;
				int newy;
				
				newx = get_cursor_chx();
				newy = cursor_y;

				next_ch(&newx,&newy);

				set_cursor_ch_position(newx,newy,command != VOIDPAD_COMMAND_CURSOR_EXTEND_RIGHT,1,1);
			}
			
			break;

		case VOIDPAD_COMMAND_CURSOR_WORD_RIGHT:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_RIGHT:
		{
			int newx;
			int newy;
			
			newx = get_cursor_chx();
			newy = cursor_y;

			next_word(&newx,&newy);

			set_cursor_ch_position(newx,newy,command != VOIDPAD_COMMAND_CURSOR_EXTEND_WORD_RIGHT,1,1);
			
			break;
		}

		case VOIDPAD_COMMAND_CURSOR_UP:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_UP:
		
			if ((command != VOIDPAD_COMMAND_CURSOR_EXTEND_UP) && (is_selection()))
			{
				set_cursor_col_position(cursor_x,sel_y1-1,command != VOIDPAD_COMMAND_CURSOR_EXTEND_UP,1,1);
			}
			else
			{
				set_cursor_col_position(cursor_x,cursor_y-1,command != VOIDPAD_COMMAND_CURSOR_EXTEND_UP,1,1);
			}
			
			break;
			
		case VOIDPAD_COMMAND_CURSOR_SCROLL_UP:

			set_vscroll(get_vscroll() - 1);

			// bring the cursor into view.
			if (cursor_y > get_vscroll() + get_vpage() - 1)
			{
				set_cursor_col_position(cursor_x,get_vscroll() + get_vpage() - 1,1,1,1);
			}
			
			break;

		case VOIDPAD_COMMAND_CURSOR_DOWN:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN:

			if ((command != VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN) && (is_selection()))
			{
				set_cursor_col_position(cursor_x,sel_y2+1,command != VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN,1,1);
			}
			else
			{
				set_cursor_col_position(cursor_x,cursor_y+1,command != VOIDPAD_COMMAND_CURSOR_EXTEND_DOWN,1,1);
			}
			
			break;
			
		case VOIDPAD_COMMAND_CURSOR_SCROLL_DOWN:

			set_vscroll(get_vscroll() + 1);
			
			// bring the cursor into view.
			if (cursor_y < get_vscroll())
			{
				set_cursor_col_position(cursor_x,get_vscroll(),1,1,1);
			}
			
			break;
			
		case VOIDPAD_COMMAND_CURSOR_PAGEUP:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEUP:
		{
			int vscroll;
								
			vscroll = get_vscroll();
			
			set_cursor_col_position(cursor_x,cursor_y - get_vpage(),command != VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEUP,1,0);
			
			set_vscroll(vscroll - get_vpage());		

			break;
		}
			
		case VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEUP:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEUP:
			set_cursor_col_position(cursor_x,get_vscroll(),command != VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEUP,1,0);		
			break;
			
		case VOIDPAD_COMMAND_CURSOR_PAGEDOWN:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEDOWN:
		{
			int vscroll;
								
			vscroll = get_vscroll();
			
			set_cursor_col_position(cursor_x,cursor_y + get_vpage(),command != VOIDPAD_COMMAND_CURSOR_EXTEND_PAGEDOWN,1,1);
			
			set_vscroll(vscroll + get_vpage());		

			break;
		}
			
		case VOIDPAD_COMMAND_CURSOR_SCROLL_PAGEDOWN:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEDOWN:
		{
			int page = get_vpage() - 1;
			if (page < 1) page = 1;
			set_cursor_col_position(cursor_x,get_vscroll() + page,command != VOIDPAD_COMMAND_CURSOR_EXTEND_SCROLL_PAGEDOWN,1,1);		
			break;			
		}
		
		case VOIDPAD_COMMAND_CURSOR_END:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_END:
			set_cursor_ch_position(get_line_chend(line_count-1),line_count-1,command != VOIDPAD_COMMAND_CURSOR_EXTEND_END,1,1);
			break;
		
		case VOIDPAD_COMMAND_CURSOR_LINE_END:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_END:
			set_cursor_ch_position(get_line_chend(cursor_y),cursor_y,command != VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_END,1,1);
			break;
		
		case VOIDPAD_COMMAND_CURSOR_HOME:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_HOME:
			set_cursor_col_position(0,0,command != VOIDPAD_COMMAND_CURSOR_EXTEND_HOME,1,1);
			break;
		
		case VOIDPAD_COMMAND_CURSOR_LINE_HOME:
		case VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_HOME:
			set_cursor_col_position(0,cursor_y,command != VOIDPAD_COMMAND_CURSOR_EXTEND_LINE_HOME,1,1);
			break;
			
		case VOIDPAD_COMMAND_DELETE_PREVIOUS_WORD:
		{
			int x;
			int y;
			int was_selection;
			
			was_selection = 0;
			
			if (is_selection())
			{
				// delete previous word and selection from cursor.
				// delete selection first.
				undo_add_selection();
				
				was_selection = 1;
			}

			x = get_cursor_chx();
			y = cursor_y;
			
			//#error doesnt work if pos is 0,0 and we have a selection.
			
			if ((prev_word(&x,&y)) || (was_selection))
			{
				if (!was_selection)
				{
					// there was no selection, so make sure we save no selection before setting it to something.
					undo_add_selection();
				}
			
				set_col_sel(get_nearest_cursor_colx(),cursor_y,ch_to_col(x,y),y);
				
				insert_text(0,0,1,0,1,0);
			}
			
			break;
		}
		
		case VOIDPAD_COMMAND_BACKSPACE:
			
			// backspace.
			if (is_selection())
			{
				insert_text(0,0,1,0,1,0);
			}
			else
			{
				int x;
				int y;
				
				x = get_cursor_chx();
				y = cursor_y;
			
				if (prev_ch(&x,&y))
				{
					// dont save no selection, make sure we push the no selection onto the undo stack.
					undo_add_selection();

					set_col_sel(get_nearest_cursor_colx(),cursor_y,ch_to_col(x,y),y);

					insert_text(0,0,1,0,1,0);
				}
			}
			
			break;
			
		case VOIDPAD_COMMAND_INSERT_NEWLINE:
		case VOIDPAD_COMMAND_INSERT_NEWLINE_ABOVE:
		case VOIDPAD_COMMAND_INSERT_NEWLINE_BELOW:
			insert_new_line(command != VOIDPAD_COMMAND_INSERT_NEWLINE,command == VOIDPAD_COMMAND_INSERT_NEWLINE_BELOW);
			break;
			
		case VOIDPAD_COMMAND_INSERT_DATE:
		{
			SYSTEMTIME st;
			wchar_t date_wbuf[MAX_PATH];
			wchar_t time_wbuf[MAX_PATH];
			
			GetLocalTime(&st);
			
			if (GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,time_wbuf,MAX_PATH))
			{
				unsigned char time_buf[MAX_PATH*4];

				WideCharToMultiByte(CP_UTF8,0,time_wbuf,-1,(char *)time_buf,MAX_PATH*4,0,0);
				
				if (GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,date_wbuf,MAX_PATH))
				{
					unsigned char date_buf[MAX_PATH*4];
					
					WideCharToMultiByte(CP_UTF8,0,date_wbuf,-1,(char *)date_buf,MAX_PATH*4,0,0);
					
					string_cat(time_buf," ");
					string_cat(time_buf,date_buf);

					insert_text(time_buf,string_length(time_buf),1,0,1,0);
				}
			}
			
			break;
		}
			
		case VOIDPAD_COMMAND_TOGGLE_INSERT_MODE:
			set_insert_mode(!insert_mode);
			break;
			
		case VOIDPAD_COMMAND_FIND_SET_SEARCH:

			if (search_string)
			{
				mem_free(search_string);
			}

			search_string = mem_alloc(size + 1);
			search_string_len = size;
			crt_copy_memory(search_string,data,size);
			search_string[size] = 0;

printf("searchstring %s\n",search_string);
	
			find_last_chx = -1;
			find_last_y = -1;

			find_start_chx = -1;
			find_start_direction = 0;
			
			break;
						
		case VOIDPAD_COMMAND_FIND_SET_REPLACE:

			if (replace_string)
			{
				mem_free(replace_string);
			}

			replace_string = mem_alloc(size + 1);
			replace_string_len = size;
			crt_copy_memory(replace_string,data,size);
			replace_string[size] = 0;

printf("replacestring %s\n",replace_string);
	
			find_last_chx = -1;
			find_last_y = -1;

			find_start_chx = -1;
			find_start_direction = 0;
			
			break;
						
		case VOIDPAD_COMMAND_FIND_CASE_ENABLE:
		case VOIDPAD_COMMAND_FIND_CASE_DISABLE:

			find_last_chx = -1;
			find_last_y = -1;

			find_start_chx = -1;
			find_start_direction = 0;

			find_case = (command == VOIDPAD_COMMAND_FIND_CASE_ENABLE);
			
			break;
			
		case VOIDPAD_COMMAND_FIND_WHOLEWORD_ENABLE:
		case VOIDPAD_COMMAND_FIND_WHOLEWORD_DISABLE:

			find_last_chx = -1;
			find_last_y = -1;

			find_start_chx = -1;
			find_start_direction = 0;

			find_wholeword = (command == VOIDPAD_COMMAND_FIND_WHOLEWORD_ENABLE);
			
			break;
		
		case VOIDPAD_COMMAND_INSERT_TEXT:
			insert_text(data,size,1,0,1,0);
			break;
	}
	
	return 0;
}

static void set_window_text(HWND hwnd,unsigned char *start,int len)
{
	wchar_t *wbuf;
	int wlen;
	
	wlen = MultiByteToWideChar(voidpad_cp,0,(char *)start,len,0,0);
	
	wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	MultiByteToWideChar(voidpad_cp,0,(char *)start,len,wbuf,wlen);
	
	wbuf[wlen] = 0;
	
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)wbuf);
	
	mem_free(wbuf);
}

static int search_compare(unsigned char *p,unsigned char *e)
{
	unsigned char *p2;
	
	p2 = search_string;
	
	for(;;)
	{
		if (!*p2)
		{
			return 1;
		}
	
		if (p >= e)
		{
			return 0;
		}

		if (find_case)
		{
			if (*p != *p2) return 0;
		}
		else
		{
			if (voidpad_tolower(*p) != voidpad_tolower(*p2)) return 0;
		}
		
		p++;
		p2++;
	}
}

static int search_reverse_compare(unsigned char *p,unsigned char *start)
{
	unsigned char *p2;
	
	p2 = search_string + search_string_len;
	
	for(;;)
	{
		p--;
		p2--;
		
		if (p2 < search_string)
		{
			return 1;
		}
	
		if (p < start)
		{
			return 0;
		}

		if (find_case)
		{
			if (*p != *p2) return 0;
		}
		else
		{
			if (voidpad_tolower(*p) != voidpad_tolower(*p2)) return 0;
		}
	}
}

// does not display messages if is_replace_all is set.
// if is_replace is set, skips the selection on non-first searches.
// direction: 1 = forward
// direction: -1 = backward.
static int search(int direction,int is_replace,int is_replace_all)
{
	int chx;
	int y;
	unsigned char *p;
	unsigned char *e;
	int skip_first;

	if (!search_string) 
	{
		return 0;
	}

	if (find_start_direction)
	{
		if (direction != find_start_direction)
		{
		printf("direction changed\n");
			find_last_chx = -1;
			find_last_y = -1;

			find_start_chx = -1;
			find_start_direction = 0;
		}
	}

	// execute search
	if (find_last_chx == -1)
	{
		if (find_start_chx != -1)
		{
			chx = find_start_chx;
			y = find_start_y;
		}
		else
		if (is_selection())
		{
			if (direction > 0)
			{
				chx = col_to_ch(sel_x2,sel_y2);
				y = sel_y2;
			}
			else
			{
				chx = col_to_ch(sel_x1,sel_y1);
				y = sel_y1;
			}
		}
		else
		{
			chx = get_cursor_chx();
			y = cursor_y;
		}

		skip_first = 0;
	}
	else
	{
		y = find_last_y;
		chx = find_last_chx;
	
		skip_first = find_found_count;
	}

	if (find_start_chx == -1)
	{
		find_start_chx = chx;
		find_start_y = y;
		find_start_direction = direction;
	}
	
	printf("chx %d, startchx %d, findlooped %d, skip first %d\n",chx,find_start_chx,0,skip_first);

	// try this line of text.
	p = line_list[y]->text + chx;
	e = line_list[y]->text + line_list[y]->len;

	for(;;)
	{
		if (skip_first)
		{
			skip_first = 0;
			
			if (is_replace)
			{
				// skip selection.
				// if there is any, if there isnt, it means we replaced the search with nothing.
				if (is_selection())
				{
					if (direction > 0)
					{
						p = line_list[y]->text + col_to_ch(sel_x2,sel_y2);
					}
					else
					{
						p = line_list[y]->text + col_to_ch(sel_x1,sel_y1);
					}

					goto skip_replace_text;
				}
				else
				{
					goto skip_empty_replace_text;
				}
			}
		}
		else
		{
			if (direction > 0)			
			{
				if (search_compare(p,e))
				{
					if ((!find_wholeword) || (is_word(line_list[y]->text,p,e,search_string_len)))
					{
		printf("found: looped=%d, startx=%d, starty=%d, x=%d, y=%d\n",0,find_start_chx,find_start_y,p - line_list[y]->text,y);
						
						if (find_last_chx == -1)
						{
							find_found_count = 1;
						}
						else
						{
							find_found_count++;
						}
						
						find_last_chx = p - line_list[y]->text;
						find_last_y = y;

						set_cursor_ch_position(p-line_list[y]->text,y,1,0,0);
						set_cursor_ch_position(p-line_list[y]->text+search_string_len,y,0,0,!is_replace);

						return 1;
					}
				}
			}
			else
			{
				if (search_reverse_compare(p,line_list[y]->text))
				{
					if ((!find_wholeword) || (is_word(line_list[y]->text,p-search_string_len,e,search_string_len)))
					{
		printf("found: looped=%d, startx=%d, starty=%d, x=%d, y=%d\n",0,find_start_chx,find_start_y,p - line_list[y]->text,y);
		
						if (find_last_chx == -1)
						{
							find_found_count = 1;
						}
						else
						{
							find_found_count++;
						}
						
						find_last_chx = p - line_list[y]->text;
						find_last_y = y;

						set_cursor_ch_position(p-line_list[y]->text-search_string_len,y,1,0,0);
						set_cursor_ch_position(p-line_list[y]->text,y,0,0,!is_replace);

						return 1;
					}
				}
			}
		}

		if (direction > 0)
		{
			p++;
			
			if (p > e)
			{
				y++;	
				if (y >= line_count)
				{
			printf("find looped\n");
			
					y = 0;
				}

				p = line_list[y]->text;
				e = line_list[y]->text + line_list[y]->len;
			}
		}

		if (direction <= 0)
		{
			p--;
			
			if (p < line_list[y]->text)
			{
				y--;	
				if (y < 0)
				{
			printf("find looped\n");

					y = line_count - 1;
				}

				p = line_list[y]->text + line_list[y]->len;
				e = line_list[y]->text + line_list[y]->len;
			}
		}
		
skip_replace_text:		

		if ((y == find_start_y) && (p - line_list[y]->text == find_start_chx))
		{
			HWND last_focus;

printf("find: reached starting point: %d %d >= %d %d\n",p - line_list[y]->text,y,find_start_chx,find_start_y);
			
			last_focus = GetFocus();
			
			if (!is_replace_all)
			{
				if (find_found_count == 0)
				{
					wchar_t wbuf[MAX_PATH];
		
					wstring_copy(wbuf,MAX_PATH,L"The following text was not found:\n\n");
					wstring_cat_voidpad_cp(wbuf,MAX_PATH,search_string);
					wstring_cat(wbuf,MAX_PATH,L"\n");
		
					MessageBox(find_hwnd ? find_hwnd : voidpad_hwnd,wbuf,L"voidPad",MB_OK|MB_ICONINFORMATION);
				}
				else
				{
					MessageBox(find_hwnd ? find_hwnd : voidpad_hwnd,L"Find reached the starting point of the search.",L"voidPad",MB_OK|MB_ICONINFORMATION);
				}
			}
			
			SetFocus(last_focus);

			find_found_count = 0;
			find_last_chx = -1;
			
			return 0;
		}

skip_empty_replace_text:
		;
	}
	
}


static void write_ini_int(HANDLE h,const utf8_t *key,int value)
{
	unsigned char buf[MAX_PATH];
	wchar_t wbuf[MAX_PATH];
	
	wstring_format_number(wbuf,value);
	
	WideCharToMultiByte(CP_UTF8,0,wbuf,-1,(char *)buf,MAX_PATH,0,0);
	
	file_write_utf8(h,(const unsigned char *)key);
	file_write_utf8(h,(const unsigned char *)"=");
	file_write_utf8(h,buf);
	file_write_utf8(h,(const unsigned char *)"\r\n");
}

static void write_ini_string(HANDLE h,const utf8_t *key,wchar_t *value)
{
	utf8_t *buf;

	buf = utf8_alloc_wchar(value);
	
	file_write_utf8(h,(const unsigned char *)key);
	file_write_utf8(h,(const unsigned char *)"=");
	file_write_utf8(h,buf);
	file_write_utf8(h,(const unsigned char *)"\r\n");
	
	mem_free(buf);
}

/*
static void file_write_ANSI(HANDLE h,const unsigned char *s,int escape)
{
	wchar_t wbuf[MAX_PATH];
	int wlen;
	unsigned char utf8[MAX_PATH];
	int len;
	DWORD num_written;
	unsigned char escape_ch;
	
	while(*s)
	{
		escape_ch = 0;
		
		if (escape)
		{
			switch(*s)
			{
				case '"': escape_ch = '"'; break;
				case '\t': escape_ch = 't'; break;
				case '\\': escape_ch = '\\'; break;
				case '\n': escape_ch = 'n'; break;
				case '\r': escape_ch = 'r'; break;
			}
		}

		if (escape_ch)
		{
			WriteFile(h,"\\",1,&num_written,0);
			WriteFile(h,&escape_ch,1,&num_written,0);
		}
		else
		{
			// convert to wchar.
			wlen = MultiByteToWideChar(voidpad_cp,0,(char *)s,1,wbuf,MAX_PATH);
		
			// convert back to UTF8
			len = WideCharToMultiByte(CP_UTF8,0,wbuf,wlen,(char *)utf8,MAX_PATH,0,0);
			WriteFile(h,utf8,len,&num_written,0);
		}

		s++;
	}
}
*/

static void file_write_utf8(HANDLE h,const unsigned char *s)
{
	DWORD num_written;
	
	WriteFile(h,s,string_length(s),&num_written,0);
}


/*
static void voidpad_reg_write_int(HKEY hkey,wchar_t *name,int value)
{
	DWORD dword_value;
	
	dword_value = (DWORD)value;
	
	RegSetValueEx(hkey,name,0,REG_DWORD,(BYTE *)&dword_value,sizeof(DWORD));
}

static void voidpad_reg_write_string(HKEY hkey,wchar_t *name,wchar_t *value)
{
	RegSetValueEx(hkey,name,0,REG_SZ,(BYTE *)value,(wstring_length(value) + 1) * sizeof(wchar_t));
}
*/
static int wstring_get_escape_char(wchar_t c)
{
	switch(c)
	{
		case '"': return '"';
		case '\t': return 't';
		case '\\': return '\\';
		case '\n': return 'n';
		case '\r': return 'r';
	}
	
	return 0;
}

static wchar_t *wstring_copy_escaped_wstring(wchar_t *buf,const wchar_t *text)
{
	wchar_t *d;
	const wchar_t *p;
	
	d = buf;
	p = text;
	
	while(*p)
	{
		int escape_c;
		
		escape_c = wstring_get_escape_char(*p);
		
		if (escape_c)
		{
			if (buf)
			{
				*d++ = '\\';
				*d++ = escape_c;
			}
			else
			{
				d++; // '\\';
				d++;
			}
		}
		else
		{
			if (buf)
			{
				*d++ = *p;
			}
			else
			{
				d++;
			}
		}
		
		p++;
	}
	
	if (buf)
	{
		*d = 0;
	}
	
	return d;
}

static void save_settings(void)
{
	HANDLE h;
	wchar_t tempname[MAX_PATH];
	wchar_t filename[MAX_PATH];
	
	GetModuleFileNameW(0,tempname,MAX_PATH);
	PathRemoveFileSpecW(tempname);
	PathAppendW(tempname,L"voidpad.ini.tmp");
	
	GetModuleFileNameW(0,filename,MAX_PATH);
	PathRemoveFileSpecW(filename);
	PathAppendW(filename,L"voidpad.ini");

	h = CreateFileW(tempname,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	
printf("save %S %d %d\n",tempname,h,GetLastError());

	if (h != INVALID_HANDLE_VALUE)
	{
		file_write_utf8(h,(const utf8_t *)"[voidpad]\r\n");
		
		write_ini_int(h,(const utf8_t *)"x",window_x);
		write_ini_int(h,(const utf8_t *)"y",window_y);
		write_ini_int(h,(const utf8_t *)"wide",window_wide);
		write_ini_int(h,(const utf8_t *)"high",window_high);
		
		write_ini_int(h,(const utf8_t *)"background_r",background_r);
		write_ini_int(h,(const utf8_t *)"background_g",background_g);
		write_ini_int(h,(const utf8_t *)"background_b",background_b);
		write_ini_int(h,(const utf8_t *)"foreground_r",foreground_r);
		write_ini_int(h,(const utf8_t *)"foreground_g",foreground_g);
		write_ini_int(h,(const utf8_t *)"foreground_b",foreground_b);
		
		write_ini_int(h,(const utf8_t *)"selected_background_r",selected_background_r);
		write_ini_int(h,(const utf8_t *)"selected_background_g",selected_background_g);
		write_ini_int(h,(const utf8_t *)"selected_background_b",selected_background_b);
		write_ini_int(h,(const utf8_t *)"selected_foreground_r",selected_foreground_r);
		write_ini_int(h,(const utf8_t *)"selected_foreground_g",selected_foreground_g);
		write_ini_int(h,(const utf8_t *)"selected_foreground_b",selected_foreground_b);
		
		write_ini_int(h,(const utf8_t *)"tab_size",voidpad_tab_size);
		write_ini_int(h,(const utf8_t *)"insert_mode",insert_mode);
		write_ini_int(h,(const utf8_t *)"clipboard_ring_size",voidpad_clipboard_ring_size);
		
		write_ini_int(h,(const utf8_t *)"view_white_space",view_white_space);
		write_ini_int(h,(const utf8_t *)"select_all_bring_into_view",voidpad_select_all_bring_into_view);
		
		write_ini_string(h,(const utf8_t *)"font_bitmap_filename",font_bitmap_filename);
		write_ini_int(h,(const utf8_t *)"font_cp",voidpad_cp);

		{
			find_t *f;
			int size;
			wchar_t *wbuf;
			wchar_t *d;
			
			size = 1;

			f = find_start;
			while(f)
			{
				size++; // '"'
					
				{
					wchar_t *wfindtext;
				
					wfindtext = wstring_alloc_voidpad_cp(f->text);
					size += (int)wstring_copy_escaped_wstring(0,wfindtext);
					mem_free(wfindtext);
				}
				
				size++; // '"'

				if (f->next)
				{
					size++;	// ','
				}
				
				f = f->next;
			}
			
			wbuf = mem_alloc(size * sizeof(wchar_t));
			d = wbuf;
			
			f = find_start;
			while(f)
			{
				*d++ = '"';
				
				{
					wchar_t *wfindtext;
				
					wfindtext = wstring_alloc_voidpad_cp(f->text);
					d = wstring_copy_escaped_wstring(d,wfindtext);
					mem_free(wfindtext);
				}
				
				*d++ = '"';
				
				if (f->next)
				{
					*d++ = ',';
				}
				
				f = f->next;
			}

			*d = 0;
			
			write_ini_string(h,(const utf8_t *)"find_history",wbuf);
			
			mem_free(wbuf);
		}	

		{
			replace_t *f;
			int size;
			wchar_t *wbuf;
			wchar_t *d;
			
			size = 1;

			f = replace_start;
			while(f)
			{
				size++; // '"'
					
				{
					wchar_t *wreplacetext;
				
					wreplacetext = wstring_alloc_voidpad_cp(f->text);
					size += (int)wstring_copy_escaped_wstring(0,wreplacetext);
					mem_free(wreplacetext);
				}
				
				size++; // '"'

				if (f->next)
				{
					size++;	// ','
				}
				
				f = f->next;
			}
			
			wbuf = mem_alloc(size * sizeof(wchar_t));
			d = wbuf;
			
			f = replace_start;
			while(f)
			{
				*d++ = '"';
				
				{
					wchar_t *wreplacetext;
				
					wreplacetext = wstring_alloc_voidpad_cp(f->text);
					d = wstring_copy_escaped_wstring(d,wreplacetext);
					mem_free(wreplacetext);
				}
				
				*d++ = '"';
				
				if (f->next)
				{
					*d++ = ',';
				}
				
				f = f->next;
			}

			*d++ = 0;
			
			write_ini_string(h,(const utf8_t *)"replace_history",wbuf);
			
			mem_free(wbuf);
		}	

		CloseHandle(h);

		if (!MoveFileExW(tempname,filename,MOVEFILE_REPLACE_EXISTING))
		{
			if (CopyFile(tempname,filename,FALSE))
			{
				DeleteFile(tempname);
			}
		}
	}
}

static void voidpad_macro_play(void)
{
	macro_t *m;
	
	if (!voidpad_macro_is_recording)
	{
		m = voidpad_macro_start;
		
		while(m)
		{
			voidpad_command_data(m->command,m->data,m->size);
			
			m = m->next;
		}
	}
}

static void voidpad_macro_record(void)
{
	if (voidpad_macro_is_recording)
	{
		SendMessage(status_hwnd,SB_SETTEXTW,0|SBT_NOBORDERS,(LPARAM)L"");
		
		voidpad_macro_is_recording = 0;
	}
	else
	{
		voidpad_macro_clear();
		
		SendMessage(status_hwnd,SB_SETTEXTW,0|SBT_NOBORDERS,(LPARAM)L"Recording...");	
		
		voidpad_macro_is_recording = 1;
	}
}

static void voidpad_macro_add(int command,const void *data,int size)
{
	macro_t *m;
	
	m = mem_alloc(sizeof(macro_t));
	
	m->command = command;

	if (size)
	{
		m->data = mem_alloc(size);
		crt_copy_memory(m->data,data,size);
		m->size = size;
	}
	else
	{
		m->data = 0;
		m->size = 0;
	}
	
	if (voidpad_macro_start)
	{
		voidpad_macro_last->next = m;
	}
	else
	{
		voidpad_macro_start = m;
	}
	
	m->next = 0;
	voidpad_macro_last = m;
}

static void voidpad_macro_clear(void)
{
	macro_t *m;
	macro_t *next_m;
	
	m = voidpad_macro_start;
	
	while(m)
	{
		next_m = m->next;
		
		if (m->data)
		{
			mem_free(m->data);
		}
		
		mem_free(m);
		
		m = next_m;
	}
	
	voidpad_macro_start = 0;
	voidpad_macro_last = 0;
}

static void voidpad_clipboard_ring_remove(voidpad_clipboard_ring_t *ccr)
{
	// remove
	if (ccr == voidpad_clipboard_ring_start)
	{
		voidpad_clipboard_ring_start = ccr->next;
	}
	else
	{
		ccr->prev->next = ccr->next;
	}
	
	if (ccr == voidpad_clipboard_ring_last)
	{
		voidpad_clipboard_ring_last = ccr->prev;
	}
	else
	{
		ccr->next->prev = ccr->prev;
	}
	
	voidpad_clipboard_ring_count--;
}

static void voidpad_clipboard_ring_delete(voidpad_clipboard_ring_t *ccr)
{
	voidpad_clipboard_ring_remove(ccr);
	
	voidpad_clipboard_ring_free(ccr);
}

static void voidpad_clipboard_ring_insert_at_end(voidpad_clipboard_ring_t *ccr)
{
	if (voidpad_clipboard_ring_start)
	{
		voidpad_clipboard_ring_last->next = ccr;
		ccr->prev = voidpad_clipboard_ring_last;
	}
	else
	{
		voidpad_clipboard_ring_start = ccr;
		ccr->prev = 0;
	}
	
	voidpad_clipboard_ring_last = ccr;
	ccr->next = 0;
	voidpad_clipboard_ring_count++;
}

static void voidpad_clipboard_ring_insert_at_start(voidpad_clipboard_ring_t *ccr)
{
	if (voidpad_clipboard_ring_last)
	{
		voidpad_clipboard_ring_start->prev = ccr;
		ccr->next = voidpad_clipboard_ring_start;
	}
	else
	{
		voidpad_clipboard_ring_last = ccr;
		ccr->next = 0;
	}
	
	voidpad_clipboard_ring_start = ccr;
	ccr->prev = 0;
	voidpad_clipboard_ring_count++;
}

static void voidpad_clipboard_ring_add(const unsigned char *text)
{
	voidpad_clipboard_ring_t *ccr;
	int len;
	
	// remove head 
	if (voidpad_clipboard_ring_count + 1 > voidpad_clipboard_ring_size)
	{
		voidpad_clipboard_ring_delete(voidpad_clipboard_ring_start);
	}
	
	ccr = mem_alloc(sizeof(voidpad_clipboard_ring_t));
	
	len = string_length(text);
	
	ccr->text = mem_alloc(len + 1);
	ccr->len = len;
	
	crt_copy_memory(ccr->text,text,len + 1);
	
	voidpad_clipboard_ring_insert_at_end(ccr);
}

static void voidpad_clipboard_ring_free(voidpad_clipboard_ring_t *cr)
{
	// free.
	mem_free(cr->text);
	mem_free(cr);
}

static void voidpad_clipboard_ring_clear(void)
{	
	voidpad_clipboard_ring_t *cr;
	
	cr = voidpad_clipboard_ring_start;
	while(cr)
	{
		voidpad_clipboard_ring_t *next_cr;
		
		next_cr = cr->next;
		
		voidpad_clipboard_ring_free(cr);
	
		cr = next_cr;
	}
	
	voidpad_clipboard_ring_start = 0;
	voidpad_clipboard_ring_last = 0;
	voidpad_clipboard_ring_count = 0;
}

static int is_selected_pixel(int x,int y)
{
	y /= font_high;

	if (y < sel_y1)
	{
		return 0;
	}

	if (y > sel_y2)
	{
		return 0;
	}

	if (y == sel_y1)
	{
		if (x < sel_x1 * font_wide) 
		{
			return 0;
		}
	}

	if (y == sel_y2)
	{
		if (x > sel_x2 * font_wide) 
		{
			return 0;
		}
	}

	return 1;
}

// you should set a new selection after calling this.
// you MUST call insert_text after this, this will add the last undo.
static void undo_add_selection(void)
{
	// save the current seletcion
	// let the undo know there will be more
	undo_add(0,1,1);
	
	// the selection MUST be deleted, otherwise, it is added to the next undo
	// when we execute the undo, it would restore the selection twice.
	delete_selection(0);
}

// you should set a new selection after calling this.
// you MUST call insert_text after this, this will add the last undo.
static void undo_clear_selection(void)
{
	// save the current seletcion
	// let the undo know there will be more
	undo_add(0,1,0);
	
	set_col_sel(0,0,0,0);
}

static void undo_add(int len,int is_more,int selection_deleted)
{
	undo_t *u;
	
	u = mem_alloc(sizeof(undo_t));

	u->inserted_len = len;
	
	if ((is_selection()) && (selection_deleted))
	{
		u->deleted_text = alloc_selection(&u->deleted_len);
		u->del_x = sel_x1;
		u->del_y = sel_y1;
	
		u->inserted_x = sel_x1;
		u->inserted_y = sel_y1;	
	}
	else
	{
		u->deleted_text = 0;
		u->deleted_len = 0;
		u->del_x = get_nearest_cursor_colx();
		u->del_y = cursor_y;
	
		u->inserted_x = get_nearest_cursor_colx();
		u->inserted_y = cursor_y;
	}

	u->mark_x = mark_x;
	u->mark_y = mark_y;
	
	u->cursor_x = cursor_x;
	u->cursor_y = cursor_y;

	u->sel_x1 = sel_x1;
	u->sel_y1 = sel_y1;
	u->sel_x2 = sel_x2;
	u->sel_y2 = sel_y2;
	
	u->is_more = is_more;
	u->is_char = 0;
	u->is_clipboard_ring = 0;
	
	u->is_saved = 0;
	
	if (undo_start)
	{
		undo_last->next = u;
		u->prev = undo_last;
	}
	else
	{
		undo_start = u;
		u->prev = 0;
	}
	
	undo_last = u;
	u->next = 0;
	
	// clear redos.
	redo_clear();
}

static void undo_delete(undo_t *u)
{
	if (u == undo_start)
	{
		undo_start = u->next;
	}
	else
	{
		u->prev->next = u->next;
	}
	
	if (u == undo_last)
	{
		undo_last = u->prev;
	}
	else
	{
		u->next->prev = u->prev;
	}
	
	undo_free(u);
}

static void redo_delete(redo_t *u)
{
	if (u == redo_start)
	{
		redo_start = u->next;
	}
	else
	{
		u->prev->next = u->next;
	}
	
	if (u == redo_last)
	{
		redo_last = u->prev;
	}
	else
	{
		u->next->prev = u->prev;
	}
	
	redo_free(u);
}

static void undo_free(undo_t *u)
{
	if (u->deleted_text)
	{
		mem_free(u->deleted_text);
	}
	
	mem_free(u);
}

static void redo_free(redo_t *u)
{
	if (u->deleted_text)
	{
		mem_free(u->deleted_text);
	}
	
	mem_free(u);
}

static void redo(void)
{
	if (redo_last)
	{
		int undo_more;
		
		undo_more = 0;
		
		for(;;)
		{
			int insert_end_x;
			int insert_end_y;
			undo_t *undo;
			
			if (undo_more)
			{
				undo_last->is_more = 1;
			}
			
			// remove inserted text.
			col_ln_add_len(redo_last->inserted_x,redo_last->inserted_y,redo_last->inserted_len,&insert_end_x,&insert_end_y);

			// save text at del_x1,del_y1 to del_x2,del_y2
			undo = mem_alloc(sizeof(undo_t));
			
			undo->is_char = 0;
			undo->is_more = 0;
			undo->is_clipboard_ring = 0;
			
			undo->inserted_len = redo_last->deleted_len;
			undo->inserted_x = redo_last->del_x;
			undo->inserted_y = redo_last->del_y;

			undo->mark_x = mark_x;
			undo->mark_y = mark_y;
			
			undo->cursor_x = cursor_x;
			undo->cursor_y = cursor_y;
			
			undo->sel_x1 = sel_x1;
			undo->sel_y1 = sel_y1;
			undo->sel_x2 = sel_x2;
			undo->sel_y2 = sel_y2;

			// select the text we added..
			// this will also clear any current selection.
			set_col_sel(redo_last->inserted_x,redo_last->inserted_y,insert_end_x,insert_end_y);

			undo->del_x = redo_last->inserted_x;
			undo->del_y = redo_last->inserted_y;
			undo->deleted_text = alloc_selection(&undo->deleted_len);
			
			undo->is_saved = redo_last->is_saved;
			
			delete_selection(0);

			if (undo_start)
			{
				undo_last->next = undo;
				undo->prev = undo_last;
			}
			else
			{
				undo_start = undo;
				undo->prev = 0;
			}
			
			undo_last = undo;
			undo->next = 0;
			
			// insert deleted text.
			set_cursor_col_position(redo_last->del_x,redo_last->del_y,1,1,0);
			insert_text(redo_last->deleted_text,redo_last->deleted_len,0,0,0,0);
			
			// restore selection, mark and cursor.
//VOID: don't move cursor after redo, use if we want to mark the current location by making a change, going somewhere else, then undo/redo to restore the position.

			/*
			set_cursor_col_position(redo_last->cursor_x,redo_last->cursor_y,0,1,0);

			mark_x = redo_last->mark_x;
			mark_y = redo_last->mark_y;
			
			set_col_sel(redo_last->sel_x1,redo_last->sel_y1,redo_last->sel_x2,redo_last->sel_y2);
			*/
			
			redo_delete(redo_last);
			
			if (!redo_last)
			{
				break;
			}
			
			if (!redo_last->is_more)
			{
				break;
			}
			
			undo_more = 1;
		}

		voidpad_bring_cursor_into_view();
		update_ch_status();
		update_col_status();
		update_ln_status();
	}
	
	if (undo_last)
	{
		if (undo_last->is_saved)
		{
			if (is_dirty)
			{
				is_dirty = 0;
				
				update_title();
			}
		}
	}
}

static void undo(void)
{
	if (undo_last)
	{
		int redo_more;
		
		redo_more = 0;
		
		for(;;)
		{
			int insert_end_x;
			int insert_end_y;
			redo_t *redo;
			
			if (redo_more)
			{
				redo_last->is_more = 1;
			}

			col_ln_add_len(undo_last->inserted_x,undo_last->inserted_y,undo_last->inserted_len,&insert_end_x,&insert_end_y);

			// save text at del_x1,del_y1 to del_x2,del_y2
			redo = mem_alloc(sizeof(redo_t));
			
			redo->is_more = 0;
			
			redo->inserted_len = undo_last->deleted_len;
			redo->inserted_x = undo_last->del_x;
			redo->inserted_y = undo_last->del_y;

			redo->mark_x = mark_x;
			redo->mark_y = mark_y;
			
			redo->cursor_x = cursor_x;
			redo->cursor_y = cursor_y;
			
			redo->sel_x1 = sel_x1;
			redo->sel_y1 = sel_y1;
			redo->sel_x2 = sel_x2;
			redo->sel_y2 = sel_y2;

			// select the text we added..
			// this will also clear any current selection.
			set_col_sel(undo_last->inserted_x,undo_last->inserted_y,insert_end_x,insert_end_y);

			redo->del_x = undo_last->inserted_x;
			redo->del_y = undo_last->inserted_y;
			redo->deleted_text = alloc_selection(&redo->deleted_len);

			redo->is_saved = undo_last->is_saved;
			
			delete_selection(0);

			if (redo_start)
			{
				redo_last->next = redo;
				redo->prev = redo_last;
			}
			else
			{
				redo_start = redo;
				redo->prev = 0;
			}
			
			redo_last = redo;
			redo->next = 0;
				
			set_cursor_col_position(undo_last->del_x,undo_last->del_y,1,1,0);
			insert_text(undo_last->deleted_text,undo_last->deleted_len,0,0,0,0);
			
			// restore selection, mark and cursor.
			set_cursor_col_position(undo_last->cursor_x,undo_last->cursor_y,0,1,0);

			mark_x = undo_last->mark_x;
			mark_y = undo_last->mark_y;
			
			set_col_sel(undo_last->sel_x1,undo_last->sel_y1,undo_last->sel_x2,undo_last->sel_y2);

			undo_delete(undo_last);
			
			// always do more if there is more undos
			if (!undo_last)
			{
				break;
			}

			if (!undo_last->is_more)
			{
				break;
			}
			
			redo_more = 1;
		}

		voidpad_bring_cursor_into_view();
		update_ch_status();
		update_col_status();
		update_ln_status();
	}
	else
	{
		MessageBeep((UINT)-1);
	}
	
	if (undo_last)
	{
		if (undo_last->is_saved)
		{
			if (is_dirty)
			{
				is_dirty = 0;
				
				update_title();
			}
		}
	}
	else
	{
		if (!is_undo_saved)
		{
			if (is_dirty)
			{
				is_dirty = 0;
				
				update_title();
			}
		}
	}
		
///////////////////
/*
{
	redo_t *redo;
printf("--------------------\n");
	redo = redo_last;
	
	while(redo)
	{
		int len;
		char *p;
		len = redo->deleted_len;
		p = (char *)redo->deleted_text;
		printf("del %d %d\n",redo->del_x,redo->del_y);
		printf("deltext ");
		while(len--)
		{
			printf("%c",*p++);
		}
		printf("\n");
		printf("sel %d %d %d %d\n",redo->sel_x1,redo->sel_y1,redo->sel_x2,redo->sel_y2);
		printf("mark %d %d\n",redo->mark_x,redo->mark_y);
		printf("cursor %d %d\n",redo->cursor_x,redo->cursor_y);
		printf("ismore %d\n",redo->is_more);
	
		redo = redo->prev;
	}

printf("====================\n");
	
}
*/
///////////////////						
}

static int get_selection_len(void)
{
	int y;
	int left;
	int right;
	int len;
	
	len = 0;
	
	y = sel_y1;
	while(y <= sel_y2)
	{
		if (y == sel_y1)
		{
			// use start
			left = col_to_ch(sel_x1,y);
		}
		else
		{
			left = 0;
		}
		
		if (y == sel_y2)
		{
			right = col_to_ch(sel_x2,y);
		}
		else
		{
			right = line_list[y]->len;
		}

		len += right - left;
		
		y++;
	}	
	
	return len;
}

static unsigned char *alloc_selection(int *plen)
{
	int len;
	
	len = get_selection_len();
	
	*plen = len;
	
	if (len)
	{
		unsigned char *buf;
		unsigned char *d;
		int y;
		int left;
		int right;

		buf = mem_alloc(len + 1);
		d = buf;

		y = sel_y1;
		while(y <= sel_y2)
		{
			if (y == sel_y1)
			{
				// use start
				left = col_to_ch(sel_x1,y);
			}
			else
			{
				left = 0;
			}
			
			if (y == sel_y2)
			{
				right = col_to_ch(sel_x2,y);
			}
			else
			{
				right = line_list[y]->len;
			}

			crt_copy_memory(d,&line_list[y]->text[left],right - left);
			d += right - left;
			
			y++;
		}	
		
		*d = 0;		

		return buf;
	}
	
	return 0;
}

// clips bottom end.
static void col_ln_add_len(int x,int y,int len,int *out_x,int *out_y)
{
	int left;
	
	left = col_to_ch(x,y);
	
	while(len)
	{
		if (left + len < line_list[y]->len)
		{
			*out_x = ch_to_col(left + len,y);
			*out_y = y;
			
			return;
		}
		else
		{
			len -= line_list[y]->len - left;

			y++;
			left = 0;
			
			if (y == line_count)
			{
				*out_x = get_line_colend(line_count-1);
				*out_y = line_count-1;
				
				return;
			}
		}
	}
	
	*out_x = ch_to_col(left,y);
	*out_y = y;
}

static void initmenupopup(HMENU menu,int id)
{
	HMENU mainmenu;
	int i;
	
printf("init menu %d\n",id);
	
	// get the main menu and enumerate the sub menus.
	// if the menu is one of these submenus,then rebuild it..
	mainmenu = GetMenu(voidpad_hwnd);
	
	if (mainmenu)
	{
		for (i=0;i<GetMenuItemCount(mainmenu);i++)
		{
			if (menu == GetSubMenu(mainmenu,i))
			{
				goto found;
			}
		}
		
		// not found;
		return ;
	
found:	

		switch(id)
		{
			case 0: // file

				os_remove_all_menu_items(menu);
				
				AppendMenuW(menu,MF_STRING,ID_FILE_NEW,L"&New\tCtrl+N");
				AppendMenuW(menu,MF_STRING,ID_FILE_OPEN,L"&Open...\tCtrl+O");
				AppendMenuW(menu,MF_STRING,ID_FILE_SAVE,L"&Save\tCtrl+S");
				AppendMenuW(menu,MF_STRING,ID_FILE_SAVE_DIRECT,L"Save Direct");
				AppendMenuW(menu,MF_STRING,ID_FILE_SAVE_AS,L"Save &As...");
				AppendMenuW(menu,MF_SEPARATOR,0,0);
				AppendMenuW(menu,MF_STRING,ID_FILE_EXIT,L"E&xit");

				break;

			case 1: // edit
				
				os_remove_all_menu_items(menu);
				
				AppendMenuW(menu,MF_STRING | (undo_last ? 0 : (MF_GRAYED|MF_DISABLED)),ID_EDIT_UNDO,L"&Undo\tCtrl+Z");
				AppendMenuW(menu,MF_STRING | (redo_last ? 0 : (MF_GRAYED|MF_DISABLED)),ID_EDIT_REDO,L"&Redo\tCtrl+Shift+Z");
				AppendMenuW(menu,MF_SEPARATOR,0,0);
				AppendMenuW(menu,MF_STRING,ID_EDIT_CUT,L"Cu&t\tCtrl+X");
				AppendMenuW(menu,MF_STRING,ID_EDIT_COPY,L"&Copy\tCtrl+C");
				AppendMenuW(menu,MF_STRING | (voidpad_can_paste() ? 0 : (MF_GRAYED|MF_DISABLED)),ID_EDIT_PASTE,L"&Paste\tCtrl+V");
				AppendMenuW(menu,MF_STRING,ID_EDIT_CYCLE_CLIPBOARD_RING,L"C&ycle Clipboard Ring\tCtrl+Shift+Ins");
				AppendMenuW(menu,MF_STRING,ID_EDIT_DELETE,L"&Delete\tDel");
				AppendMenuW(menu,MF_STRING,ID_EDIT_DELETE_LINE,L"&Delete Line\tCtrl+Y");
				AppendMenuW(menu,MF_SEPARATOR,0,0);
				AppendMenuW(menu,MF_STRING,ID_EDIT_SELECT_ALL,L"Select &All\tCtrl+A");
				AppendMenuW(menu,MF_SEPARATOR,0,0);
				AppendMenuW(menu,MF_STRING,ID_EDIT_FIND,L"&Find\tCtrl+F");
				AppendMenuW(menu,MF_STRING,ID_EDIT_FIND_NEXT,L"Find &Next\tF3");
				AppendMenuW(menu,MF_STRING,ID_EDIT_REPLACE,L"&Replace\tCtrl+H");
				AppendMenuW(menu,MF_STRING,ID_EDIT_GOTO,L"&Goto\tCtrl+G");
				AppendMenuW(menu,MF_SEPARATOR,0,0);
		
				{
					HMENU advanced_menu;
					
					advanced_menu = CreatePopupMenu();

					AppendMenuW(menu,MF_POPUP,(UINT_PTR)advanced_menu,L"&Advanced");
				
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_MAKE_LOWERCASE,L"Make Lowercase\tCtrl+U");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_MAKE_UPPERCASE,L"Make Uppercase\tCtrl+Shift+U");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_INCREMENT_INTEGER,L"Increment Integer\tCtrl+I");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_DECREMENT_INTEGER,L"Decrement Integer\tCtrl+Shift+I");
					AppendMenuW(advanced_menu,MF_STRING | (view_white_space ? MF_CHECKED : 0),ID_EDIT_VIEW_WHITE_SPACE,L"View White Space");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_INCREASE_LINE_INDENT,L"Increase Line Indent");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_DECREASE_LINE_INDENT,L"Decrease Line Indent");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_INSERT_HEX,L"Edit &Hex...");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_INSERT_UTF8_SEQUENCE,L"Edit &UTF-8...");
					AppendMenuW(advanced_menu,MF_STRING,ID_EDIT_INSERT_UTF16_SEQUENCE,L"&Edit UTF-16...");
				}

				break;
				
			case 2: // tools

				os_remove_all_menu_items(menu);

				AppendMenuW(menu,MF_STRING | ((voidpad_macro_start && (!voidpad_macro_is_recording)) ? 0 : (MF_GRAYED|MF_DISABLED)),ID_TOOLS_MACRO_PLAY,L"Play Macro\tCtrl+Shift+P");
					
				if (voidpad_macro_is_recording)
				{
					AppendMenuW(menu,MF_STRING,ID_TOOLS_MACRO_RECORD,L"&Stop Recording Macro\tCtrl+Shift+R");
				}
				else
				{
					AppendMenuW(menu,MF_STRING,ID_TOOLS_MACRO_RECORD,L"&Record Macro\tCtrl+Shift+R");
				}

				AppendMenuW(menu,MF_SEPARATOR,0,0);
				AppendMenuW(menu,MF_STRING,ID_TOOLS_OPTIONS,L"&Options...");

				break;
				
			case 3: // help

				os_remove_all_menu_items(menu);

#ifdef VERSION_REGISTRATION
				if (!voidpad_is_registered)
				{
					AppendMenuW(menu,MF_STRING,ID_HELP_REGISTER,L"&Register...");
					AppendMenuW(menu,MF_SEPARATOR,0,0);
				}
#endif
				
				AppendMenuW(menu,MF_STRING,ID_HELP_ABOUT,L"&About\tCtrl+F1");

				break;
		}
	}
}

void os_remove_all_menu_items(HMENU hmenu)
{	
	// remove all items
	// we loop until all the items are removed
	// we remove the first item in the menu each time we delete an item
	// use deletemenu to recursively destroy submenus.
	// removemenu does not destroy sub menus.
	for(;;)
	{
		if (!DeleteMenu(hmenu,0,MF_BYPOSITION)) break;
	}
}

static void get_col_ln_from_x_y(int x,int y,int *pch,int *pln)
{
	int col;
	int ln;
	int next_col;
	int ofx;
	
	x -= 2;
	
	col = x / font_wide;
	ln = y / font_high;
	
	ln += get_vscroll();
	col += get_hscroll();
	
	ln = clip_ln(ln);
	col = clip_col(col,ln);

	// snap.
	col = col_to_ch(col,ln);
	next_col = ch_to_col(col + 1,ln);;
	col = ch_to_col(col,ln);
	
	ofx = x - (col - get_hscroll()) * font_wide;
	
	if (ofx > (((next_col - col) * font_wide) / 2))
	{
		col = next_col;
	}
	
	*pch = col;	
	*pln = ln;
}

static int get_integer(const unsigned char *p,int len)
{
	int i;
	int sign;
	int base;
	
	i = 0;
	
	if ((len >= 1) && (*p == '-'))
	{
		sign = -1;

		p++;
		len--;
	}
	else
	{
		sign = 1;
	}
	
	if ((len >= 2) && (*p == '0') && ((p[1] == 'x') || (p[1] == 'X')))
	{
		base = 16;
		len -= 2;
		p += 2;
	}
	else
	{
		base = 10;
	}
	
	while(len)
	{
		if ((base == 8) && (*p >= '0') && (*p <= '7'))
		{
			i = (i * base) + *p - '0';
		}
		else
		if ((base >= 10) && (*p >= '0') && (*p <= '9'))
		{
			i = (i * base) + *p - '0';
		}
		else
		if ((base == 16) && (*p >= 'A') && (*p <= 'F'))
		{
			i = (i * base) + *p - 'A' + 10;
		}
		else
		if ((base == 16) && (*p >= 'a') && (*p <= 'f'))
		{
			i = (i * base) + *p - 'a' + 10;
		}
		else
		{
			break;
		}
	
		len--;
		p++;
	}
	
	return sign * i;
}

static void redo_clear(void)
{
	redo_t *u;
	
	u = redo_start;
	while(u)
	{
		redo_t *next_u;
		
		next_u = u->next;
		
		redo_free(u);
		
		u = next_u;
	}
	
	redo_start = 0;
	redo_last = 0;
}

static void undo_clear(void)
{
	undo_t *u;
	
	u = undo_start;
	while(u)
	{
		undo_t *next_u;
		
		next_u = u->next;
		
		undo_free(u);
		
		u = next_u;
	}
	
	undo_start = 0;
	undo_last = 0;
}

// Everything uses IsAppThemed to determine if visual styles is enabled.
// this used to be isThemeActive and would return TRUE even when visual styles was disabled.
static BOOL _os_emulate_IsAppThemed(void)
{
	return FALSE;
}

// use both IsAppThemed AND IsThemeActive to determine if we are themed.
static BOOL _os_emulate_IsThemeActive(void)
{
	return FALSE;
}


// only sets the proc if it was found.
// make sure you set proc to 0 before calling.
static void _os_get_proc_address(HMODULE hmodule,const char *name,FARPROC *proc)
{
	FARPROC p;
	
	p = GetProcAddress(hmodule,name);
	if (p) 
	{
		*proc = p;
	}
}

static void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = _os_MonitorFromRect(window_rect,MONITOR_DEFAULTTOPRIMARY);
	if (hmonitor)
	{
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hmonitor,&mi);
		
		CopyRect(monitor_rect,&mi.rcWork);
	}
	else
	{
		// work area
		SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)monitor_rect,0);
	}
}

// make a rect fully visible
// pushes the rect onto the screen, the same as menus.
// does not allow monitor overlapping.
// if you have a window that is fully visible, but half is displayed on one display, and half on another, this function will
// push it onto just one of the displays. (the one with the most area shown)
// make sure you call os_is_rect_completely_visible before calling this.
static void os_make_rect_completely_visible(RECT *prect)
{
	int wide;
	int high;
	RECT monitor_rect;

	// we hit something so reposition the window on this monitor.
	// note that MonitorFromRect can return null, even when MONITOR_DEFAULTTOPRIMARY is specified.
	os_MonitorRectFromWindowRect(prect,&monitor_rect);
	
	// get window width and height,
	wide = prect->right - prect->left;
	high = prect->bottom - prect->top;

	// push the window 
	if (prect->right > monitor_rect.right) 
	{
		prect->left = monitor_rect.right - wide;
		prect->right = monitor_rect.right;
	}
	
	if (prect->bottom > monitor_rect.bottom) 
	{
		prect->top = monitor_rect.bottom - high;
		prect->bottom = monitor_rect.bottom;
	}

	// push the window 
	if (prect->left < monitor_rect.left) 
	{
		prect->left = monitor_rect.left;
		prect->right = monitor_rect.left + wide;
	}
	
	if (prect->top < monitor_rect.top) 
	{
		prect->top = monitor_rect.top;
		prect->bottom = monitor_rect.top + high;
	}
}

static HMONITOR WINAPI _os_Emulate_MonitorFromRect(LPCRECT lprc,DWORD dwFlags)
{
	return 0;
}

typedef struct {
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    wchar_t menu;
    wchar_t windowClass;
    wchar_t title;
    WORD pointsize;
    WORD weight;
    BYTE italic;
    BYTE charset;
    WCHAR typeface[32];
} _os_DLGTEMPLATEEX;

static INT_PTR os_create_blank_dialog(HWND parent,DLGPROC proc,void *param)
{
	HGLOBAL hgbl;
	INT_PTR ret;
	_os_DLGTEMPLATEEX *blank_dialog;

	hgbl = GlobalAlloc(GMEM_ZEROINIT,sizeof(_os_DLGTEMPLATEEX));
	blank_dialog = (_os_DLGTEMPLATEEX *)GlobalLock(hgbl);
	
	crt_zero_memory(blank_dialog,sizeof(_os_DLGTEMPLATEEX));
 
	// Define a dialog box.
	blank_dialog->dlgVer = 1;
	blank_dialog->signature = 0xffff;
	blank_dialog->exStyle = WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT ;
	blank_dialog->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_3DLOOK | DS_MODALFRAME | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | DS_FIXEDSYS | DS_SETFONT;
	blank_dialog->pointsize = 8;
	blank_dialog->weight = 400;
	blank_dialog->italic = 0;
	blank_dialog->charset = 0;
	wstring_copy(blank_dialog->typeface,32,L"MS Shell Dlg");

	GlobalUnlock(hgbl); 
	
	ret = DialogBoxIndirectParam(GetModuleHandle(0),(LPDLGTEMPLATE)hgbl,parent,proc,(LPARAM)param); 
	
	GlobalFree(hgbl); 
	
	return ret;
}

static void os_center_dialog(HWND hwnd,int wide,int high)
{
	RECT rect;
	RECT parent_rect;
	
	rect.left = 0;
	rect.top = 0;
	rect.right = wide;
	rect.bottom = high;

	AdjustWindowRect(&rect,GetWindowStyle(hwnd),FALSE);
	
	GetWindowRect(GetParent(hwnd),&parent_rect);
	
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;
	
	rect.left = parent_rect.left + ((parent_rect.right - parent_rect.left) / 2) - (wide / 2);
	rect.top = parent_rect.top + ((parent_rect.bottom - parent_rect.top) / 2) - (high / 2);
	rect.right = rect.left + wide;
	rect.bottom = rect.top + high;

	os_make_rect_completely_visible(&rect);
	
	SetWindowPos(hwnd,0,rect.left,rect.top,wide,high,SWP_NOZORDER|SWP_NOACTIVATE);
}


static INT_PTR __stdcall _os_ds_setfont_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_SETFONT:
		{
			LOGFONT lf;

			GetObject((HFONT)wParam,sizeof(LOGFONT),&lf);

			os_default_hfont = CreateFontIndirect(&lf);
			
			EndDialog(hwnd,0);
			
			break;
		}
	}
		
	return 0;
}

// ugly mess to get the default gui font
// we create a blank dialog which will be passed WM_SETFONT.
// DialogBoxIndirectParam will convert "MS Shell Dlg" to "MS Shell Dlg 2" on windows Vista+ for us.
static void os_load_default_hfont(void)
{
	HGLOBAL hgbl;
	_os_DLGTEMPLATEEX *blank_dialog;

	hgbl = GlobalAlloc(GMEM_ZEROINIT,sizeof(_os_DLGTEMPLATEEX));
	blank_dialog = (_os_DLGTEMPLATEEX *)GlobalLock(hgbl);
	
	crt_zero_memory(blank_dialog,sizeof(_os_DLGTEMPLATEEX));
 
	// Define a dialog box.
	blank_dialog->dlgVer = 1;
	blank_dialog->signature = 0xffff;
	blank_dialog->exStyle = WS_EX_NOACTIVATE;
	blank_dialog->style = WS_POPUP | WS_SYSMENU | DS_MODALFRAME | DS_FIXEDSYS | DS_SETFONT;
	blank_dialog->pointsize = 8;
	blank_dialog->weight = 400;
	blank_dialog->italic = 0;
	blank_dialog->charset = 0;
	wstring_copy(blank_dialog->typeface,32,L"MS Shell Dlg");
	
	GlobalUnlock(hgbl); 
	
	DialogBoxIndirectParam(GetModuleHandle(0),(LPDLGTEMPLATE)hgbl,0,_os_ds_setfont_proc,(LPARAM)0); 
	
	GlobalFree(hgbl); 
}

static INT_PTR __stdcall voidpad_multiline_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_CHAR:
			if (wParam == 1) 
			{
				return 0;
			}
			break;
			
		case WM_KEYDOWN:
		
			switch(wParam)
			{
				case 'A':
				
					if (GetKeyState(VK_CONTROL) < 0)
					{
						SendMessage(hwnd, EM_SETSEL, 0, -1);

						return 0;
					}
					break;
			}

			break;
	}
	
	return CallWindowProc(voidpad_old_edit_proc,hwnd,msg,wParam,lParam);
}

static int is_word(unsigned char *start,unsigned char *p,unsigned char *e,int len)
{
	if (p > start)
	{
		if (is_alphanum(p[-1]))
		{
			return 0;
		}
	}

	if (p + len < e)
	{
		if (is_alphanum(p[len]))
		{
			return 0;
		}
	}
	
	return 1;
}

static int is_alphanum(int c)
{
	if ((c >= 'a') && (c <= 'z')) return 1;
	if ((c >= 'A') && (c <= 'Z')) return 1;
	if ((c >= '0') && (c <= '9')) return 1;
	if (c == '_') return 1;

	return 0;	
}

static void voidpad_find_delete(find_t *f)
{
	if (f == find_start)
	{
		find_start = f->next;
	}
	else
	{
		f->prev->next = f->next;
	}
	
	if (f == find_last)
	{
		find_last = f->prev;
	}
	else
	{
		f->next->prev = f->prev;
	}
	
	find_count--;
	mem_free(f->text);
	mem_free(f);
}

static find_t *find_find(const unsigned char *text)
{
	find_t *f;
	
	f = find_start;
	while(f)
	{
		if (string_compare(text,f->text) == 0)
		{
			return f;
		}
	
		f = f->next;
	}
	
	return 0;
}

static find_t *find_add(const unsigned char *text)
{
	if (*text)
	{
		find_t *f;
		int len;
		
		f = find_find(text);
		if (f)
		{
			voidpad_find_delete(f);
		}
		
		if (find_count + 1 > VOIDPAD_FIND_MAX)
		{
			voidpad_find_delete(find_start);
		}
		
		f = mem_alloc(sizeof(find_t));
		
		len = string_length(text);
		f->text = mem_alloc(len + 1);
		crt_copy_memory(f->text,text,len + 1);
		
		if (find_start)
		{
			find_last->next = f;
			f->prev = find_last;
		}
		else
		{
			find_start = f;	
			f->prev = 0;
		}
		
		f->next = 0;
		find_last = f;
		find_count++;
		
		return f;
	}
	
	return 0;
}

static void voidpad_replace_delete(replace_t *f)
{
	if (f == replace_start)
	{
		replace_start = f->next;
	}
	else
	{
		f->prev->next = f->next;
	}
	
	if (f == replace_last)
	{
		replace_last = f->prev;
	}
	else
	{
		f->next->prev = f->prev;
	}
	
	replace_count--;
	mem_free(f->text);
	mem_free(f);
}

static replace_t *replace_find(const unsigned char *text)
{
	replace_t *f;
	
	f = replace_start;
	while(f)
	{
		if (string_compare(text,f->text) == 0)
		{
			return f;
		}
	
		f = f->next;
	}
	
	return 0;
}

static replace_t *replace_add(const unsigned char *text)
{
	replace_t *f;
	int len;
	
	f = replace_find(text);
	if (f)
	{
		voidpad_replace_delete(f);
	}
	
	if (replace_count + 1 > VOIDPAD_REPLACE_MAX)
	{
		voidpad_replace_delete(replace_start);
	}
	
	f = mem_alloc(sizeof(replace_t));
	
	len = string_length(text);
	f->text = mem_alloc(len + 1);
	crt_copy_memory(f->text,text,len + 1);
	
	if (replace_start)
	{
		replace_last->next = f;
		f->prev = replace_last;
	}
	else
	{
		replace_start = f;	
		f->prev = 0;
	}
	
	f->next = 0;
	replace_last = f;
	replace_count++;
	
	return f;
}

static void voidpad_combobox_insert_string(HWND hwnd,int id,int index,unsigned char *text)
{
	int len;
	int wlen;
	wchar_t *wbuf;
	
	len = string_length(text);

	wlen = MultiByteToWideChar(voidpad_cp,0,(char *)text,len,0,0);
	wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	MultiByteToWideChar(voidpad_cp,0,(char *)text,len,wbuf,wlen);
	wbuf[wlen] = 0;

	ComboBox_InsertString(GetDlgItem(hwnd,id),index,wbuf);
	
	mem_free(wbuf);
}

static wchar_t *wstring_alloc_window_text(HWND hwnd)
{
	wchar_t *wbuf;
	int wlen;
	
	wlen = GetWindowTextLength(hwnd);
	wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	GetWindowText(hwnd,wbuf,(WPARAM)(wlen+1));
	
	return wbuf;
}

static wchar_t *os_alloc_combo_list_text(HWND hwnd,int index)
{
	wchar_t *wbuf;
	int wlen;
	
	wlen = ComboBox_GetLBTextLen(hwnd,index);
	wbuf = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	ComboBox_GetLBText(hwnd,index,wbuf);
	
	return wbuf;
}

static unsigned char *os_alloc_wchar_to_voidpad_cp(wchar_t *wbuf)
{
	int wlen;
	int len;
	unsigned char *buf;
	
	wlen = wstring_length(wbuf);
	
	len = WideCharToMultiByte(voidpad_cp,0,wbuf,wlen,0,0,0,0);
	
	buf = mem_alloc(len + 1);
	
	WideCharToMultiByte(voidpad_cp,0,wbuf,wlen,(char *)buf,len,0,0);
	
	buf[len] = 0;
	
	return buf;
}

static unsigned char *utf8_alloc_wchar(const wchar_t *wbuf)
{
	int wlen;
	int len;
	unsigned char *buf;
	
	wlen = wstring_length(wbuf);
	
	len = WideCharToMultiByte(CP_UTF8,0,wbuf,wlen,0,0,0,0);
	
	buf = mem_alloc(len + 1);
	
	WideCharToMultiByte(CP_UTF8,0,wbuf,wlen,(char *)buf,len,0,0);
	
	buf[len] = 0;
	
	return buf;
}

static void voidpad_find_update_search_combobox(void)
{
	find_t *f;
	
	SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),WM_SETREDRAW,FALSE,0);

	f = find_start;
	while(f)
	{
		voidpad_combobox_insert_string(find_hwnd,ID_FIND_COMBO,0,f->text);
		
		f = f->next;
	}
	
	ComboBox_SetCurSel(GetDlgItem(find_hwnd,ID_FIND_COMBO),0);
	
	SendMessage(GetDlgItem(find_hwnd,ID_FIND_COMBO),WM_SETREDRAW,TRUE,0);
	InvalidateRect(GetDlgItem(find_hwnd,ID_FIND_COMBO),0,FALSE);
}

static void voidpad_find_update_replace_combobox(void)
{
	replace_t *f;

	SendMessage(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),WM_SETREDRAW,FALSE,0);
	
	ComboBox_ResetContent(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO));
	
	f = replace_start;
	while(f)
	{
		voidpad_combobox_insert_string(find_hwnd,ID_FIND_REPLACE_COMBO,0,f->text);
		
		f = f->next;
	}

	ComboBox_SetCurSel(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),0);

	SendMessage(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),WM_SETREDRAW,TRUE,0);
	InvalidateRect(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),0,FALSE);
}

static int is_indent(int y)
{
	if (line_list[y]->len)
	{
		if (line_list[y]->text[0] == '\t') 
		{
			return 1;
		}
	}
	
	return 0;
}

static void voidpad_bring_cursor_into_view(void)
{
	// bring y into view.
	// even if we didnt change the position.
	if (cursor_y < get_vscroll())
	{
		set_vscroll(cursor_y);
	}

	// bring y into view.
	// even if we didnt change the position.
	if (cursor_y + 1 > get_vscroll() + get_vpage())
	{
		set_vscroll(cursor_y + 1 - get_vpage());
	}

	// bring x into view.
	{
		int clipx;
		
		clipx = get_nearest_cursor_colx();
		
		if (clipx < get_hscroll())
		{
			set_hscroll(clipx);
		}
		
		// bring x into view.
		if (clipx + 1 > get_hscroll() + get_hpage())
		{
			set_hscroll(clipx + 1 - get_hpage());
		}
	}
}

static void voidpad_indent(int isshift)
{
	int y;
	int backup_sel_y1;
	int backup_sel_y2;
	int backup_cursor_chx;
	int backup_cursor_y;
	int backup_mark_chx;
	int backup_mark_y;
	
	if (is_selection())
	{
		backup_sel_y1 = sel_y1;
		backup_sel_y2 = sel_y2;
	}
	else
	{
		backup_sel_y1 = cursor_y;
		backup_sel_y2 = cursor_y;
	}
	
	backup_cursor_chx = col_to_ch(cursor_x,cursor_y);
	backup_cursor_y = cursor_y;
	backup_mark_chx = col_to_ch(mark_x,mark_y);
	backup_mark_y = mark_y;
	
	if ((sel_x2 == 0) && (backup_sel_y2 != backup_sel_y1)) 
	{
		// we could be at position 0 with a single line.
		backup_sel_y2--;
	}
	
	if (isshift)
	{
		for(y = backup_sel_y1;y<=backup_sel_y2;y++)
		{
			if (is_indent(y)) 
			{
				goto found_tab;
			}
		}

		// no tabs... nothing to do.
		return;
	}
	
found_tab:

	if (is_selection())
	{
		undo_clear_selection();
	}
	
	for(y = backup_sel_y1;y<=backup_sel_y2;y++)
	{
		// merge undos.
		// we might not have done an indent last line, 
		// in that case undo_last will point to the undo_clear_selection.
		if (y != backup_sel_y1)
		{
			undo_last->is_more = 1;
		}
		
		set_cursor_ch_position(0,y,1,1,0);
		if (isshift)
		{
			int indent;
			
			indent = is_indent(y);
		
			if (indent)
			{
				set_cursor_ch_position(1,y,0,1,0);
				
				if (y == backup_cursor_y)
				{
					if (backup_cursor_chx)
					{
						backup_cursor_chx--;
					}
				}

				if (y == backup_mark_y)
				{
					if (backup_mark_chx)
					{
						backup_mark_chx--;
					}
				}
			}

			insert_text(0,0,1,0,0,0);
		}
		else
		{
			insert_text((unsigned char *)"\t",1,1,0,0,0);
		}
	}
	
	if (backup_cursor_chx)
	{
		if (!isshift)
		{
			backup_cursor_chx++;
		}
	}		
					
	if (backup_mark_chx)
	{
		if (!isshift)
		{
			backup_mark_chx++;
		}
	}
	
	set_cursor_ch_position(backup_mark_chx,backup_mark_y,1,1,0);
	set_cursor_ch_position(backup_cursor_chx,backup_cursor_y,0,1,1);
}

// hide the cursor before scrolling.
static void cursor_hide(void)
{
	if (cursor_flash)
	{
		RECT rect;
		
		cursor_flash = 0;

		// invalidate
		get_cursor_rect(&rect);

		InvalidateRect(edit_hwnd,&rect,FALSE);
	}
}

static void select_main(int word_left,int word_right,int word_y)
{
	voidpad_set_capture(VOIDPAD_CAPTURE_SELECT);
	
	printf("select Enter\n");
	
	for(;;)
	{
		MSG msg;
		
		if (!GetMessage(&msg,0,0,0)) 
		{
			return;
		}
		
		if (msg.message == WM_LBUTTONUP)
		{
			break;
		}		
		
		if (msg.message == WM_RBUTTONDOWN)
		{
			break;
		}		
		
		if (msg.message == WM_CANCELMODE)
		{
			break;
		}
		
		if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE))
		{
			set_cursor_col_position(mark_x,mark_y,1,1,0);
			break;
		}
	
		if ((GetCapture() != edit_hwnd)) 
		{
			break;
		}
		
		if (msg.message == WM_MOUSEMOVE)
		{
			int x;
			int y;
			int isctrl;
			int isalt;
			int isshift;
			
			isctrl = GetKeyState(VK_CONTROL) < 0;
			isalt = GetKeyState(VK_MENU) < 0;
			isshift = GetKeyState(VK_SHIFT) < 0;
		
			get_col_ln_from_x_y(GET_X_LPARAM(msg.lParam),GET_Y_LPARAM(msg.lParam),&x,&y);
			
			if (word_y != -1)
			{
				if (compare_ch_ln(x,y,word_right,mark_y) > 0)
				{
					set_cursor_ch_position(word_left,word_y,1,1,0);
				}
				else
				if (compare_ch_ln(x,y,word_left,mark_y) < 0)
				{
					set_cursor_ch_position(word_right,word_y,1,1,0);
				}
				else
				{
					set_cursor_ch_position(word_left,word_y,1,1,0);
					set_cursor_ch_position(word_right,word_y,0,1,1);
					
					goto select_word;
				}
			}

			set_cursor_col_position(x,y,0,1,1);
			
select_word:			
;

		}

		DispatchMessage(&msg);
	}

	printf("select leave\n");

	voidpad_release_capture(VOIDPAD_CAPTURE_SELECT);
}

static int voidpad_tolower(int c)
{
	if ((c >= 'A') && (c <= 'Z'))
	{
		return c - ('A' - 'a');
	}
	
	return c;
}

static void voidpad_set_capture(int flag)
{
	if (GetCapture() != edit_hwnd)
	{
		SetCapture(edit_hwnd);
	}
	
	voidpad_capture |= flag;
}

static void voidpad_release_capture(int flag)
{
	voidpad_capture &= ~flag;

	if (!voidpad_capture)
	{
		ReleaseCapture();
	}
}

static int voidpad_can_paste(void)
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))	
	{
		return 1;
	}

	if (IsClipboardFormatAvailable(voidpad_clipboard_format))	
	{
		return 1;
	}
	
	return 0;
}

#ifdef _DEBUG

static void *mem_alloc_FILE_LINE(char *file,int line,int size)
{
	mem_t *m;
	
	m = (mem_t *)HeapAlloc(GetProcessHeap(),0,size + sizeof(mem_t) + sizeof(DWORD));
	if (!m) DebugBreak();
	
	m->file = file;
	m->line = line;
	m->size = size;
	
	if (mem_start)
	{	
		mem_last->next = m;
		m->prev = mem_last;
	}
	else
	{
		mem_start = m;
		m->prev = 0;
	}
	
	mem_last = m;
	m->next = 0;
	
	m->magic = 0xdeadbeef;
	*(DWORD *)(((char *)(m + 1)) + size) = 0xdeadbeef;
	
	return m + 1;
}

static void mem_free_FILE_LINE(char *file,int line,void *ptr)
{
	mem_t *m;
	
	m = ((mem_t *)ptr) - 1;
	
	if (m->magic != 0xdeadbeef)
	{
		printf("underrun\n");
		
		DebugBreak();
	}
	
	if (*(DWORD *)(((char *)ptr) + m->size) != 0xdeadbeef)
	{
		printf("overflow\n");
		
		DebugBreak();
	}
	
	if (mem_start == m)
	{
		mem_start = m->next;
	}
	else
	{
		m->prev->next = m->next;
	}
	
	if (mem_last == m)
	{
		mem_last = m->prev;
	}
	else
	{
		m->next->prev = m->prev;
	}

	HeapFree(GetProcessHeap(),0,m);
}

static void mem_DEBUG(void)
{
	mem_t *m;
	
	m = mem_start;
	while(m)
	{
		if (m->magic != 0xdeadbeef)
		{
			printf("mem: %s(%d): underflow\n",m->file,m->line);
		}

		if (*(DWORD *)(((char *)(m + 1)) + m->size) != 0xdeadbeef)
		{
			printf("mem: %s(%d): overflow\n",m->file,m->line);
		}
		
		printf("mem: %s(%d): leak %d bytes\n",m->file,m->line,m->size);
	
		m = m->next;
	}
}

#else

static void *mem_alloc(int size)
{
	void *p;
	
	p = HeapAlloc(GetProcessHeap(),0,size);
	if (!p)
	{
		MessageBoxW(0,L"voidPad",L"Out Of Memory",MB_OK);
	}
	
	return p;
}

static void mem_free(void *ptr)
{
	HeapFree(GetProcessHeap(),0,ptr);
}

#endif

static void voidpad_find_search_changed(void)
{
	wchar_t *wbuf;
	unsigned char *buf;
	int index;
	int len;
	
	index = ComboBox_GetCurSel(GetDlgItem(find_hwnd,ID_FIND_COMBO));
	if (index != CB_ERR)
	{
		wbuf = os_alloc_combo_list_text(GetDlgItem(find_hwnd,ID_FIND_COMBO),index);
	}
	else
	{
		wbuf = wstring_alloc_window_text(GetDlgItem(find_hwnd,ID_FIND_COMBO));
	}
	
	buf = os_alloc_wchar_to_voidpad_cp(wbuf);
	len = string_length(buf);
	
	voidpad_command_data(VOIDPAD_COMMAND_FIND_SET_SEARCH,buf,len);

	mem_free(buf);
	mem_free(wbuf);

	EnableWindow(GetDlgItem(find_hwnd,IDOK),len ? TRUE : FALSE);
	if (IsWindowVisible(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON)))
	{
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACE_BUTTON),len ? TRUE : FALSE);
		EnableWindow(GetDlgItem(find_hwnd,ID_FIND_REPLACEALL_BUTTON),len ? TRUE : FALSE);
	}
}

static void voidpad_find_replace_changed(void)
{
	wchar_t *wbuf;
	unsigned char *buf;
	int index;
	int len;
	
	index = ComboBox_GetCurSel(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO));
	if (index != CB_ERR)
	{
		wbuf = os_alloc_combo_list_text(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO),index);
	}
	else
	{
		wbuf = wstring_alloc_window_text(GetDlgItem(find_hwnd,ID_FIND_REPLACE_COMBO));
	}
	
	buf = os_alloc_wchar_to_voidpad_cp(wbuf);
	len = string_length(buf);
	
	voidpad_command_data(VOIDPAD_COMMAND_FIND_SET_REPLACE,buf,len);

	mem_free(buf);
	mem_free(wbuf);
}

// tries to avoid selecting text when resizing a combobox.
static void combobox_set_position(HWND hwnd,int id,int x,int y,int wide,int high)
{
	RECT rect;
	UINT flags;
	
	hwnd = GetDlgItem(hwnd,id);
	
	flags = SWP_NOACTIVATE|SWP_NOZORDER;
	
	GetWindowRect(hwnd,&rect);

	// ignore height changes	
	if (rect.right - rect.left == wide)
	{
		flags |= SWP_NOSIZE;
	}
	
	SetWindowPos(hwnd,0,x,y,wide,high,flags);
}

static HTREEITEM voidpad_options_add_treeitem(HWND hwnd,int id,HTREEITEM parent,int index,const wchar_t *name)
{
	TVINSERTSTRUCT tvis;
	HTREEITEM result;
	
	tvis.hInsertAfter = TVI_LAST;
	tvis.hParent = parent;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = (wchar_t *)name;
	tvis.item.lParam = index;

	result = TreeView_InsertItem(GetDlgItem(hwnd,id),&tvis);
	
	return result;
}

static INT_PTR CALLBACK voidpad_options_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,VOIDPAD_OPTIONS_WIDE,VOIDPAD_OPTIONS_HIGH);
			
			SetWindowText(hwnd,L"Options");
				
			x = 6;
			y = 6;
			wide = VOIDPAD_OPTIONS_WIDE - 12;
			high = VOIDPAD_OPTIONS_HIGH - 12;
			
			high -= 23 + 6;
	
			x += 122 + 6;
			wide -= 122 + 6;
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"OK",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75-6-75,y+high + 6,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75-6-75,y+high + 6,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
				
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Apply",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75,y+high + 6,75,23,
				hwnd,(HMENU)ID_OPTIONS_APPLY,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,ID_OPTIONS_APPLY));
			
			EnableWindow(GetDlgItem(hwnd,ID_OPTIONS_APPLY),FALSE);

			x -= 122 + 6;
			wide += 122 + 6;
				
			// static goto what:
			CreateWindowExW(
				WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
				WC_TREEVIEW,
				L"",
				WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_TABSTOP | WS_VSCROLL | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_TRACKSELECT | TVS_DISABLEDRAGDROP | WS_GROUP,
				x,y,122,high,
				hwnd,(HMENU)ID_OPTIONS_TREE,GetModuleHandle(0),0);
				
			os_set_default_font(GetDlgItem(hwnd,ID_OPTIONS_TREE));

			{
				HTREEITEM general_htreeitem;
				
				general_htreeitem = voidpad_options_add_treeitem(hwnd,ID_OPTIONS_TREE,0,VOIDPAD_OPTIONS_PAGE_GENERAL,L"General");
				voidpad_options_add_treeitem(hwnd,ID_OPTIONS_TREE,general_htreeitem,VOIDPAD_OPTIONS_PAGE_EDITOR,L"Editor");
				voidpad_options_add_treeitem(hwnd,ID_OPTIONS_TREE,general_htreeitem,VOIDPAD_OPTIONS_PAGE_FONT,L"Font");

				TreeView_Expand(GetDlgItem(hwnd,ID_OPTIONS_TREE),general_htreeitem,TVE_EXPAND);
				TreeView_Select(GetDlgItem(hwnd,ID_OPTIONS_TREE),general_htreeitem,TVGN_CARET);
			}

			// select a tree item
			TreeView_SelectItem(GetDlgItem(hwnd,ID_OPTIONS_TREE),TreeView_GetRoot(GetDlgItem(hwnd,ID_OPTIONS_TREE)));

			// setup the first page_hwnd.
			voidpad_options_treeview_changed(hwnd);	
				
			return TRUE;
		}
		
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case ID_OPTIONS_TREE:

					switch(((NMHDR *)lParam)->code)
					{
						case TVN_SELCHANGEDA:
						case TVN_SELCHANGEDW:
						
							voidpad_options_treeview_changed(hwnd);
							
							break;
					}
					
					break;
			}
			
			return 0;
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
				case ID_OPTIONS_APPLY:
					
					if (LOWORD(wParam) != IDCANCEL)
					{
						if (IsWindowEnabled(GetDlgItem(hwnd,ID_OPTIONS_APPLY)))
						{
							HWND page_hwnd;
							int is_dirty;
							
							is_dirty = 0;

							// General
							page_hwnd = GetDlgItem(hwnd,voidpad_options_pages[VOIDPAD_OPTIONS_PAGE_GENERAL].page_id);
							
							if (page_hwnd)
							{
								// context menus
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_ALL_CONTEXT_MENU) == BST_CHECKED)
								{
									if (!GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_ALL_CONTEXT_MENU),GWLP_USERDATA))
									{
										voidpad_install_all_context_menu();
									}
								}
								else
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_ALL_CONTEXT_MENU) == BST_UNCHECKED)
								{
									if (GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_ALL_CONTEXT_MENU),GWLP_USERDATA))
									{
										voidpad_uninstall_all_context_menu();
									}
								}

								// txt association
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_TXT_ASSOCIATION) == BST_CHECKED)
								{
									if (!GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_TXT_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_install_association(L".txt",L"Text Document");
									}
								}
								else
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_TXT_ASSOCIATION) == BST_UNCHECKED)
								{
									if (GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_TXT_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_uninstall_association(L".txt");
									}
								}
								
								// ini association
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_INI_ASSOCIATION) == BST_CHECKED)
								{
									if (!GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_INI_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_install_association(L".ini",L"Configuration Settings");
									}
								}
								else
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_INI_ASSOCIATION) == BST_UNCHECKED)
								{
									if (GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_INI_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_uninstall_association(L".ini");
									}
								}
								
								// nfo association
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_NFO_ASSOCIATION) == BST_CHECKED)
								{
									if (!GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_NFO_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_install_association(L".nfo",L"Info File");
									}
								}
								else
								if (IsDlgButtonChecked(page_hwnd,ID_OPTIONS_NFO_ASSOCIATION) == BST_UNCHECKED)
								{
									if (GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_NFO_ASSOCIATION),GWLP_USERDATA))
									{
										voidpad_uninstall_association(L".nfo");
									}
								}
							}
		
							// editor
							page_hwnd = GetDlgItem(hwnd,voidpad_options_pages[VOIDPAD_OPTIONS_PAGE_EDITOR].page_id);
							
							if (page_hwnd)
							{
								int tab_size;
								
								tab_size = voidpad_clip_tabsize(GetDlgItemInt(page_hwnd,ID_OPTIONS_TABSIZE_EDIT,0,FALSE));
								
								if (voidpad_tab_size != tab_size)
								{
									voidpad_tab_size = tab_size;
									
									voidpad_tab_size_changed();
								}
							}		
		
							// font
							page_hwnd = GetDlgItem(hwnd,voidpad_options_pages[VOIDPAD_OPTIONS_PAGE_FONT].page_id);
							
							if (page_hwnd)
							{
								COLORREF newcolor;
								
								newcolor = GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_FOREGROUND_COLOR_BUTTON),GWLP_USERDATA);
								if ((GetRValue(newcolor) != foreground_r) || (GetGValue(newcolor) != foreground_g) || (GetBValue(newcolor) != foreground_b))
								{
									foreground_r = GetRValue(newcolor);
									foreground_g = GetGValue(newcolor);
									foreground_b = GetBValue(newcolor);
									is_dirty = 1;
								}
								
								newcolor = GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_BACKGROUND_COLOR_BUTTON),GWLP_USERDATA);
								if ((GetRValue(newcolor) != background_r) || (GetGValue(newcolor) != background_g) || (GetBValue(newcolor) != background_b))
								{
									background_r = GetRValue(newcolor);
									background_g = GetGValue(newcolor);
									background_b = GetBValue(newcolor);
									is_dirty = 1;
								}
								
								newcolor = GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON),GWLP_USERDATA);
								if ((GetRValue(newcolor) != selected_foreground_r) || (GetGValue(newcolor) != selected_foreground_g) || (GetBValue(newcolor) != selected_foreground_b))
								{
									selected_foreground_r = GetRValue(newcolor);
									selected_foreground_g = GetGValue(newcolor);
									selected_foreground_b = GetBValue(newcolor);
									is_dirty = 1;
								}
								
								newcolor = GetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON),GWLP_USERDATA);
								if ((GetRValue(newcolor) != selected_background_r) || (GetGValue(newcolor) != selected_background_g) || (GetBValue(newcolor) != selected_background_b))
								{
									selected_background_r = GetRValue(newcolor);
									selected_background_g = GetGValue(newcolor);
									selected_background_b = GetBValue(newcolor);
									is_dirty = 1;
								}
								
								{
									wchar_t wbuf[MAX_PATH];
									
									GetWindowText(GetDlgItem(page_hwnd,ID_OPTIONS_FONT_BITMAP_EDIT),wbuf,MAX_PATH);

									if (wstring_compare(font_bitmap_filename,wbuf) != 0)
									{	
										mem_free(font_bitmap_filename);
										font_bitmap_filename = wstring_alloc(wbuf);

										font_load();
										
										// recalculate text_wide
										voidpad_tab_size_changed();
										
										// resize window.
										size_window();
										
										// invalidate.
										is_dirty = 1;
									}
								}

								voidpad_cp = GetDlgItemInt(page_hwnd,ID_OPTIONS_CP_EDIT,0,FALSE);
							}		
							
							if (is_dirty)
							{
								InvalidateRect(edit_hwnd,0,FALSE);
							}

							save_settings();
						}
					}
					
					if (LOWORD(wParam) == ID_OPTIONS_APPLY)
					{
						EnableWindow(GetDlgItem(hwnd,ID_OPTIONS_APPLY),FALSE);
					}
					else
					{
						EndDialog(hwnd,0);
					}
				
					break;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static void voidpad_options_main(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_options_proc,0);
}

static HWND voidpad_options_add_page(HWND hwnd,int tabid,int pageid,const wchar_t *name)
{
	HWND page_hwnd;
	HWND tab_hwnd;
	int x;
	int y;
	int wide;
	int high;
	RECT rect;
	TCITEMW tci;

	x = 6;
	y = 6;
	wide = VOIDPAD_OPTIONS_WIDE - 12;
	high = VOIDPAD_OPTIONS_HIGH - 12;
	
	high -= 23 + 6;
	x += 122 + 6;
	wide -= 122 + 6;
	
	tab_hwnd = CreateWindowEx(
		WS_EX_NOPARENTNOTIFY,
		WC_TABCONTROL,
		L"",
		TCS_HOTTRACK | TCS_TABS | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD| WS_VISIBLE,
		x,y,wide,high,
		hwnd,(HMENU)(uintptr_t)tabid,GetModuleHandle(0),0);

	os_set_default_font(GetDlgItem(hwnd,tabid));
	
	tci.mask = TCIF_TEXT;
	tci.pszText = (wchar_t *)name;
	
	SendMessage(GetDlgItem(hwnd,tabid),TCM_INSERTITEMW,(WPARAM)TabCtrl_GetItemCount(GetDlgItem(hwnd,tabid)),(LPARAM)&tci);
	
	// calculate the tab page window rect.
	GetWindowRect(tab_hwnd,&rect);
	
	MapWindowRect(0,hwnd,&rect);
	
	TabCtrl_AdjustRect(tab_hwnd,FALSE,&rect);
	
	// create the tab page.
	{
		HGLOBAL hgbl;
		
		struct blank_dialog_s
		{
			DWORD style;
			DWORD dwExtendedStyle;
			WORD cdit;
			short x;
			short y;
			short cx;
			short cy;
			short menu;
			short predefined_dialog_class;
			short name;
			
		}*blank_dialog;
		
		hgbl = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct blank_dialog_s));
		blank_dialog = (struct blank_dialog_s *)GlobalLock(hgbl);
	 
		// Define a dialog box.
		blank_dialog->style = WS_CHILD | DS_3DLOOK | DS_CONTROL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		blank_dialog->dwExtendedStyle = WS_EX_CONTROLPARENT;
		blank_dialog->cdit = 0;
		blank_dialog->x = 0;
		blank_dialog->y = 0;
		blank_dialog->cx = 0; 
		blank_dialog->cy = 0;
		blank_dialog->menu = 0;
		blank_dialog->predefined_dialog_class = 0;
		blank_dialog->name = 0;

		GlobalUnlock(hgbl);
		page_hwnd = CreateDialogIndirectParam(GetModuleHandle(0),(LPDLGTEMPLATE)hgbl,hwnd,voidpad_options_page_proc,0);
		GlobalFree(hgbl); 

		SetWindowLong(page_hwnd,GWL_ID,pageid);
		SetWindowPos(page_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE);

		BringWindowToTop(page_hwnd);

		// apply theme	
		if (os_EnableThemeDialogTexture)
		{
			os_EnableThemeDialogTexture(page_hwnd,ETDT_ENABLETAB);
		}
	}

	return page_hwnd;
}

static void voidpad_options_treeview_changed(HWND hwnd)
{
	TV_ITEM tvi;
	int apply_enabled;
	
	SendMessage(GetDlgItem(hwnd,ID_OPTIONS_APPLY),WM_SETREDRAW,FALSE,0);
	apply_enabled = IsWindowEnabled(GetDlgItem(hwnd,ID_OPTIONS_APPLY));

	// get the current index
	tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd,ID_OPTIONS_TREE));
	
	if (tvi.hItem)
	{
		tvi.mask = TVIF_PARAM;
	    
		TreeView_GetItem(GetDlgItem(hwnd,ID_OPTIONS_TREE),&tvi);
		
		// show the new page_hwnd before hidding the old page_hwnd
		// this will reduce the flickering between page_hwnd changing.
		if (!GetDlgItem(hwnd,voidpad_options_pages[tvi.lParam].page_id)) 
		{
			HWND page_hwnd;
			int x;
			int y;
			int wide;
			
printf("add page %d\n",tvi.lParam);

			page_hwnd = voidpad_options_add_page(hwnd,voidpad_options_pages[tvi.lParam].tab_id,voidpad_options_pages[tvi.lParam].page_id,voidpad_options_pages[tvi.lParam].name);
			
			// setup offsets relative to page_hwnd
			/*
			{
				RECT rect;
				
				rect.left = 6 + 122 + 6 + 12;
				rect.top = 6 + 12 + 12 + 6;
				rect.right = 480 - 6 - 12;
			}*/
			
			x = 6;
			y = 6;
			wide = 274-6-12;

			switch(tvi.lParam)
			{
				case VOIDPAD_OPTIONS_PAGE_GENERAL:
				{
					os_dlg_create_checkbox(page_hwnd,ID_OPTIONS_ALL_CONTEXT_MENU,0,voidpad_is_all_context_menu(),L"All file types &context menu",x,y,wide);
					y += 15 + 6;

					os_dlg_create_checkbox(page_hwnd,ID_OPTIONS_TXT_ASSOCIATION,0,voidpad_is_association(L".txt"),L"&txt association",x,y,wide);
					y += 15 + 6;
					
					os_dlg_create_checkbox(page_hwnd,ID_OPTIONS_INI_ASSOCIATION,0,voidpad_is_association(L".ini"),L"&ini association",x,y,wide);
					y += 15 + 6;
					
					os_dlg_create_checkbox(page_hwnd,ID_OPTIONS_NFO_ASSOCIATION,0,voidpad_is_association(L".nfo"),L"n&fo association",x,y,wide);
					y += 15 + 6;
											
					os_dlg_create_button(page_hwnd,ID_OPTIONS_ALL_ASSOCATIONS,0,L"&All",x,y,75);
					os_dlg_create_button(page_hwnd,ID_OPTIONS_NONE_ASSOCATIONS,0,L"&None",x+75+6,y,75);
					y += 15 + 6;
					
					break;
				}			
				
				case VOIDPAD_OPTIONS_PAGE_EDITOR:
				{
					int static_wide;

					static_wide = 0;
					static_wide = expand_wide(page_hwnd,static_wide,L"&Tab size:");
					static_wide += 6;
					
					// static goto what:
					os_dlg_create_static(page_hwnd,ID_OPTIONS_TABSIZE_STATIC,0,L"&Tab size:",x,y+3,static_wide);
							
					// edit
					CreateWindowExW(
						WS_EX_CLIENTEDGE,
						L"EDIT",
						L"",
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL,
						x+static_wide,y,75,21,
						page_hwnd,(HMENU)ID_OPTIONS_TABSIZE_EDIT,GetModuleHandle(0),0);

					os_set_default_font(GetDlgItem(page_hwnd,ID_OPTIONS_TABSIZE_EDIT));
					
					SetDlgItemInt(page_hwnd,ID_OPTIONS_TABSIZE_EDIT,voidpad_tab_size,FALSE);

					break;
				}	
							
				case VOIDPAD_OPTIONS_PAGE_FONT:
				{
					int static_wide;

					static_wide = 0;

					static_wide = expand_wide(page_hwnd,static_wide,L"&Foreground color:");
					static_wide = expand_wide(page_hwnd,static_wide,L"&Background color:");
					static_wide = expand_wide(page_hwnd,static_wide,L"&Selected foreground color:");
					static_wide = expand_wide(page_hwnd,static_wide,L"Selected Background color:");
					static_wide += 6;
					
					// static goto what:
					os_dlg_create_static(page_hwnd,ID_OPTIONS_FOREGROUND_COLOR_STATIC,0,L"&Foreground color:",x,y+4,static_wide);
					os_dlg_create_button(page_hwnd,ID_OPTIONS_FOREGROUND_COLOR_BUTTON,BS_BITMAP,L"",static_wide+x,y,75);
					
					y += 23 + 6;

					// static goto what:
					os_dlg_create_static(page_hwnd,ID_OPTIONS_BACKGROUND_COLOR_STATIC,0,L"&Background color:",x,y+4,static_wide);
					os_dlg_create_button(page_hwnd,ID_OPTIONS_BACKGROUND_COLOR_BUTTON,BS_BITMAP,L"",static_wide+x,y,75);
					
					y += 23 + 6;

					// static goto what:
					os_dlg_create_static(page_hwnd,ID_OPTIONS_SELECTED_FOREGROUND_COLOR_STATIC,0,L"&Selected foreground color:",x,y+4,static_wide);
					os_dlg_create_button(page_hwnd,ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON,BS_BITMAP,L"",static_wide+x,y,75);
					
					y += 23 + 6;

					// static goto what:
					os_dlg_create_static(page_hwnd,ID_OPTIONS_SELECTED_BACKGROUND_COLOR_STATIC,0,L"Selected Bac&kground color:",x,y+4,static_wide);
					os_dlg_create_button(page_hwnd,ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON,BS_BITMAP,L"",static_wide+x,y,75);
					
					y += 23 + 6;

					SetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_FOREGROUND_COLOR_BUTTON),GWLP_USERDATA,RGB(foreground_r,foreground_g,foreground_b));
					SetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_BACKGROUND_COLOR_BUTTON),GWLP_USERDATA,RGB(background_r,background_g,background_b));
					SetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON),GWLP_USERDATA,RGB(selected_foreground_r,selected_foreground_g,selected_foreground_b));
					SetWindowLongPtr(GetDlgItem(page_hwnd,ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON),GWLP_USERDATA,RGB(selected_background_r,selected_background_g,selected_background_b));

					voidpad_options_color_changed(page_hwnd,ID_OPTIONS_FOREGROUND_COLOR_BUTTON);
					voidpad_options_color_changed(page_hwnd,ID_OPTIONS_BACKGROUND_COLOR_BUTTON);
					voidpad_options_color_changed(page_hwnd,ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON);
					voidpad_options_color_changed(page_hwnd,ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON);
					
					y += 6;

					os_dlg_create_static(page_hwnd,0,0,L"Specify a custom monochrone bitmap of 32 x 8",x,y,wide);
					y += 15;
					os_dlg_create_static(page_hwnd,0,0,L"characters. Empty = internal IBM PC font.",x,y,wide);

					y += 15 + 12;

					// bitmap filename
					static_wide = 0;
					static_wide = expand_wide(page_hwnd,static_wide,L"Font bit&map:");
					static_wide += 6;

					os_dlg_create_static(page_hwnd,ID_OPTIONS_FONT_BITMAP_STATIC,0,L"Font bit&map:",x,y+4,static_wide);
						
					CreateWindowExW(
						WS_EX_CLIENTEDGE,
						L"EDIT",
						font_bitmap_filename,
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
						x+static_wide,y+1,wide - static_wide - 25-6,21,
						page_hwnd,(HMENU)ID_OPTIONS_FONT_BITMAP_EDIT,GetModuleHandle(0),0);

					os_set_default_font(GetDlgItem(page_hwnd,ID_OPTIONS_FONT_BITMAP_EDIT));
					
					// browse
					os_dlg_create_button(page_hwnd,ID_OPTIONS_FONT_BITMAP_BROWSE,0,L"...",x+wide-25,y,25);
							
					y += 23 + 6;
					
					// cp
					y += 6;
					os_dlg_create_static(page_hwnd,0,0,L"Specify the font's code page.",x,y,wide);
					y += 15;
					os_dlg_create_static(page_hwnd,0,0,L"437 = IBM PC. 0 = ANSI.",x,y,wide);
					y += 15+12;
					
					static_wide = 0;
					static_wide = expand_wide(page_hwnd,static_wide,L"Font &code page:");
					static_wide += 6;
					
					os_dlg_create_static(page_hwnd,ID_OPTIONS_CP_STATIC,0,L"Font &code page:",x,y+3,static_wide);
					
					CreateWindowExW(
						WS_EX_CLIENTEDGE,
						L"EDIT",
						L"",
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER,
						x+static_wide,y,75,21,
						page_hwnd,(HMENU)ID_OPTIONS_CP_EDIT,GetModuleHandle(0),0);
						
					SetDlgItemInt(page_hwnd,ID_OPTIONS_CP_EDIT,voidpad_cp,FALSE);

					os_set_default_font(GetDlgItem(page_hwnd,ID_OPTIONS_CP_EDIT));
					
					y += 21 + 6;
					
					break;
				}
			}
		}

		// show the new tab
		ShowWindow(GetDlgItem(hwnd,voidpad_options_pages[tvi.lParam].tab_id),SW_SHOWNA);

		// show new page
		ShowWindow(GetDlgItem(hwnd,voidpad_options_pages[tvi.lParam].page_id),SW_SHOWNA);
		

		// hide old tabs

		{
			int i;
			
			for(i=0;i<VOIDPAD_OPTIONS_PAGE_COUNT;i++)
			{
				if (i != tvi.lParam)
				{
					// hide page_hwnd first
					{
						HWND page_hwnd;
						
						page_hwnd = GetDlgItem(hwnd,voidpad_options_pages[i].page_id);

						if (page_hwnd)
						{
							ShowWindow(page_hwnd,SW_HIDE);
						}
					}

					// hide tab_hwnd	
					{
						HWND tab_hwnd;
					
						tab_hwnd = GetDlgItem(hwnd,voidpad_options_pages[i].tab_id);
						if (tab_hwnd)
						{
							ShowWindow(tab_hwnd,SW_HIDE);
						}
					}
				}
			}
		}
		
		// focus first dlg item.
		{
			HWND focus_hwnd;
			
			focus_hwnd = GetFocus();
			
			while(focus_hwnd)
			{
				if (!IsWindowVisible(focus_hwnd))
				{
					HWND new_focus_hwnd;
					HWND page_hwnd;
					
					page_hwnd = GetDlgItem(hwnd,voidpad_options_pages[tvi.lParam].page_id);
					
					new_focus_hwnd = GetNextDlgTabItem(page_hwnd,0,FALSE);
					
					if (new_focus_hwnd)
					{
						// set the next dlg control.
						// this selects editbox text etc...
						SendMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)new_focus_hwnd,TRUE);
					}
					
					break;
				}
				
				focus_hwnd = GetParent(focus_hwnd);
			}
		}		
	}

	EnableWindow(GetDlgItem(hwnd,ID_OPTIONS_APPLY),apply_enabled);
	SendMessage(GetDlgItem(hwnd,ID_OPTIONS_APPLY),WM_SETREDRAW,TRUE,0);
	InvalidateRect(GetDlgItem(hwnd,ID_OPTIONS_APPLY),0,FALSE);
}

static INT_PTR CALLBACK voidpad_options_page_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
			
			switch(LOWORD(wParam))
			{
				case ID_OPTIONS_ALL_CONTEXT_MENU:
				case ID_OPTIONS_TXT_ASSOCIATION:
				case ID_OPTIONS_INI_ASSOCIATION:
				case ID_OPTIONS_NFO_ASSOCIATION:
				
					EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
				
					break;
					
				case ID_OPTIONS_ALL_ASSOCATIONS:
					CheckDlgButton(hwnd,ID_OPTIONS_TXT_ASSOCIATION,BST_CHECKED);
					CheckDlgButton(hwnd,ID_OPTIONS_INI_ASSOCIATION,BST_CHECKED);
					CheckDlgButton(hwnd,ID_OPTIONS_NFO_ASSOCIATION,BST_CHECKED);
					EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
					break;

				case ID_OPTIONS_NONE_ASSOCATIONS:
					CheckDlgButton(hwnd,ID_OPTIONS_TXT_ASSOCIATION,BST_UNCHECKED);
					CheckDlgButton(hwnd,ID_OPTIONS_INI_ASSOCIATION,BST_UNCHECKED);
					CheckDlgButton(hwnd,ID_OPTIONS_NFO_ASSOCIATION,BST_UNCHECKED);
					EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
					break;
					
				case ID_OPTIONS_FOREGROUND_COLOR_BUTTON:
				case ID_OPTIONS_BACKGROUND_COLOR_BUTTON:
				case ID_OPTIONS_SELECTED_FOREGROUND_COLOR_BUTTON:
				case ID_OPTIONS_SELECTED_BACKGROUND_COLOR_BUTTON:
				{
					CHOOSECOLOR cc;
					
					crt_zero_memory(&cc,sizeof(CHOOSECOLOR));
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = GetParent(hwnd);
					cc.rgbResult = (COLORREF)GetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA);
					cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;
					cc.lpCustColors = voidpad_choose_color_cust_colors;
					
					if (ChooseColor(&cc))
					{
						SetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA,cc.rgbResult);
					
						voidpad_options_color_changed(hwnd,LOWORD(wParam));
						
						EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
					}
					
					break;
				}
					
				case ID_OPTIONS_TABSIZE_EDIT:
				case ID_OPTIONS_FONT_BITMAP_EDIT:
				case ID_OPTIONS_CP_EDIT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
					}
					break;
					
				case ID_OPTIONS_FONT_BITMAP_BROWSE:
				{
					OPENFILENAMEW ofn;
					wchar_t wbuf[MAX_PATH];

					crt_zero_memory(&ofn,sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					*wbuf = 0;
					
					ofn.hInstance = GetModuleHandle(0);
					ofn.hwndOwner = voidpad_hwnd;
					ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = wbuf;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = L"Open Font Bitmap";
					ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
					
					if (GetOpenFileNameW(&ofn))
					{
						SetWindowText(GetDlgItem(hwnd,ID_OPTIONS_FONT_BITMAP_EDIT),wbuf);
						
						EnableWindow(GetDlgItem(GetParent(hwnd),ID_OPTIONS_APPLY),TRUE);
					}
					
					break;
				}
			}
			break;
	}
	
	return 0;
}

static void os_dlg_create_checkbox(HWND parent,int id,DWORD extra_style,int checked,const wchar_t *text,int x,int y,int wide)
{
	HWND hwnd;
	
	hwnd = CreateWindowEx(
		0,
		WC_BUTTON,
		text,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | extra_style,
		x,y,wide,15,
		parent,(HMENU)(uintptr_t)id,GetModuleHandle(0),0);
	
	CheckDlgButton(parent,id,checked?BST_CHECKED:BST_UNCHECKED);
	
	SetWindowLongPtr(hwnd,GWLP_USERDATA,checked);
	
	os_set_default_font(hwnd);
}

static void voidpad_install_all_context_menu(void)
{
	wchar_t filename[MAX_PATH];
	wchar_t command[MAX_PATH];
	HKEY hkey;
	
	voidpad_uninstall_all_context_menu();

	GetModuleFileName(0,filename,MAX_PATH);
	
	wstring_copy(command,MAX_PATH,L"\"");
	wstring_cat(command,MAX_PATH,filename);
	wstring_cat(command,MAX_PATH,L"\" \"%1\"");

	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,L"*\\shell\\voidPad\\command",0,0,0,KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		voidpad_set_registry_string(hkey,0,command);

		RegCloseKey(hkey);
	}
}

static void voidpad_uninstall_all_context_menu(void)
{
	SHDeleteKeyW(HKEY_CLASSES_ROOT,L"*\\shell\\voidPad");
}

static int voidpad_is_all_context_menu(void)
{
	int ret;
	HKEY hkey;
	
	ret = 0;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,L"*\\shell\\voidPad\\command",0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];
		
		if (voidpad_get_registry_string(hkey,0,wbuf,MAX_PATH))
		{
			wchar_t filename[MAX_PATH];
			wchar_t command[MAX_PATH];

			GetModuleFileName(0,filename,MAX_PATH);
			
			wstring_copy(command,MAX_PATH,L"\"");
			wstring_cat(command,MAX_PATH,filename);
			wstring_cat(command,MAX_PATH,L"\" \"%1\"");
			
			printf("command: %S\n",command);
			printf("registry:%S\n",wbuf);
			
			if (wstring_compare(wbuf,command) == 0)
			{
				ret = 1;
			}
		}
		
		RegCloseKey(hkey);
	}

	return ret;
}

// use the default class description, ie: TXT File
static void voidpad_install_association(const wchar_t *association,const wchar_t *description)
{
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	wchar_t key[MAX_PATH];
	wchar_t default_icon[MAX_PATH];
	
	// make sure we uninstall old voidpad associations first.
	voidpad_uninstall_association(association);

	wstring_copy(class_name,MAX_PATH,L"voidPad");
	wstring_cat(class_name,MAX_PATH,association);

	wstring_copy(default_icon,MAX_PATH,L"voidPad");
	wstring_cat(default_icon,MAX_PATH,association);
	wstring_cat(default_icon,MAX_PATH,L"\\DefaultIcon");

	wstring_copy(key,MAX_PATH,class_name);
	wstring_cat(key,MAX_PATH,L"\\shell\\open\\command");
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,default_icon,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[MAX_PATH];
		wchar_t command[MAX_PATH];

		GetModuleFileName(0,filename,MAX_PATH);
		
		wstring_copy(command,MAX_PATH,filename);
		wstring_cat(command,MAX_PATH,L", 1");
		
		voidpad_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}	
		
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,class_name,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		voidpad_set_registry_string(hkey,0,description);
		
		RegCloseKey(hkey);
	}		
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[MAX_PATH];
		wchar_t command[MAX_PATH];

		GetModuleFileName(0,filename,MAX_PATH);
		
		wstring_copy(command,MAX_PATH,L"\"");
		wstring_cat(command,MAX_PATH,filename);
		wstring_cat(command,MAX_PATH,L"\" \"%1\"");
		
		voidpad_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}

	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];
		
		if (!voidpad_get_registry_string(hkey,L"voidPad.Backup",wbuf,MAX_PATH))
		{
			if (!voidpad_get_registry_string(hkey,0,wbuf,MAX_PATH))
			{
				*wbuf = 0;
			}
			
			voidpad_set_registry_string(hkey,L"voidPad.Backup",wbuf);
		}

		voidpad_set_registry_string(hkey,0,class_name);
		
		RegCloseKey(hkey);
	}
}

static void voidpad_uninstall_association(const wchar_t *association)
{
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	
	wstring_copy(class_name,MAX_PATH,L"voidPad");
	wstring_cat(class_name,MAX_PATH,association);
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];
		
		if (voidpad_get_registry_string(hkey,L"voidPad.Backup",wbuf,MAX_PATH))
		{
			voidpad_set_registry_string(hkey,0,wbuf);

			RegDeleteValueW(hkey,L"voidPad.Backup");
		}

		RegCloseKey(hkey);
	}
	
	SHDeleteKeyW(HKEY_CLASSES_ROOT,class_name);
}

static int voidpad_is_association(const wchar_t *association)
{
	int ret;
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	wchar_t key[MAX_PATH];
	
	wstring_copy(class_name,MAX_PATH,L"voidPad");
	wstring_cat(class_name,MAX_PATH,association);
	
	wstring_copy(key,MAX_PATH,class_name);
	wstring_cat(key,MAX_PATH,L"\\shell\\open\\command");
	
	ret = 0;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,association,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];

		if (voidpad_get_registry_string(hkey,0,wbuf,MAX_PATH))
		{
			if (wstring_compare(wbuf,class_name) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];

		if (voidpad_get_registry_string(hkey,0,wbuf,MAX_PATH))
		{
			wchar_t filename[MAX_PATH];
			wchar_t command[MAX_PATH];

			GetModuleFileName(0,filename,MAX_PATH);
			
			wstring_copy(command,MAX_PATH,L"\"");
			wstring_cat(command,MAX_PATH,filename);
			wstring_cat(command,MAX_PATH,L"\" \"%1\"");
			
			if (wstring_compare(wbuf,command) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	return ret == 2;
}

static void wstring_cat(wchar_t *buf,int size,const wchar_t *s)
{
	wchar_t *d;
	
	size--;
	
	d = buf;
	while(*d)
	{
		d++;
		size--;
	}
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s;
		s++;
		size--;
	}
	
	*d = 0;
}

static void wstring_cat_voidpad_cp(wchar_t *buf,int size,const unsigned char *s)
{
	wchar_t *ws;
	
	ws = wstring_alloc_voidpad_cp(s);
	
	wstring_cat(buf,size,ws);
	
	mem_free(ws);
}

static int wstring_compare(const wchar_t *s1,const wchar_t *s2)
{
	const wchar_t *p1;
	const wchar_t *p2;
	int c;
	
	p1 = s1;
	p2 = s2;
	
	for(;;)
	{
		c = *p1 - *p2;
		if (c) return c;
		
		if (!*p1) break;
		if (!*p2) break;
		
		p1++;
		p2++;
	}
	
	return 0;
}

static int voidpad_get_registry_string(HKEY hkey,const wchar_t *value,wchar_t *wbuf,int size_in_wchars)
{
	DWORD cbData;
	DWORD type;
	
	cbData = size_in_wchars * sizeof(wchar_t);
	
	if (RegQueryValueExW(hkey,value,0,&type,(BYTE *)wbuf,&cbData) == ERROR_SUCCESS)
	{
		if ((type == REG_SZ) || (type == REG_EXPAND_SZ))
		{
			return 1;
		}
	}
	
	return 0;
}

static int voidpad_set_registry_string(HKEY hkey,const wchar_t *value,const wchar_t *wbuf)
{
	if (RegSetValueExW(hkey,value,0,REG_SZ,(BYTE *)wbuf,(wstring_length(wbuf) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS)
	{
		return 1;
	}
	
	return 0;
}


static void os_dlg_create_button(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide)
{
	HWND hwnd;
	
	// ok button
	hwnd = CreateWindowExW(
		0,
		L"BUTTON",
		text,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| style,
		x,y,wide,23,
		parent,(HMENU)id,GetModuleHandle(0),0);

	os_set_default_font(hwnd);
}

static void os_dlg_create_static(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide)
{
	HWND hwnd;
	
	// ok button
	hwnd = CreateWindowExW(
		0,
		L"STATIC",
		text,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | style,
		x,y,wide,15,
		parent,(HMENU)id,GetModuleHandle(0),0);

	os_set_default_font(hwnd);
}

static void os_dlg_create_edit(HWND parent,int id,DWORD style,wchar_t *text,int x,int y,int wide)
{
	HWND hwnd;
	
	// ok button
	hwnd = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | style,
		x,y,wide,21,
		parent,(HMENU)id,GetModuleHandle(0),0);

	os_set_default_font(hwnd);
}

static void voidpad_tab_size_changed(void)
{
	int y;

	text_wide = 0;
	
	for(y=0;y<line_count;y++)
	{
		const unsigned char *p;
		const unsigned char *e;
		int col;
		
		p = line_list[y]->text;
		e = p + line_list[y]->len;
		
		if ((e - p >= 2) && (e[-2] == '\r') && (e[-1] == '\n'))
		{
			e -= 2;
		}
		else
		if ((e - p >= 1) && (e[-1] == '\n'))
		{
			e--;
		}
		
		col = 0;

		while(p != e)
		{
			if (*p == '\t')
			{
				col = ((col / voidpad_tab_size) + 1) * voidpad_tab_size;
			}
			else
			{
				col++;
			}

			p++;
		}

		if (col + 2 > text_wide)
		{
			text_wide = col + 2;
		}
	}
	
	// reset selection, as we could have had a tab selected.
	set_col_sel(0,0,0,0);
	
	// resize with new text_wide.
	size_window();

	// invalidate as tabs could have moved text left or right.
	InvalidateRect(edit_hwnd,0,FALSE);
}

// since a tab_size of 0 would cause a divide by zero.
static int voidpad_clip_tabsize(int tab_size)
{
	if (tab_size < 1) return 1;
	if (tab_size > 256) return 256;
	
	return tab_size;
}

static void voidpad_options_color_changed(HWND hwnd,int id)
{
	HDC memhdc;
	HBITMAP hbitmap;
	
	{
		HDC hdc;
		
		hdc = GetDC(hwnd);
		
		memhdc = CreateCompatibleDC(hdc);
		
		hbitmap = CreateCompatibleBitmap(hdc,75-8,23-8);
		
		ReleaseDC(hwnd,hdc);
	}
	
	{
		HGDIOBJ last_hbitmap;
		
		last_hbitmap = SelectObject(memhdc,hbitmap);
		
		// draw the button
		{
			HBRUSH hbrush;
			RECT rect;
			
			hbrush = CreateSolidBrush(GetWindowLongPtr(GetDlgItem(hwnd,id),GWLP_USERDATA));

			rect.left = 0;
			rect.top = 0;
			rect.right = 75-8;
			rect.bottom = 23-8;
			
			FillRect(memhdc,&rect,GetStockObject(BLACK_BRUSH));

			rect.left = 1;
			rect.top = 1;
			rect.right = 75-8-1;
			rect.bottom = 23-8-1;
			
			FillRect(memhdc,&rect,hbrush);
			
			DeleteBrush(hbrush);
		}
		
		SelectObject(memhdc,last_hbitmap);
	}
	
	DeleteDC(memhdc);

	SendMessage(GetDlgItem(hwnd,id),BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hbitmap);
}

static wchar_t *wstring_alloc(const wchar_t *s)
{
	wchar_t *p;
	int wlen;
	
	wlen = wstring_length(s);
	
	p = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	crt_copy_memory(p,s,(wlen+1) * sizeof(wchar_t));
	
	return p;
}

static wchar_t *wstring_alloc_utf8(const utf8_t *s)
{
	wchar_t *p;
	int wlen;
	
	wlen = MultiByteToWideChar(CP_UTF8,0,(char *)s,string_length(s),0,0);
	
	p = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	MultiByteToWideChar(CP_UTF8,0,(char *)s,string_length(s),p,wlen);
	p[wlen] = 0;
	
	return p;
}

static wchar_t *wstring_alloc_voidpad_cp(const utf8_t *s)
{
	wchar_t *p;
	int wlen;
	
	wlen = MultiByteToWideChar(voidpad_cp,0,(char *)s,string_length(s),0,0);
	
	p = mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	MultiByteToWideChar(voidpad_cp,0,(char *)s,string_length(s),p,wlen);
	p[wlen] = 0;
	
	return p;
}

static int string_compare(const unsigned char *s1,const unsigned char *s2)
{
	const unsigned char *p1;
	const unsigned char *p2;
	int c;
	
	p1 = s1;
	p2 = s2;
	
	for(;;)
	{
		c = *p1 - *p2;
		if (c) return c;
		
		if (!*p1) break;
		if (!*p2) break;
		
		p1++;
		p2++;
	}
	
	return 0;
}

void __cdecl mainCRTStartup() 
{
	int mainret;

	mainret = main(0,0);

    ExitProcess(mainret);
}

void __cdecl WinMainCRTStartup() 
{
	int mainret;
	STARTUPINFO si;
	
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);

	mainret = WinMain(GetModuleHandle(0),0,0,si.wShowWindow);

    ExitProcess(mainret);
}

static int expand_wide(HWND hwnd,int wide,const wchar_t *text)
{
	HDC hdc;
	SIZE sz;
	wchar_t *wbuf;
	HGDIOBJ last_font;
	
	hdc = GetDC(hwnd);
	wbuf = wstring_alloc(text);
	last_font = SelectObject(hdc,os_default_hfont);
	
	// remove amps.
	{
		wchar_t *p;
		wchar_t *d;
		
		p = wbuf;
		d = wbuf;
		
		while(*p)
		{
			if (*p != '&')
			{
				*d++ = *p;
			}
			
			p++;
		}
		*d = 0;
	}
	
	GetTextExtentPoint32W(hdc,wbuf,wstring_length(wbuf),&sz);
	
	if (sz.cx > wide)
	{
		wide = sz.cx;
	}
	
	SelectObject(hdc,last_font);
	mem_free(wbuf);
	ReleaseDC(hwnd,hdc);
	
	return wide;
}

static void font_load(void)
{
	if (font_hbitmap)
	{
		SelectObject(font_hdc,last_font_bitmap);
		DeleteObject(font_hbitmap);
		
		font_hbitmap = 0;
	}
	
	printf("font_hdc %d %d\n",font_hdc,GetLastError());
	
	if (*font_bitmap_filename)
	{
		font_hbitmap = LoadImageW(0,font_bitmap_filename,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

		if (!font_hbitmap)
		{
			font_hbitmap = LoadBitmapW(GetModuleHandle(0),MAKEINTRESOURCEW(IDB_BITMAP1));
		}
	}
	else
	{
printf("load bitmap font %d %d\n",GetModuleHandle(0),MAKEINTRESOURCEW(IDB_BITMAP1));
		font_hbitmap = LoadBitmapW(GetModuleHandle(0),MAKEINTRESOURCEW(IDB_BITMAP1));
	}
	
	font_wide = 8;
	font_high = 16;

	{
		BITMAP b;
		
		if (GetObject(font_hbitmap,sizeof(BITMAP),&b))
		{
			font_wide = b.bmWidth / 32;
			font_high = b.bmHeight / 8;
		}
	}

	printf("font_hbitmap %d %d\n",font_hbitmap,GetLastError());
	
	last_font_bitmap = SelectObject(font_hdc,font_hbitmap);
}

static int os_rename_file(const wchar_t *old_name,const wchar_t *new_name,DWORD flags)
{
	int ret;
	
	ret = 0;

	// try MoveFileEx
	if (MoveFileExW(old_name,new_name,flags))
	{
		ret = 1;
	}
	else
	{
		// if it failed, check to see if it was implemented.
		if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) 
		{
			DeleteFileW(new_name);
			
			// if the move below fails we have lost all our data!
			// rename the old one.
			if (MoveFileW(old_name,new_name))
			{
				ret = 1;
			}
		}
	}

	return ret;
}

INT_PTR CALLBACK voidpad_help_about_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;

			os_center_dialog(hwnd,320,292);
			
			SetWindowText(hwnd,L"About voidPad");
				
			x = 12*3;
			y = 88+12*3;
			wide = 320 - 24;
			high = 292 - 24;
	
			// static goto what:
			os_dlg_create_static(hwnd,ID_GOTO_WHAT_STATIC,0,L"voidPad",x,y,wide);
			y += 15 + 3;

			os_dlg_create_static(hwnd,ID_GOTO_WHAT_STATIC,0,L"Version " VERSION_STRING L" (" VERSION_MACHINE_TARGET L")",x,y,wide);
			y += 15 + 3;
			
			os_dlg_create_static(hwnd,ID_GOTO_WHAT_STATIC,0,L"Copyright © 2014 voidtools",x,y,wide);
			y += 15 + 3;

#ifdef VERSION_REGISTRATION
			{
				wchar_t wbuf[MAX_PATH];
				
				if (voidpad_is_registered)
				{
					wstring_copy(wbuf,MAX_PATH,L"Registered to: ");
					wstring_cat(wbuf,MAX_PATH,voidpad_registered_name);
				}
				else
				{
					wstring_copy(wbuf,MAX_PATH,L"Unregistered");
				}
				
				os_dlg_create_static(hwnd,ID_GOTO_WHAT_STATIC,0,wbuf,x,y,wide);
				y += 15 + 3;
			}
#endif
			
			y = 292-23-12-12-2;
			
			CreateWindowExW(
				WS_EX_STATICEDGE,
				L"STATIC",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | SS_SUNKEN,
				0,88,320,2,
				hwnd,0,GetModuleHandle(0),0);			

			CreateWindowExW(
				WS_EX_STATICEDGE,
				L"STATIC",
				L"",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | SS_SUNKEN,
				0,y,320,2,
				hwnd,0,GetModuleHandle(0),0);			

			y += 12+2;
			
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"OK",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				12+wide-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));

			SetFocus(GetDlgItem(hwnd,IDOK));
				
			return TRUE;
		}
		
		case WM_CTLCOLORSTATIC:
			return (LRESULT)(HBRUSH)GetStockObject(WHITE_BRUSH);
		
		case WM_ERASEBKGND:
			return TRUE;
		
		case WM_PAINT:
		{
			HBITMAP hbitmap;
			HDC mem_hdc;
			HGDIOBJ last_hbitmap;
			
			RECT rect;
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			
			rect.left = 0;
			rect.top = 0;
			rect.right = 320;
			rect.bottom = 292-23-12-12;
			FillRect(ps.hdc,&rect,GetStockObject(WHITE_BRUSH));
			
			rect.left = 0;
			rect.top = 292-23-12-12;
			rect.right = 320;
			rect.bottom = 292;
			FillRect(ps.hdc,&rect, (HBRUSH) (COLOR_BTNFACE + 1));
			
			mem_hdc = CreateCompatibleDC(ps.hdc);
			hbitmap = LoadBitmapW(GetModuleHandle(0),MAKEINTRESOURCEW(IDB_BITMAP2));
			last_hbitmap = SelectObject(mem_hdc,hbitmap);

			GetClientRect(hwnd,&rect);
			StretchBlt(ps.hdc,((rect.right - rect.left) / 2) - (320/2),0,320,112,mem_hdc,0,0,320,112,SRCCOPY);
			
			SelectObject(mem_hdc,last_hbitmap);
			DeleteObject(hbitmap);
			DeleteDC(mem_hdc);
			
			EndPaint(hwnd,&ps);
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hwnd,0);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static void voidpad_help_about(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_help_about_proc,0);
}

#ifdef VERSION_REGISTRATION

INT_PTR CALLBACK voidpad_help_register_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG:
		{
			int wide;
			int high;
			int x;
			int y;
			int static_wide;
			int dash_wide;

			os_center_dialog(hwnd,256,101);
			
			SetWindowText(hwnd,L"Register voidPad");
				
			x = 12;
			y = 12;
			wide = 256 - 24;
			high = 101 - 24;

			static_wide = 0;
			static_wide = expand_wide(hwnd,static_wide,L"&Name:");
			static_wide = expand_wide(hwnd,static_wide,L"&Key:");
			static_wide += 6;

			dash_wide = 0;
			dash_wide = expand_wide(hwnd,dash_wide,L"-");
			dash_wide += 6;
	
			// static goto what:
			os_dlg_create_static(hwnd,0,0,L"&Name:",x,y+3,static_wide);
			os_dlg_create_edit(hwnd,ID_HELP_REGISTER_NAME,ES_AUTOHSCROLL,L"",x+static_wide,y,wide-static_wide);
			y += 21 + 6;

			os_dlg_create_static(hwnd,0,0,L"&Key:",x,y+3,static_wide);
			os_dlg_create_edit(hwnd,ID_HELP_REGISTER_KEY,ES_AUTOHSCROLL,L"",x+static_wide,y,wide-static_wide);
			voidpad_help_register_old_key_proc = SubclassWindow(GetDlgItem(hwnd,ID_HELP_REGISTER_KEY),voidpad_help_register_key_proc);
			y += 21 + 6;
			
			// ok button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"OK",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP| BS_DEFPUSHBUTTON,
				x+wide-75-6-75,y,75,23,
				hwnd,(HMENU)IDOK,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDOK));
			
			// cancel button
			CreateWindowExW(
				0,
				L"BUTTON",
				L"Cancel",
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
				x+wide-75,y,75,23,
				hwnd,(HMENU)IDCANCEL,GetModuleHandle(0),0);

			os_set_default_font(GetDlgItem(hwnd,IDCANCEL));
			
			help_register_update_ok(hwnd);
			
			SetFocus(GetDlgItem(hwnd,ID_HELP_REGISTER_NAME));
				
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			// Parse the menu selections:
			switch (LOWORD(wParam))
			{
				case ID_HELP_REGISTER_NAME:
				case ID_HELP_REGISTER_KEY:

					if (HIWORD(wParam) == EN_CHANGE)
					{
						help_register_update_ok(hwnd);
					}
					
					break;
			
				case IDOK:
				case IDCANCEL:

					if (LOWORD(wParam) == IDOK)
					{
						mem_free(voidpad_registered_name);
						mem_free(voidpad_registered_key);
						
						voidpad_registered_name = wstring_alloc_window_text(GetDlgItem(hwnd,ID_HELP_REGISTER_NAME));
						voidpad_registered_key = wstring_alloc_window_text(GetDlgItem(hwnd,ID_HELP_REGISTER_KEY));
						
						voidpad_is_registered = help_register_is_valid(voidpad_registered_name,voidpad_registered_key);
						if (voidpad_is_registered)
						{
							HANDLE h;
							wchar_t wbuf[MAX_PATH];

							GetModuleFileName(0,wbuf,MAX_PATH);
							PathRemoveFileSpec(wbuf);
							PathAppend(wbuf,L"voidpad.key");
								
							h = CreateFileW(wbuf,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
							if (h != INVALID_HANDLE_VALUE)
							{
								file_write_utf8(h,(const utf8_t *)"[voidpad]\r\n");
								write_string(h,(const utf8_t *)"name",voidpad_registered_name);
								write_string(h,(const utf8_t *)"key",voidpad_registered_key);
								
								CloseHandle(h);
							}
						}
						
						update_title();
					}
					
					EndDialog(hwnd,0);
					return TRUE;
			}
			
			break;
		}
	}
	
	return FALSE;
}

static void voidpad_help_register(void)
{
	os_create_blank_dialog(voidpad_hwnd,voidpad_help_register_proc,0);
}

static void help_register_update_ok(HWND hwnd)
{
	wchar_t *name;
	wchar_t *key;
	
	name = wstring_alloc_window_text(GetDlgItem(hwnd,ID_HELP_REGISTER_NAME));
	key = wstring_alloc_window_text(GetDlgItem(hwnd,ID_HELP_REGISTER_KEY));
	
	EnableWindow(GetDlgItem(hwnd,IDOK),help_register_is_valid(name,key));
	
	mem_free(key);
	mem_free(name);
}

static int help_register_is_valid(const wchar_t *name,const wchar_t *key)
{
	DWORD crc;
	int i;
	utf8_t *name_utf8;
	
	Sleep(1);

	crc = 0;
	
	name_utf8 = utf8_alloc_wchar(name);
	
	crc = crc_add(crc,(const utf8_t *)"voidpad");
	crc = crc_add(crc,name_utf8);

	mem_free(name_utf8);
	
	for(i=0;i<8;i++)
	{
		if (key_table[(crc >> ((7-i) * 4)) & 0xf] != key[i])
		{
			return 0;
		}
	}
	
	return 1;
}

static DWORD crc_add(DWORD crc,const utf8_t *s)
{	
	crc ^= 0xFFFFFFFF;
	
	while(*s)
	{
		crc = crc_table[(crc ^ *s) & 0xff] ^ (crc >> 8);
		
		s++;
	}
	
	return crc ^ 0xFFFFFFFF;
}

static INT_PTR __stdcall voidpad_help_register_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_CHAR:
		
			if ((wParam >= 'a') && (wParam <= 'z'))
			{
				wParam += 'A' - 'a';
			}
			
			break;
	}
	
	return CallWindowProc(voidpad_help_register_old_key_proc,hwnd,msg,wParam,lParam);
}

#endif

static int _voidpad_hexchar(int n)
{
	if (n < 10)
	{
		return '0' + n;		
	}

	return 'A' + n - 10;		
}

static int _voidpad_hexvalue(int h)
{
	if ((h >= '0') && (h <= '9'))
	{
		return h - '0';
	}

	if ((h >= 'A') && (h <= 'F'))
	{
		return h - 'A' + 10;
	}

	if ((h >= 'a') && (h <= 'f'))
	{
		return h - 'a' + 10;
	}
	
	return 0;
}
