#include "ncurses_util.h"

namespace installer {

NcursesUtil* NcursesUtil::m_instance = nullptr;
QMutex  NcursesUtil::m_mutex;

NcursesUtil::NcursesUtil()
{
    colorSetup();
}

NcursesUtil::~NcursesUtil()
{

}

NcursesUtil* NcursesUtil::getInstance()
{
    if (m_instance == nullptr)
        {
            QMutexLocker locker(&m_mutex);
            if (m_instance == nullptr)
            {
                m_instance = new NcursesUtil();
            }
        }
        return m_instance;
}


void NcursesUtil::colorSetup()
{
    int color_table[][3] = {
        {SCREEN_FG, SCREEN_BG, SCREEN_HL},//0
        {SHADOW_FG, SHADOW_BG, SHADOW_HL},
        {DIALOG_FG, DIALOG_BG, DIALOG_HL},
        {TITLE_FG, TITLE_BG, TITLE_HL},
        {BORDER_FG, BORDER_BG, BORDER_HL},
        {BUTTON_ACTIVE_FG, BUTTON_ACTIVE_BG, BUTTON_ACTIVE_HL},
        {BUTTON_INACTIVE_FG, BUTTON_INACTIVE_BG, BUTTON_INACTIVE_HL},
        {BUTTON_KEY_ACTIVE_FG, BUTTON_KEY_ACTIVE_BG, BUTTON_KEY_ACTIVE_HL},
        {BUTTON_KEY_INACTIVE_FG, BUTTON_KEY_INACTIVE_BG,
         BUTTON_KEY_INACTIVE_HL},
        {BUTTON_LABEL_ACTIVE_FG, BUTTON_LABEL_ACTIVE_BG,
         BUTTON_LABEL_ACTIVE_HL},
        {BUTTON_LABEL_INACTIVE_FG, BUTTON_LABEL_INACTIVE_BG,
         BUTTON_LABEL_INACTIVE_HL},
        {INPUTBOX_FG, INPUTBOX_BG, INPUTBOX_HL},
        {INPUTBOX_BORDER_FG, INPUTBOX_BORDER_BG, INPUTBOX_BORDER_HL},
        {SEARCHBOX_FG, SEARCHBOX_BG, SEARCHBOX_HL},
        {SEARCHBOX_TITLE_FG, SEARCHBOX_TITLE_BG, SEARCHBOX_TITLE_HL},
        {SEARCHBOX_BORDER_FG, SEARCHBOX_BORDER_BG, SEARCHBOX_BORDER_HL},
        {POSITION_INDICATOR_FG, POSITION_INDICATOR_BG, POSITION_INDICATOR_HL},
        {MENUBOX_FG, MENUBOX_BG, MENUBOX_HL},
        {MENUBOX_BORDER_FG, MENUBOX_BORDER_BG, MENUBOX_BORDER_HL},
        {ITEM_FG, ITEM_BG, ITEM_HL},
        {ITEM_SELECTED_FG, ITEM_SELECTED_BG, ITEM_SELECTED_HL},
        {TAG_FG, TAG_BG, TAG_HL},
        {TAG_SELECTED_FG, TAG_SELECTED_BG, TAG_SELECTED_HL},
        {TAG_KEY_FG, TAG_KEY_BG, TAG_KEY_HL},
        {TAG_KEY_SELECTED_FG, TAG_KEY_SELECTED_BG, TAG_KEY_SELECTED_HL},
        {CHECK_FG, CHECK_BG, CHECK_HL},
        {CHECK_SELECTED_FG, CHECK_SELECTED_BG, CHECK_SELECTED_HL},
        {UARROW_FG, UARROW_BG, UARROW_HL},
        {DARROW_FG, DARROW_BG, DARROW_HL},
        {EDIT_FG, EDIT_BG, EDIT_HL},
        {COMMENT_FG, COMMENT_GB, COMMENT_HL},
        {WARNINT_FG, WARNINT_GB, WARNINT_HL},
        {ERROR_FG, ERROR_GB, ERROR_HL},
        {LIST_VIEW_ITEM_SELECTD_FG, LIST_VIEW_ITEM_SELECTD_GB, LIST_VIEW_ITEM_SELECTD_HL}, // 33
        /* new style */
        {BUTTON_FG, BUTTON_GB, BUTTON_HL},  // 34
    };	/* color_table */

    int i;


    start_color();
    /* Initialize color pairs */
    for (i = 0; i < ATTRIBUTE_COUNT; i++)
        init_pair(i + 1, color_table[i][0], color_table[i][1]);

    /* Setup color attributes */
    for (i = 0; i < ATTRIBUTE_COUNT; i++)
        attributes[i] = C_ATTR(color_table[i][2], i + 1);

}


}
