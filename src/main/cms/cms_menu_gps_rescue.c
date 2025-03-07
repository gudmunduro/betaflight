/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#ifdef USE_CMS_GPS_RESCUE_MENU

#include "cli/settings.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_gps_rescue.h"

#include "config/feature.h"

#include "config/config.h"

#include "flight/gps_rescue.h"


static uint16_t gpsRescueConfig_angle; //degrees
static uint16_t gpsRescueConfig_initialAltitudeM; //meters
static uint16_t gpsRescueConfig_descentDistanceM; //meters
static uint16_t gpsRescueConfig_rescueGroundspeed; // centimeters per second
static uint16_t gpsRescueConfig_throttleMin;
static uint16_t gpsRescueConfig_throttleMax;
static uint16_t gpsRescueConfig_throttleHover;
static uint8_t gpsRescueConfig_minSats;
static uint16_t gpsRescueConfig_minRescueDth; //meters
static uint8_t gpsRescueConfig_allowArmingWithoutFix;
static uint16_t gpsRescueConfig_throttleP, gpsRescueConfig_throttleI, gpsRescueConfig_throttleD;
static uint16_t gpsRescueConfig_velP, gpsRescueConfig_velI, gpsRescueConfig_velD;
static uint16_t gpsRescueConfig_yawP;
static uint16_t gpsRescueConfig_targetLandingAltitudeM;
static uint16_t gpsRescueConfig_targetLandingDistanceM;
static uint8_t gpsRescueConfig_altitudeMode;
static uint16_t gpsRescueConfig_ascendRate;
static uint16_t gpsRescueConfig_descendRate;

static long cms_menuGpsRescuePidOnEnter(void)
{

    gpsRescueConfig_throttleP = gpsRescueConfig()->throttleP;
    gpsRescueConfig_throttleI = gpsRescueConfig()->throttleI;
    gpsRescueConfig_throttleD = gpsRescueConfig()->throttleD;

    gpsRescueConfig_yawP = gpsRescueConfig()->yawP;

    gpsRescueConfig_velP = gpsRescueConfig()->velP;
    gpsRescueConfig_velI = gpsRescueConfig()->velI;
    gpsRescueConfig_velD = gpsRescueConfig()->velD;

    return 0;
}

static long cms_menuGpsRescuePidOnExit(const OSD_Entry *self)
{
    UNUSED(self);

    gpsRescueConfigMutable()->throttleP = gpsRescueConfig_throttleP;
    gpsRescueConfigMutable()->throttleI = gpsRescueConfig_throttleI;
    gpsRescueConfigMutable()->throttleD = gpsRescueConfig_throttleD;

    gpsRescueConfigMutable()->yawP = gpsRescueConfig_yawP;

    gpsRescueConfigMutable()->velP = gpsRescueConfig_velP;
    gpsRescueConfigMutable()->velI = gpsRescueConfig_velI;
    gpsRescueConfigMutable()->velD = gpsRescueConfig_velD;

    return 0;
}

const OSD_Entry cms_menuGpsRescuePidEntries[] =
{
    {"--- GPS RESCUE PID---", OME_Label, NULL, NULL, 0},

    { "THROTTLE P",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleP, 0, 500, 1 }, REBOOT_REQUIRED },
    { "THROTTLE I",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleI, 0, 500, 1 }, REBOOT_REQUIRED },
    { "THROTTLE D",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleD, 0, 500, 1 }, REBOOT_REQUIRED },

    { "YAW P",             OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_yawP, 0, 500, 1 }, REBOOT_REQUIRED },

    { "VELOCITY P",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_velP, 0, 500, 1 }, REBOOT_REQUIRED },
    { "VELOCITY I",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_velI, 0, 500, 1 }, REBOOT_REQUIRED },
    { "VELOCITY D",        OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_velD, 0, 500, 1 }, REBOOT_REQUIRED },

    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu cms_menuGpsRescuePid = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUGPSRPID",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cms_menuGpsRescuePidOnEnter,
    .onExit = cms_menuGpsRescuePidOnExit,
    .checkRedirect = NULL,
    .entries = cms_menuGpsRescuePidEntries,
};

static long cmsx_menuGpsRescueOnEnter(void)
{

    gpsRescueConfig_angle = gpsRescueConfig()->angle;
    gpsRescueConfig_initialAltitudeM = gpsRescueConfig()->initialAltitudeM;
    gpsRescueConfig_descentDistanceM = gpsRescueConfig()->descentDistanceM;
    gpsRescueConfig_rescueGroundspeed = gpsRescueConfig()->rescueGroundspeed;
    gpsRescueConfig_throttleMin = gpsRescueConfig()->throttleMin  ;
    gpsRescueConfig_throttleMax = gpsRescueConfig()->throttleMax;
    gpsRescueConfig_throttleHover = gpsRescueConfig()->throttleHover;
    gpsRescueConfig_minSats = gpsRescueConfig()->minSats;
    gpsRescueConfig_minRescueDth = gpsRescueConfig()->minRescueDth;
    gpsRescueConfig_allowArmingWithoutFix = gpsRescueConfig()->allowArmingWithoutFix;
    gpsRescueConfig_targetLandingDistanceM = gpsRescueConfig()->targetLandingDistanceM;
    gpsRescueConfig_targetLandingAltitudeM = gpsRescueConfig()->targetLandingAltitudeM;
    gpsRescueConfig_altitudeMode = gpsRescueConfig()->altitudeMode;
    gpsRescueConfig_ascendRate = gpsRescueConfig()->ascendRate;
    gpsRescueConfig_descendRate = gpsRescueConfig()->descendRate;

    return 0;
}

