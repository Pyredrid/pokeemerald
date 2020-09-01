#if DEBUG

#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "international_string_util.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/songs.h"

#define DEBUG_MAIN_MENU_WIDTH 12
#define DEBUG_MAIN_MENU_HEIGHT 7

#define DEBUG_NUMBER_DISPLAY_WIDTH 10
#define DEBUG_NUMBER_DISPLAY_HEIGHT 4

#define DEBUG_NUMBER_DIGITS_FLAGS 5
#define DEBUG_NUMBER_DIGITS_VARIABLES 5
#define DEBUG_NUMBER_DIGITS_ITEMS 4
#define DEBUG_NUMBER_DIGITS_ITEM_QUANTITY 2

void Debug_ShowMainMenu(void);

static void Debug_DestroyMainMenu(u8);

static void DebugAction_Flags(u8 taskId);
static void Flags_SelectFlag(u8 taskId);
static void Flags_ToggleFlag(u8 taskId);
static void Flags_DestroySelectFlag(u8 taskId);

static void DebugAction_Variables(u8 taskId);
static void Variables_SelectVariable(u8 taskId);
static void Variables_SetVariableValue(u8 taskId);

static void DebugAction_WarpToMap(u8 taskId);
static void WarpToMap_SelectMapBank(u8 taskId);
static void WarpToMap_SelectMap(u8 taskId);
static void WarpToMap_SelectWarp(u8 taskId);

static void DebugAction_GiveItem(u8 taskId);
static void GiveItem_SelectItemId(u8 taskId);
static void GiveItem_SelectItemQuantity(u8 taskId);
static void GiveItem_DestroySelectItem(u8 taskId);

static void DebugAction_HealParty(u8 taskId);

static void DebugAction_AccessPC(u8 taskId);

static void DebugAction_GivePokemon(u8 taskId);

static void DebugAction_Cancel(u8);

static void DebugTask_HandleMainMenuInput(u8);

enum {
    DEBUG_MENU_ITEM_FLAGS,
    DEBUG_MENU_ITEM_VARIABLES,
    DEBUG_MENU_ITEM_WARP_TO_MAP,
    DEBUG_MENU_ITEM_GIVE_ITEM,
    DEBUG_MENU_ITEM_HEAL_PARTY,
    DEBUG_MENU_ITEM_ACCESS_PC,
    DEBUG_MENU_ITEM_GIVE_POKEMON,
    DEBUG_MENU_ITEM_CANCEL
};

static const u8 gDebugText_Flags[] =            _("Flags");
static const u8 gDebugText_Variables[] =        _("Variables");
static const u8 gDebugText_WarpToMap[] =        _("Warp To Map");
static const u8 gDebugText_GiveItem[] =         _("Give Item");
static const u8 gDebugText_HealParty[] =        _("Heal Party");
static const u8 gDebugText_AccessPC[] =         _("Access PC");
static const u8 gDebugText_GivePokemon[] =      _("Give Pokemon");
static const u8 gDebugText_Cancel[] =           _("Cancel");

static const u8 gText_Flag[] =          _("  Flag: {STR_VAR_1}   \n  Is Set? {STR_VAR_2} \n{A_BUTTON}: Toggle Flag\n{STR_VAR_3}");
static const u8 gText_FlagSet[] =       _("TRUE");
static const u8 gText_FlagUnset[] =     _("FALSE");

static const u8 gText_Variable[] =  _("  Variable:       \n  {STR_VAR_1}    \n\n{STR_VAR_2}");
static const u8 gText_VariableValueSet[] =  _("  Variable: {STR_VAR_1}\n Value: {STR_VAR_1}    \n\n{STR_VAR_2}");

static const u8 gText_ItemQuantity[] =  _("  Quantity:       \n  {STR_VAR_1}    \n\n{STR_VAR_2}");
static const u8 gText_ItemID[] =        _("Item ID: {STR_VAR_3}\n{STR_VAR_1}    \n\n{STR_VAR_2}");

