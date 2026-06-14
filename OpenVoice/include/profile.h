/*
 * OpenVoice AAC Communicator
 * include/profile.h - Voice Profile API
 *
 * Declares the Profile data structure and the load/save/default functions
 * that persist user voice settings between sessions.
 *
 * Standard: C99
 */

#ifndef OPENVOICE_PROFILE_H
#define OPENVOICE_PROFILE_H

/* -------------------------------------------------------------------------
 * File path
 * ---------------------------------------------------------------------- */
#define PROFILE_PATH  "data/profile.cfg"

/* -------------------------------------------------------------------------
 * Parameter bounds (eSpeak NG ranges)
 * ---------------------------------------------------------------------- */
#define PITCH_MIN   -10   /* semitones below neutral */
#define PITCH_MAX    10   /* semitones above neutral */
#define PITCH_DEF     0   /* neutral (SSML +0st)     */

#define SPEED_MIN    80
#define SPEED_MAX    450
#define SPEED_DEF    175

#define VOLUME_MIN   0
#define VOLUME_MAX   200
#define VOLUME_DEF   100

/* Language tag length limit */
#define LANG_TAG_LEN 16

/* -------------------------------------------------------------------------
 * Data type
 * ---------------------------------------------------------------------- */

/**
 * Profile - stores all user-configurable voice synthesis parameters.
 */
typedef struct {
    int  pitch;              /**< Pitch: -10 to +10 semitones (SSML F0)   */
    int  speed;              /**< Speed:  80-450 words per minute          */
    int  volume;             /**< Volume: 0-200 (100 = default)            */
    char language[LANG_TAG_LEN]; /**< eSpeak language tag, e.g. "en-us"   */
} Profile;

/* -------------------------------------------------------------------------
 * Functions
 * ---------------------------------------------------------------------- */

/**
 * profile_init - zero-initialise a Profile struct.
 *
 * @param p  Profile to initialise.
 */
void profile_init(Profile *p);

/**
 * profile_set_defaults - fill a Profile with sensible default values.
 *
 * @param p  Profile to populate.
 */
void profile_set_defaults(Profile *p);

/**
 * profile_load - parse a key=value config file into a Profile.
 *
 * Unrecognised keys are silently ignored so future additions stay
 * backwards-compatible.
 *
 * @param p     Destination Profile (must already be initialised).
 * @param path  Path to the profile.cfg file.
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int profile_load(Profile *p, const char *path);

/**
 * profile_save - write a Profile to a key=value config file.
 *
 * Creates the file (and leading directories if possible) when absent.
 *
 * @param p     Source Profile.
 * @param path  Destination path.
 * Returns 0 on success, -1 on I/O error.
 */
int profile_save(const Profile *p, const char *path);

/**
 * profile_clamp - clamp all numeric fields to their valid ranges.
 *
 * @param p  Profile to clamp in-place.
 */
void profile_clamp(Profile *p);

#endif /* OPENVOICE_PROFILE_H */
