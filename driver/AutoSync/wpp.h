#pragma once


#define WPP_CONTROL_GUIDS                                          \
    WPP_DEFINE_CONTROL_GUID(                                       \
        myDriverTraceGuid, (84bdb2e9,829e,41b3,b891,02f454bc2baa), \
        WPP_DEFINE_BIT(TRACE_ALL)			/* bit  0 = 0x00000001 */ \
        WPP_DEFINE_BIT(TRACE_DRIVER)        /* bit  1 = 0x00000002 */ \
        WPP_DEFINE_BIT(TRACE_DEVICE)        /* bit  2 = 0x00000004 */ \
        WPP_DEFINE_BIT(TRACE_QUEUE)         /* bit  3 = 0x00000008 */ \
        )


#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)




//
// This comment block is scanned by the trace preprocessor to convert the KdPrintEx function.
// Note the double parentheses for the KdPrint message, for compatiblility with the KdPrintEx function.
//
// begin_wpp config
// FUNC TraceCrit{LEVEL=TRACE_LEVEL_CRITICAL}(FLAGS, MSG, ...);
// FUNC TraceErr{LEVEL=TRACE_LEVEL_ERROR}(FLAGS, MSG, ...);
// FUNC TraceWarn{LEVEL=TRACE_LEVEL_WARNING}(FLAGS, MSG, ...);
// FUNC TraceInfo{LEVEL=TRACE_LEVEL_INFORMATION}(FLAGS, MSG, ...);
// FUNC TraceVerb{LEVEL=TRACE_LEVEL_VERBOSE}(FLAGS, MSG, ...);
// FUNC LogCrit{LEVEL=TRACE_LEVEL_CRITICAL, FLAGS=TRACE_ALL}(MSG, ...);
// FUNC LogErr{LEVEL=TRACE_LEVEL_ERROR, FLAGS=TRACE_ALL}(MSG, ...);
// FUNC LogWarn{LEVEL=TRACE_LEVEL_WARNING, FLAGS=TRACE_ALL}(MSG, ...);
// FUNC LogInfo{LEVEL=TRACE_LEVEL_INFORMATION, FLAGS=TRACE_ALL}(MSG, ...);
// FUNC LogVerb{LEVEL=TRACE_LEVEL_VERBOSE, FLAGS=TRACE_ALL}(MSG, ...);
// end_wpp
//