static const u8 digitInidicator_1[] =               _("{LEFT_ARROW}+1{RIGHT_ARROW}        ");
static const u8 digitInidicator_10[] =              _("{LEFT_ARROW}+10{RIGHT_ARROW}       ");
static const u8 digitInidicator_100[] =             _("{LEFT_ARROW}+100{RIGHT_ARROW}      ");
static const u8 digitInidicator_1000[] =            _("{LEFT_ARROW}+1000{RIGHT_ARROW}     ");
static const u8 digitInidicator_10000[] =           _("{LEFT_ARROW}+10000{RIGHT_ARROW}    ");
static const u8 digitInidicator_100000[] =          _("{LEFT_ARROW}+100000{RIGHT_ARROW}   ");
static const u8 digitInidicator_1000000[] =         _("{LEFT_ARROW}+1000000{RIGHT_ARROW}  ");
static const u8 digitInidicator_10000000[] =        _("{LEFT_ARROW}+10000000{RIGHT_ARROW} ");
const u8 * const gText_DigitIndicator[] =
{
    digitInidicator_1,
    digitInidicator_10,
    digitInidicator_100,
    digitInidicator_1000,
    digitInidicator_10000,
    digitInidicator_100000,
    digitInidicator_1000000,
    digitInidicator_10000000
};

static const s32 sPowersOfTen[] =
{
             1,
            10,
           100,
          1000,
         10000,
        100000,
       1000000,
      10000000,
     100000000,
    1000000000,
};

static const struct ListMenuItem sDebugMenuItems[] =
{
    [DEBUG_MENU_ITEM_FLAGS] = {gDebugText_Flags, DEBUG_MENU_ITEM_FLAGS},
    [DEBUG_MENU_ITEM_VARIABLES] = {gDebugText_Variables, DEBUG_MENU_ITEM_VARIABLES},
    [DEBUG_MENU_ITEM_WARP_TO_MAP] = {gDebugText_WarpToMap, DEBUG_MENU_ITEM_WARP_TO_MAP},
    [DEBUG_MENU_ITEM_GIVE_ITEM] = {gDebugText_GiveItem, DEBUG_MENU_ITEM_GIVE_ITEM},
    [DEBUG_MENU_ITEM_HEAL_PARTY] = {gDebugText_HealParty, DEBUG_MENU_ITEM_HEAL_PARTY},
    [DEBUG_MENU_ITEM_ACCESS_PC] = {gDebugText_AccessPC, DEBUG_MENU_ITEM_ACCESS_PC},
    [DEBUG_MENU_ITEM_GIVE_POKEMON] = {gDebugText_GivePokemon, DEBUG_MENU_ITEM_GIVE_POKEMON},
    [DEBUG_MENU_ITEM_CANCEL] = {gDebugText_Cancel, DEBUG_MENU_ITEM_CANCEL}
};

static void (*const sDebugMenuActions[])(u8) =
{
    [DEBUG_MENU_ITEM_FLAGS] = DebugAction_Flags,
    [DEBUG_MENU_ITEM_VARIABLES] = DebugAction_Variables,
    [DEBUG_MENU_ITEM_WARP_TO_MAP] = DebugAction_WarpToMap,
    [DEBUG_MENU_ITEM_GIVE_ITEM] = DebugAction_GiveItem,
    [DEBUG_MENU_ITEM_HEAL_PARTY] = DebugAction_HealParty,
    [DEBUG_MENU_ITEM_ACCESS_PC] = DebugAction_AccessPC,
    [DEBUG_MENU_ITEM_GIVE_POKEMON] = DebugAction_GivePokemon,
    [DEBUG_MENU_ITEM_CANCEL] = DebugAction_Cancel
};

static const struct WindowTemplate sDebugMenuWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = DEBUG_MAIN_MENU_WIDTH,
    .height = 2 * DEBUG_MAIN_MENU_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct ListMenuTemplate sDebugMenuListTemplate =
{
    .items = sDebugMenuItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .totalItems = ARRAY_COUNT(sDebugMenuItems),
    .maxShowed = DEBUG_MAIN_MENU_HEIGHT,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

static const struct WindowTemplate sDebugNumberDisplayWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 4 + DEBUG_MAIN_MENU_WIDTH,
    .tilemapTop = 1,
    .width = DEBUG_NUMBER_DISPLAY_WIDTH,
    .height = 2 * DEBUG_NUMBER_DISPLAY_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

void Debug_ShowMainMenu(void) {
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 menuTaskId;
    u8 inputTaskId;

    // create window
    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sDebugMenuWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    // create list menu
    menuTemplate = sDebugMenuListTemplate;
    menuTemplate.windowId = windowId;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    // draw everything
    CopyWindowToVram(windowId, 3);

    // create input handler task
    inputTaskId = CreateTask(DebugTask_HandleMainMenuInput, 3);
    gTasks[inputTaskId].data[0] = menuTaskId;
    gTasks[inputTaskId].data[1] = windowId;
}

static void Debug_DestroyMainMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);
    DestroyTask(taskId);
    EnableBothScriptContexts();
}

