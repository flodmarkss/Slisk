# Slisk v0.9.8: controlled login navigation (EXPERIMENTAL)

## Evidence from PS4 test #6
- Xbox USB receiver 045e:0719 claimed; Xbox A decoded.
- MBus reported DEVICE_ADDED subType 2, deviceId 0xa030d.
- One neutral InsertData returned 0.
- No DEVICE_OWNER_CHANGED appeared before the 8-second timeout.
- DeleteDevice returned 0.

## Change in this branch
To test the hypothesis that the user-picker needs input from the newly created
virtual pad *before* assigning an owner, phase 2 now injects only explicit
Xbox A, D-pad and Guide/PS presses. Neutral reports are sent between input events.
Until a distinct owner is confirmed, all other buttons, axes and triggers remain
blocked. The owner comparison, 8-second owner timeout and DeleteDevice cleanup
remain in force. Xbox Back+Start aborts early.

## Safety and validation
This has host unit tests and a PS4 cross-compile check, NOT validation on a real
console. Previous tests left both controllers unable to navigate the picker
until virtual pad deletion. Do not run unless prepared for an unresponsive
account selection and a console restart. Do not run multiple payloads at once.

To test: with original DS4 already signed in, start the ELF, press Xbox A,
and try only explicitly pressing the Xbox D-pad/A or Guide in the picker.
If unresponsive, wait for automatic cleanup or hold Back+Start on Xbox.
Afterwards share /data/slisk_x360/control4free.log. The most useful records are
PRE_OWNER_NAV_INSERT, VDA_KLOG_EVENT, VDA_OWNER_SEEN and timeout cleanup.
