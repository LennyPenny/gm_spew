gm_spew
==============

get enginespew in gmod!


Adds a hook called EngineSpew with the following args:
type
message
group
level

If you return a string on that message, the message will be modified to that string
If you return anything but nil the message will be changed to ""
If you return nil (nothing) the message won't be changed