static void DebugTask_HandleMainMenuInput(u8 taskId)
{
    void (*func)(u8);
    u32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    if (gMain.newKeys & A_BUTTON)
    {
        PlaySE(SE_SELECT);
        if ((func = sDebugMenuActions[input]) != NULL)
            func(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Debug_DestroyMainMenu(taskId);
    }
}

static void DebugAction_Flags(u8 taskId)
{
    u8 windowId;

    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sDebugNumberDisplayWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    CopyWindowToVram(windowId, 3);

    //Display initial Flag
    if(FlagGet(0) == TRUE)
    {
        StringCopyPadded(gStringVar2, gText_FlagSet, CHAR_SPACE, 15);
    }
    else
    {
        StringCopyPadded(gStringVar2, gText_FlagUnset, CHAR_SPACE, 15);
    }
    StringCopy(gStringVar3, gText_DigitIndicator[0]);
    ConvertIntToDecimalStringN(gStringVar1, 0, STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_FLAGS);
    StringExpandPlaceholders(gStringVar4, gText_Flag);

    AddTextPrinterParameterized(windowId, 1, gStringVar4, 1, 1, 0, NULL);

    gTasks[taskId].func = Flags_SelectFlag;
    gTasks[taskId].data[2] = windowId;
    gTasks[taskId].data[3] = 0;            //Current Flag
    gTasks[taskId].data[4] = 0;            //Digit Selected
}

static void Flags_SelectFlag(u8 taskId)
{
    if (gMain.newKeys & A_BUTTON)
    {
        //Toggle Flag
        if(FlagGet(gTasks[taskId].data[3]) == TRUE)
        {
            FlagClear(gTasks[taskId].data[3]);
        }
        else
        {
            FlagSet(gTasks[taskId].data[3]);
        }
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Flags_DestroySelectFlag(taskId);
        return;
    }

    if(gMain.newKeys & DPAD_UP)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].data[3] += sPowersOfTen[gTasks[taskId].data[4]];
        if(gTasks[taskId].data[3] >= FLAGS_COUNT){
            gTasks[taskId].data[3] = FLAGS_COUNT - 1;
        }
    }
    if(gMain.newKeys & DPAD_DOWN)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].data[3] -= sPowersOfTen[gTasks[taskId].data[4]];
        if(gTasks[taskId].data[3] < 0){
            gTasks[taskId].data[3] = 0;
        }
    }
    if(gMain.newKeys & DPAD_LEFT)
    {
        PlaySE(SE_SELECT);
        if(gTasks[taskId].data[4] > 0)
        {
            gTasks[taskId].data[4] -= 1;
        }
    }
    if(gMain.newKeys & DPAD_RIGHT)
    {
        PlaySE(SE_SELECT);
        if(gTasks[taskId].data[4] < 3)
        {
            gTasks[taskId].data[4] += 1;
        }
    }

    if (gMain.newKeys & DPAD_ANY || gMain.newKeys & A_BUTTON)
    {
        if(FlagGet(gTasks[taskId].data[3]) == TRUE)
        {
            StringCopyPadded(gStringVar2, gText_FlagSet, CHAR_SPACE, 15);
        }
        else
        {
            StringCopyPadded(gStringVar2, gText_FlagUnset, CHAR_SPACE, 15);
        }
        StringCopy(gStringVar3, gText_DigitIndicator[gTasks[taskId].data[4]]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_FLAGS);
        StringExpandPlaceholders(gStringVar4, gText_Flag);

        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);
    }
}
static void Flags_DestroySelectFlag(u8 taskId)
{

    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    ClearStdWindowAndFrame(gTasks[taskId].data[2], TRUE);
    RemoveWindow(gTasks[taskId].data[2]);

    DestroyTask(taskId);
    EnableBothScriptContexts();
}


static void DebugAction_Variables(u8 taskId)
{
}
static void Variables_SelectVariable(u8 taskId)
{
}
static void Variables_SetVariableValue(u8 taskId)
{
}

static void DebugAction_WarpToMap(u8 taskId)
{
}
static void WarpToMap_SelectMapBank(u8 taskId)
{
}
static void WarpToMap_SelectMap(u8 taskId)
{
}
static void WarpToMap_SelectWarp(u8 taskId)
{
}

