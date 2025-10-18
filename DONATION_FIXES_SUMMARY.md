# Donation System Fixes - Complete Summary

## Problems Found

### 1. **OBJ_VNUM_PIT was not configured (VALUE = 0)**
   - **Symptom:** Logs showed `OBJ_VNUM_PIT=0`
   - **Impact:** Pit checking was completely skipped (code checks `if (OBJ_VNUM_PIT > 0)`)
   - **Fix:** Added `OBJ_VNUM_PIT 3008` to `/home/ro/eldoria-github/data/const.txt`
   - **Location:** Line 65 in const.txt

### 2. **Donation save wasn't integrated into copyover**
   - **Original state:** `save_donation_pits()` was only called during periodic updates
   - **Fix:** Added calls to `save_donation_pits()` and `save_tokens()` in `do_copyover()` function
   - **Location:** `src/act_wiz.c` lines 5111-5112

### 3. **Load error: "Fread_obj_donation: # not found"**
   - **Cause:** The file was being read but format was unexpected
   - **Status:** This should resolve once objects are actually being saved

## Files Modified

1. **`src/donation.c`**
   - Added comprehensive debug logging
   - Fixed file paths to use `../area/donation.dat` and `../area/tokens.dat`
   - Added runtime checks instead of compile-time `#ifdef`
   - Added `#include "const.h"` for proper declarations
   - Enhanced logging to show room names, object names, and pointer addresses

2. **`src/act_wiz.c`**
   - Added `save_donation_pits()` and `save_tokens()` calls in `do_copyover()` (line 5111-5112)
   - Added function declarations (line 82-83)

3. **`data/const.txt`**
   - Added `OBJ_VNUM_PIT 3008` configuration

## What Should Happen Now

After a **copyover**, you should see logs like:

```
save_donation_pits: Starting save, OBJ_VNUM_PIT=3008
save_donation_pits: Found donation room 3037 (The donation room), contents=yes, room->contents=0x...
save_donation_pits: Saving object vnum 221 (a lantern) from donation room
save_donation_pits: Found pit in room XXXX
save_donation_pits: Saving object vnum XXX (item name) from pit
save_donation_pits: Saved X objects from Y donation rooms and Z pits
```

## Donation Rooms Found
The system found 8 donation rooms:
- 1607
- 2244
- 3037 (Midgaard)
- 3160
- 3161
- 9320
- 9325
- 9778

## Testing Steps

1. **Do a copyover** to load the new binary with OBJ_VNUM_PIT configured
2. **Place an object** in donation room 3037
3. **Place an object** in a pit (vnum 3008)
4. **Run `asave world`** or wait for auto-save / do another copyover
5. **Check stderr/logs** for the debug messages
6. **Verify `../area/donation.dat`** contains object data
7. **Do another copyover**
8. **Check if objects persist** in the rooms

## Debug Output Location

All debug messages go to **stderr** which is typically:
- The console where the MUD was started
- A log file if stderr is redirected (check your startup script)
- The timestamp format is: `Thu Oct 16 16:47:06 2025 :: message`

## Key Insight About Original Problem

The rooms showed `contents=no` even though you said objects were there because:
- Either objects were already gone by the time save ran
- Or the object placing hadn't actually moved them to `room->contents`
- With OBJ_VNUM_PIT now configured as 3008, pit objects will be properly detected

The enhanced logging will now show exactly what's in each room at save time, making it easy to diagnose any remaining issues.

