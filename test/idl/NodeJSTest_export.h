
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl NodeJSTest
// ------------------------------
#ifndef NODEJSTEST_EXPORT_H
#define NODEJSTEST_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (NODEJSTEST_HAS_DLL)
#  define NODEJSTEST_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && NODEJSTEST_HAS_DLL */

#if !defined (NODEJSTEST_HAS_DLL)
#  define NODEJSTEST_HAS_DLL 1
#endif /* ! NODEJSTEST_HAS_DLL */

#if defined (NODEJSTEST_HAS_DLL) && (NODEJSTEST_HAS_DLL == 1)
#  if defined (NODEJSTEST_BUILD_DLL)
#    define NodeJSTest_Export ACE_Proper_Export_Flag
#    define NODEJSTEST_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define NODEJSTEST_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* NODEJSTEST_BUILD_DLL */
#    define NodeJSTest_Export ACE_Proper_Import_Flag
#    define NODEJSTEST_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define NODEJSTEST_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* NODEJSTEST_BUILD_DLL */
#else /* NODEJSTEST_HAS_DLL == 1 */
#  define NodeJSTest_Export
#  define NODEJSTEST_SINGLETON_DECLARATION(T)
#  define NODEJSTEST_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* NODEJSTEST_HAS_DLL == 1 */

// Set NODEJSTEST_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (NODEJSTEST_NTRACE)
#  if (ACE_NTRACE == 1)
#    define NODEJSTEST_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define NODEJSTEST_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !NODEJSTEST_NTRACE */

#if (NODEJSTEST_NTRACE == 1)
#  define NODEJSTEST_TRACE(X)
#else /* (NODEJSTEST_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define NODEJSTEST_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (NODEJSTEST_NTRACE == 1) */

#endif /* NODEJSTEST_EXPORT_H */

// End of auto generated file.