static void DebugAction_GiveItem(u8 taskId)
{
    u8 windowId;

    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sDebugNumberDisplayWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    CopyWindowToVram(windowId, 3);

    //Display initial ID
    StringCopy(gStringVar2, gText_DigitIndicator[0]);
    ConvertIntToDecimalStringN(gStringVar3, 1, STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_ITEMS);
    CopyItemName(1, gStringVar1);
    StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
    StringExpandPlaceholders(gStringVar4, gText_ItemID);
    AddTextPrinterParameterized(windowId, 1, gStringVar4, 1, 1, 0, NULL);

    gTasks[taskId].func = GiveItem_SelectItemId;
    gTasks[taskId].data[2] = windowId;
    gTasks[taskId].data[3] = 1;            //Current ID
    gTasks[taskId].data[4] = 0;            //Digit Selected
}
static void GiveItem_SelectItemId(u8 taskId)
{
    if (gMain.newKeys & DPAD_ANY)
    {
        PlaySE(SE_SELECT);

        if(gMain.newKeys & DPAD_UP)
        {
            gTasks[taskId].data[3] += sPowersOfTen[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] >= ITEMS_COUNT){
                gTasks[taskId].data[3] = ITEMS_COUNT - 1;
            }
        }
        if(gMain.newKeys & DPAD_DOWN)
        {
            gTasks[taskId].data[3] -= sPowersOfTen[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] < 0){
                gTasks[taskId].data[3] = 0;
            }
        }
        if(gMain.newKeys & DPAD_LEFT)
        {
            if(gTasks[taskId].data[4] > 0)
            {
                gTasks[taskId].data[4] -= 1;
            }
        }
        if(gMain.newKeys & DPAD_RIGHT)
        {
            if(gTasks[taskId].data[4] < DEBUG_NUMBER_DIGITS_ITEMS-1)
            {
                gTasks[taskId].data[4] += 1;
            }
        }

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].data[4]]);
        CopyItemName(gTasks[taskId].data[3], gStringVar1);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_ITEMS);
        StringExpandPlaceholders(gStringVar4, gText_ItemID);

        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);
    }

    if (gMain.newKeys & A_BUTTON)
    {
        gTasks[taskId].data[5] = gTasks[taskId].data[3];
        gTasks[taskId].data[3] = 0;
        gTasks[taskId].data[4] = 0;

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].data[4]]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_ITEM_QUANTITY);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, gText_ItemQuantity);
        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);

        gTasks[taskId].func = GiveItem_SelectItemQuantity;
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        GiveItem_DestroySelectItem(taskId);
    }
}

static void GiveItem_SelectItemQuantity(u8 taskId)
{
    if (gMain.newKeys & DPAD_ANY)
    {
        PlaySE(SE_SELECT);

        if(gMain.newKeys & DPAD_UP)
        {
            gTasks[taskId].data[3] += sPowersOfTen[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] >= 100){
                gTasks[taskId].data[3] = 99;
            }
        }
        if(gMain.newKeys & DPAD_DOWN)
        {
            gTasks[taskId].data[3] -= sPowersOfTen[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] < 0){
                gTasks[taskId].data[3] = 0;
            }
        }
        if(gMain.newKeys & DPAD_LEFT)
        {
            if(gTasks[taskId].data[4] > 0)
            {
                gTasks[taskId].data[4] -= 1;
            }
        }
        if(gMain.newKeys & DPAD_RIGHT)
        {
            if(gTasks[taskId].data[4] < 2)
            {
                gTasks[taskId].data[4] += 1;
            }
        }

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].data[4]]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, DEBUG_NUMBER_DIGITS_ITEM_QUANTITY);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, gText_ItemQuantity);

        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);
    }

    if (gMain.newKeys & A_BUTTON)
    {
        PlaySE(MUS_FANFA4);
        AddBagItem(gTasks[taskId].data[5], gTasks[taskId].data[3]);
        GiveItem_DestroySelectItem(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        GiveItem_DestroySelectItem(taskId);
    }
}

static void GiveItem_DestroySelectItem(u8 taskId)
{

    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    ClearStdWindowAndFrame(gTasks[taskId].data[2], TRUE);
    RemoveWindow(gTasks[taskId].data[2]);

    DestroyTask(taskId);
    EnableBothScriptContexts();
}

static void DebugAction_HealParty(u8 taskId)
{
    PlaySE(SE_KAIFUKU);
    HealPlayerParty();
    Debug_DestroyMainMenu(taskId);
}

static void DebugAction_AccessPC(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
    PlaySE(SE_PC_ON);
    ScriptContext1_SetupScript(EventScript_PC);
}

static void DebugAction_GivePokemon(u8 taskId)
{
}

static void DebugAction_Cancel(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
}

#endif
