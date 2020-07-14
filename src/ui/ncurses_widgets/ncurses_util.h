#ifndef INSTALL_NCURSES_UTIL_H
#define INSTALL_NCURSES_UTIL_H

#include <curses.h>
#include <QMutex>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


namespace installer {

#define SCREEN_FG                    COLOR_CYAN//0
#define SCREEN_BG                    COLOR_BLUE
#define SCREEN_HL                    TRUE

#define SHADOW_FG                    COLOR_BLACK
#define SHADOW_BG                    COLOR_BLACK
#define SHADOW_HL                    TRUE

#define DIALOG_FG                    COLOR_BLACK
#define DIALOG_BG                    COLOR_WHITE
#define DIALOG_HL                    FALSE

#define TITLE_FG                     COLOR_YELLOW
#define TITLE_BG                     COLOR_WHITE
#define TITLE_HL                     TRUE

#define BORDER_FG                    COLOR_WHITE
#define BORDER_BG                    COLOR_WHITE
#define BORDER_HL                    TRUE

// 5
#define BUTTON_ACTIVE_FG             COLOR_WHITE//5
#define BUTTON_ACTIVE_BG             COLOR_RED
#define BUTTON_ACTIVE_HL             FALSE

#define BUTTON_INACTIVE_FG           COLOR_BLACK
#define BUTTON_INACTIVE_BG           COLOR_RED
#define BUTTON_INACTIVE_HL           FALSE

#define BUTTON_KEY_ACTIVE_FG         COLOR_WHITE
#define BUTTON_KEY_ACTIVE_BG         COLOR_BLUE
#define BUTTON_KEY_ACTIVE_HL         TRUE

#define BUTTON_KEY_INACTIVE_FG       COLOR_RED
#define BUTTON_KEY_INACTIVE_BG       COLOR_WHITE
#define BUTTON_KEY_INACTIVE_HL       FALSE

#define BUTTON_LABEL_ACTIVE_FG       COLOR_YELLOW
#define BUTTON_LABEL_ACTIVE_BG       COLOR_BLUE
#define BUTTON_LABEL_ACTIVE_HL       TRUE

#define BUTTON_LABEL_INACTIVE_FG     COLOR_BLACK//10
#define BUTTON_LABEL_INACTIVE_BG     COLOR_WHITE
#define BUTTON_LABEL_INACTIVE_HL     TRUE

#define INPUTBOX_FG                  COLOR_BLACK
#define INPUTBOX_BG                  COLOR_WHITE
#define INPUTBOX_HL                  FALSE

#define INPUTBOX_BORDER_FG           COLOR_BLACK
#define INPUTBOX_BORDER_BG           COLOR_WHITE
#define INPUTBOX_BORDER_HL           FALSE

#define SEARCHBOX_FG                 COLOR_BLACK
#define SEARCHBOX_BG                 COLOR_WHITE
#define SEARCHBOX_HL                 FALSE

#define SEARCHBOX_TITLE_FG           COLOR_YELLOW
#define SEARCHBOX_TITLE_BG           COLOR_WHITE
#define SEARCHBOX_TITLE_HL           TRUE

#define SEARCHBOX_BORDER_FG          COLOR_WHITE//15
#define SEARCHBOX_BORDER_BG          COLOR_WHITE
#define SEARCHBOX_BORDER_HL          TRUE

#define POSITION_INDICATOR_FG        COLOR_YELLOW
#define POSITION_INDICATOR_BG        COLOR_WHITE
#define POSITION_INDICATOR_HL        TRUE

#define MENUBOX_FG                   COLOR_BLACK
#define MENUBOX_BG                   COLOR_WHITE
#define MENUBOX_HL                   FALSE

#define MENUBOX_BORDER_FG            COLOR_WHITE
#define MENUBOX_BORDER_BG            COLOR_WHITE
#define MENUBOX_BORDER_HL            TRUE

#define ITEM_FG                      COLOR_BLACK
#define ITEM_BG                      COLOR_WHITE
#define ITEM_HL                      FALSE

#define ITEM_SELECTED_FG             COLOR_WHITE//20
#define ITEM_SELECTED_BG             COLOR_BLUE
#define ITEM_SELECTED_HL             TRUE

#define TAG_FG                       COLOR_YELLOW
#define TAG_BG                       COLOR_WHITE
#define TAG_HL                       TRUE

#define TAG_SELECTED_FG              COLOR_YELLOW
#define TAG_SELECTED_BG              COLOR_BLUE
#define TAG_SELECTED_HL              TRUE

#define TAG_KEY_FG                   COLOR_YELLOW
#define TAG_KEY_BG                   COLOR_WHITE
#define TAG_KEY_HL                   TRUE

#define TAG_KEY_SELECTED_FG          COLOR_YELLOW
#define TAG_KEY_SELECTED_BG          COLOR_BLUE
#define TAG_KEY_SELECTED_HL          TRUE

#define CHECK_FG                     COLOR_BLACK//25
#define CHECK_BG                     COLOR_WHITE
#define CHECK_HL                     FALSE

#define CHECK_SELECTED_FG            COLOR_WHITE
#define CHECK_SELECTED_BG            COLOR_BLUE
#define CHECK_SELECTED_HL            TRUE

#define UARROW_FG                    COLOR_GREEN
#define UARROW_BG                    COLOR_WHITE
#define UARROW_HL                    TRUE

#define DARROW_FG                    COLOR_GREEN
#define DARROW_BG                    COLOR_WHITE
#define DARROW_HL                    TRUE

#define EDIT_FG                      COLOR_BLACK
#define EDIT_BG                      COLOR_CYAN
#define EDIT_HL                      TRUE

#define COMMENT_FG                   COLOR_BLUE//30
#define COMMENT_GB                   COLOR_WHITE
#define COMMENT_HL                   TRUE

#define WARNINT_FG                   COLOR_YELLOW
#define WARNINT_GB                   COLOR_WHITE
#define WARNINT_HL                   TRUE

#define ERROR_FG                     COLOR_RED
#define ERROR_GB                     COLOR_WHITE
#define ERROR_HL                     TRUE

#define LIST_VIEW_ITEM_SELECTD_FG    COLOR_BLUE
#define LIST_VIEW_ITEM_SELECTD_GB    COLOR_GREEN
#define LIST_VIEW_ITEM_SELECTD_HL    TRUE

/* End of default color definitions */

#define C_ATTR(x,y)                  ((x ? A_BOLD : 0) | COLOR_PAIR((y)))
#define COLOR_NAME_LEN               10
#define COLOR_COUNT                  8


#define ATTRIBUTE_COUNT               38

/* new style */
#define BUTTON_FG COLOR_BLACK
#define BUTTON_GB COLOR_YELLOW
#define BUTTON_HL TRUE


class NcursesUtil
{
public:
    ~NcursesUtil();
    static NcursesUtil* getInstance();