static long cmsx_menuGpsRescueOnExit(const OSD_Entry *self)
{
    UNUSED(self);


    gpsRescueConfigMutable()->angle = gpsRescueConfig_angle;
    gpsRescueConfigMutable()->initialAltitudeM = gpsRescueConfig_initialAltitudeM;
    gpsRescueConfigMutable()->descentDistanceM = gpsRescueConfig_descentDistanceM;
    gpsRescueConfigMutable()->rescueGroundspeed = gpsRescueConfig_rescueGroundspeed;
    gpsRescueConfigMutable()->throttleMin = gpsRescueConfig_throttleMin;
    gpsRescueConfigMutable()->throttleMax = gpsRescueConfig_throttleMax;
    gpsRescueConfigMutable()->throttleHover = gpsRescueConfig_throttleHover;
    gpsRescueConfigMutable()->minSats = gpsRescueConfig_minSats;
    gpsRescueConfigMutable()->minRescueDth = gpsRescueConfig_minRescueDth;
    gpsRescueConfigMutable()->allowArmingWithoutFix = gpsRescueConfig_allowArmingWithoutFix;
    gpsRescueConfigMutable()->targetLandingDistanceM = gpsRescueConfig_targetLandingDistanceM;
    gpsRescueConfigMutable()->targetLandingAltitudeM = gpsRescueConfig_targetLandingAltitudeM;
    gpsRescueConfigMutable()->altitudeMode = gpsRescueConfig_altitudeMode;
    gpsRescueConfigMutable()->ascendRate = gpsRescueConfig_ascendRate;
    gpsRescueConfigMutable()->descendRate = gpsRescueConfig_descendRate;

    return 0;
}

const OSD_Entry cmsx_menuGpsRescueEntries[] =
{
    {"--- GPS RESCUE ---", OME_Label, NULL, NULL, 0},

    { "ANGLE",             OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_angle, 0, 200 ,1 }, REBOOT_REQUIRED },
    { "MIN DIST HOME   M", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_minRescueDth, 50, 1000 ,1 }, REBOOT_REQUIRED },
    { "INITAL ALT      M", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_initialAltitudeM, 20, 100, 1 }, REBOOT_REQUIRED },
    { "ALTITUDE MODE"    , OME_TAB, NULL, &(OSD_TAB_t) { &gpsRescueConfig_altitudeMode, 2, lookupTableRescueAltitudeMode}, REBOOT_REQUIRED },
    { "DESCENT DIST    M", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_descentDistanceM, 30, 500, 1 }, REBOOT_REQUIRED },
    { "LANDING ALT     M", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_targetLandingAltitudeM, 3, 10, 1 }, REBOOT_REQUIRED },
    { "LANDING DIST    M", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_targetLandingDistanceM, 5, 15, 1 }, REBOOT_REQUIRED },
    { "GROUND SPEED CM/S", OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_rescueGroundspeed, 30, 3000, 1 }, REBOOT_REQUIRED },
    { "THROTTLE MIN",      OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleMin, 1000, 2000, 1 }, REBOOT_REQUIRED },
    { "THROTTLE MAX",      OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleMax, 1000, 2000, 1 }, REBOOT_REQUIRED },
    { "THROTTLE HOV",      OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_throttleHover, 1000, 2000, 1 }, REBOOT_REQUIRED },
    { "ASCEND RATE",       OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_ascendRate, 100, 2500, 1 }, REBOOT_REQUIRED },
    { "DESCEND RATE",      OME_UINT16, NULL, &(OSD_UINT16_t){ &gpsRescueConfig_descendRate, 100, 500, 1 }, REBOOT_REQUIRED },    
    { "ARM WITHOUT FIX",   OME_Bool,  NULL, &gpsRescueConfig_allowArmingWithoutFix, REBOOT_REQUIRED },
    { "MIN SATELITES",     OME_UINT8, NULL, &(OSD_UINT8_t){ &gpsRescueConfig_minSats, 5, 50, 1 }, REBOOT_REQUIRED },
    { "GPS RESCUE PID",    OME_Submenu, cmsMenuChange, &cms_menuGpsRescuePid, 0},

    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu cmsx_menuGpsRescue = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUGPSRES",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuGpsRescueOnEnter,
    .onExit = cmsx_menuGpsRescueOnExit,
    .checkRedirect = NULL,
    .entries = cmsx_menuGpsRescueEntries,
};

#endif
