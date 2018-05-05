/*
 *  Copyright (C) 2002-2013  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string>
#include "menudef.h"
void SetVal(const std::string secname, std::string preval, const std::string val);

#ifdef __WIN32__
#include "programs.h"

void ToggleMenu(bool pressed);
void mem_conf(std::string memtype, int option);
void UnMount(int i_drive);
void BrowseFolder( char drive , std::string drive_type );
void Mount_Img(char drive, std::string realpath);
void Mount_Img_Floppy(char drive, std::string realpath);
void Mount_Img_HDD(char drive, std::string realpath);
void DOSBox_SetMenu(void);
void DOSBox_NoMenu(void);
void DOSBox_RefreshMenu(void);
void ToggleMenu(bool pressed);
void D3D_PS(void);
void DOSBox_CheckOS(int &id, int &major, int &minor);
void MountDrive(char drive, const char drive2[DOS_PATHLENGTH]);
void MountDrive_2(char drive, const char drive2[DOS_PATHLENGTH], std::string drive_type);
void MENU_Check_Drive(HMENU handle, int cdrom, int floppy, int local, int image, int automount, int umount, char drive);
bool MENU_SetBool(std::string secname, std::string value);
void MENU_swapstereo(bool enabled);
void GUI_Shortcut(int select);
void* GetSetSDLValue(int isget, std::string target, void* setval);
void Go_Boot(const char boot_drive[_MAX_DRIVE]);
void Go_Boot2(const char boot_drive[_MAX_DRIVE]);
void OpenFileDialog(char * path_arg);
void OpenFileDialog_Img(char drive);
void GFX_SetTitle(Bit32s cycles, Bits frameskip, Bits timing, bool paused);
void change_output(int output);
void res_input(bool type, const char * res);
void res_init(void);
int Reflect_Menu(void);
extern bool DOSBox_Kor(void);

extern unsigned int hdd_defsize;
extern char hdd_size[20];
extern HWND GetHWND(void);
extern void GetDefaultSize(void);
#define SCALER(opscaler,opsize) \
	if ((render.scale.op==opscaler) && (render.scale.size==opsize))

#define SCALER_SW(opscaler,opsize) \
	if ((render.scale.op==opscaler) && (render.scale.size==opsize) && (!render.scale.hardware))

#define SCALER_HW(opscaler,opsize) \
	if ((render.scale.op==opscaler) && (render.scale.size==opsize) && (render.scale.hardware))

#define SCALER_2(opscaler,opsize) \
	((render.scale.op==opscaler) && (render.scale.size==opsize))

#define SCALER_SW_2(opscaler,opsize) \
	((render.scale.op==opscaler) && (render.scale.size==opsize) && (!render.scale.hardware))

#define SCALER_HW_2(opscaler,opsize) \
	((render.scale.op==opscaler) && (render.scale.size==opsize) && (render.scale.hardware))

#define AUTOMOUNT(name,name2) \
	(((GetDriveType(name) == 2) || (GetDriveType(name) == 3) || (GetDriveType(name) == 4) || (GetDriveType(name) == 5) || (GetDriveType(name) == 6)))&&(!Drives[name2-'A'])

#else

// dummy Win32 functions for less #ifdefs
#define GetHWND() (0)
#define SetMenu(a,b)
#define DragAcceptFiles(a,b)
#define GetMenu(a) (0)

// menu.cpp replacements; the optimizer will completely remove code based on these
#define DOSBox_SetMenu()
#define DOSBox_NoMenu()
#define DOSBox_RefreshMenu()
#define DOSBox_CheckOS(a, b, c) do { (a)=0; (b)=0; (c)=0; } while(0)
#define VER_PLATFORM_WIN32_NT (1)
#define DOSBox_Kor() !strncmp("ko", getenv("LANG"), 2) // dirty hack.

/* menu interface mode */
#define DOSBOXMENU_NULL     (0)
#define DOSBOXMENU_HMENU    (1)
#if defined(WIN32) && !defined(C_SDL2)
# define DOSBOXMENU_TYPE    DOSBOXMENU_HMENU
#else
# define DOSBOXMENU_TYPE    DOSBOXMENU_NULL
#endif

#include <map>
#include <vector>

class DOSBoxMenu {
    public:
        class item;
    public:
        enum item_type_t {
            item_type_id=0,
            submenu_type_id,
            separator_type_id,
            vseparator_type_id,