    chtype screen_attr() {
        return attributes[0];
    }

    chtype shadow_attr() {
        return attributes[1];
    }

    chtype dialog_attr() {
        return attributes[2];
    }

    chtype title_attr() {
        return attributes[3];
    }

    chtype border_attr() {
        return attributes[4];
    }

    chtype button_active_attr() {
        return attributes[5];
    }

    chtype button_inactive_attr() {
        return attributes[6];
    }

    chtype button_key_active_attr() {
        return attributes[7];
    }

    chtype button_key_inactive_attr() {
        return attributes[8];
    }

    chtype button_label_active_attr() {
        return attributes[9];
    }

    chtype button_label_inactive_attr() {
        return attributes[10];
    }

    chtype inputbox_attr() {
        return attributes[11];
    }

    chtype inputbox_border_attr() {
        return attributes[12];
    }

    chtype searchbox_attr() {
        return attributes[13];
    }

    chtype searchbox_title_attr() {
        return attributes[14];
    }

    chtype searchbox_border_attr() {
        return attributes[15];
    }

    chtype position_indicator_attr() {
        return attributes[16];
    }

    chtype menubox_attr() {
        return attributes[17];
    }

    chtype menubox_border_attr() {
        return attributes[18];
    }

    chtype item_attr() {
        return attributes[19];
    }

    chtype item_selected_attr() {
        return attributes[20];
    }
    chtype tag_attr() {
        return attributes[21];
    }

    chtype tag_selected_attr() {
        return attributes[22];
    }

    chtype tag_key_attr() {
        return attributes[23];
    }

    chtype tag_key_selected_attr() {
        return attributes[24];
    }

    chtype check_attr() {
        return attributes[25];
    }

    chtype check_selected_attr() {
        return attributes[26];
    }

    chtype uarrow_attr() {
        return attributes[27];
    }

    chtype darrow_attr() {
        return attributes[28];
    }

    chtype edit_attr() {
        return attributes[29];
    }

    chtype comment_attr() {
        return attributes[30];
    }

    chtype warnint_attr() {
        return attributes[31];
    }

    chtype error_attr() {
        return attributes[32];
    }

    chtype list_view_item_select() {
        return attributes[33];
    }

    chtype button() {
        return attributes[34];
    }

//    chtype button_select() {
//        return attributes[38];
//    }

private:
    NcursesUtil();
    void colorSetup();
    static NcursesUtil* m_instance;
    static QMutex  m_mutex;
    chtype attributes[ATTRIBUTE_COUNT];

};


};

#endif
