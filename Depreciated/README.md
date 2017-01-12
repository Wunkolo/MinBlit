This is the _depreciated_ version of MinBlit.

This version of MinBlit is _header only_ and requires the user to include `MinBlit.hpp`
in at least one source file with `MINBLT_IMPLEMENT` defined before the inclusion.
This version of MinBlit manipulates a-one channel 8bpp(thats bits-per-pixel) and
is intended to be used with a 256-color palette during its orignal design. It
avoids floating point arithmetic as much as possible except during the
calculation of 2D Vector lenghts(`Length()`) with floating point precision provided by
`FLT_EVAL_METHOD`. An integer-only alternative(`iLength()`) is also provided for
whole-number length approximations.