            MAX_id
        };
    public:
        typedef uint16_t                item_handle_t;
        typedef void                  (*callback_t)(DOSBoxMenu * const,item * const);
        typedef void*                   mapper_event_t;     /* CEvent* pointer */
    public:
        class displaylist : public std::vector<item_handle_t> {
            public:
                                        displaylist();
                                        ~displaylist();
            protected:
                bool                    changed = false;
        };
    public:
        static constexpr item_handle_t  unassigned_item_handle = ((item_handle_t)(0xFFFFU)); 
        static constexpr callback_t     unassigned_callback = NULL;
        static constexpr mapper_event_t unassigned_mapper_event = NULL;
    public:
        struct accelerator {
            char                        key = 0;            /* ascii code i.e. 'g' */
            unsigned char               key_instance = 0;   /* which occurrence of the letter in the text */
        };
    public:
        class item {
            friend DOSBoxMenu;

            public:
                                        item();
                                        ~item();
            protected:
                std::string             name;               /* item name */
                std::string             text;               /* item text */
                std::string             shortcut_text;      /* shortcut text on the right */
                std::string             description;        /* description text */
                struct accelerator      accelerator;        /* menu accelerator */
            protected:
                item_handle_t           master_id = unassigned_item_handle;
                enum item_type_t        type = item_type_id;
            protected:
                struct status {
                                        status() : changed(false), allocated(false),
                                                   enabled(true), checked(false),
                                                   in_use(false) { };

                    unsigned int        changed:1;
                    unsigned int        allocated:1;
                    unsigned int        enabled:1;
                    unsigned int        checked:1;
                    unsigned int        in_use:1;
                } status;
            protected:
                callback_t              callback_func = unassigned_callback;
                mapper_event_t          mapper_event_ptr = unassigned_mapper_event;
            public:
                displaylist             display_list;
            public:
                uint64_t                user_defined = 0;
            protected:
                item&                   allocate(const item_handle_t id,const enum item_type_t type,const std::string &name);
                void                    deallocate(void);
            public:
                inline bool has_vis_text(void) const {
                    return type <= submenu_type_id;
                }
                inline bool has_vis_shortcut_text(void) const {
                    return type <= item_type_id;
                }
                inline bool has_vis_description(void) const {
                    return false;
                }
                inline bool has_vis_accelerator(void) const {
                    return type <= item_type_id;
                }
                inline bool has_vis_enabled(void) const {
                    return type <= submenu_type_id;
                }
                inline bool can_enable(void) const {
                    return type <= submenu_type_id;
                }
                inline bool has_vis_checked(void) const {
                    return type <= item_type_id;
                }
                inline bool can_check(void) const {
                    return type <= item_type_id;
                }
            public:
                inline item &check(const bool f=true) {
                    if (status.checked != f) {
                        status.checked  = f;
                        if (can_check() && has_vis_checked())
                            status.changed = 1;
                    }

                    return *this;
                }
            public:
                inline item &enable(const bool f=true) {
                    if (status.enabled != f) {
                        status.enabled  = f;
                        if (can_enable() && has_vis_enabled())
                            status.changed = 1;
                    }

                    return *this;
                }
            public:
                inline item_type_t get_type(void) const {
                    return type;
                }
            public:
                inline const callback_t get_callback_function(void) const {
                    return callback_func;
                }
                inline item &set_callback_function(const callback_t f) {
                    callback_func = f;
                    return *this;
                }
            public:
                inline item &set_mapper_event(const mapper_event_t e) {
                    mapper_event_ptr = e;
                    return *this;
                }
            public:
                inline const std::string &get_text(void) const {
                    return text;
                }
                inline item &set_text(const std::string &str) {
                    if (has_vis_text())
                        status.changed = 1;

                    text = str;
                    return *this;
                }
            public:
                inline const std::string &get_shortcut_text(void) const {
                    return shortcut_text;
                }
                inline item &set_shortcut_text(const std::string &str) {
                    if (has_vis_shortcut_text())
                        status.changed = 1;

                    shortcut_text = str;
                    return *this;
                }
            public:
                inline const std::string &get_description(void) const {
                    return description;
                }
                inline item &set_description(const std::string &str) {
                    if (has_vis_description())
                        status.changed = 1;

                    description = str;
                    return *this;
                }
            public:
                inline const struct accelerator &get_accelerator(void) const {
                    return accelerator;
                }
                inline item &set_accelerator(const struct accelerator &str) {
                    if (has_vis_accelerator())
                        status.changed = 1;

                    accelerator = str;
                    return *this;
                }
        };
    public:
                                        DOSBoxMenu();
                                        ~DOSBoxMenu();
    public:
        item&                           get_item(const item_handle_t i);
        item&                           get_item(const std::string &name);
        item&                           alloc_item(const enum item_type_t type,const std::string &name);
        void                            delete_item(const item_handle_t i);
        void                            clear_all_menu_items(void);
    public:
        displaylist                     display_list;
    protected:
        std::vector<item>               master_list;
        std::map<std::string,item_handle_t> name_map;
        item_handle_t                   master_list_alloc = 0;
    public:
        static constexpr size_t         master_list_limit = 4096;
};

#endif
