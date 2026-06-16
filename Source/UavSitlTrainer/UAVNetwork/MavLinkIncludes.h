// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifndef MAVLINK_USE_MESSAGE_INFO
	#define MAVLINK_USE_MESSAGE_INFO 1
#endif

#define MAVLINK_NO_CONVERSION_HELPERS

#pragma push_macro("check")
#undef check

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS
extern "C"
{
#include "common/mavlink.h"
#ifdef MAVLINK_NO_CONVERSION_HELPERS
	#undef MAVLINK_NO_CONVERSION_HELPERS
	#include "mavlink_get_info.h"
	#define MAVLINK_NO_CONVERSION_HELPERS
	#include "mavlink_get_info.h"
#endif
}
PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS

#pragma pop_macro("check")