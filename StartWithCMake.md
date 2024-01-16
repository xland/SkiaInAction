SK_DEBUG与SK_RELEASE

Input is not validated: out of bounds values of x or y trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if SkColorType is kUnknown_SkColorType or
pixel address is nullptr.